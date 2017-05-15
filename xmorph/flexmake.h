# if !defined( _flexmake_h_ )
# define _flexmake_h_

# include <namespace.h>
# include <xmorph/scandict.h>
# include "mlmadefs.h"
# include <string.h>

namespace LIBMORPH_NAMESPACE
{
  class  stringCollect
  {
    byte_t*       output;
    const byte_t* prefix;
    size_t        ccpref;
    int           fcount;

  public:     // collector
    stringCollect( byte_t* o, const byte_t* p, size_t l ):
      output( o ), prefix( p ), ccpref( l ), fcount( 0 )  {}
    void  AddStr( const byte_t* addstr, size_t cchstr )
      {
        size_t        cbcopy;
        const byte_t* szcopy;

        for ( szcopy = prefix, cbcopy = ccpref; cbcopy-- > 0; )
          *output++ = *szcopy++;

        for ( ++fcount; cchstr-- > 0; )
          *output++ = *addstr++;

        *output++ = '\0';
      }
    int   GetLen() const
      {
        return fcount;
      }
  };

  struct  doCollectFlex
  {
    doCollectFlex( stringCollect& o, const byte_t* t, word16_t g, byte_t f ):
        output( o ), grinfo( g ), bflags( f )
      {  (void)t;  }

  public:     // functor
    int   operator () ( const byte_t* pflist, const byte_t* ptrace, unsigned ltrace ) const
      {
        int   ngrams = *pflist++;
      
        while ( ngrams-- > 0 )
        {
          word16_t  cginfo = getword16( pflist );
          byte_t    cflags = *pflist++;
        
          if ( cginfo == grinfo && (bflags & cflags) != 0 )
            output.AddStr( ptrace, ltrace );
        }
        return 0;
      }

  protected:  // variables
    stringCollect&  output;
    const word16_t  grinfo;
    const byte_t    bflags;

  };
  
  inline  int   BuildFlexSet(       byte_t* pszout,
                              const byte_t* ptable,
                              const byte_t* prefix, size_t  ccpref,
                              word16_t      grinfo, byte_t  bother )
  {
    byte_t        atrack[0x100];
    stringCollect output( pszout, prefix, ccpref );

    return RecursGetTrack<byte_t, int>( doCollectFlex( output, atrack, grinfo, bother ), ptable, atrack, 0 ) >= 0 ?
      output.GetLen() : -1;
  }

} // end namespace

# endif // _flexmake_h_
