/******************************************************************************

    libmorphrus - dictiorary-based morphological analyser for Russian.

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
# if !defined( __libmorph_rus_grammap_h__ )
# define __libmorph_rus_grammap_h__ 
# include "mlmadefs.h"
# include "xmorph/typedefs.h"

namespace libmorph {
namespace rus {

  //==============================================================================
  // Method:: mapCaseInfo
  // Функция строит абсолютный номер падежной формы, учитывая грамматическое
  // описание формы (в нижнем слове) и одушевленность (в верхнем слове).
  //==============================================================================
  inline  byte_t  mapCaseInfo( unsigned info )
  {
    int   idcase = (info & gfFormMask) >> 12;

    return ( idcase > 3 ? idcase + 1 :
           ( idcase < 3 ? idcase :
           ( info & (afNotAlive << 16) ? 3 : 4 ) ) );
  }

  //==============================================================================
  // Method:: mapGenderOf
  // Функция извлекает номер рода и числа для переданного грамматического описания
  //==============================================================================
  inline  byte_t  mapGenderOf( unsigned info )
  {
    return (byte_t)( info & gfMultiple ? 3 : ((info & gfGendMask) >> 9) - 1 );
  }

  //==============================================================================
  // Method:: mapGendInfo
  // Функция строит номер родо-падежной комбинации
  // Предусматривается 7 падежей на каждый род и число (6 регулярных + 1
  // дополнительный вин.) и одной краткой формы, 4 рода * 8 = 32 значения
  //==============================================================================
  inline  byte_t  mapGendInfo( unsigned info )
  {
    return mapGenderOf( info ) * 8 + ( info & gfShortOne ? 7 : mapCaseInfo( info ) );
  }

  //==============================================================================
  // Method:: mapDefsInfo
  // Функция строит номер формы прилагательного по грамматическому описанию ее
  //==============================================================================
  inline byte_t MapDefsInfo( unsigned info )
  {
    return ( info & gfAdverb   ? 65 :
           ( info & gfCompared ? 64 : ( info & gfRetForms ? 32 : 0 ) + mapGendInfo( info ) ) );
  }

  //==============================================================================
  // Method:: mapNounInfo
  // Функция строит номер формы существительного по его грамматическому описанию
  //==============================================================================
  inline byte_t MapNounInfo( unsigned info )
  {
    return ( info & gfMultiple ? 10 : 0 ) + mapCaseInfo( info );
  }

  //==============================================================================
  // Эдакий очередной чисто служебный макрос, который отражает изменение форм
  // по времени с учетом возвратных форм в число в диапазоне 0..3
  //==============================================================================
  inline  byte_t  mapVerbTant( unsigned info )
  {
    return (byte_t)( ( info & gfRetForms ? 2 : 0 ) + ( info & gfMultiple ? 1 : 0 ) );
  }

  //==============================================================================
  // Макрос, перебирающий личные формы и отражающий их в диапазон чисел 0..11
  // с учетом изменения по времени и, соответственно, возвратности
  //==============================================================================
  inline  byte_t  mapPersonal( unsigned info )
  {
    return (byte_t)(((info & gfVerbFace) >> 3) - 1) * 4 + mapVerbTant( info );
  }

  //==============================================================================
  // Перебор всех форм активного причастия и отражение их в диапазон 0..63
  //==============================================================================
  inline  byte_t  mapActvPart( unsigned info )
  {
    return (byte_t)( info & gfRetForms ? 32 : 0 ) + mapGendInfo( info );
  }

  //==============================================================================
  // Перебор всех возможных причастий и деепричастий и отражение их в диапазон
  // идентификаторов 0..97
  //==============================================================================
  inline  byte_t  mapAllParts( unsigned info )
  {
    return (byte_t)( (info & gfVerbForm) == vfVerbActive ? mapActvPart( info ) :
                   ( (info & gfVerbForm) == vfVerbPassiv ? 64 + mapGendInfo( info ) :
                     96 + ( info & gfRetForms ? 1 : 0 ) ) );
  }

  inline byte_t MapVerbInfo( unsigned info )
  {
    switch ( info & gfVerbTime )
    {
    // Инфинитив, база 0, длина 2
      case vtInfinitiv:
        return ( info & gfRetForms ? 1 : 0 );
    // Императив, база 2, длина 4
      case vtImperativ:
        return 2 + mapVerbTant( info );
    // Будущее время, база 6, длина 12
      case vtFuture:
        return 6 + mapPersonal( info );
    // Настоящее время, база 18
      case vtPresent:
      // Личные формы, база 18, длина 12
        if ( (info & gfVerbForm) == 0 ) return 18 + mapPersonal( info );
      // Все причастия, база 30, длина 98
          else return 30 + mapAllParts( info );
    // Прошедшее время, база 128
      case vtPast:
      // Личные формы, база 128, длина 8
        if ( (info & gfVerbForm) == 0 ) return 128 + ( info & gfRetForms ? 4 : 0 )
          + mapGenderOf( info );
      // Все причастия, база 136, длина 98
        else return 136 + mapAllParts( info );
      default:
        return 255;
    }
  }

  inline byte_t MapWordInfo( word16_t wdInfo,
                             word16_t grInfo,
                             byte_t   bflags )
  {
    word32_t  dwinfo = (((unsigned)bflags) << 16) | grInfo;

    return  IsVerb     ( wdInfo ) ? MapVerbInfo( dwinfo ) :
            IsAdjective( wdInfo ) ? MapDefsInfo( dwinfo ) : MapNounInfo( dwinfo );
  }

  inline
  auto  getCaseInfo( uint8_t  idform, uint16_t grbase ) -> flexinfo
  {
  // Творительный и выше
    if ( idform >= 5 )
      return { uint16_t(grbase | (idform - 1) << 12), afAnimated | afNotAlive };

  // Дательный и ниже
    if ( idform <= 2 )
      return { uint16_t(grbase | idform << 12), afAnimated | afNotAlive };

  // Винительный падеж
    return { uint16_t(grbase | (3 << 12)), uint8_t(idform == 3 ? afNotAlive : afAnimated) };
  }

  inline
  auto  getGendInfo( formid_t idform, uint16_t grbase ) -> flexinfo
  {
    if ( idform >= 24 ) grbase |= gfMultiple;
      else grbase |= ((idform / 8) + 1) << 9;

    if ( (idform % 8) == 7 )
      return { uint16_t(grbase | gfShortOne), afAnimated | afNotAlive };

    if ( idform < 32 )
      return getCaseInfo( idform % 8, grbase );

    return { uint16_t(-1), uint8_t(-1) };
  }

  inline word16_t getGenderOf( byte_t form )
  {
    return ( form == 3 ? gfMultiple : (form + 1) << 9 );
  }

  inline
  uint16_t  getVerbTant( byte_t form )
  {
    return ( form >= 2 ? gfRetForms : 0 ) | ( (form % 2) == 1 ? gfMultiple : 0 );
  }

  inline
  auto  getPersonal( formid_t idform, uint16_t tenses ) -> flexinfo
  {
    return { uint16_t(tenses | ((idform / 4 + 1) << 3) | getVerbTant( idform % 4 )),
      afAnimated + afNotAlive };
  }

  inline
  auto  getAllParts( formid_t idform, uint16_t tenses ) -> flexinfo
  {
    if ( idform >= 96 )
      return { uint16_t(tenses | vfVerbDoing | ((idform % 2) == 1 ? gfRetForms : 0)), afAnimated + afNotAlive };

    if ( idform >= 64 )
      return getGendInfo( idform - 64, tenses | vfVerbPassiv );

    return getGendInfo( idform % 32, tenses | vfVerbActive | (idform >= 32 ? gfRetForms : 0) );
  }

  inline
  auto  GetDefsInfo( uint8_t  idform ) -> flexinfo
  {
    if ( idform == 65 )
      return { gfAdverb, afAnimated | afNotAlive };

    if ( idform == 64 )
      return { gfCompared, afAnimated | afNotAlive };

    return getGendInfo( idform % 32, idform >= 32 ? gfRetForms : 0 );
  }

  inline
  auto  GetNounInfo( uint8_t  idform ) -> flexinfo
  {
    if ( idform <= 17 )
      return getCaseInfo( idform % 10, idform >= 10 ? gfMultiple : 0 );

    return { uint16_t(-1), uint8_t(-1) };
  }

  inline
  auto  GetVerbInfo( formid_t idform ) -> flexinfo
  {
  // Инфинитив
    if ( idform < 2 )
      return { uint16_t(vtInfinitiv | (idform == 1 ? gfRetForms : 0)), afAnimated + afNotAlive };

  // Императив
    if ( idform < 6 )
      return { uint16_t(vtImperativ | getVerbTant( idform - 2 )), afAnimated + afNotAlive };

  // Будущее время
    if ( idform < 18 )
      return getPersonal( idform - 6, vtFuture );

  // Настоящее время, личные формы, база 18, длина 12
    if ( idform < 30 )
      return getPersonal( idform - 18, vtPresent );

  // Все причастия, база 30, длина 98
    if ( idform < 128 )
      return getAllParts( idform - 30, vtPresent );

  // Прошедшее время
  // Личные формы
    if ( idform < 136 )
      return { uint16_t(vtPast | getGenderOf( idform % 4 ) | (idform >= 132 ? gfRetForms : 0)), afAnimated | afNotAlive };

    // Все причастия
    // Проверить диапазон форм
    if ( idform < 234 )
      return getAllParts( idform - 136, vtPast );

    return { uint16_t(-1), uint8_t(-1) };
  }

  inline
  auto  GetGramInfo( uint8_t  wdinfo, formid_t idform ) -> flexinfo
  {
    if ( IsVerb( wdinfo ) )
      return GetVerbInfo( idform );

    if ( IsAdjective( wdinfo ) )
      return GetDefsInfo( idform );

    if ( (wdinfo & wfPlural) != 0 && idform < 10 )
      return { uint16_t(-1), uint8_t(-1) };

    return GetNounInfo( idform );
  }

}}  // end namespace

# endif // !__libmorph_rus_grammap_h__ 
