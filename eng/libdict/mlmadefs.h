/******************************************************************************

    libfuzzyrus - fuzzy morphological analyser for Russian.

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
# if !defined( __libmorph_eng_mlmadefs_h__ )
# define __libmorph_eng_mlmadefs_h__

// Определить кроссплатформенные типы данных
# include "../include/mlma1033.h"
# include "xmorph/typedefs.h"

namespace libmorpheng {
  extern unsigned char  flexTree[];
  extern unsigned char  classmap[];
  extern unsigned char  stemtree[];
  extern unsigned char  lidstree[];
}

namespace libmorph {
namespace eng
{
  // Define common types used in the analyser
  typedef unsigned char   byte08_t;
  typedef unsigned short  word16_t;
  typedef lexeme_t        word32_t;

# if !defined( mlma_grammarecord_defined )
# define mlma_grammarecord_defined
  typedef struct tagGramInfo
  {
    unsigned char  wInfo;
    unsigned char  iForm;
    unsigned short gInfo;
    unsigned char  other;
  } SGramInfo;
# endif

  // stem access class

  struct  steminfo
  {
    uint16_t  wdinfo;
    uint16_t  tfoffs;

  public:     // init
    steminfo( uint16_t oclass = (uint16_t)-1 )
      {
        if ( oclass != (uint16_t)-1 )
          Load( (const uint8_t*)libmorpheng::classmap + oclass );
      }
    steminfo( uint16_t winfo, uint16_t foffs ):
      wdinfo( winfo ),
      tfoffs( foffs ) {}
    steminfo& Load( const byte_t* pclass )
      {
        wdinfo = getword16( pclass );
        tfoffs = getword16( pclass );
        return *this;
      }
    unsigned      MinCapScheme() const
      {
        return (wdinfo >> 7) & 0x3;
      }
    const byte_t* GetFlexTable() const
      {
        return tfoffs != 0 ? tfoffs + libmorpheng::flexTree : nullptr;
      }
  };

#define MAX_WORD_FORMS  32

}} // end libmorph::eng namespace

# endif // !__libmorph_eng_mlmadefs_h__
