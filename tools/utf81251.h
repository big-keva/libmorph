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
# pragma once
# if !defined( __utf8to1251_h__ )
# define __utf8to1251_h__
# include "moonycode/codes.h"
# include <string>

inline  std::string utf8to1251( const char* s )
{
  size_t      utflen = codepages::utf8::strlen( s );
  std::string winstr( utflen + 1, ' ' );

  winstr.resize( codepages::mbcstombcs(
                 codepages::codepage_1251, (char*)winstr.c_str(), winstr.size(),
                 codepages::codepage_utf8, s ) );

  return winstr;
}

inline  std::string toCodepage( unsigned cp, const char* s )
{
  std::string str;

  if ( cp == codepages::codepage_utf8 ) str.resize( strlen( s ) * 6 );
    else str.resize( strlen( s ) );

  codepages::mbcstombcs( cp, (char*)str.c_str(), str.length(), codepages::codepage_1251, s );

  return (str.shrink_to_fit(), str);
}

# endif   // __utf8to1251_h__
