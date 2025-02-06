/******************************************************************************

    libmorphrus - dictiorary-based morphological analyser for Russian.

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
# if !defined( __libmorph_rus_mlmadefs_h__ )
# define __libmorph_rus_mlmadefs_h__

// Определить кроссплатформенные типы данных
# include "../include/mlma1049.h"
# include "xmorph/capsheme.h"
# include "xmorph/typedefs.h"

namespace libmorphrus {
  extern unsigned char  flexTree[];
  extern unsigned char  mxTables[];
  extern unsigned char  mixTypes[];
  extern unsigned char  classmap[];
  extern unsigned char  stemtree[];
  extern unsigned char  lidstree[];
}

namespace libmorph {
namespace rus {

  # define ffNNext       0x80              /* Nesessary-next flex-item level         */
  # define ffONext       0x40              /* Optional-next flex-item level          */

  # define wfPostSt   0x8000       /* Stem has post-text definition          */
  # define wfMixTab   0x4000       /* Stem has mix-table reference           */
  # define wfFlexes   0x2000       /* Stem has flex-table reference          */

  extern const unsigned char  pspMinCapValue[];

  // Макроопределения для вычисления легальной ступени чередования

  inline int  GetVerbMixPowerType3( word16_t gramInfo )
  {
    int   vtense = gramInfo & gfVerbTime;

    return ( (vtense == vtPresent) || ( vtense == vtFuture) ? 2 : 1 );
  }

  inline int  GetVerbMixPowerType2( word16_t gramInfo )
  {
    int   vtense = gramInfo & gfVerbTime;
    bool  passiv = (gramInfo & gfVerbForm) == vfVerbPassiv;

    return ( (vtense == vtPast) && passiv ? 3 :
           ( (vtense != vtPast) && (vtense != vtInfinitiv) ? 2 : 1 ) );
  }

  inline int  GetVerbMixPowerType1( word16_t gramInfo )
  {
    return ( (gramInfo & (gfVerbTime|gfVerbForm)) == (vtPast|vfVerbPassiv) ? 3 :
           ( (gramInfo & (gfVerbFace|gfMultiple)) == vbFirstFace ? 2 : 1 ) );
  }

  inline int  GetVerbMixPower( word16_t stemInfo, word16_t gramInfo )
  {
    int   mixType = (stemInfo & 0x1800) >> 11;

    return ( mixType == 0 ? GetVerbMixPowerType1( gramInfo ) :
           ( mixType == 1 ? GetVerbMixPowerType2( gramInfo ) :
           ( mixType == 2 ? GetVerbMixPowerType3( gramInfo ) : 1 ) ) );
  }

    #define MascNotAnimMixPower(GramInfo)   \
      (((((GramInfo) & (gfFormMask | gfMultiple)) == 0) || (((GramInfo) & (gfFormMask | gfMultiple)) == (3 << 12))) ? 1 : 2)

    #define MascAnimateMixPower(GramInfo)   \
      ((((GramInfo) & (gfFormMask | gfMultiple)) == 0) ? 1 : 2)

    #define MascMixedMixPower(GramInfo, AFlags)   \
      ((((AFlags) & afAnimated) != 0) ? MascAnimateMixPower((GramInfo)) :  \
        MascNotAnimMixPower((GramInfo)))

    #define FemnNotAnimMixPower(GramInfo)   \
      ((((GramInfo) & (gfFormMask | gfMultiple)) == ((1 << 12) | gfMultiple)) ? 2 : 1)

    #define FemnAnimateMixPower(GramInfo)   \
      (((((GramInfo) & (gfFormMask | gfMultiple)) == ((1 << 12) | gfMultiple)) || \
        (((GramInfo) & (gfFormMask | gfMultiple)) == ((3 << 12) | gfMultiple))) ? 2 : 1)

    #define FemnMixedMixPower(GramInfo, AFlags)   \
      ((((AFlags) & afAnimated) != 0) ? FemnAnimateMixPower((GramInfo)) :  \
        FemnNotAnimMixPower((GramInfo)))

    #define GetAdjectivMixPower(GramInfo)   \
      (((GramInfo) == gfCompared) ? 3 :       \
       (((GramInfo) == ((1 << 9) | gfShortOne)) ? 2 : 1))

  // Некоторые полезные функции, выясняющие часть речи

  inline  bool  IsVerb( word16_t wbInfo )
  {
    return (wbInfo & 0x3F) <= 6;
  }

  inline  bool  IsAdjective( word16_t wbInfo )
  {
    wbInfo &= 0x3F;

    return (wbInfo >= 25 && wbInfo <= 28) || wbInfo == 34 || wbInfo == 36 || wbInfo == 42;
  }

  inline  bool  IsParticiple( word16_t grInfo )
  {
    return (grInfo & gfVerbForm) == vfVerbActive || (grInfo & gfVerbForm) == vfVerbPassiv;
  }

  // stem access class

  struct  steminfo
  {
    word16_t        wdinfo;
    word16_t        tfoffs;
    word16_t        mtoffs;

  public:     // init
    steminfo( uint16_t offset = (uint16_t)-1 )
      {
        if ( offset != (uint16_t)-1 )
          Load( (const uint8_t*)libmorphrus::classmap + offset );
      }
    steminfo( uint16_t winfo, uint16_t foffs, uint16_t moffs ):
      wdinfo( winfo ),
      tfoffs( foffs ),
      mtoffs( moffs ) {}
    steminfo& Load( const byte_t* pclass )
      {
        wdinfo = getword16( pclass );
        tfoffs = (wdinfo & wfFlexes) != 0 || (wdinfo & 0x3f) == 51 ? getword16( pclass ) : 0;
        mtoffs = (wdinfo & wfMixTab) != 0 ? getword16( pclass ) : 0;
        return *this;
      }
    unsigned      MinCapScheme() const
      {
        return pspMinCapValue[wdinfo & 0x3f];
      }
    const byte_t* GetFlexTable() const
      {
        return tfoffs != 0 && (wdinfo & 0x3f) != 51 ? (tfoffs << 0x0004) + libmorphrus::flexTree : NULL;
      }
    const byte_t* GetSwapTable() const
      {
        return mtoffs != 0 ? mtoffs + libmorphrus::mxTables : NULL;
      }
    int           GetSwapLevel( word16_t grinfo, byte_t bflags ) const
      {
        switch ( libmorphrus::mixTypes[wdinfo & 0x3F] )
        {
          case 1:   return GetVerbMixPower( wdinfo, grinfo );
          case 2:   return MascNotAnimMixPower( grinfo );
          case 3:   return MascAnimateMixPower( grinfo );
          case 4:   return MascMixedMixPower( grinfo, bflags );
          case 5:   return FemnNotAnimMixPower( grinfo );
          case 6:   return FemnAnimateMixPower( grinfo );
          case 7:   return FemnMixedMixPower( grinfo, bflags );
          case 8:   return GetAdjectivMixPower( grinfo );
          default:  return 1;
        }
      }
    //=====================================================================
    // Meth: FindDictForm
    // Функция строит грамматическую информацию о нормальной форме слова,
    // используя тип этого слова, грамматическую информацию об отождествлении
    // и настройки поиска и нормализации.
    // Нормальной формой считается:
    // Для существительных - именительный падеж единственного числа;
    // Для прилагательных - именительный падеж мужского рода;
    // Для глаголов - инфинитив (или причастная форма - по настройкам).
    //=====================================================================
    flexinfo      FindDictForm( const flexinfo& grinfo, unsigned dwsets ) const
      {
        if ( IsVerb( wdinfo ) )
        {
          if ( (dwsets & nfAdjVerbs) != 0 && IsParticiple( grinfo.gramm ) )
            return { uint16_t((grinfo.gramm & (gfVerbTime|gfVerbForm)) | (1 << 9)), afAnimated| afNotAlive };
          else
            return { vtInfinitiv, afAnimated| afNotAlive };
        }
          else
        {
          uint16_t nfinfo = (IsAdjective( wdinfo ) ? (1 << 9) : 0)
            | ((wdinfo & wfMultiple) != 0 ? gfMultiple : 0);

          return { nfinfo, afAnimated | afNotAlive };
        }
      }

  };

}} // end namespace

# endif // __libmorph_rus_mlmadefs_h__
