# if !defined( _mlmadefs_h_ )
# define _mlmadefs_h_

// Определить кроссплатформенные типы данных
# include "../include/mlma1058.h"

# if defined( LIBMORPH_NAMESPACE )
namespace LIBMORPH_NAMESPACE
{
# endif   // LIBMORPH_NAMESPACE

// Define common types used in the analyser
# if !defined( __byte_t_defined__ )
#   define  __byte_t_defined__
   typedef unsigned char   byte_t;
# endif   // !__byte_t_defined__
# if !defined( __word16_t_defined__)
#   define __word16_t_defined__
    typedef unsigned short  word16_t;
# endif   // !__word16_t_defined__
# if !defined( __word32_t_defined__)
#   define __word32_t_defined__
      typedef lexeme_t      word32_t;
# endif   // !__word32_t_defined__

  // Define data access macros
  # if defined( WIN32 ) || defined( WIN16 ) || defined( INTEL_SYSTEM )

  #   if !defined( GetWord16 )
  #     define GetWord16( pv )     *(word16_t*)(pv)
  #   endif  // GetWord16
  #   if !defined( GetWord32 )
  #     define GetWord32( pv )     *(word32_t*)(pv)
  #   endif  // GetWord32

  #   define Swap16( sw )
  #   define Swap32( sw )

  # elif defined( SUN )

  #   if !defined( GetWord16 )
  #     define GetWord16( pv )  ( ((byte08_t*)(pv))[0] | (((byte08_t*)(pv))[1] << 8) )
  #   endif  // GetWord16
  #   if !defined( GetWord32 )
  #     define GetWord32( pv )  ( ((byte08_t*)(pv))[0] | (((byte08_t*)(pv))[1] << 8)  \
                               | (((byte08_t*)(pv))[2] << 16) | (((byte08_t*)(pv))[3] << 24))
  #   endif  // GetWord32

  #   define Swap16( sw ) (sw) = (((sw) & 0x00FF) << 8) | ((sw) >> 8)
  #   define Swap32( sw ) (sw) = ((sw) << 24) | (((sw) & 0x0000FF00) << 8)  \
                              | (((sw) & 0x00FF0000) >> 8) | ((sw) >> 24)

  # endif // macros definitions

  // Грамматическая информация, используемая при общении с вызывающими
  // программами
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

  inline  word32_t  GetLexeme( const void* buffer, unsigned ncbyte )
  {
    const byte_t*   lpdata = (const byte_t*)buffer;
    word32_t        nlexid = 0;
    unsigned        nshift;

    for ( nshift = 0; nshift < ncbyte; nshift++ )
      nlexid |= (unsigned)(*lpdata++) << (nshift * 8);
    return nlexid;
  }

  // Описание одного отождествления на таблицах окончаний
  typedef struct tagFlexInfo
  {
    word16_t  gInfo;
    byte_t    other;
  } SFlexInfo;

  # define tfCompressed  0x80              // Признак закомпрессованности таблицы окончаний
  # define ffNNext       0x80              /* Nesessary-next flex-item level         */
  # define ffONext       0x40              /* Optional-next flex-item level          */

  # define wfSuffix   0x8000       /* Stem has pseudo-suffix                 */
  # define wfPostSt   0x4000       /* Stem has post-text definition          */
  # define wfMixTab   0x8000       /* Stem has mix-table reference           */
  # define wfFlexes   0x4000       /* Stem has flex-table reference          */
  # define wfOldWord  0x0080       /* Old word, at most not used now         */

  // Глобальные данные, которые никто не трогает

  extern unsigned char  flexTree[];
  extern unsigned char  mxTables[];
  extern unsigned char  mixTypes[];
  extern unsigned char  classmap[];
  extern unsigned char  stemtree[];
  extern unsigned char  lidstree[];

  // Функции min и max - свои, так как в разных компиляторах
  // используются разные способы определения этих функций

  template <class T>
  inline T minimal( T v1, T v2 )
    {  return ( v1 <= v2 ? v1 : v2 );  }

  template <class T>
  inline T maximal( T v1, T v2 )
    {  return ( v1 >= v2 ? v1 : v2 );  }

  // Функции доступа к таблицам окончаний

  inline  word32_t  getserial( const byte_t*& p )
  {
    byte_t    bfetch = *p++;
    word32_t  serial = bfetch & ~0x80;
    int       nshift = 1;

    while ( (bfetch & 0x80) != 0 )
      serial |= (((word32_t)(bfetch = *p++) & ~0x80) << (nshift++ * 7));
    return serial;
  }

  inline  word16_t  getword16( const byte_t*& p )
  {
    word16_t  v = *(word16_t*)p;
      p = sizeof(word16_t) + p;
    return v;
  }

  inline  size_t    lexkeylen( byte_t* p, unsigned nlexid )
  {
    byte_t* o = p;

    if ( (nlexid & ~0x000000ff) == 0 )  { *p++ = (byte_t)nlexid;  }
      else
    if ( (nlexid & ~0x0000ffff) == 0 )  { *p++ = (byte_t)(nlexid >> 8); *p++ = (byte_t)nlexid;  }
      else
    if ( (nlexid & ~0x00ffffff) == 0 )  { *p++ = (byte_t)(nlexid >> 16);  *p++ = (byte_t)(nlexid >> 8); *p++ = (byte_t)nlexid;  }
      else
    {  *p++ = (byte_t)(nlexid >> 24);  *p++ = (byte_t)(nlexid >> 16);  *p++ = (byte_t)(nlexid >> 8); *p++ = (byte_t)nlexid;  }

    return p - o;
  }

  // Макроопределения для вычмсления легальной ступени чередования

  // GetVerbMixPower_0 - чередование в 1 ед. наст.(буд.) и прич.страд.прош.

  inline  int GetVerbMixPower_0( word16_t verbType, word16_t gramInfo )
  {
    verbType &= 0x3F;

    if ( (gramInfo & (gfVerbTime | gfVerbForm)) == (vtPast | vfVerbPassiv) )
      return 3;
    if ( (gramInfo & (gfVerbFace | gfMultiple)) != vbFirstFace )
      return 1;
    if ( ( verbType == 24 ) || ( verbType == 25 ) )
      return 2;
    if ( (gramInfo & gfVerbTime) != vtPresent )
      return 1;
    return ( (verbType == 22) || (verbType == 23) || (verbType == 26)
      || (verbType == 27) ? 2 : 1 );
  }

  // GetVerbMixPower_1 - чередование в 1 ед., 3 мн. наст.(буд.) и
  // в глагольных формах наст. (2 прич. и дееприч.)

  inline  int GetVerbMixPower_1( word16_t verbType, word16_t gramInfo )
  {
    word16_t  verbTime = gramInfo & gfVerbTime;

    verbType &= 0x3F;

    if ( (gramInfo & (gfVerbTime | gfVerbForm)) == (vtPast | vfVerbPassiv) )
      return 3;
    if ( (verbTime == vtInfinitiv) || (verbTime == vtPast) || (verbTime == vtImperativ) )
      return 1;
    if ( (gramInfo & gfVerbForm) != 0 )
      return 2;
    if ( ( (gramInfo & (gfVerbFace | gfMultiple)) != vbFirstFace )
      && ( (gramInfo & (gfVerbFace | gfMultiple)) != (vbThirdFace | gfMultiple) ) )
        return 1;
    if ( ( verbType == 24 ) || ( verbType == 25 ) )
      return 2;
    if ( verbTime != vtPresent )
      return 1;
    return ( ( verbType == 22 ) || ( verbType == 23 )
          || ( verbType == 26 ) || ( verbType == 27 ) ? 2 : 1 );
  }

    // GetVerbMixPower_2 - чередование во всех формах наст.(буд.)

    #define GetVerbMixPower_2( VerbType, GramInfo )                             \
      (                                                                         \
        (                                                                       \
          (((GramInfo) & gfVerbTime) == vtPast)                                 \
            ||                                                                  \
          (((GramInfo) & gfVerbTime) == vtInfinitiv)                            \
        ) ? 1 :                                                                 \
        (                                                                       \
          (((GramInfo) & gfVerbTime) == vtImperativ) ? 2 :                      \
          (                                                                     \
            (                                                                   \
              (                                                                 \
                (                                                               \
                  (((VerbType) & 0x3F) == 22) || (((VerbType) & 0x3F) == 23)    \
                    ||                                                          \
                  (((VerbType) & 0x3F) == 26) || (((VerbType) & 0x3F) == 27)    \
                )                                                               \
                  &&                                                            \
                (((GramInfo) & gfVerbTime) == vtPresent)                        \
              )                                                                 \
                ||                                                              \
              (((VerbType) & 0x3F) == 24)                                       \
                ||                                                              \
              (((VerbType) & 0x3F) == 25)                                       \
            ) ? 2 : 1                                                           \
          )                                                                     \
        )                                                                       \
      )                                                                         

  inline  int GetVerbMixPower( word16_t verbType, word16_t gramInfo, word16_t mtOffs )
  {
    mtOffs &= 0xC000;

    return ( mtOffs == 0x0000 ? GetVerbMixPower_2( verbType, gramInfo ) :
           ( mtOffs == 0x4000 ? GetVerbMixPower_0( verbType, gramInfo ) :
                                GetVerbMixPower_1( verbType, gramInfo ) ) );
  }

  #define MascNotAnimMixPower_0(GramInfo) \
    (((((GramInfo) & (gfFormMask | gfMultiple)) == 0) || (((GramInfo) & (gfFormMask | gfMultiple)) == (3 << 12))) ? 1 : 2)

  #define MascNotAnimMixPower_1(GramInfo) \
    ((((GramInfo) & (gfFormMask | gfMultiple)) == (5 << 12)) ? 2 : 1)

  #define MascNotAnimMixPower( GramInfo, mtOffs )                               \
    ((((mtOffs) & 0xC000) == 0) ? MascNotAnimMixPower_0( (GramInfo) ) :         \
      MascNotAnimMixPower_1( (GramInfo) ) )

  #define MascAnimateMixPower_0(GramInfo)   \
    ((((GramInfo) & (gfFormMask | gfMultiple)) == 0) ? 1 : 2)

  #define MascAnimateMixPower_1(GramInfo)   \
    ((((GramInfo) & (gfFormMask | gfMultiple)) == (6 << 12)) ? 2 : 1)

  #define MascAnimateMixPower(GramInfo, mtOffs)                                       \
    ((((mtOffs) & 0xC000) == 0) ? MascAnimateMixPower_0( (GramInfo) ) :               \
      MascAnimateMixPower_1( (GramInfo) ) )

  #define MascMixedMixPower(GramInfo, AFlags, mtOffs)                                 \
    ((((AFlags) & afAnimated) != 0) ? MascAnimateMixPower( (GramInfo), (mtOffs) ) :   \
      MascNotAnimMixPower( (GramInfo), (mtOffs) ))

  #define FemnNotAnimMixPower_0(GramInfo)                                           \
    (                                                                               \
      (((GramInfo) & (gfFormMask | gfMultiple)) == ((1 << 12) | gfMultiple)) ? 2 :  \
      (                                                                             \
        ((((GramInfo) & (gfFormMask | gfMultiple)) == (2 << 12))                    \
           ||                                                                       \
         (((GramInfo) & (gfFormMask | gfMultiple)) == (5 << 12))) ? 3 : 1           \
      )                                                                             \
    )

  #define FemnNotAnimMixPower_1(GramInfo)                                           \
    (                                                                               \
      (((GramInfo) & (gfFormMask | gfMultiple)) == (4 << 12)) ? 2 :                 \
      (                                                                             \
        ((((GramInfo) & (gfFormMask | gfMultiple)) == (1 << 12))                    \
           ||                                                                       \
         (((GramInfo) & (gfFormMask | gfMultiple)) == (3 << 12))                    \
           ||                                                                       \
         (((GramInfo) & (gfFormMask | gfMultiple)) == (6 << 12))) ? 1 : 3           \
      )                                                                             \
    )

  #define FemnNotAnimMixPower(GramInfo, mtOffs)                                       \
    ((((mtOffs) & 0xC000) == 0) ? FemnNotAnimMixPower_0( (GramInfo) ) :               \
      FemnNotAnimMixPower_1( (GramInfo) ) )

  #define FemnAnimateMixPower_0(GramInfo)                                           \
    (                                                                               \
     (                                                                              \
      (                                                                             \
       (((GramInfo) & (gfFormMask | gfMultiple)) == ((1 << 12) | gfMultiple))       \
         ||                                                                         \
       (((GramInfo) & (gfFormMask | gfMultiple)) == ((3 << 12) | gfMultiple))       \
      ) ? 2 :                                                                       \
      (                                                                             \
       ((((GramInfo) & (gfFormMask | gfMultiple)) == (2 << 12))                     \
        ||                                                                          \
        (((GramInfo) & (gfFormMask | gfMultiple)) == (5 << 12))) ? 3 : 1            \
      )                                                                             \
     )                                                                              \
    )

  #define FemnAnimateMixPower_1(GramInfo) FemnNotAnimMixPower_1( (GramInfo) )

  #define FemnAnimateMixPower( GramInfo, mtOffs )                                     \
    ((((mtOffs) & 0xC000) == 0) ? FemnAnimateMixPower_0( (GramInfo) ) :               \
      FemnAnimateMixPower_1( (GramInfo) ) )

  #define FemnMixedMixPower(GramInfo, AFlags, mtOffs)   \
    ((((AFlags) & afAnimated) != 0) ? FemnAnimateMixPower( (GramInfo), (mtOffs) ) :  \
      FemnNotAnimMixPower( (GramInfo), (mtOffs) ))

  // stem access class

  struct  steminfo
  {
    word16_t        wdinfo;
    word16_t        tfoffs;
    word16_t        mtoffs;

  public:     // init
    steminfo( const byte_t* pclass = NULL )
      {
        if ( pclass != NULL )
          Load( pclass );
      }
    steminfo& Load( const byte_t* pclass )
      {
        wdinfo = getword16( pclass );
        tfoffs = (wdinfo & wfFlexes) != 0 || (wdinfo & 0x3f) == 51 ? getword16( pclass ) : 0;
        mtoffs = (wdinfo & wfMixTab) != 0 ? getword16( pclass ) : 0;
        return *this;
      }
    const unsigned  MinCapScheme() const
      {
        return (wdinfo & 0x0180) >> 7;
      }
    const byte_t* GetFlexTable() const
      {
        return tfoffs != 0 ? (tfoffs << 0x0004) + flexTree : NULL;
      }
    const byte_t* GetSwapTable() const
      {
        return mtoffs != 0 ? (mtoffs & ~0xc000) + mxTables : NULL;
      }
    const int       GetSwapLevel( word16_t grinfo, byte_t bflags ) const
      {
        switch ( mixTypes[wdinfo & 0x3F] )
        {
          case 1:   return GetVerbMixPower( wdinfo, grinfo, mtoffs );
          case 2:   return MascNotAnimMixPower( grinfo, mtoffs );
          case 3:   return MascAnimateMixPower( grinfo, mtoffs );
          case 4:   return MascMixedMixPower( grinfo, bflags, mtoffs );
          case 5:   return FemnNotAnimMixPower( grinfo, mtoffs );
          case 6:   return FemnAnimateMixPower( grinfo, mtoffs );
          case 7:   return FemnMixedMixPower( grinfo, bflags, mtoffs );
          default:  return 1;
        }
      }
  };

  // Некоторые полезные функции, выясняющие часть речи

  inline  bool  IsVerb( word16_t wbInfo )
  {
    wbInfo &= 0x3F;

    return ( wbInfo == 22 ) || ( wbInfo == 24 ) || ( wbInfo == 26 );
  }

  inline  bool  IsAdjective( word16_t wbInfo )
  {
    wbInfo &= 0x3F;

    return ( wbInfo == 16 ) || ( wbInfo == 18 )
        || ( wbInfo == 19 ) || ( wbInfo == 21 );
  }

  inline  bool  IsParticiple( word16_t grInfo )
  {
    return (grInfo & gfVerbForm) == vfVerbActive || (grInfo & gfVerbForm) == vfVerbPassiv;
  }

  //=====================================================================
  // Meth: GetNormalInfo
  // Функция строит грамматическую информацию о нормальной форме слова,
  // используя тип этого слова, грамматическую информацию об отождествлении
  // и настройки поиска и нормализации.
  // Нормальной формой считается:
  // Для существительных - именительный падеж единственного числа;
  // Для прилагательных - именительный падеж мужского рода;
  // Для глаголов - инфинитив (или причастная форма - по настройкам).
  //=====================================================================
  inline word16_t GetNormalInfo( word16_t wbInfo,
                                 word16_t grInfo,
                                 unsigned nFlags )
  {
    word16_t  nfInfo = 0;

    if ( IsVerb( wbInfo ) )
    {
      nfInfo = vtInfinitiv;
      if ( ( nFlags & nfAdjVerbs ) && IsParticiple( grInfo ) )
        nfInfo = (word16_t)((grInfo & (gfVerbTime|gfVerbForm)) | (1 << 9));
    }
      else
    if ( IsAdjective( wbInfo ) )
      nfInfo = 1 << 9;
    if ( wbInfo & wfMultiple )
      nfInfo |= gfMultiple;
    return nfInfo;
  }

# if defined( LIBMORPH_NAMESPACE )
} // end namespace
# endif   // LIBMORPH_NAMESPACE

# endif // _mlmadefs_h_
