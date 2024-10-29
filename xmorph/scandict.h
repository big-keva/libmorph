# if !defined( __libmorph_scandict_h__ )
# define  __libmorph_scandict_h__
# include "typedefs.h"
# include <cstring>
# include <cstddef>
# include <cstdint>
# include <climits>
# include <cassert>

# if !defined( wfPostSt )
#   define  wfPostSt    0x8000        /* Stem has post-text definition          */
# endif

namespace libmorph {

  class Counters
  {
  protected:
    static  auto  getvalue( unsigned char*, const unsigned char*& s ) -> unsigned char
    {
      return *s++;
    }

    static  auto  getvalue( unsigned short*, const unsigned char*& s ) -> unsigned short
    {
      unsigned char   blower = *s++;
      unsigned short  bupper = *s++;
      return blower | (bupper << 8);
    }

    template <class countype>
    struct counter
    {
      static  bool      hasupper( countype a )
        {  return (a & (1 << (sizeof(countype) * CHAR_BIT - 1))) != 0;  }
      static  int       getlower( countype a )
        {  return a & ~(1 << (sizeof(countype) * CHAR_BIT - 1));  }
      static  countype  getvalue( const unsigned char*& s )
        {  return Counters::getvalue( (countype*)nullptr, s );  }
    };
  };

  class   Flat final: public Counters
  {
    template <class collector>  class LookupList;
    template <class collector>  class SelectView;

    template <class sizetype>
    struct  scan_stack
    {
      fragment              thestr;
      unsigned char         chfind;
      const unsigned char*  thedic;
      sizetype              aflags;
      int                   ccount;

    public:     // init
      auto  setlevel( const unsigned char* p, const fragment& s ) -> scan_stack*
        {
          ccount = counter<sizetype>::getlower(
          aflags = counter<sizetype>::getvalue( thedic = p ) );
          thestr = s;
            chfind = (thestr.len) > 0 ? *thestr.str : 0;
          return this;
        }
      auto  findchar() -> const unsigned char*
        {
          while ( ccount-- > 0 )
          {
            unsigned char         chnext = *thedic++;
            unsigned              sublen = getserial( thedic );
            const unsigned char*  subdic = thedic;  thedic += sublen;

            if ( chfind == chnext )
              return subdic;
            if ( chfind >  chnext && !counter<sizetype>::hasupper( aflags ) )
              return nullptr;
          }
          return nullptr;
        }
    };

  public:
   /*
    * Tree( ... )
    *
    * Сканер древесного словаря в поисках словоформы. Нерекурсивный обход дерева
    * с вызовом алгоритма-параметра для найденного узла.
    */
    template <class aflags, class action, class result = decltype((*(action*)0)(nullptr, {}))> static
    auto  ScanTree( const action& doitem, const byte_t* thedic, const fragment& src ) -> result;

    template <class collector> static
    auto  ScanList( const collector& c ) -> LookupList<collector>
      {  return LookupList<collector>( c );  }

    template <class collector> static
    auto  ViewList( const collector& c, const uint8_t* p ) -> SelectView<collector>
      {  return SelectView<collector>( c, p );  }

    template <class aflags, class action, class result = decltype((*(action*)0)(nullptr, {}) )> static
    auto  GetTrack( const action&   doitem,
                    const uint8_t*  thedic,
                          uint8_t*  ptrack,
                          size_t    ltrack,
                    const uint8_t*  dicpos ) -> result
    {
      aflags  uflags = counter<aflags>::getvalue( thedic );
      int     ncount = counter<aflags>::getlower( uflags );

      while ( ncount-- > 0 )
      {
        auto  chnext = *thedic++;
        auto  sublen = getserial( thedic );
        auto  subdic = thedic;  thedic += sublen;
        int   nerror;

        if ( dicpos == nullptr || (dicpos >= subdic && dicpos <= thedic) )
        {
          ptrack[ltrack] = chnext;

          if ( (nerror = GetTrack<aflags, action, result>( doitem, subdic, ptrack, ltrack + 1, dicpos )) != 0 )
            return nerror;
        }
      }
      return counter<aflags>::hasupper( uflags ) ? doitem( thedic, { ptrack, ltrack } ) : (result)0;
    }

  };

  // Flat::ScanTree

  template <class aflags, class action, class result>
  result  Flat::ScanTree( const action& doitem, const byte_t* thedic, const fragment& src )
  {
    scan_stack<aflags>  astack[0x40];     // never longer words
    scan_stack<aflags>* pstack;
    result              retval;

    for ( (pstack = astack)->setlevel( thedic, src ); pstack >= astack; )
    {
      auto  subdic = pstack->findchar();

      // check if not found or no more data
      if ( subdic == nullptr )
      {
        if ( counter<aflags>::hasupper( pstack->aflags ) )
        {
          if ( (retval = doitem( pstack->thedic, pstack->thestr )) != (result)0 )
            return retval;
        }
        --pstack;
      }
        else
      pstack = (pstack + 1)->setlevel( subdic, pstack->thestr.next() );
    }
    return (result)0;
  }

  // Flat::ScanList

  template <class collector>
  class  Flat::LookupList
  {
    const collector&  output;

  public:     // initialization
    LookupList( const collector& a ): output( a ) {}

  public:     // scaner
    int   operator ()( const uint8_t* pstems, const fragment& thestr ) const
    {
      auto  ucount = getserial( pstems );

      while ( ucount-- > 0 )
      {
        auto  chrmin = *pstems++;
        auto  chrmax = *pstems++;
        auto  nlexid = getserial( pstems );
        auto  oclass = getword16( pstems );
        auto  suffix = fragment{ nullptr, 0 };
        auto  ccflex = thestr.len;
        int   nerror;

      // check postfix
        if ( (oclass & wfPostSt) != 0 )
        {
          pstems += (suffix = { pstems + 1, *pstems }).len + 1;

          if ( suffix.len > ccflex )
            continue;

          if ( memcmp( thestr.end() - suffix.len, suffix.str, suffix.len ) != 0 )
            continue;

          ccflex -= suffix.len;
        }

      // оценить, может ли хотя бы потенциально такое окончание быть у основ начиная с этой и далее
        if ( ccflex > 0 )
        {
          if ( *thestr.str > chrmax ) break;
          if ( *thestr.str < chrmin ) continue;
        }

      // check capitalization scheme
        if ( (nerror = output( nlexid, oclass & 0x7fff, fragment{ thestr.str, ccflex }, suffix )) != 0 )
          return nerror;
      }
      return 0;
    }
  };

  // Flat::SelectView

  template <class collector>
  class  Flat::SelectView
  {
    const collector&  output;
    const uint8_t*    dicpos;

  public:     // initialization
    SelectView( const collector& a, const uint8_t* p ):
      output( a ), dicpos( p )  {}

  public:     // scaner
    int   operator () ( const uint8_t* pstems, const fragment& thestr ) const
    {
      auto  ucount = getserial( pstems );

      while ( ucount-- > 0 )
      {
        auto  thepos = pstems;
        auto  nlexid = getserial( pstems += 2 );
        auto  oclass = getword16( pstems );
        auto  suffix = fragment{};

      // check postfix
        if ( (oclass & wfPostSt) != 0 ) pstems += (suffix = { pstems + 1, *pstems }).len + 1;
          else suffix = { nullptr, 0 };

      // check match
        if ( thepos == dicpos )
          return output( nlexid, oclass & ~wfPostSt, thestr, suffix );
      }

      return 0;
    }
  };

}  // end namespace

# endif  // !__libmorph_scandict_h__
