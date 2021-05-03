# include "pageman.h"
# include "mlmadefs.h"
# include <assert.h>
# include <string.h>

namespace __libmorpheng__
{

  // CPageManager

  unsigned char** CPageMan::FindPage( unsigned char   chword,
                                      unsigned char** lplast )
  {
    if ( lplast == NULL ) lplast = ppages + npages - 1;
      else --lplast;

    for ( ; lplast >= ppages; --lplast )
    {
      unsigned char*  lppage = *lplast;
      int             ccount = lppage[2];

      assert( ccount > 0 );

      if ( lppage[3] == '\0' )
        break;
      if ( chword < lppage[3] || chword > lppage[ccount + 2] )
        continue;
      break;
    }
    return ( lplast >= ppages ? lplast : NULL );
  }

  // CLIDsManager

  SLidsRef*   CLIDsMan::FindPage( lexeme_t  lid )
  {
    SLidsRef* lpcurr = pageList;
    unsigned  ucount = numPages;

    while ( ucount-- > 0 )
    {
      if ( GetWord32( &lpcurr->maxvalue ) >= lid
        && GetWord32( &lpcurr->minvalue ) <= lid )
          return lpcurr;
      ++lpcurr;
    }
    return NULL;
  }

} // end __libmorpheng__ namespace

