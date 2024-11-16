# if !defined( __libmorph_rus_codepages_hpp__ )
# define __libmorph_rus_codepages_hpp__
# include "moonycode/codes.h"
# include <cstddef>

namespace libmorph {
namespace rus {

  class MbcsCoder
  {
    const unsigned  encode;
    char*           outbeg = nullptr;
    char*           outptr = nullptr;
    char*           outend = nullptr;

  public:
    MbcsCoder( char* beg, char* end, unsigned enc ):
      encode( enc ),
      outbeg( beg ),
      outptr( beg ),
      outend( end ) {}
    MbcsCoder( char* beg, size_t len, unsigned enc ):
      MbcsCoder( beg, beg + len, enc ) {}
    template <size_t N>
    MbcsCoder( char (&buf)[N], unsigned enc ):
      MbcsCoder( buf, N, enc )  {}

    auto  object() -> MbcsCoder&  {  return *this;  }

  public:
    auto  size() const -> size_t  {  return outptr - outbeg;  }

    bool  operator == ( nullptr_t ) const {  return outbeg == nullptr;  }
    bool  operator != ( nullptr_t ) const {  return !(*this == nullptr);  }

  public:
    bool  append( char c )  {  return append( &c, 1 );  }
    bool  append( const char*, size_t );
    auto  getptr() const -> const char* {  return outptr;  }
  };

  inline  size_t  ToWidechar( widechar* out, size_t cch, const char* str, size_t len = (size_t)-1 )
    {  return codepages::mbcstowide( codepages::codepage_1251, out, cch, str, len );  }

  template <size_t N>
  inline  size_t  ToWidechar( widechar (&out)[N], const char* str, size_t len )
    {  return ToWidechar( out, N, str, len );  }

  inline  size_t  ToInternal( char* out, size_t cch, const widechar* str, size_t len )
    {  return codepages::widetombcs( codepages::codepage_1251, out, cch, str, len );  }

  template <size_t N>
  inline  size_t  ToInternal( char (&out)[N], const widechar* str, size_t len )
    {  return ToInternal( out, N, str, len );  }

  inline  size_t  ToCodepage(
    unsigned  dst_cp,       char* out, size_t cch,
    unsigned  src_cp, const char* str, size_t len = (size_t)-1 )
  {
    if ( dst_cp != src_cp )
      return codepages::mbcstombcs( dst_cp, out, cch, src_cp, str, len );

    if ( len == (size_t)-1 )
      for ( len = 0; str[len] != 0; ++len ) (void)NULL;

    if ( len >= cch )
      return (size_t)-1;

    memcpy( out, str, len );
      return out[len] = '\0', len;
  }

  inline  size_t  ToInternal( char* out, size_t cch, unsigned cps, const char* str, size_t len = (size_t)-1 )
    {  return ToCodepage( codepages::codepage_1251, out, cch, cps, str, len );  }

  template <size_t N>
  inline  size_t  ToInternal( char (&out)[N], unsigned cps, const char* str, size_t len = (size_t)-1 )
    {  return ToCodepage( codepages::codepage_1251, out, N, cps, str, len );  }

  inline  size_t  ToCodepage( unsigned cps, char* out, size_t cch, const char* str, size_t len = (size_t)-1 )
    {  return ToCodepage( cps, out, cch, codepages::codepage_1251, str, len );  }

  template <size_t N>
  inline  size_t  ToCodepage( unsigned cps, char (&out)[N], const char* str, size_t len = (size_t)-1 )
    {  return ToCodepage( cps, out, N, codepages::codepage_1251, str, len );  }

// MbcsCoder implementation

  inline  bool  MbcsCoder::append( const char* p, size_t l )
  {
    auto  enclen = ToCodepage( encode, outptr, outend - outptr, p, l );

    return enclen != (size_t)-1 ? (outptr += enclen), true : false;
  }

}}

# endif // __libmorph_rus_codepages_hpp__
