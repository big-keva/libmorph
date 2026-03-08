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
struct XxLemmInfo
{
  lexeme_t                                lexid;
  std::optional<std::basic_string<Char>>  lemma;
  std::optional<std::vector<SGramInfo>>   grams;
};

struct MbLemmInfo: XxLemmInfo<char>     {};
struct WcLemmInfo: XxLemmInfo<widechar> {};

struct MbStrMatch: SStrMatch  {};
struct WcStrMatch: SStrMatch  {};

template <class Mlma>
class MorphAPI
{
  Mlma*   mlma;

  typename Mlma::LemmType lxbuff[32];
  typename Mlma::CharType stbuff[2048];
  typename Mlma::GramType grbuff[512];

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
    if ( getAPI( codepage.c_str(), (void**)&mlma ) != 0 || mlma == nullptr )
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
      return mlma->FindMatch( stringData( pattern ), [&]( lexeme_t nlexid, int n, const SStrMatch* p )
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

PYBIND11_MODULE(libmorphrus, m)
{
  m.doc() = "libmorphrus for Python API";

  py::class_<MorphAPI<IMlmaMbXX>>( m, "mlmaruMb" )
    .def( py::init( []( std::string cp )
      {  return new MorphAPI<IMlmaMbXX>( mlmaruGetAPI, cp );  } ) )
    .def( "part_of_sp", &MorphAPI<IMlmaMbXX>::part_of_sp, "Get technical part of speach of lexeme or 0 if invalid" )
    .def( "check_word", &MorphAPI<IMlmaMbXX>::check_word, "Check spelling of a word and return 1 (OK) or 0 (misspelled)" )
    .def( "lemmatize",  &MorphAPI<IMlmaMbXX>::lemmatize,
      HELP_LEMMATIZE )
    .def( "build_form", &MorphAPI<IMlmaMbXX>::build_form, "Build word form by lexeme and form id return a list of forms" )
    .def( "find_match", &MorphAPI<IMlmaMbXX>::find_match, "Search lexemes and forms matching the passed template" );

  py::class_<MorphAPI<IMlmaWcXX>>( m, "mlmaruWc" )
    .def( py::init( []()
      {  return new MorphAPI<IMlmaWcXX>( mlmaruGetAPI, "utf-16" );  } ) )
    .def( "part_of_sp", &MorphAPI<IMlmaWcXX>::part_of_sp, "Get technical part of speach of lexeme or 0 if invalid" )
    .def( "check_word", &MorphAPI<IMlmaWcXX>::check_word, "Check spelling of a word and return 1 (OK) or 0 (misspelled)" )
    .def( "lemmatize", &MorphAPI<IMlmaWcXX>::lemmatize,
      HELP_LEMMATIZE )
    .def( "build_form", &MorphAPI<IMlmaWcXX>::build_form, "Build word form by lexeme and form id return a list of forms" )
    .def( "find_match", &MorphAPI<IMlmaWcXX>::find_match, "Search lexemes and forms matching the passed template" );

  py::class_<SGramInfo>(m, "GramInfo")
    .def_readonly( "wdInfo", &SGramInfo::wdInfo )
    .def_readonly( "idForm", &SGramInfo::idForm )
    .def_readonly( "grInfo", &SGramInfo::grInfo )
    .def_readonly( "bFlags", &SGramInfo::bFlags );

  py::class_<MbLemmInfo>(m, "MbLemmInfo")
    .def_readonly( "lexid", &MbLemmInfo::lexid )
    .def_readonly( "lemma", &MbLemmInfo::lemma )
    .def_readonly( "grams", &MbLemmInfo::grams );

  py::class_<WcLemmInfo>(m, "WcLemmInfo")
    .def_readonly( "lexid", &WcLemmInfo::lexid )
    .def_readonly( "lemma", &WcLemmInfo::lemma )
    .def_readonly( "grams", &WcLemmInfo::grams );

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
