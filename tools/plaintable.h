# if !defined( __plaintable_h__ )
#	define	__plaintable_h__
# include "wordtree.h"
# include <algorithm>
# include <vector>

namespace libmorph
{

  struct graminfo
  {
    uint16_t  grinfo;
    uint8_t   bflags;

  public:
    graminfo( uint16_t g = 0, uint8_t b = 0 ): grinfo( g ), bflags( b ) {}

  public:
    bool  operator == ( const graminfo& r ) const {  return grinfo == r.grinfo && bflags == r.bflags;  }
  };

  struct gramlist: public std::vector<graminfo>
  {
    void  Insert( const graminfo& g )
      {
        if ( std::find( begin(), end(), g ) == end() )
          push_back( g );
      }
  };
}

inline  size_t  GetBufLen( const libmorph::gramlist& gl )
{
  return 1 + gl.size() * 3;
}

template <class O>
inline  O*      Serialize( O* o, const libmorph::gramlist& gl )
{
  o = ::Serialize( o, (char)gl.size() );

  for ( auto p = gl.begin(); o != nullptr && p != gl.end(); ++p )
    o = ::Serialize( ::Serialize( o, &p->grinfo, sizeof(p->grinfo) ), &p->bflags, sizeof(p->bflags) );

  return o;
}

namespace libmorph
{

  class FlexTree
  {
    const uint8_t*  tables;     // global flex tables pointer

  public:     // construction
    FlexTree( const void* p_tables ): tables( (const uint8_t*)p_tables )  {}

  public:     // generator
    wordtree<gramlist>  operator () ( unsigned tfoffs ) const
      {
        wordtree<gramlist>  inflex;
        char                szflex[0x100];

        CreateTree( inflex, tfoffs, 0, 0xff, szflex, 0 );

        return std::move( inflex );
      }

  protected:
    void  CreateTree( wordtree<gramlist>& wotree, unsigned  tfoffs,
                      uint16_t            grInfo, uint8_t   bFlags,
                      char*               prefix, size_t    ccpref ) const
      {
        auto  ftable = (tfoffs << 1) + tables;

        for ( auto nitems = *ftable++; nitems-- > 0; )
        {
          auto      bflags = *ftable++;
          auto      grinfo = *(uint16_t*)ftable;  ftable += sizeof(uint16_t);
          auto      szflex = ftable;
          auto      ccflex = *szflex++;
          unsigned  ofnext;
    
          ftable = szflex + ccflex;
    
          if ( (bflags & 0xc0) != 0 )
          {
            ofnext = *(uint16_t*)ftable;
            ftable += sizeof(uint16_t);
          }
            else
          ofnext = 0;

          memcpy( prefix + ccpref, szflex, ccflex );

          if ( (bflags & 0x80) == 0 )
            wotree.Insert( prefix, ccpref + ccflex )->Insert( graminfo( grInfo | grinfo, bFlags & bflags ) );

          if ( ofnext != 0 )
            CreateTree( wotree, ofnext, grInfo | grinfo, bFlags & bflags, prefix, ccpref + ccflex );
        }
      }
  };

}  // libmorph namespace

# endif // __plaintable_h__
