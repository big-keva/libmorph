# include <pybind11/morph-pad.hpp>
# include <moonycode/codes.h>
# include <eng.h>

# define HELP_MORPH_BUILDFORM   \
  "Builds the specified word form by lexeme and form id.\n"   \
  "\n"                                                        \
  "Arguments:\n"                                              \
  "  nlexid - lexeme id;\n"                                   \
  "  formid - form id.\n"                                     \
  "Returns a list of possible spellings, most often one.\n"

# define HELP_MORPH_CHECKWORD   \
  "Check  s the spelling of a passed word accodring to bit flags.\n"  \
  "\n"                                                                \
  "Arguments:\n"                                                      \
  "  word   - word string to check;\n"                                \
  "  flags  - a combination of bit flags:\n"                          \
  "    * SF_IGNORE_CAPITALS = 0x0002 - ignore capitalization.\n"      \
  "\n"                                                                \
  "Returns 1 if word is spelled correctly, else 0.\n"

# define HELP_MORPH_FINDMATCH   \
  "Searches for all the forms of any lexeme matching the passed template.\n"  \
  "Template contain letters and fnmatch wildcards * and ?, where * matches any characters,\n"\
  "and ? matches one character.\n"                                            \
  "Found forms are grouped by lexeme id and passed to the callback lexeme by lexeme until\n" \
  "callback returns nonzero result.\n"                                        \
  "\n"                                                                        \
  "Arguments:\n"                                                              \
  "  template - word form fnmatch-style string;\n"                            \
  "  callback - function matching the prototype fn( lexeme: int, forms: list ),\n"  \
  "             where list is an array of MbStrMatch or WcStrMatch structures.\n"

# define HELP_MORPH_LEMMATIZE   \
  "Performs dictionary analysis of the given word according to the settings and returns an array of identifications.\n" \
  "Arguments:\n"  \
  "  word  - the word to analyze;\n"        \
  "  flags - a combination of settings:\n"  \
  "    * SF_IGNORE_CAPITALS = 0x0002 - ignore capitalization;\n"        \
  "    * SF_NO_LEMMA_STRING = 0x0004 - do not generate normal forms;\n" \
  "    * SF_NO_GRAMMAR_DATA = 0x0008 - do not generate grammatical descriptions.\n" \
  "Returns:\n"    \
  "  An array of elements, each of which necessarily contains a lexeme identifier and, depending\n" \
  "  on the settings, may contain a lemma (the dictionary form of the word) and an array of grammatical\n" \
  "  descriptions of the corresponding forms.\n"

# define HELP_MORPH_PART_OFSP   \
  "Gets the technical part-of-speech value for the lexeme specified by id.\n"  \
  "\n"                          \
  "Arguments:\n"                \
  "  nlexid - lexeme id.\n"     \
  "\n"                          \
  "Returns:\n"                  \
  "  Integer technical part of speech (see https://libmorph.ru/html.html).\n"

# define HELP_FUZZY_LEMMATIZE   \
  "Performs a probabilistic analysis of the given word according to the settings and returns\n" \
  "an array of identifications.\n"                                           \
  "\n"                                                                              \
  "Arguments:\n"  \
  "  word  - the word to analyze;\n"        \
  "  flags - a combination of settings:\n"  \
  "    * SF_IGNORE_CAPITALS = 0x0002 - ignore capitalization;\n"        \
  "    * SF_NO_LEMMA_STRING = 0x0004 - do not generate normal forms;\n" \
  "    * SF_NO_GRAMMAR_DATA = 0x0008 - do not generate grammatical descriptions.\n" \
  "Returns:\n"    \
  "  An array of elements, each of which necessarily contains a lemma and lexeme identifier and, depending\n" \
  "  on the settings, may contain a lemma (the dictionary form of the word) and an array of grammatical\n" \
  "  descriptions of the corresponding forms.\n"  \
  "Возвращает:\n"     \
  "  An array of elements, each containing the length of a stem and, depending on the settings, may contain a lemma\n"  \
  "  (the dictionary form of the word) and an array of grammatical descriptions of the corresponding forms.\n"

PYBIND11_MODULE(pyengmorph, m)
{
  m.doc() = "libmorpheng for Python API";

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
    .def_readonly( "fProb", &MbStemInfo::fWeight )
    .def_readonly( "lemma", &MbStemInfo::stLemma )
    .def_readonly( "lStem", &MbStemInfo::stemLen )
    .def_readonly( "clsId", &MbStemInfo::lexType )
    .def_readonly( "grams", &MbStemInfo::grammar );

  py::class_<WcStemInfo>(m, "WcStemInfo")
    .def_readonly( "fProb", &WcStemInfo::fWeight )
    .def_readonly( "lemma", &WcStemInfo::stLemma )
    .def_readonly( "lStem", &WcStemInfo::stemLen )
    .def_readonly( "clsId", &WcStemInfo::lexType )
    .def_readonly( "grams", &WcStemInfo::grammar );

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

  py::class_<MorphAPI<IMlmaMbXX>>( m, "mlmaenMb" )
    .def( py::init( []( std::string cp )
      {  return new MorphAPI<IMlmaMbXX>( mlmaenGetAPI, cp );  } ) )
    .def( "build_form", &MorphAPI<IMlmaMbXX>::build_form, HELP_MORPH_BUILDFORM )
    .def( "check_word", &MorphAPI<IMlmaMbXX>::check_word, HELP_MORPH_CHECKWORD )
    .def( "find_match", &MorphAPI<IMlmaMbXX>::find_match, HELP_MORPH_FINDMATCH )
    .def( "lemmatize",  &MorphAPI<IMlmaMbXX>::lemmatize,  HELP_MORPH_LEMMATIZE )
    .def( "part_of_sp", &MorphAPI<IMlmaMbXX>::part_of_sp, HELP_MORPH_PART_OFSP );

  py::class_<MorphAPI<IMlmaWcXX>>( m, "mlmaenWc" )
    .def( py::init( []()
      {  return new MorphAPI<IMlmaWcXX>( mlmaenGetAPI, "utf-16" );  } ) )
    .def( "build_form", &MorphAPI<IMlmaWcXX>::build_form, HELP_MORPH_BUILDFORM )
    .def( "check_word", &MorphAPI<IMlmaWcXX>::check_word, HELP_MORPH_CHECKWORD )
    .def( "find_match", &MorphAPI<IMlmaWcXX>::find_match, HELP_MORPH_FINDMATCH )
    .def( "lemmatize", &MorphAPI<IMlmaWcXX>::lemmatize,   HELP_MORPH_LEMMATIZE )
    .def( "part_of_sp", &MorphAPI<IMlmaWcXX>::part_of_sp, HELP_MORPH_PART_OFSP );

  py::class_<FuzzyAPI<IMlfaMbXX>>( m, "mlfaenMb" )
    .def( py::init( []( std::string cp )
      {  return new FuzzyAPI<IMlfaMbXX>( mlfaenGetAPI, cp );  } ) )
    .def( "part_of_sp", &FuzzyAPI<IMlfaMbXX>::part_of_sp, "Get technical part of speach of rammatical class or 0 if invalid" )
    .def( "get_sample", &FuzzyAPI<IMlfaMbXX>::get_models, "Get the list of sample words for a grammatical class id" )
    .def( "lemmatize", &FuzzyAPI<IMlfaMbXX>::lemmatize,   HELP_FUZZY_LEMMATIZE )
    .def( "build_form", &FuzzyAPI<IMlfaMbXX>::build_form, "Build word form by lexeme and form id return a list of forms" );

  py::class_<FuzzyAPI<IMlfaWcXX>>( m, "mlfaenWc" )
    .def( py::init( []()
      {  return new FuzzyAPI<IMlfaWcXX>( mlfaenGetAPI, "utf-16" );  } ) )
    .def( "part_of_sp", &FuzzyAPI<IMlfaWcXX>::part_of_sp, "Get technical part of speach of rammatical class or 0 if invalid" )
    .def( "get_sample", &FuzzyAPI<IMlfaWcXX>::get_models, "Get the list of sample words for a grammatical class id" )
    .def( "lemmatize", &FuzzyAPI<IMlfaWcXX>::lemmatize,   HELP_FUZZY_LEMMATIZE )
    .def( "build_form", &FuzzyAPI<IMlfaWcXX>::build_form, "Build word form by lexeme id and form id and return a list of forms" );
}
