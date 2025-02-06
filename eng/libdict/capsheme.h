/******************************************************************************

    libmorpheng - dictionary-based morphological analyser for English.

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
# if !defined( _capsheme_h_ )
# define _capsheme_h_

// For better compatibility and neighbour usage of different morphological
// modules in one UNIX application all the objects are packed to the
// single namespace named __libmorphrus__

namespace __libmorpheng__
{
  //=====================================================================
  // 
  // 
  // 
  // 00 
  // 
  // 
  //=====================================================================

  unsigned  GetCapScheme( const char*     lpWord,
                          char*           lpDest,
                          unsigned        cbBuff,
                          unsigned&       scheme );
  void      SetCapScheme( char*           lpWord,
                          unsigned        scheme );
  unsigned  GetMinScheme( unsigned        nParts,
                          unsigned        minCap,
                          const char*     lpWord = 0 );
  void      SetLowerCase( unsigned char*  lpWord );

  inline  unsigned  GetWordCaps( unsigned short wdInfo )
  {
    return (wdInfo & 0x0180) >> 7;
  }

  inline  bool  IsGoodShemeMin2( unsigned scheme )
  {
    return scheme == 0x0102 || scheme == 0x020A || scheme == 0x032A;
  }

  inline  bool  IsGoodShemeMin1( unsigned scheme )
  {
    return scheme == 0x0101 || scheme == 0x0205 || scheme == 0x0311
        || IsGoodShemeMin2( scheme );
  }

  inline  bool  IsGoodShemeMin0( unsigned scheme )
  {
    return scheme == 0x0100 || scheme == 0x0200 || scheme == 0x0300
        || scheme == 0x0201 || scheme == 0x0301 || IsGoodShemeMin1( scheme );
  }

  inline  bool  IsGoodSheme( unsigned scheme, unsigned minCap )
  {
    return ( minCap == 2 ? IsGoodShemeMin2( scheme ) :
           ( minCap == 1 ? IsGoodShemeMin1( scheme ) :
           ( minCap == 0 ? IsGoodShemeMin0( scheme ) : false ) ) );
  }

} // __libmorpheng__ namespace

# endif // _capsheme_h_
