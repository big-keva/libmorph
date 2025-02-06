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
# include "buildforms.hpp"
# include <cstdint>

namespace libmorph {

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
    const fragment&   suffix )
 {
    uint8_t atrack[0x40];
    int     ntails = 0;
    auto    rtrack = Flat::GetTrack<uint8_t>( [&]( const uint8_t* tabptr, const fragment& szflex )
    {
      for ( auto ngrams = *tabptr++; ngrams-- > 0; )
      {
        auto  fxnext = flexinfo{ getword16( tabptr ), *tabptr++ };

        if ( fxnext.gramm == fxinfo.gramm && (fxnext.flags & fxinfo.flags) != 0 )
        {
          return output.append( (const char*)prefix.str, prefix.len )
              && output.append( (const char*)szflex.str, szflex.len )
              && output.append( (const char*)suffix.str, suffix.len )
              && output.append( '\0' ) ? ++ntails, 0 : -1;
        }
      }
      return 0;
    }, ptable, atrack, 0, nullptr );

    return rtrack == 0 ? ntails : rtrack;
 }

}
