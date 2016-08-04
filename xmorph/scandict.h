# if !defined( __scandict_h__ )
# define  __scandict_h__
# include <assert.h>
# include <limits.h>
# include <stddef.h>

# if defined( LIBMORPH_NAMESPACE )
namespace LIBMORPH_NAMESPACE {
# endif  // LIBMORPH_NAMESPACE

  template <class O>
  class  const_action
  {
    O&  o;

  public:     // construction
    const_action( O& r ): o( r )
      {}

  public:     // actor templates
    template <class A>
    int     operator () ( A a ) const
      {  return o( a );  } 
    template <class A, class B>
    int     operator () ( A a, B b ) const
      {  return o( a, b );  } 
    template <class A, class B, class C>
    int     operator () ( A a, B b, C c ) const
      {  return o( a, b, c );  } 
    template <class A, class B, class C, class D>
    int     operator () ( A a, B b, C c, D d ) const
      {  return o( a, b, c, d );  } 
  };

  inline  unsigned  __xmorph__getserial__( const unsigned char*& p )
  {
    unsigned char bfetch = *p++;
    unsigned      serial = bfetch & ~0x80;
    int           nshift = 1;

    while ( (bfetch & 0x80) != 0 )
      serial |= (((unsigned)(bfetch = *p++) & ~0x80) << (nshift++ * 7));
    return serial;
  }

  template <class flattype>
  inline  bool  hasupper( flattype a )
    {  return (a & (1 << (sizeof(a) * CHAR_BIT - 1))) != 0;  }

  template <class flagtype>
  inline  int   getlower( flagtype a )
    {  return a & ~(1 << (sizeof(a) * CHAR_BIT - 1));  }

  template <class flagtype>
  struct  scan_stack
  {
    const unsigned char*  thestr;
    size_t                cchstr;
    unsigned char         chfind;
    const unsigned char*  thedic;
    flagtype              aflags;
    int                   ccount;

  public:     // init
    scan_stack*     setlevel( const unsigned char* p, const unsigned char* s, size_t l )
      {
        ccount = getlower( aflags = *(flagtype*)(thedic = p) );
          thedic += sizeof(flagtype);
        thestr = s;
          chfind = (cchstr = l) > 0 ? *thestr : 0;
        return this;
      }
    const unsigned char* findchar()
      {
        while ( ccount-- > 0 )
        {
          unsigned char         chnext = *thedic++;
          unsigned              sublen = __xmorph__getserial__( thedic );
          const unsigned char*  subdic = thedic;  thedic += sublen;

          if ( chfind == chnext )
            return subdic;
          if ( chfind >  chnext && !hasupper( aflags ) )
            return 0;
        }
        return 0;
      }
  };

  template <class aflags, class result, class action>
  result  LinearScanDict( action&               doitem, const unsigned char*  thedic,
                          const unsigned char*  thestr, size_t                cchstr )
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

      if ( hasupper( pstack->aflags ) )
        if ( (retval = doitem( pstack->thedic, pstack->thestr, pstack->cchstr )) != (result)0 )
          return retval;

      --pstack;
    }
    return (result)0;
  }

  template <class aflags, class result, class action>
  result  RecursScanDict( const action&         action, const unsigned char*  thedic,
                          const unsigned char*  thestr, size_t                cchstr )
  {
    aflags        uflags = *(aflags*)thedic;  thedic += sizeof(aflags);
    int           ncount = getlower( uflags );
    unsigned char chfind;
    result        retval;

    assert( cchstr > 0 );

    for ( chfind = *thestr; ncount-- > 0; )
    {
      unsigned char         chnext = *thedic++;
      unsigned              sublen = __xmorph__getserial__( thedic );
      const unsigned char*  subdic = thedic;  thedic += sublen;

      if ( chnext == chfind && cchstr > 0 )
        if ( (retval = RecursScanDict<aflags, result, action>( action, subdic, thestr + 1, cchstr - 1 )) != (result)0 )
          return retval;
    }
    return hasupper( uflags ) ? action( thedic, thestr, cchstr ) : (result)0;
  }

  template <class aflags, class result, class action>
  result  RecursGetTrack( const action&         doitem,
                          const unsigned char*  thedic,
                          unsigned char*        ptrack,
                          unsigned              ltrack,
                          const unsigned char*  dicpos = NULL )
  {
    aflags    uflags = *(aflags*)thedic;  thedic += sizeof(aflags);
    int       ncount = getlower( uflags );

    while ( ncount-- > 0 )
    {
      unsigned char         chnext = *thedic++;
      unsigned              sublen = __xmorph__getserial__( thedic );
      const unsigned char*  subdic;
      int                   nerror;

      thedic = (subdic = thedic) + sublen;

      if ( dicpos == NULL || ( dicpos >= subdic && dicpos <= thedic ) )
      {
        ptrack[ltrack] = chnext;

        if ( (nerror = RecursGetTrack<aflags, result, action>( doitem, subdic, ptrack, ltrack + 1, dicpos )) != 0 )
          return nerror;
      }
    }
    return hasupper( uflags ) ? doitem( thedic, ptrack, ltrack ) : (result)0;
  }

  template <class flagtype, class result, class action>
  result  ForEachElement( const action& output, const unsigned char* thedic )
  {
    flagtype  bflags;
    result    retval;
    int       acount;

    for ( acount = getlower( bflags = *(flagtype*)thedic ), thedic += sizeof(flagtype); acount-- > 0 ; )
    {
      unsigned        sublen = __xmorph__getserial__( ++thedic );

      if ( (retval = EnumDict<flagtype, result>( output, thedic )) != 0 ) return retval;
        else  thedic += sublen;
    }
    return hasupper( bflags ) ? output( thedic ) : (result)0;
  }

# if defined( LIBMORPH_NAMESPACE )
}  // LIBMORPH_NAMESPACE
# endif  // LIBMORPH_NAMESPACE

# endif  // __scandict_h__
