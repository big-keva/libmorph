# include <api.hpp>
# include <vector>
# include <cstdio>
# include <cerrno>
#include <numeric>
#include <rus.h>

struct  PspEntry: std::vector<double>
{
  double  weight;
};

std::vector<PspEntry> pspfidMatrix;
double                totalLexemes = 0.0;

auto  MapVerbFid( uint8_t fid ) -> uint8_t
{
  return (fid >= 6 && fid <= 17) ? fid + 12 : fid;
}

auto  ToPartOfSp( uint8_t psp ) -> uint8_t
{
  switch ( psp & 0x3f )
  {
    case 1: case 2: case 3: case 4: case 5: case 6:
      return 1;
    case 7: case 8: case 9:
      return 7;
    case 13: case 14: case 15:
      return 13;
    case 16: case 17: case 18:
      return 16;
    case 25: case 26:
      return 25;
    case 38: case 39: case 40: case 41:
    case 42: case 44: case 45: case 46:
      return psp;
    default:
      return 0;
  }
}

int   Accumulate( IMlmaMbXX& morpho, FILE* infile )
{
  char  szline[0x4000];

  while ( fgets( szline, sizeof(szline), infile ) != nullptr )
  {
    auto  srctop = szline;

    while ( *srctop != '\0' )
    {
      const char* srcorg;

      while ( *srctop != '\0' && (unsigned char)*srctop <= 0xc0 )
        ++srctop;

      for ( srcorg = srctop; (unsigned char)*srctop >= 0xc0 || *srctop == '-'; ++srctop )
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
            auto  partSp = ToPartOfSp( lemmas[i].pgrams->wdInfo );

            // suppress non-exclusive imperatives
            if ( partSp == 1 && lemmas[i].pgrams->idForm == 2 && lcount > 1 )
              continue;
            if ( lemmas[i].pgrams->idForm == 0xff )
              continue;
            // suppress other words
            if ( partSp == 0 )
              continue;

            // reserve and increment psp
            if ( pspfidMatrix.size() <= partSp )
              pspfidMatrix.resize( partSp + 1 );

            pspfidMatrix[partSp].weight += 1.0 / lcount;

            // list the forms for this lexeme
            for ( int g = 0; g < lemmas[i].ngrams; ++g )
            {
              auto  formid = partSp == 1 ? MapVerbFid(
                lemmas[i].pgrams[g].idForm ) :
                lemmas[i].pgrams[g].idForm;

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
    psp, tab.size() );

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

  for ( size_t i = 0; i != pspfidMatrix.size(); ++i )
    DumpRanges( out, pspfidMatrix[i], i );

  fprintf( out, "  static struct\n"
                "  {\n"
                "    const float     weight;\n"
                "    const float*    ranges;\n"
                "    const uint16_t  maxLen;\n"
                "  } pspfid_Table[%d] =\n"
                "  {", pspfidMatrix.size() );

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
}

int   main()
{
  IMlmaMbXX*  morpho;

  mlmaruGetAPI( LIBMORPH_API_4_MAGIC ":" "1251", (void**)&morpho );

  Accumulate( *morpho, "/home/keva/dev/libmorph/contrib/moonycode/text/kondrashov.txt" );

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
