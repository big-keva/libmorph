/******************************************************************************

    libmorph - morphological analysers.

    Copyright (C) 1994-2016 Andrew Kovalenko aka Keva

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
# if !defined( __libmorph_buildforms_hpp__ )
# define __libmorph_buildforms_hpp__
# include "xmorph/scandict.h"
# include <cstdint>

namespace libmorph {

  class Collector
  {
    char*   outbeg = nullptr;
    char*   outptr = nullptr;
    char*   outend = nullptr;

  public:
    Collector( char* beg, char* end ):
      outbeg( beg ),
      outptr( beg ),
      outend( end ) {}
    Collector( char* beg, size_t len ):
      Collector( beg, beg + len ) {}

    auto  get() -> Collector& {  return *this;  }

  public:
    auto  getptr() const -> const char*
      {  return outptr;  }
    bool  append( char c )
      {
        return outptr != outend ? (*outptr++ = c), true : false;
      }
    bool  append( const char* p, size_t l )
      {
        if ( l + outptr > outend )
          return false;
        while ( l-- > 0 )
          *outptr++ = *p++;
        return true;
      }

    bool  operator == ( nullptr_t ) const {  return outbeg == nullptr;  }
    bool  operator != ( nullptr_t ) const {  return !(*this == nullptr);  }
  };

  inline  auto  MakeCollector( char* beg, char* end ) -> Collector
    {  return Collector( beg, end );  }

  inline  auto  MakeCollector( char* beg, size_t len ) -> Collector
    {  return Collector( beg, beg + len );  }

  template <size_t N>
  auto  MakeCollector( char (&vec)[N] ) -> Collector
    {  return Collector( vec, N );  }

 /*
  * GetFlexForms( ... )
  * Формирует массив форм слова, добавляя окончания к переданному префиксу.
  * Тиражирует префикс на нужное количество форм по мере их построения.
  * Возвращает количество построенных форм слова или -1 при переполнении
  * аккумулятора.
  */
  int   GetFlexForms(
    Collector&        output,
    const uint8_t*    ptable,
    const flexinfo&   fxinfo,
    const fragment&   prefix,
    const fragment&   suffix = {} );

}

# endif // !__libmorph_buildforms_hpp__
