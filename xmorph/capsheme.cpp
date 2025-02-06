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
# include "capsheme.h"
# include <cstdint>

namespace libmorph
{
  const unsigned char (&CapScheme::capStateMatrix)[6][4] =
  {
    { first_was, all_small, error_cap, error_cap },
    { error_cap, all_small, all_small, error_cap },
    { error_cap, first_cap, first_cap, error_cap },
    { word_caps, error_cap, word_caps, error_cap },
    { word_caps, first_cap, word_caps, error_cap },
    { error_cap, error_cap, error_cap, error_cap }
  };

  //=====================================================================
  // Функция вычисляет схему капитализации всего слова, его длину и
  // создает образ слова, переведенный в нижний регистр
  //=====================================================================
  auto  CapScheme::Get(
    unsigned char* output, size_t outlen,
    const char*    srctop, size_t srclen ) const -> unsigned
  {
    unsigned char*  outend = output + outlen;
    const char*     srcend;
    unsigned        scheme;
    int             cdefis;
    bool            bvalid;

    if ( srclen == (size_t)-1 ) for ( srcend = srctop; *srcend != '\0'; ++srcend ) (void)0;
      else srcend = srctop + srclen;

    for ( cdefis = 0, scheme = 0, bvalid = true; srctop < srcend && output < outend; )
    {
      unsigned  subcap = undefined;
      unsigned  chtype;

      while ( srctop < srcend && output < outend && (chtype = charTypeMatrix[(uint8_t)*srctop]) != CT_DLMCHAR )
      {
        subcap = capStateMatrix[subcap][chtype];
          *output++ = toLoCaseMatrix[(unsigned char)*srctop++];
      }

      if ( output >= outend )
        return (unsigned)-1;

      bvalid &= ((subcap = capStateMatrix[subcap][CT_DLMCHAR]) != error_cap);
        scheme |= (subcap - 1) << (cdefis << 1);

      if ( srctop != srcend )
      {
        if ( output >= outend )
          return (unsigned)-1;

        if ( charTypeMatrix[uint8_t( *output++ = *srctop++ )] == CT_DLMCHAR )  ++cdefis;
          else return (unsigned)-1;
      }
    }

    if ( output < outend ) *output = '\0';
      else return (unsigned)-1;

    return (((unsigned)(output + outlen - outend)) << 16) | ((cdefis + 1) << 8) | (bvalid ? scheme : 0xff);
  }

  //=====================================================================
  // Функция приводит слово к нужной схеме капитализации, т. е. к той,
  // которая передана в качестве cSheme. Предполагается, что слово подано
  // в нижнем регистре, так как именно таким образом организован словарь.
  //=====================================================================
  auto  CapScheme::Set( unsigned char* str, size_t len, uint8_t min ) const -> unsigned char*
  {
    auto  strorg = str;
    auto  scheme = uint16_t{};
    int   nparts = 1;

  // first check length and get part count
    if ( len == (size_t)-1 )
    {
      for ( len = 0; str[len] != 0; ++len )
        nparts += charTypeMatrix[str[len]] == CT_DLMCHAR;
    }
      else
    {
      for ( auto beg = str, end = beg + len; beg != end; ++beg )
        nparts += charTypeMatrix[*beg] == CT_DLMCHAR;
    }

  // Определить собственно схему капитализации
    switch ( nparts )
    {
      case 1:
        scheme = min;
        break;
      case 2:
        scheme = min | (min << 2);
        break;
      case 3:
        scheme = min | (min << 4) | ((min & 0x02) << 2);
        break;
      default:
        scheme = 0;
    }

  // приложить схему капитализации к строке
    for ( ; nparts-- > 0 && *str != '\0'; scheme >>= 2 )
    {
      unsigned  strcap = scheme & 0x03;   // Возможные значения схемы капитализации - 0 (a), 1 (Aa) или 2 (AA)

      for ( ; *str != '\0' && charTypeMatrix[*str] != CT_DLMCHAR; strcap &= ~0x01, ++str )
        *str = strcap ? toUpCaseMatrix[*str] : *str;

      str += (charTypeMatrix[*str] == CT_DLMCHAR ? 1 : 0);
    }
    return strorg;
  }

} // end lubmorph namespace
