# include "plaintable.h"
# include <cstdint>
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
    bool  operator != ( const graminfo& r ) const {  return !(*this == r);  }
  };

  void  Insert( std::vector<graminfo>& l, const graminfo& g )
    {
      if ( std::find( l.begin(), l.end(), g ) == l.end() )
        l.push_back( g );
    }

}

constexpr inline
size_t  GetBufLen( const libmorph::graminfo& );
template <class O> inline
O*      Serialize( O*, const libmorph::graminfo& );

# include "wordtree.h"

constexpr inline
size_t  GetBufLen( const libmorph::graminfo& ) {  return 3;  }

template <class O> inline
O*      Serialize( O* o, const libmorph::graminfo& gl )
{
  return ::Serialize( ::Serialize( o, &gl.grinfo, sizeof(gl.grinfo) ), &gl.bflags, sizeof(gl.bflags) );
}

namespace libmorph
{

  class FlexTree
  {
    using gramlist = std::vector<graminfo>;

  public:     // construction
    FlexTree( const void* p_tables ): tables( (const uint8_t*)p_tables )  {}

  public:     // generator
    std::vector<char> operator () ( unsigned tfoffs ) const
      {
        wordtree<gramlist>  inflex;
        std::vector<char>   serial;
        char                szflex[0x100];

        CreateTree( inflex, tfoffs, 0, 0xff, szflex, 0 );

        serial.resize( inflex.GetBufLen() );
        inflex.Serialize( serial.data() );

        return std::move( serial );
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
            Insert( *wotree.Insert( prefix, ccpref + ccflex ), graminfo( grInfo | grinfo, bFlags & bflags ) );

          if ( ofnext != 0 )
            CreateTree( wotree, ofnext, grInfo | grinfo, bFlags & bflags, prefix, ccpref + ccflex );
        }
      }

  protected:
    const uint8_t*  tables;     // global flex tables pointer

  };

  std::vector<char> CreatePlainTable( const char* tables, size_t offset )
  {
    return FlexTree( tables )( offset );
  }

}  // libmorph namespace
