# include <pybind11/pybind11.h>
# include <pybind11/stl.h>
# include <rus.h>
# include <stdexcept>
# include <vector>
# include <moonycode/codes.h>

const unsigned SF_IGNORE_CAPITALS = 0x0002;
const unsigned SF_NO_LEMMA_STRING = 0x0004;   // не строить строки при лемматизации
const unsigned SF_NO_GRAMMAR_DATA = 0x0008;   // не сохранять грамматики

namespace py = pybind11;

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
class FuzzyAPI
{
  Mlma*   mlfa;

  typename Mlma::StemType lxbuff[32];
  typename Mlma::CharType stbuff[2048];
  typename Mlma::GramType grbuff[512];

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
    if ( getAPI( codepage.c_str(), (void**)&mlfa ) != 0 || mlfa == nullptr )
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
  auto  get_sample( unsigned uclass ) -> stringType
  {
    return mlfa->GetSample( uclass );
  }

 /*
  * lemmatize( string, options ) -> []
  */
  auto  lemmatize( const stringType& pyword, unsigned dwsets ) -> py::list
  {
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

 /*
  * find_match( string, callback )
  */
  int   find_match( const stringType& pattern, py::function callback )
  {
    try
    {
      return mlfa->FindMatch( stringData( pattern ), [&]( lexeme_t nlexid, int n, const SStrMatch* p )
        {
          auto  aforms = py::list();

          try
          {
            for ( int i = 0; i < n; ++i )
              aforms.append( *(const XXStrMatch*)(p + i) );

            return callback( nlexid, aforms ).cast<int>();
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

# define HELP_LEMMATIZE   \
  "Выполняет морфологический анализ поданного слова в соответствии с настройками\n" \
  "и возвращает массив отождествлений.\n"                                           \
  "\n"                                                                              \
  "Аргументы:\n"                                                                    \
  "  word   - слово для анализа;\n"                                                 \
  "  flags  - сочетание настроек:\n"                                                \
  "    * SF_IGNORE_CAPITALS = 0x0002 - игнорировать капитализацию;\n"               \
  "    * SF_NO_LEMMA_STRING = 0x0004 - не строить нормальные формы;\n"              \
  "    * SF_NO_GRAMMAR_DATA = 0x0008 - не строить грамматические описания.\n"       \
  "\n"                                                                              \
  "Возвращает:\n"                                                                   \
  "  Массив отождествлений. Каждый элемент обязательно содержит идентификатор лексемы\n"  \
  "  и, в зависимости от настроек, может содержать лемму (нормальную или словарную\n" \
  "  форму слова) и массив грамматических описаний соответствующих форм.\n"

PYBIND11_MODULE(libfuzzyrus, m)
{
  m.doc() = "libfuzzyrus for Python API";

  py::class_<FuzzyAPI<IMlfaMbXX>>( m, "mlfaruMb" )
    .def( py::init( []( std::string cp )
      {  return new FuzzyAPI<IMlfaMbXX>( mlfaruGetAPI, cp );  } ) )
    .def( "part_of_sp", &FuzzyAPI<IMlfaMbXX>::part_of_sp, "Get technical part of speach of lexeme or 0 if invalid" )
    .def( "get_sample", &FuzzyAPI<IMlfaMbXX>::get_sample, "Get sample word for a class" )
    .def( "lemmatize", &FuzzyAPI<IMlfaMbXX>::lemmatize,
      HELP_LEMMATIZE )
    .def( "build_form", &FuzzyAPI<IMlfaMbXX>::build_form, "Build word form by lexeme and form id return a list of forms" );
//    .def( "find_match", &FuzzyAPI<IMlfaMbXX>::find_match, "Search lexemes and forms matching the passed template" );

  py::class_<FuzzyAPI<IMlfaWcXX>>( m, "mlfaruWc" )
    .def( py::init( []()
      {  return new FuzzyAPI<IMlfaWcXX>( mlfaruGetAPI, "utf-16" );  } ) )
    .def( "part_of_sp", &FuzzyAPI<IMlfaWcXX>::part_of_sp, "Get technical part of speach of lexeme or 0 if invalid" )
    .def( "get_sample", &FuzzyAPI<IMlfaWcXX>::get_sample, "Get sample word for a class" )
    .def( "lemmatize", &FuzzyAPI<IMlfaWcXX>::lemmatize,
      HELP_LEMMATIZE )
    .def( "build_form", &FuzzyAPI<IMlfaWcXX>::build_form, "Build word form by lexeme and form id return a list of forms" );
//    .def( "find_match", &FuzzyAPI<IMlfaWcXX>::find_match, "Search lexemes and forms matching the passed template" );

  py::class_<SGramInfo>(m, "GramInfo")
    .def_readonly( "wdInfo", &SGramInfo::wdInfo )
    .def_readonly( "idForm", &SGramInfo::idForm )
    .def_readonly( "grInfo", &SGramInfo::grInfo )
    .def_readonly( "bFlags", &SGramInfo::bFlags );

  py::class_<MbStemInfo>(m, "MbStemInfo")
    .def_readonly( "stemLen", &MbStemInfo::stemLen )
    .def_readonly( "lexType", &MbStemInfo::lexType )
    .def_readonly( "fWeight", &MbStemInfo::fWeight )
    .def_readonly( "stLemma", &MbStemInfo::stLemma )
    .def_readonly( "grammar", &MbStemInfo::grammar );

  py::class_<WcStemInfo>(m, "WcStemInfo")
    .def_readonly( "stemLen", &WcStemInfo::stemLen )
    .def_readonly( "lexType", &WcStemInfo::lexType )
    .def_readonly( "fWeight", &WcStemInfo::fWeight )
    .def_readonly( "stLemma", &WcStemInfo::stLemma )
    .def_readonly( "grammar", &WcStemInfo::grammar );

  py::class_<MbStrMatch>(m, "MbStrMatch")
    .def_property_readonly( "text", []( const MbStrMatch& s )
      {  return py::bytes( std::string_view( s.sz, s.cc ) );  } )
    .def_readonly( "form", &MbStrMatch::id )
    .def("__repr__", []( const MbStrMatch& s )
      {
        return "WcStrMatch(id=" + std::to_string( s.id ) + ", text='"
          + std::string( s.sz, s.cc ) + "')";
      } );

  py::class_<WcStrMatch>(m, "WcStrMatch")
    .def_property_readonly( "text", []( const WcStrMatch& s )
      {  return py::cast( std::u16string_view( s.ws, s.cc ) );  } )
    .def_readonly( "form", &WcStrMatch::id )
    .def("__repr__", []( const WcStrMatch& s )
      {
        return "WcStrMatch(id=" + std::to_string( s.id ) + ", text='"
          + codepages::widetombcs( codepages::codepage_utf8, s.ws, s.cc ) + "')";
      } );

  m.attr("SF_IGNORE_CAPITALS") = py::int_(0x0002);
  m.attr("SF_NO_LEMMA_STRING") = py::int_(0x0004);
  m.attr("SF_NO_GRAMMAR_DATA") = py::int_(0x0008);
}
