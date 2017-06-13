# if !defined( __grammap_h__ )
# define __grammap_h__

# include "typedefs.h"

# if defined( LIBMORPH_NAMESPACE )
namespace LIBMORPH_NAMESPACE {
# endif  // LIBMORPH_NAMESPACE

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

  inline void     getCaseInfo( byte_t     form,
                               word16_t&  info,
                               byte_t&    flag )
  {
    flag = afAnimated | afNotAlive;
    if ( form >= 5 ) info |= (form - 1) << 12;
      else
    if ( form <= 2 ) info |= form << 12;
      else
    {
      info |= 3 << 12;  // Винительный падеж
      if ( form == 3 ) flag = afNotAlive;
        else flag = afAnimated;
    }
  }

  inline bool getGendInfo( byte_t     form,
                           word16_t&  info,
                           byte_t&    flag )
  {
    flag = afAnimated | afNotAlive;
    if ( form >= 24 ) info |= gfMultiple;
      else info |= ((form / 8) + 1) << 9;
    if ( (form % 8) == 7 ) info |= gfShortOne;
      else getCaseInfo( form % 8, info, flag );
    return ( form < 32 );
  }

  inline word16_t getGenderOf( byte_t form )
  {
    return ( form == 3 ? gfMultiple : (form + 1) << 9 );
  }

  inline word16_t getVerbTant( byte_t form )
  {
    return ( form >= 2 ? gfRetForms : 0 ) | ( (form % 2) == 1 ? gfMultiple : 0 );
  }

  inline word16_t getPersonal( byte_t form )
  {
    return ((form / 4 + 1) << 3) | getVerbTant( form % 4 );
  }

  inline  bool    getAllParts( byte_t     form,
                               word16_t&  info,
                               byte_t&    flag )
  {
    flag = afAnimated | afNotAlive;

    if ( form >= 96 ) info |= vfVerbDoing | ( (form % 2) == 1 ? gfRetForms : 0 );
      else
    if ( form >= 64 )
    {
      info |= vfVerbPassiv;
      return getGendInfo( form - 64, info, flag );
    }
      else
    {
      if ( form >= 32 ) info |= gfRetForms;
      info |= vfVerbActive;
      return getGendInfo( form % 32, info, flag );
    }
    return true;
  }

  inline  bool  GetDefsInfo( byte_t     form,
                             word16_t&  info,
                             byte_t&    flag )
  {
    flag = afAnimated | afNotAlive;

    if ( form == 65 ) info = gfAdverb;
      else
    if ( form == 64 ) info = gfCompared;
      else
    {
      info = ( form >= 32 ? gfRetForms : 0 );
      return getGendInfo( form % 32, info, flag );
    }
    return true;
  }

  inline  bool  GetNounInfo( byte_t     form,
                             word16_t&  info,
                             byte_t&    flag )
  {
    info = form >= 10 ? gfMultiple : 0;
    getCaseInfo( form % 10, info, flag );
    return form <= 17;
  }

  inline  bool  GetVerbInfo( byte_t     form,
                             word16_t&  info,
                             byte_t&    flag )
  {
    flag = afAnimated | afNotAlive;
  // Инфинитив
    if ( form < 2 ) info = vtInfinitiv | ( form == 1 ? gfRetForms : 0 );
      else
  // Императив
    if ( form < 6 ) info = vtImperativ | getVerbTant( form - 2 );
      else
  // Будущее время
    if ( form < 18 ) info = vtFuture | getPersonal( form - 6 );
      else
  // Настоящее время
    if ( form < 128 )
    {
      info = vtPresent;

    // Личные формы, база 18, длина 12
      if ( form < 30 ) info |= getPersonal( form - 18 );
    // Все причастия, база 30, длина 98
        else return getAllParts( form - 30, info, flag );
    }
      else
  // Прошедшее время
    {
      info = vtPast;

    // Проверить диапазон форм
      if ( form >= 234 )
        return false;

    // Личные формы
      if ( form < 136 ) info |= ( form >= 132 ? gfRetForms : 0 ) | getGenderOf( form % 4 );
        else
    // Все причастия
      return getAllParts( form - 136, info, flag );
    }
    return true;
  }

  inline bool GetWordInfo( byte_t     wInfo,
                           byte_t     nform,
                           word16_t&  gInfo,
                           byte_t&    flags )
  {
    if ( IsVerb( wInfo ) )      return GetVerbInfo( nform, gInfo, flags );
      else
    if ( IsAdjective( wInfo ) ) return GetDefsInfo( nform, gInfo, flags );
      else                      return GetNounInfo( nform, gInfo, flags );
  }

# if defined( LIBMORPH_NAMESPACE )
}  // end namespace
# endif  // LIBMORPH_NAMESPACE

# endif // __grammap_h__
