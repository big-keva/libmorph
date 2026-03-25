# include "morph-pad.hpp"
# include <moonycode/codes.h>
# include <rus.h>

# define HELP_MORPH_LEMMATIZE   \
  "Выполняет словарный анализ поданного слова в соответствии с настройками\n"       \
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
  "  Массив элементов, каждый из которых обязательно содержит идентификатор лексемы\n"  \
  "  и, в зависимости от настроек, может содержать лемму (словарную форму слова)\n" \
  "  и массив грамматических описаний соответствующих форм.\n"

# define HELP_FUZZY_LEMMATIZE   \
  "Выполняет вероятностный анализ поданного слова в соответствии с настройками\n"   \
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
  "  Массив элементов, каждый из которых содержит длину основы и, в зависимости от настроек,\n"  \
  "  может содержать лемму (словарную форму слова) и массив грамматических описаний\n" \
  "  соответствующих форм.\n"

PYBIND11_MODULE(pyrusmorph, m)
{
  m.doc() = "libmorphrus for Python API";

  m.attr("SF_IGNORE_CAPITALS") = py::int_(0x0002);
  m.attr("SF_NO_LEMMA_STRING") = py::int_(0x0004);
  m.attr("SF_NO_GRAMMAR_DATA") = py::int_(0x0008);

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

  py::class_<MorphAPI<IMlmaMbXX>>( m, "mlmaruMb" )
    .def( py::init( []( std::string cp )
      {  return new MorphAPI<IMlmaMbXX>( mlmaruGetAPI, cp );  } ) )
    .def( "part_of_sp", &MorphAPI<IMlmaMbXX>::part_of_sp, "Get technical part of speach of lexeme or 0 if invalid" )
    .def( "check_word", &MorphAPI<IMlmaMbXX>::check_word, "Check spelling of a word and return 1 (OK) or 0 (misspelled)" )
    .def( "lemmatize",  &MorphAPI<IMlmaMbXX>::lemmatize,  HELP_MORPH_LEMMATIZE )
    .def( "build_form", &MorphAPI<IMlmaMbXX>::build_form, "Build word form by lexeme and form id return a list of forms" )
    .def( "find_match", &MorphAPI<IMlmaMbXX>::find_match, "Search lexemes and forms matching the passed template" );

  py::class_<MorphAPI<IMlmaWcXX>>( m, "mlmaruWc" )
    .def( py::init( []()
      {  return new MorphAPI<IMlmaWcXX>( mlmaruGetAPI, "utf-16" );  } ) )
    .def( "part_of_sp", &MorphAPI<IMlmaWcXX>::part_of_sp, "Get technical part of speach of lexeme or 0 if invalid" )
    .def( "check_word", &MorphAPI<IMlmaWcXX>::check_word, "Check spelling of a word and return 1 (OK) or 0 (misspelled)" )
    .def( "lemmatize", &MorphAPI<IMlmaWcXX>::lemmatize,   HELP_MORPH_LEMMATIZE )
    .def( "build_form", &MorphAPI<IMlmaWcXX>::build_form, "Build word form by lexeme and form id return a list of forms" )
    .def( "find_match", &MorphAPI<IMlmaWcXX>::find_match, "Search lexemes and forms matching the passed template" );

  py::class_<FuzzyAPI<IMlfaMbXX>>( m, "mlfaruMb" )
    .def( py::init( []( std::string cp )
      {  return new FuzzyAPI<IMlfaMbXX>( mlfaruGetAPI, cp );  } ) )
    .def( "part_of_sp", &FuzzyAPI<IMlfaMbXX>::part_of_sp, "Get technical part of speach of lexeme or 0 if invalid" )
    .def( "get_sample", &FuzzyAPI<IMlfaMbXX>::get_models, "Get sample word for a class" )
    .def( "lemmatize", &FuzzyAPI<IMlfaMbXX>::lemmatize,   HELP_FUZZY_LEMMATIZE )
    .def( "build_form", &FuzzyAPI<IMlfaMbXX>::build_form, "Build word form by lexeme and form id return a list of forms" );

  py::class_<FuzzyAPI<IMlfaWcXX>>( m, "mlfaruWc" )
    .def( py::init( []()
      {  return new FuzzyAPI<IMlfaWcXX>( mlfaruGetAPI, "utf-16" );  } ) )
    .def( "part_of_sp", &FuzzyAPI<IMlfaWcXX>::part_of_sp, "Get technical part of speach of lexeme or 0 if invalid" )
    .def( "get_sample", &FuzzyAPI<IMlfaWcXX>::get_models, "Get sample word for a class" )
    .def( "lemmatize", &FuzzyAPI<IMlfaWcXX>::lemmatize,   HELP_FUZZY_LEMMATIZE )
    .def( "build_form", &FuzzyAPI<IMlfaWcXX>::build_form, "Build word form by lexeme and form id return a list of forms" );
}
