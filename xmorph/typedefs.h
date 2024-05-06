# if !defined( __typedefs_h__ )
# define __typedefs_h__
# include <cstddef>

# if defined( LIBMORPH_NAMESPACE )
namespace LIBMORPH_NAMESPACE {
# endif  // LIBMORPH_NAMESPACE

// Define common types used in the analyser
# if !defined( __byte_t_defined__ )
#   define  __byte_t_defined__
    typedef unsigned char   byte_t;
# endif  // !__byte_t_defined__

# if !defined( __word16_t_defined__ )
#   define __word16_t_defined__
    typedef unsigned short  word16_t;
# endif  // !__word16_t_defined__

# if !defined( __word32_t_defined__ )
#   define __word32_t_defined__
    typedef unsigned int    word32_t;
# endif  // !__word32_t_defined__

  struct fragment
  {
    const byte_t* str;
    size_t        len;

  public:
    auto  begin() const -> const byte_t*  {  return str;  }
    auto  end() const -> const byte_t*  {  return str + len;  }

    bool  empty() const {  return len == 0 || str == nullptr;  }

    auto  next() const -> fragment {  return { str + 1, len - 1 };  }
    auto  size() const -> size_t  {  return len;  }

    auto  operator ++() -> fragment&  {  return ++str, --len, *this;  }

    auto  front() const -> byte_t {  return *str;  }
  };

  struct flexinfo
  {
    word16_t  gramm;
    byte_t    flags;
  };

  template <class T, size_t N>
  constexpr size_t  array_len( T (&)[N] )
  {
    return (size_t)N;
  }

  inline  unsigned  getserial( const byte_t*& p )
  {
    byte_t    bfetch = *p++;
    unsigned  serial = bfetch & ~0x80;
    int       nshift = 1;

    while ( (bfetch & 0x80) != 0 )
      serial |= (((unsigned)(bfetch = *p++) & ~0x80) << (nshift++ * 7));
    return serial;
  }

  inline  word16_t  getword16( const byte_t*& p )
  {
    byte_t    blower = *p++;
    word16_t  bupper = *p++;

    return blower | (bupper << 8);
  }

  inline  size_t    lexkeylen( byte_t* p, unsigned nlexid )
  {
    byte_t* o = p;

    if ( (nlexid & ~0x000000ff) == 0 )  { *p++ = (byte_t)nlexid;  }
      else
    if ( (nlexid & ~0x0000ffff) == 0 )  { *p++ = (byte_t)(nlexid >> 8); *p++ = (byte_t)nlexid;  }
      else
    if ( (nlexid & ~0x00ffffff) == 0 )  { *p++ = (byte_t)(nlexid >> 16);  *p++ = (byte_t)(nlexid >> 8); *p++ = (byte_t)nlexid;  }
      else
    {  *p++ = (byte_t)(nlexid >> 24);  *p++ = (byte_t)(nlexid >> 16);  *p++ = (byte_t)(nlexid >> 8); *p++ = (byte_t)nlexid;  }

    return p - o;
  }

# if defined( LIBMORPH_NAMESPACE )
}
# endif  // LIBMORPH_NAMESPACE

# endif  // __typedefs_h__
