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
# if !defined( __libmorph_charlist_h__ )
# define __libmorph_charlist_h__

namespace libmorph {

  class Charset
  {
    enum: size_t
    {
      element_bits = sizeof(uint32_t) * CHAR_BIT,
      charset_size = 0x100 / element_bits
    };
    uint32_t  uchars[charset_size];

  public:
    Charset()
      {  memset( uchars, 0, sizeof(uchars) );  }

  public:
    void  operator()( uint8_t c )
      {  uchars[c / element_bits] |= 1 << (c % element_bits);  }
    void  operator()( char c )
      {  return (*this)( uint8_t(c) );  }
    template <class Collector>
    int   operator()( Collector& out ) const
    {
      int   ncount = 0;

      for ( size_t o = 0; o != charset_size; ++o )
        if ( uchars[o] != 0 )
        {
          for ( size_t b = 0; b != element_bits; ++b )
            if ( (uchars[o] & (1 << b)) != 0 )
            {
              if ( out.append( char(o * element_bits + b) ) ) ++ncount;
                else return -1;
            }
        }
      return ncount;
    }
  };

}

# endif // !__libmorph_charlist_h__
