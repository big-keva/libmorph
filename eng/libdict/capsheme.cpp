/******************************************************************************

    libmorpheng - dictionary-based morphological analyser for English.

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
# include <assert.h>

namespace __libmorpheng__
{
  # define SHEME_UNDEFINED 0     // 
  # define SHEME_ALL_SMALL 1     // 
  # define SHEME_FIRST_CAP 2     // 
  # define SHEME_WORD_CAPS 3     // 
  # define SHEME_FIRST_WAS 4     // 
  # define SHEME_ERROR_CAP 5     // 
  # define SHEME_OVERFLOW  6

  # define CT_CAPITAL  0
  # define CT_REGULAR  1
  # define CT_DLMCHAR  2         // 
  # define CT_INVALID  3

  static unsigned char capMatrix[6][4] =
  {
    { SHEME_FIRST_WAS, SHEME_ALL_SMALL, SHEME_ERROR_CAP, SHEME_ERROR_CAP },
    { SHEME_ERROR_CAP, SHEME_ALL_SMALL, SHEME_ALL_SMALL, SHEME_ERROR_CAP },
    { SHEME_ERROR_CAP, SHEME_FIRST_CAP, SHEME_FIRST_CAP, SHEME_ERROR_CAP },
    { SHEME_WORD_CAPS, SHEME_ERROR_CAP, SHEME_WORD_CAPS, SHEME_ERROR_CAP },
    { SHEME_WORD_CAPS, SHEME_FIRST_CAP, SHEME_WORD_CAPS, SHEME_ERROR_CAP },
    { SHEME_ERROR_CAP, SHEME_ERROR_CAP, SHEME_ERROR_CAP, SHEME_ERROR_CAP }
  };

  static unsigned char charTypeMatrix[256] =
  {
  // Characters in range 0x00 - 0x1f
    CT_DLMCHAR, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
  // Space till plus, characters in range 0x20 - 0x3f
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_REGULAR,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_REGULAR,         // Comma is a regular char
    CT_DLMCHAR,         // Minus - a defis - is a delimiter
                            CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
  // Characters in range 0x40 - 0x5f
    CT_INVALID, CT_CAPITAL, CT_CAPITAL, CT_CAPITAL,
    CT_CAPITAL, CT_CAPITAL, CT_CAPITAL, CT_CAPITAL,
    CT_CAPITAL, CT_CAPITAL, CT_CAPITAL, CT_CAPITAL,
    CT_CAPITAL, CT_CAPITAL, CT_CAPITAL, CT_CAPITAL,
    CT_CAPITAL, CT_CAPITAL, CT_CAPITAL, CT_CAPITAL,
    CT_CAPITAL, CT_CAPITAL, CT_CAPITAL, CT_CAPITAL,
    CT_CAPITAL, CT_CAPITAL, CT_CAPITAL, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
  // Characters in range 0x60 - 0x7f
    CT_INVALID, CT_REGULAR, CT_REGULAR, CT_REGULAR,
    CT_REGULAR, CT_REGULAR, CT_REGULAR, CT_REGULAR,
    CT_REGULAR, CT_REGULAR, CT_REGULAR, CT_REGULAR,
    CT_REGULAR, CT_REGULAR, CT_REGULAR, CT_REGULAR,
    CT_REGULAR, CT_REGULAR, CT_REGULAR, CT_REGULAR,
    CT_REGULAR, CT_REGULAR, CT_REGULAR, CT_REGULAR,
    CT_REGULAR, CT_REGULAR, CT_REGULAR, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
  // Characters in range 0x80 - 0xff
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID
  };

  unsigned char toUpCaseMatrix[256] =
  {
  /* Characters in range 0 - 31       */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
  /* Space till plus, characters in range 32 - 63 */
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
  /* Characters in range 64 - 127     */
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
    0x60, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x5A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
  /* Characters in range 0x80 - 0x9F  */
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
    0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
  /* Characters in range 0xA0 - 0xA7  */
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
    0xB8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
    0xB0, 0xB1, 0xB2, 0xB2, 0xA5, 0xB5, 0xB6, 0xB7,
    0xA8, 0xB9, 0xAA, 0xBB, 0xBC, 0xBD, 0xBE, 0xAF,
  /* Characters in range 0xC0 - 0xDF, cyrillic capitals */
    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
    0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
    0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
  /* Regular characters in range 0xC0 - 0xDF  */
    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
    0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
    0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF
  };

  unsigned char toLoCaseMatrix[256] =
  {
  // Characters in range 0 - 31
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
  // Space till plus, characters in range 32 - 63
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
  // Characters in range 64 - 127
    0x40, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
  // Characters in range 0x80 - 0x9F
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
    0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
  // Characters in range 0xA0 - 0xA7
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
    0xB8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7,
    0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
  // Characters in range 0xC0 - 0xDF, cyrillic capitals
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
    0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
    0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF,
  // Regular characters in range 0xE0 - 0xFF
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
    0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
    0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
  };

  //=====================================================================
  // 
  // 
  //=====================================================================
  static unsigned GetFragmentCapScheme( const char* lpWord,
                                        char*       lpDest,
                                        unsigned    cbBuff,
                                        unsigned&   cbText )
  {
    unsigned  bSheme = SHEME_UNDEFINED;
    unsigned  chType;

    cbText = 0;

  // 
    while ( (chType = charTypeMatrix[(unsigned char)lpWord[0]]) != CT_DLMCHAR
      && cbText < cbBuff )
    {
    // 
      lpDest[0] = (char)toLoCaseMatrix[(unsigned char)lpWord[0]];

    // 
      bSheme = capMatrix[bSheme][chType];

    // 
      lpWord++;
      lpDest++;
      cbText++;
    }
    bSheme = capMatrix[bSheme][CT_DLMCHAR];
  // 
    return ( cbText < cbBuff ? bSheme : SHEME_OVERFLOW );
  }

  //=====================================================================
  // 
  // 
  //=====================================================================
  unsigned  GetCapScheme( const char* lpWord,
                          char*       lpDest,
                          unsigned    cbBuff,
                          unsigned&   scheme )
  {
    bool      fValid = true;
    unsigned  cbText = 0;
    unsigned  cDefis = 0;
    unsigned  shemes = 0;
    unsigned  cfSize;
    unsigned  bSheme;

    while ( lpWord[cbText] != '\0' && cbText < cbBuff )
    {
    // 
      bSheme = GetFragmentCapScheme( lpWord + cbText, lpDest + cbText,
        cbBuff - cbText, cfSize );

    // 
      fValid &= bSheme != SHEME_ERROR_CAP && ( lpWord[cbText + cfSize] == '\0'
        || lpWord[cbText + cfSize] == '-' );

    // 
      if ( bSheme == SHEME_OVERFLOW )
        return (unsigned)-1;

    // 
      if ( fValid ) shemes |= (bSheme - 1) << (cDefis++ << 1);

    // 
      cbText += cfSize;

    // 
      if ( lpWord[cbText] == '-' )
      {
        lpDest[cbText] = '-';
        cbText++;
      }
        else
      fValid &= ( lpWord[cbText] == '\0' );
    }
  // 
    lpDest[cbText] = 0;
    scheme = ( fValid ? shemes | (cDefis << 8) : 0 );
    return ( cbText < cbBuff ? cbText : (unsigned)-1 );
  }

  //=====================================================================
  // 
  // 
  // 
  //=====================================================================
  void  SetCapScheme( char*     lpWord,
                      unsigned  scheme )
  {
    unsigned  count = scheme >> 8;

    while ( count > 0 && lpWord[0] != '\0' )
    {
    // 
      unsigned  local = scheme & 0x03;

    // 
      while ( *lpWord != '\0' && *lpWord != '-' )
      {
        if ( local > 0 )
          *lpWord = (char)toUpCaseMatrix[(unsigned char)*lpWord];
        if ( local == 1 )
          local = 0;
        lpWord++;
      }

    // 
      if ( *lpWord == '-' )
        lpWord++;

    // 
      scheme >>= 2;
    }
  }

  unsigned  GetMinScheme( unsigned    nparts,
                          unsigned    minCap,
                          const char* lpword )
  {
    assert( minCap == 0 || minCap == 1 || minCap == 2 );

  // 
    if ( nparts == 0 )
      for ( nparts = 1; *lpword != '\0'; lpword++ )
        if ( *lpword == '-' )
          ++nparts;

  // 
    switch ( nparts )
    {
      case 1:
        return 0x0100 | minCap;
      case 2:
        return 0x0200 | minCap | (minCap << 2);
      case 3:
        return 0x0300 | minCap | (minCap << 4) | ((minCap & 0x02) << 2);
      default:
        return 0x0100;
    }
  }

  void  SetLowerCase( unsigned char* lpWord )
  {
    for ( ;; )
    {
      if ( (lpWord[0] = toLoCaseMatrix[lpWord[0]]) == '\0' )
        break;
      ++lpWord;
    }
  }

} // end __libmorpheng__ namespace

