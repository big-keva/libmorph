# include "flexmake.h"
# include "scandict.h"
# include <algorithm>

namespace LIBMORPH_NAMESPACE
{

  class FlexCollector
  {
    const flexinfo  inflex;
    const fragment  prefix;

    byte_t*         output;
    int             ntails;

  public:
    FlexCollector( byte_t* flush, const flexinfo& gramm, const fragment& spref ):
      inflex( gramm ),
      prefix( spref ),
      output( flush ),
      ntails( 0 ) {}
    FlexCollector( byte_t* flush, const flexinfo& gramm ):
      FlexCollector( flush, gramm, { nullptr, 0 } )  {}

  public:
    int   operator () ( const byte_t* ptable, const byte_t* pszstr, unsigned cchstr )
    {
      for ( int ngrams = *ptable++; ngrams-- > 0; )
      {
        word16_t  cginfo = getword16( ptable );
        byte_t    cflags = *ptable++;

        if ( cginfo != inflex.gramm || (cflags & inflex.flags) == 0 )
          continue;

        output =
          std::copy( pszstr, pszstr + cchstr,
          std::copy( prefix.begin(), prefix.end(), output ) );

        *output++ = 0, ++ntails;
      }
      return 0;
    }
    int  count() const  {  return ntails;  }
  };

  int   BuildFlexSet(
    byte_t*         pszout,
    const byte_t*   ptable,
    const flexinfo& inflex,
    const fragment& prefix )
  {
    byte_t        atrack[0x100];
    FlexCollector output( pszout, inflex, prefix );

    return RecursGetTrack<byte_t, int>( output, ptable, atrack, 0 ) >= 0 ?
      output.count() : -1;
  }

} // end namespace
