# include <api.hpp>
# include <cstring>
# include <vector>
# include <cstdio>
# include <cerrno>
# include <numeric>
# include <eng.h>

struct  PspEntry: std::vector<double>
{
  double  weight;
};

std::vector<PspEntry> pspfidMatrix;
double                totalLexemes = 0.0;

int   Accumulate( IMlmaMbXX& morpho, FILE* infile )
{
  char  szline[0x4000];

  while ( fgets( szline, sizeof(szline), infile ) != nullptr )
  {
    auto  srctop = szline;

    while ( *srctop != '\0' )
    {
      const char* srcorg;

      while ( *srctop != '\0' && (unsigned char)*srctop < 0x41 )
        ++srctop;

      for ( srcorg = srctop; (unsigned char)*srctop >= 0x41 || *srctop == '-' || *srctop == '\''; ++srctop )
        (void)NULL;

      if ( srctop - srcorg > 3 )
      {
        SLemmInfoA  lemmas[0x20];
        SGramInfo   gramma[0x40];
        int         lcount = morpho.Lemmatize( { srcorg, size_t(srctop - srcorg) },
          lemmas, {}, gramma, sfIgnoreCapitals );

        if ( lcount > 0 )
        {
          ++totalLexemes;

          for ( int i = 0; i < lcount; ++i )
          {
            auto  partSp = lemmas[i].pgrams->wdInfo;

            if ( lemmas[i].pgrams->idForm == 0xff )
              continue;

            // suppress other words except 1, 2, 3, 4, 17
            if ( partSp == 0 || (partSp > 4 && partSp < 17) || (partSp > 17) )
              continue;

            // reserve and increment psp
            if ( pspfidMatrix.size() <= partSp )
              pspfidMatrix.resize( partSp + 1 );

            pspfidMatrix[partSp].weight += 1.0 / lcount;

            // list the forms for this lexeme
            for ( unsigned g = 0; g < lemmas[i].ngrams; ++g )
            {
              auto  formid = lemmas[i].pgrams[g].idForm;

              if ( pspfidMatrix[partSp].size() <= formid )
                pspfidMatrix[partSp].resize( formid + 1 );
              pspfidMatrix[partSp][formid] += 1.0 / lemmas[i].ngrams;
            }
          }
        }
      }
    }
  }
  return 0;
}

int   Accumulate( IMlmaMbXX& morpho, const char* szpath )
{
  auto  infile = fopen( szpath, "rt" );
  int   nerror;

  if ( infile == NULL )
    return fprintf( stderr, "Could not open file '%s'\n", szpath ), ENOENT;

  nerror = Accumulate( morpho, infile );

  fclose( infile );
  return nerror;
}

void  DumpRanges( FILE* out, const std::vector<double>& tab, uint8_t psp )
{
  auto  prefix = "\n    ";

  if ( tab.size() == 0 )
    return;

  fprintf( out, "  static const float psp%02x_ranges[0x%02x] =\n  {",
    psp, uint8_t(tab.size()) );

  for ( size_t i = 0; i != tab.size(); )
  {
    fprintf( out, "%s%6.4f", prefix, tab[i] );
    if ( (++i % 12) == 0 )  prefix = ",\n    ";
      else prefix = ", ";
  }

  fprintf( out, "\n  };\n" );
}

void  DumpMatrix( FILE* out )
{
  const char*  prefix = "";

  fputs( "namespace NAMESPACE {\n\n", out );

  for ( size_t i = 0; i != pspfidMatrix.size(); ++i )
    DumpRanges( out, pspfidMatrix[i], i );

  fprintf( out, "  struct tagPspProbTable\n"
                "  {\n"
                "    const float           weight;\n"
                "    const float*          ranges;\n"
                "    const unsigned short  maxLen;\n"
                "  } pspfidProbTable[%d] =\n"
                "  {", int(pspfidMatrix.size()) );

  for ( unsigned i = 0; i != pspfidMatrix.size(); ++i, prefix = "," )
  {
    auto&  next = pspfidMatrix[i];

    if ( next.size() != 0 )
    {
      fprintf( out, "%s\n    { %6.4f, psp%02x_ranges, %d }",
        prefix, next.weight, i, int(next.size()) );
    }
      else
    fprintf( out, "%s\n    { 0.0000, nullptr, 0 }",
      prefix );
  }
  fprintf( out, "\n  };\n" );

  fputs( "\n}\n", out );
}

int   main( int argc, char* argv[] )
{
  IMlmaMbXX*  morpho;
  int         nerror;

  mlmaenGetAPI( LIBMORPH_API_4_MAGIC ":" "utf-8", (void**)&morpho );

  if ( argc < 2 )
    return fprintf( stderr, "usage: %s sample-text.txt\n", argv[0] ), EINVAL;

  if ( (nerror = Accumulate( *morpho, argv[1] )) != 0 )
    return nerror;

  double  glsumm = 0.0f;

  for ( auto& next: pspfidMatrix )
  {
    double  fmsumm = std::accumulate( next.begin(), next.end(), 0.0 );

    for ( auto& form: next )
      form /= fmsumm;

    glsumm += next.weight;
  }

  for ( auto& next: pspfidMatrix )
    next.weight /= glsumm;

  DumpMatrix( stdout );

  return 0;
}
