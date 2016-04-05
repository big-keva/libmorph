# include "flexmake.h"

namespace LIBMORPH_NAMESPACE
{

  int   CreateDestStringsOnTable( byte08_t*       pszout,
                                  byte08_t*       curstr,
                                  unsigned        curlen,
                                  const byte08_t* ptable,
                                  word16_t        grInfo,
                                  byte08_t        bother )
  {
    byte08_t  bflags;
    int       ccount = (bflags = *ptable++) & 0x7f;
    int       nfound;

    for ( nfound = 0; ccount-- > 0; )
    {
      const byte08_t* subdic;
      int             sublen;

      curstr[curlen] = *ptable++;
        sublen = getserial( ptable );
      ptable = sublen + (subdic = ptable);

      for ( sublen = CreateDestStringsOnTable( pszout, curstr, curlen + 1, subdic, grInfo, bother );
        sublen-- > 0; pszout += strlen( (char*)pszout ) + 1, ++nfound ) (void)NULL;
    }

    if ( (bflags & 0x80) != 0 )
    {
      int   ngrams = *ptable++;
      
      while ( ngrams-- > 0 )
      {
        word16_t  grinfo = getword16( ptable );
        byte08_t  cflags = *ptable++;
        
        if ( grInfo == grinfo && (bother & cflags) != 0 )
        {
          pszout = curlen + (byte08_t*)memcpy( pszout, curstr, curlen );
            *pszout++ = 0;
          ++nfound;
        }
      }
    }

    return nfound;
  }

} // end namespace
