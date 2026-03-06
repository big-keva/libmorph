# include "../mlma-api.h"
# include <mtc/sharedLibrary.hpp>
# include <string_view>
# include <cstring>
# include <vector>
# include <cstdio>
# include <chrono>
# include <unistd.h>
# include <sys/stat.h>

bool  is_alpha_extended( char c )
{
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c & 0x80) != 0;
}

auto  GetWords( const char* beg, const char* end ) -> std::vector<std::string_view>
{
  std::vector<std::string_view> out;

  while ( beg < end )
  {
    const char* top;

    while ( beg != end && !is_alpha_extended( *beg ) )
      ++beg;

    if ( (top = beg++) == end )
      continue;

    while ( beg != end && is_alpha_extended( *beg ) )
       ++beg;

    if ( beg - top > 1 )
      out.emplace_back( top, beg - top );
  }
  return out;
}

int   Benchmark( IMlmaMbXX* morpho, const std::vector<std::string_view>& subset )
{
  unsigned  nTotal = 0;
  unsigned  nValid = 0;
  auto      tstart = std::chrono::steady_clock::now();
  double    elapse;

  for ( int i = 0; i < 100; ++i, nTotal += subset.size() )
  {
    for ( auto& next: subset )
    {
      SLemmInfoA  lemmas[32];

      if ( morpho->Lemmatize( { next.data(), next.size() }, lemmas, {}, {}, sfIgnoreCapitals ) != 0 )
        ++nValid;
    }
  }

  elapse = std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::steady_clock::now() - tstart ).count() / 1000.0;

  return fprintf( stdout,
    "total words: %u\n"
    "known words: %u (%4.1f%%)\n"
    "elapsed time: %.1f seconds\n"
    "measurements: %u WPS\n", nTotal, nValid, nValid * 100.0 / nTotal,
      elapse, unsigned(nTotal / elapse) ), 0;
}

int   Benchmark( IMlmaMbXX* morpho, int fd )
{
  struct stat fistat;
  auto        buffer = std::vector<char>();
  auto        awords = std::vector<const char*>();
  int         nerror;

// read source file
  if ( fstat( fd, &fistat ) == -1 )
  {
    return fprintf( stderr, "fstat() failed, error '%s'\n",
      strerror( nerror = errno ) ), nerror;
  }

  buffer.resize( fistat.st_size );

  if ( read( fd, buffer.data(), fistat.st_size ) != fistat.st_size )
  {
    return fprintf( stderr, "read() failed, error '%s'\n",
      strerror( nerror = errno ) ), nerror;
  }

// wordbreak and perform the benchmark
  return Benchmark( morpho, GetWords( buffer.data(), buffer.data() + buffer.size() ) );
}

int   Benchmark( IMlmaMbXX* morpho, const char* szpath )
{
  int  infile = open( szpath, O_RDONLY );
  int  nerror;

  if ( infile == -1 )
  {
    return fprintf( stderr, "Failed to open file %s, error '%s'\n", szpath,
      strerror( nerror = errno ) ), nerror;
  }

  nerror = Benchmark( morpho, infile );

  return close( infile ), nerror;
}

const char about[] = "benchmark - measure morphological analyser speed (Words Per Second)\n"
  "Usage: benchmark lib-path createFn[:codepage] sample-text\n"
  "\t" "lib-path is a full path lo the shared library of analyser to be loaded;\n"
  "\t" "createFn is a name of C-style interface access function;\n"
  "\t" "codepage is a multibyte text codepage name (1251, koi8, 866, utf-f); default is utf-8."
  "Reads source file, breaks words by spaces and punctuations and measures lemmatizer speed.";

int   main( int argc, char* argv[] )
{
  mtc::SharedLibrary  module;
  std::string         stMake;
  const char*         encode = "utf-8";
  libmorphGetAPI      fnMake;
  IMlmaMbXX*          morpho;
  size_t              colpos;

  if ( argc < 4 )
    return fprintf( stdout, about ), 0;

// load the library
  if ( (module = mtc::SharedLibrary::Load( argv[1], mtc::disable_exceptions )) == nullptr )
    return fprintf( stderr, "Error: could not load library '%s'\n", argv[1] );

// get function name and encoding
  if ( (colpos = (stMake = argv[2]).find_first_of( ":" )) != std::string::npos )
  {
    if ( (encode = argv[2] + 1 + colpos)[0] == '\0' )
      return fprintf( stderr, "Error: empty encoding string\n" );
    stMake.resize( colpos );
  }

// get the accessor
  if ( (fnMake = (libmorphGetAPI)module.Find( stMake.c_str(), mtc::disable_exceptions )) == nullptr )
    return fprintf( stderr, "Error: could not find function '%s'\n", stMake.c_str() );

// get the interface
  if ( fnMake( encode, (void**)&morpho ) != 0 )
    return fprintf( stderr, "Error: could not load morphological analyser API. Invalid codepage string?\n" );

// measure module speed
  return Benchmark( morpho, argv[3] );
}
