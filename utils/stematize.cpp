# include "../api.hpp"
# include <mtc/sharedLibrary.hpp>
# include <string_view>
# include <cstring>
# include <vector>
# include <cstdio>
# include <chrono>
# include <unistd.h>

int   Stematize( IMlfaMbXX& morpho, const char* string )
{
  SStemInfoA  stmset[0x40];
  char        stbuff[0x100];
  SGramInfo   grbuff[0x40];
  int         scount = morpho.Lemmatize( string,
    stmset,
    stbuff,
    grbuff, sfIgnoreCapitals );
  auto  prefix = (const char*)"";

  if ( scount == 0 )
    return fprintf( stdout, "[]" ), 0;

  fprintf( stdout, "[" );

  for ( int i = 0; i < scount; ++i, prefix = "," )
  {
    auto& next = stmset[i];
    auto  pref = (const char*)"";

    fprintf( stdout, "%s\n  { \"lemma\": \"%s\",\t\"score\": %6.4f, \"class\": %3u, \"psp\": %2d, \"forms\": [",
      prefix,
      next.plemma,
      next.weight,
      next.nclass,
      next.pgrams->wdInfo & 0x3f );

    for ( unsigned g = 0; g < next.ngrams; ++g, pref = ", " )
      fprintf( stdout, "%s%d", pref, next.pgrams[g].idForm );

    fputs( "] }", stdout );

    prefix = ",";
  }

  return fputs( "\n]", stdout ), 0;
}

const char about[] = "stematize - dump lemmatization results for a word\n"
  "Usage: stematize lib-path createFn[:codepage] word\n"
  "\t" "lib-path is a full path lo the shared library of analyser to be loaded;\n"
  "\t" "createFn is a name of C-style interface access function;\n"
  "\t" "codepage is a multibyte text codepage name (1251, koi8, 866, utf-f), default is utf-8.";

int   main( int argc, char* argv[] )
{
  mtc::SharedLibrary  module;
  std::string         fnName;
  std::string         encode = ":utf-8";
  libmorphGetAPI      getAPI;
  IMlfaMbXX*          morpho;
  size_t              colpos;

  if ( argc < 4 )
    return fprintf( stdout, about ), 0;

// load the library
  if ( (module = mtc::SharedLibrary::Load( argv[1], mtc::disable_exceptions )) == nullptr )
    return fprintf( stderr, "Error: could not load library '%s'\n", argv[1] );

// get function name and encoding
  if ( (colpos = (fnName = argv[2]).find_first_of( ":" )) != std::string::npos )
  {
    if ( (encode = argv[2] + colpos)[0] == '\0' )
      return fprintf( stderr, "Error: empty encoding string\n" );
    fnName.resize( colpos );
  }

// get the accessor
  if ( (getAPI = (libmorphGetAPI)module.Find( fnName.c_str(), mtc::disable_exceptions )) == nullptr )
    return fprintf( stderr, "Error: could not find function '%s'\n", fnName.c_str() );

// get the interface
  if ( getAPI( (LIBFUZZY_API_4_MAGIC + encode).c_str(), (void**)&morpho ) != 0 )
    return fprintf( stderr, "Error: could not load morphological analyser API. Invalid codepage string?\n" );

// measure module speed
  return Stematize( *morpho, argv[3] );
}
