# pragma once
# if !defined( __plaintable_h__ )
# define __plaintable_h__
# include "serialize.h"
# include <cstddef>

namespace libmorph
{
  auto  CreatePlainTable( const char* tables, size_t offset ) -> std::vector<char>;
}  // libmorph namespace

namespace libmorph  {
namespace plaintab  {

//
// grammatical structure && vector<graminfo> manipulation
//
  struct graminfo
  {
    uint16_t  grinfo;
    uint8_t   bflags;

  public:
    graminfo( uint16_t g = 0, uint8_t b = 0 ): grinfo( g ), bflags( b ) {}

  public:
    bool  operator == ( const graminfo& r ) const {  return grinfo == r.grinfo && bflags == r.bflags;  }
    bool  operator != ( const graminfo& r ) const {  return !(*this == r);  }

  public:
    auto  GetBufLen() const -> size_t
      {  return 3;  }
    template <class O>
    O*    Serialize( O* o ) const
      {  return ::Serialize( ::Serialize( o, &grinfo, sizeof(grinfo) ), &bflags, sizeof(bflags) );  }
  };

  void  Insert( std::vector<graminfo>& l, const graminfo& g )
    {
      if ( std::find( l.begin(), l.end(), g ) == l.end() )
        l.push_back( g );
    }

  inline  word16_t  getword16( const byte_t*& p )
  {
    byte_t    blower = *p++;
    word16_t  bupper = *p++;

    return blower | (bupper << 8);
  }

}}


# include "wordtree.h"

namespace libmorph  {
namespace plaintab  {

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

        return serial;
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
          auto      grinfo = getword16( ftable );
          auto      szflex = ftable;
          auto      ccflex = *szflex++;
          unsigned  ofnext;
    
          ftable = szflex + ccflex;
    
          if ( (bflags & 0xc0) != 0 ) ofnext = getword16( ftable );
            else ofnext = 0;

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

}}

namespace libmorph {

  std::vector<char> CreatePlainTable( const char* tables, size_t offset )
  {
    return plaintab::FlexTree( tables )( offset );
  }

}

# endif // __plaintable_h__
