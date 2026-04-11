# include <pybind11/pybind11.h>
# include <pybind11/stl.h>
# include <api.hpp>
# include <stdexcept>
# include <vector>

const unsigned SF_IGNORE_CAPITALS = 0x0002;
const unsigned SF_NO_LEMMA_STRING = 0x0004;   // не строить строки при лемматизации
const unsigned SF_NO_GRAMMAR_DATA = 0x0008;   // не сохранять грамматики

namespace py = pybind11;

template <class Char>
struct XxLemmInfo
{
  lexeme_t                                lexid;
  std::optional<std::basic_string<Char>>  lemma;
  std::optional<std::vector<SGramInfo>>   grams;
};

struct MbLemmInfo: XxLemmInfo<char>     {};
struct WcLemmInfo: XxLemmInfo<widechar> {};

template <class Char>
struct XxStemInfo
{
  unsigned                                stemLen;
  unsigned                                lexType;
  float                                   fWeight;
  std::optional<std::basic_string<Char>>  stLemma;
  std::optional<std::vector<SGramInfo>>   grammar;

  template <class mlfaStemInfo>
  XxStemInfo( const mlfaStemInfo& stem ):
    stemLen( stem.ccstem ),
    lexType( stem.nclass ),
    fWeight( stem.weight )
  {
    using string_type = std::basic_string<
      typename std::decay<decltype(*stem.plemma)>::type>;

    if ( stem.plemma != nullptr )
      stLemma = string_type( stem.plemma );

    if ( stem.ngrams != 0 )
      grammar = std::move( std::vector<SGramInfo>( stem.pgrams, stem.pgrams + stem.ngrams ) );
  }
};

struct MbStemInfo: XxStemInfo<char>
{
  MbStemInfo( const SStemInfoA& s ): XxStemInfo( s )  {}
};

struct WcStemInfo: XxStemInfo<widechar>
{
  WcStemInfo( const SStemInfoW& s ): XxStemInfo( s )  {}
};

struct MbStrMatch: SStrMatch  {};
struct WcStrMatch: SStrMatch  {};

template <class Mlma>
class MorphAPI
{
  Mlma*   mlma;

  using stringType = typename std::conditional<std::is_same<typename Mlma::CharType, widechar>::value,
    std::u16string, py::bytes>::type;
  using stringData = typename std::conditional<std::is_same<typename Mlma::CharType, widechar>::value,
    const std::u16string&, std::string>::type;
  using XXStrMatch = typename std::conditional<std::is_same<typename Mlma::CharType, widechar>::value,
    WcStrMatch, MbStrMatch>::type;
  using XXLemmInfo = typename std::conditional<std::is_same<typename Mlma::CharType, widechar>::value,
    WcLemmInfo, MbLemmInfo>::type;

public:
  MorphAPI( libmorphGetAPI getAPI, const std::string& codepage )
  {
    if ( getAPI( (LIBMORPH_API_4_MAGIC ":" + codepage).c_str(), (void**)&mlma ) != 0 || mlma == nullptr )
      throw std::runtime_error( "Failed to create libmorphrus with codepage '" + codepage + "'" );
  }
 ~MorphAPI()
  {
    if ( mlma != nullptr )
      mlma->Detach();
  }

 /*
  * part_of_sp( lexeme )
  */
  auto  part_of_sp( lexeme_t nlexid ) -> uint8_t
  {
    return mlma->GetWdInfo( nlexid );
  }

 /*
  * check_word( string, flags )
  */
  int   check_word( const stringType& pyword, unsigned dwsets )
  {
    return mlma->CheckWord( stringData( pyword ),
      (dwsets & SF_IGNORE_CAPITALS) != 0 ? sfIgnoreCapitals : 0 );
  }

 /*
  * lemmatize( string, options ) -> []
  */
  auto  lemmatize( const stringType& pyword, unsigned dwsets ) -> py::list
  {
    typename Mlma::LemmType lxbuff[32];
    typename Mlma::CharType stbuff[2048];
    typename Mlma::GramType grbuff[512];

    auto  szword = stringData( pyword );
    auto  l_size = sizeof(lxbuff) / sizeof(lxbuff[0]);
    auto  s_size = (dwsets & SF_NO_LEMMA_STRING) == 0 ? sizeof(stbuff) / sizeof(stbuff[0]) : 0;
    auto  g_size = (dwsets & SF_NO_GRAMMAR_DATA) == 0 ? sizeof(grbuff) / sizeof(grbuff[0]) : 0;
    auto  nlemma = mlma->Lemmatize( szword.c_str(), szword.length(),
      l_size != 0 ? lxbuff : nullptr, l_size,
      s_size != 0 ? stbuff : nullptr, s_size,
      g_size != 0 ? grbuff : nullptr, g_size, (dwsets & SF_IGNORE_CAPITALS) != 0 ? sfIgnoreCapitals : 0 );
    auto  output = py::list();

    for ( int i = 0; i < nlemma; ++i )
    {
      auto& lemm = lxbuff[i];
      auto  next = XXLemmInfo{ lxbuff[i].nlexid, {}, {} };

      if ( lemm.plemma != nullptr )
        next.lemma = stringType( lemm.plemma );

      if ( lemm.ngrams != 0 )
        next.grams = std::move( std::vector<SGramInfo>( lemm.pgrams, lemm.pgrams + lemm.ngrams ) );

      output.append( next );
    }
    return output;
  }

 /*
  * build_form( lexeme, formid ) -> []
  */
  auto  build_form( lexeme_t nlexid, formid_t idform ) -> py::list
  {
    typename Mlma::CharType stbuff[2048];

    auto  output = py::list();
    auto  nforms = mlma->BuildForm( stbuff, sizeof(stbuff) / sizeof(stbuff[0]), nlexid, idform );
    auto  pforms = stbuff;

    for ( int i = 0; i < nforms; ++i )
    {
      output.append(
        stringType( pforms ) );
      while ( *pforms++ != 0 )
        (void)NULL;
    }
    return output;
  }

 /*
  * find_match( string, callback )
  */
  int   find_match( const stringType& pattern, py::function callback )
  {
    try
    {
      return mlma->FindMatch( stringData( pattern ), [this, callback]( lexeme_t nlexid, int n, const SStrMatch* p ) -> int
        {
          auto  aforms = py::list();

          try
          {
            for ( int i = 0; i < n; ++i )
              aforms.append( *(const XXStrMatch*)(p + i) );

            py::object result = callback( nlexid, aforms );
              return result.cast<int>();
          }
          catch ( const py::error_already_set& xp )
          {
            return -1;
          }
        } );
    }
    catch ( const py::error_already_set& xp )
    {
      throw;
    }
  }

};

template <class Mlma>
class FuzzyAPI
{
  Mlma*   mlfa;

  using stringType = typename std::conditional<std::is_same<typename Mlma::CharType, widechar>::value,
    std::u16string, py::bytes>::type;
  using stringData = typename std::conditional<std::is_same<typename Mlma::CharType, widechar>::value,
    const std::u16string&, std::string>::type;
  using XXStrMatch = typename std::conditional<std::is_same<typename Mlma::CharType, widechar>::value,
    WcStrMatch, MbStrMatch>::type;
  using XXStemInfo = typename std::conditional<std::is_same<typename Mlma::CharType, widechar>::value,
    WcStemInfo, MbStemInfo>::type;

public:
  FuzzyAPI( libmorphGetAPI getAPI, const std::string& codepage )
  {
    if ( getAPI( (LIBFUZZY_API_4_MAGIC ":" + codepage).c_str(), (void**)&mlfa ) != 0 || mlfa == nullptr )
      throw std::runtime_error( "Failed to create libmorphrus with codepage '" + codepage + "'" );
  }
 ~FuzzyAPI()
  {
    if ( mlfa != nullptr )
      mlfa->Detach();
  }

 /*
  * part_of_sp( lexeme )
  */
  auto  part_of_sp( unsigned uclass ) -> uint8_t
  {
    return mlfa->GetWdInfo( uclass );
  }

 /*
  * get_sample( uclass ) -> string
  */
  auto  get_models( unsigned uclass ) -> std::vector<stringType>
  {
    auto  models = mlfa->GetModels( uclass );
    auto  output = std::vector<stringType>();

    for ( auto& next: models )
      output.emplace_back( stringType( next ) );

    return output;
  }

 /*
  * lemmatize( string, options ) -> []
  */
  auto  lemmatize( const stringType& pyword, unsigned dwsets ) -> py::list
  {
    typename Mlma::StemType lxbuff[32];
    typename Mlma::CharType stbuff[2048];
    typename Mlma::GramType grbuff[512];

    auto  szword = stringData( pyword );
    auto  l_size = sizeof(lxbuff) / sizeof(lxbuff[0]);
    auto  s_size = (dwsets & SF_NO_LEMMA_STRING) == 0 ? sizeof(stbuff) / sizeof(stbuff[0]) : 0;
    auto  g_size = (dwsets & SF_NO_GRAMMAR_DATA) == 0 ? sizeof(grbuff) / sizeof(grbuff[0]) : 0;
    auto  nlemma = mlfa->Lemmatize( szword.c_str(), szword.length(),
      l_size != 0 ? lxbuff : nullptr, l_size,
      s_size != 0 ? stbuff : nullptr, s_size,
      g_size != 0 ? grbuff : nullptr, g_size, (dwsets & SF_IGNORE_CAPITALS) != 0 ? sfIgnoreCapitals : 0 );
    auto  output = py::list();

    for ( int i = 0; i < nlemma; ++i )
      output.append( XXStemInfo( lxbuff[i] ) );

    return output;
  }

 /*
  * build_form( lexeme, formid ) -> []
  */
  auto  build_form( const stringType& szword, lexeme_t nclass, formid_t idform ) -> py::list
  {
    typename Mlma::CharType stbuff[2048];

    auto  output = py::list();
    auto  nforms = mlfa->BuildForm( stbuff, stringData( szword ), nclass, idform );
    auto  pforms = stbuff;

    for ( int i = 0; i < nforms; ++i )
    {
      output.append(
        stringType( pforms ) );
      while ( *pforms++ != 0 )
        (void)NULL;
    }
    return output;
  }

};

