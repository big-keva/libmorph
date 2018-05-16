# if !defined( __scandict_h__ )
# define  __scandict_h__
# include "typedefs.h"
# include <assert.h>
# include <limits.h>
# include <stddef.h>

# if defined( LIBMORPH_NAMESPACE )
namespace LIBMORPH_NAMESPACE {
# endif  // LIBMORPH_NAMESPACE

  template <class T>
  struct loader
  {
    static T  get( const unsigned char*& )  {}
  };

  template <>
  struct loader<unsigned char>
  {
    static  unsigned char get( const unsigned char*& s )
      {  return *s++;  }
  };

  template <>
  struct loader<unsigned short>
  {
    static unsigned short get( const unsigned char*& s )
      {
        unsigned char   blower = *s++;
        unsigned short  bupper = *s++;
        return blower | (bupper << 8);
      }
  };

  template <class countype>
  struct counter
  {
    static  bool      hasupper( countype a )
      {  return (a & (1 << (sizeof(countype) * CHAR_BIT - 1))) != 0;  }
    static  int       getlower( countype a )
      {  return a & ~(1 << (sizeof(countype) * CHAR_BIT - 1));  }
    static  countype  getvalue( const unsigned char*& s )
      {  return loader<countype>::get( s );  }
  };

  template <class sizetype>
  struct  scan_stack
  {
    const unsigned char*  thestr;
    size_t                cchstr;
    unsigned char         chfind;
    const unsigned char*  thedic;
    sizetype              aflags;
    int                   ccount;

  public:     // init
    scan_stack*     setlevel( const unsigned char* p, const unsigned char* s, size_t l )
      {
        ccount = counter<sizetype>::getlower(
        aflags = counter<sizetype>::getvalue( thedic = p ) );
        thestr = s;
          chfind = (cchstr = l) > 0 ? *thestr : 0;
        return this;
      }
    const unsigned char* findchar()
      {
        while ( ccount-- > 0 )
        {
          unsigned char         chnext = *thedic++;
          unsigned              sublen = getserial( thedic );
          const unsigned char*  subdic = thedic;  thedic += sublen;

          if ( chfind == chnext )
            return subdic;
          if ( chfind >  chnext && !counter<sizetype>::hasupper( aflags ) )
            return 0;
        }
        return 0;
      }
  };

  template <class aflags, class result, class action>
  result  LinearScanDict( action&       doitem, const byte_t* thedic,
                          const byte_t* thestr, size_t        cchstr )
  {
    scan_stack<aflags>  astack[0x40];     // never longer words
    scan_stack<aflags>* pstack;
    result              retval;

    for ( (pstack = astack)->setlevel( thedic, thestr, cchstr ); pstack >= astack; )
    {
      const unsigned char*  subdic;

    // check if not found or no more data
      if ( (subdic = pstack->findchar()) != 0 )
      {
        pstack = (pstack + 1)->setlevel( subdic, pstack->thestr + 1, pstack->cchstr - 1 );
        continue;
      }

      if ( counter<aflags>::hasupper( pstack->aflags ) )
        if ( (retval = doitem( pstack->thedic, pstack->thestr, pstack->cchstr )) != (result)0 )
          return retval;

      --pstack;
    }
    return (result)0;
  }

  template <class aflags, class result, class action>
  result  RecursScanDict( const action&         doitem, const unsigned char*  thedic,
                          const unsigned char*  thestr, size_t                cchstr )
  {
    aflags        uflags = counter<aflags>::getvalue( thedic );
    int           ncount = counter<aflags>::getlower( uflags );
    unsigned char chfind;
    result        retval;

    assert( cchstr > 0 );

    for ( chfind = *thestr; ncount-- > 0; )
    {
      unsigned char         chnext = *thedic++;
      unsigned              sublen = getserial( thedic );
      const unsigned char*  subdic = thedic;  thedic += sublen;

      if ( chnext == chfind && cchstr > 0 )
        if ( (retval = RecursScanDict<aflags, result, action>( doitem, subdic, thestr + 1, cchstr - 1 )) != (result)0 )
          return retval;
    }
    return counter<aflags>::hasupper( uflags ) ? doitem( thedic, thestr, cchstr ) : (result)0;
  }

  template <class aflags, class result, class action>
  result  RecursGetTrack( const action&         doitem,
                          const unsigned char*  thedic,
                          unsigned char*        ptrack,
                          unsigned              ltrack,
                          const unsigned char*  dicpos = NULL )
  {
    aflags    uflags = counter<aflags>::getvalue( thedic );
    int       ncount = counter<aflags>::getlower( uflags );

    while ( ncount-- > 0 )
    {
      unsigned char         chnext = *thedic++;
      unsigned              sublen = getserial( thedic );
      const unsigned char*  subdic;
      int                   nerror;

      thedic = (subdic = thedic) + sublen;

      if ( dicpos == NULL || (dicpos >= subdic && dicpos <= thedic) )
      {
        ptrack[ltrack] = chnext;

        if ( (nerror = RecursGetTrack<aflags, result, action>( doitem, subdic, ptrack, ltrack + 1, dicpos )) != 0 )
          return nerror;
      }
    }
    return counter<aflags>::hasupper( uflags ) ? doitem( thedic, ptrack, ltrack ) : (result)0;
  }

# if defined( LIBMORPH_NAMESPACE )
}  // LIBMORPH_NAMESPACE
# endif  // LIBMORPH_NAMESPACE

# endif  // __scandict_h__
