/******************************************************************************

    libmorph - morphological analysers.

    Copyright (C) 1994-2025 Andrew Kovalenko aka Keva

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

    Commercial license is available upon request.

    Contacts:
      email: keva@meta.ua, keva@rambler.ru
      Phone: +7(495)648-4058, +7(926)513-2991, +7(707)129-1418

******************************************************************************/
# if !defined( __libmorph_typedefs_h__ )
# define __libmorph_typedefs_h__
# include <algorithm>
# include <cstdint>
# include <cstddef>

namespace libmorph {

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
    const uint8_t*  str;
    size_t          len;

  public:
    auto  begin() const -> const uint8_t*  {  return str;  }
    auto  end() const -> const uint8_t*  {  return str + len;  }

    bool  empty() const {  return len == 0 || str == nullptr;  }

    auto  size() const -> size_t  {  return len;  }

    auto  operator ++() -> fragment&  {  return ++str, --len, *this;  }

    auto  front() const -> uint8_t {  return *str;  }

    auto  next() const -> fragment {  return { str + 1, len - 1 };  }
    auto  left( size_t n ) -> fragment  {  return { str, std::min( len, n ) };  }
    auto  right( size_t n ) -> fragment  {  return { str + len - std::min( len, n ), std::min( len, n ) };  }
  };

  struct flexinfo
  {
    uint16_t  gramm;
    uint8_t   flags;

  public:
    bool  operator == ( const flexinfo& f ) const {  return gramm == f.gramm && flags == f.flags;  }
    bool  operator != ( const flexinfo& f ) const {  return !(*this == f);  }
    
  };

  template <class T, size_t N>
  constexpr size_t  array_len( T (&)[N] )
  {
    return (size_t)N;
  }

  inline  unsigned  getserial( const uint8_t*& p )
  {
    uint8_t   bfetch = *p++;
    unsigned  serial = bfetch & ~0x80;
    int       nshift = 1;

    while ( (bfetch & 0x80) != 0 )
      serial |= (((unsigned)(bfetch = *p++) & ~0x80) << (nshift++ * 7));
    return serial;
  }

  inline  uint16_t  getword16( const uint8_t*& p )
  {
    uint8_t   blower = *p++;
    uint16_t  bupper = *p++;

    return blower | (bupper << 8);
  }

  inline  size_t    lexkeylen( uint8_t* p, unsigned nlexid )
  {
    auto  o( p );

    if ( (nlexid & ~0x000000ff) == 0 )  { *p++ = (uint8_t)nlexid;  }
      else
    if ( (nlexid & ~0x0000ffff) == 0 )  { *p++ = (uint8_t)(nlexid >> 8); *p++ = (uint8_t)nlexid;  }
      else
    if ( (nlexid & ~0x00ffffff) == 0 )  { *p++ = (uint8_t)(nlexid >> 16);  *p++ = (uint8_t)(nlexid >> 8); *p++ = (uint8_t)nlexid;  }
      else
    {  *p++ = (uint8_t)(nlexid >> 24);  *p++ = (uint8_t)(nlexid >> 16);  *p++ = (uint8_t)(nlexid >> 8); *p++ = (uint8_t)nlexid;  }

    return p - o;
  }

} // end namespace

# endif  // !__libmorph_typedefs_h__
