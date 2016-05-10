# if !defined( _mlmadefs_h_ )
# define _mlmadefs_h_

// Определить кроссплатформенные типы данных
# include "../include/mlma1058.h"

# if defined( LIBMORPH_NAMESPACE )
namespace LIBMORPH_NAMESPACE
{
# endif   // LIBMORPH_NAMESPACE

  // Define common types used in the analyser
  typedef unsigned char   byte08_t;
  typedef unsigned short  word16_t;
  typedef lexeme_t        word32_t;

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
    const byte08_t* lpdata = (const byte08_t*)buffer;
    word32_t        nlexid = 0;
    unsigned        nshift;

    for ( nshift = 0; nshift < ncbyte; nshift++ )
      nlexid |= (unsigned)(*lpdata++) << (nshift * 8);
    return nlexid;
  }

  // Описание одного отождествления на таблицах окончаний
  typedef struct tagFlexInfo
  {
    word16_t gInfo;
    byte08_t other;
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

  extern unsigned char  fxTables[];
  extern unsigned char  mxTables[];
  extern unsigned char  suffixes[];
  extern unsigned char  classBox[];
  extern char           mixTypes[];

  // Функции min и max - свои, так как в разных компиляторах
  // используются разные способы определения этих функций

  template <class T>
  inline T minimal( T v1, T v2 )
    {  return ( v1 <= v2 ? v1 : v2 );  }

  template <class T>
  inline T maximal( T v1, T v2 )
    {  return ( v1 >= v2 ? v1 : v2 );  }

  // Функции доступа к таблицам окончаний

  inline  word16_t  GetFlexInfo( void* item )   // extract grammatical information;
    {  return GetWord16( ((char*)item) + 1 ); }

  inline  word16_t  GetFlexNext( void* item )
    {  return GetWord16( ((char*)item) + 5 ); }

  inline  byte08_t* GetInfoByID( word16_t id )
    {  return classBox + GetWord16( ((word16_t*)classBox) + (id & 0x0FFF) ); }

  inline  word16_t  GetFlexText( void* item )
    {  return GetWord16( ((char*)item) + 3 ); }

  inline  const byte08_t* FetchSuffix( int id )
    {  return suffixes + GetWord16( ((word16_t*)suffixes) + id );  }

  #define GetMixTabCount( mixTab )  GetWord16( (mixTab) )

  inline  word16_t  GetMixTabItems( const void* mixTab, int index )
    {  return GetWord16( ((word16_t*)mixTab) + index + 1 ); }

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

  inline int  GetMixPower( word16_t   stemInfo,
                           word16_t   gramInfo,
                           byte08_t   bFlags,
                           word16_t   mtOffs )
  {
    int   idtype = mixTypes[stemInfo & 0x3F];

    return ( idtype == 1 ? GetVerbMixPower( stemInfo, gramInfo, mtOffs ) :
           ( idtype == 2 ? MascNotAnimMixPower( gramInfo, mtOffs ) :
           ( idtype == 3 ? MascAnimateMixPower( gramInfo, mtOffs ) :
           ( idtype == 4 ? MascMixedMixPower( gramInfo, bFlags, mtOffs ) :
           ( idtype == 5 ? FemnNotAnimMixPower( gramInfo, mtOffs ) :
           ( idtype == 6 ? FemnAnimateMixPower( gramInfo, mtOffs ) :
           ( idtype == 7 ? FemnMixedMixPower( gramInfo, bFlags, mtOffs ) : 1 ) ) ) ) ) ) );
  }

  inline int  GetSkipCount( const byte08_t* stemList )
  {
    int       cbSkip = 2;
    word16_t  idStem = GetWord16( stemList );

  // Если у слова есть псевдосуффикс, пропустить еще байт
    if ( idStem & wfSuffix )
      cbSkip++;
  // Если есть нефлективный текст в постпозиции, пропустить
  // его длину и один байт
    if ( idStem & wfPostSt )
      cbSkip += stemList[cbSkip] + 1;
  // Пропустить размер идентификатора лексемы
    return cbSkip + ((idStem & 0x3000) >> 12) + 1;
  }

  // Прототип функций, вызываемых на словаре при успешном отождествлении
  struct  foundAction
  {
    const byte08_t* origin;   // the string origin
    unsigned        scheme;   // the capitalization scheme
    unsigned        nflags;   // the scanning flags
    int             nerror;   // != 0 if something failed
  public:
    virtual int   Register( const byte08_t* pwinfo,
                            const byte08_t* pszstr,
                            SGramInfo*      pginfo,
                            unsigned        cginfo ) = 0;
  };

  int   FlexComp( word16_t        tabOffs,
                  const byte08_t* pszWord,
                  int             wordLen,
                  SFlexInfo*      grInfo,
                  word16_t        cmnInfo,
                  byte08_t        cmnFlag,
                  unsigned        options,
                  const byte08_t* pszPost );

  int   StemComp( const byte08_t* pwinfo,
                  const byte08_t* pszstr,
                  unsigned        cchstr,
                  SGramInfo*      pginfo,
                  foundAction&    action );

  bool  ScanPage( const byte08_t* lppage,
                  unsigned        offset,
                  const byte08_t* pszstr,
                  unsigned        cchstr,
                  foundAction&    action );

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

  # define MAX_WORD_FORMS  32

  // Provide windows.h definitions if needed
  // Отключить использование ненужных определений - OLE, графику
  // Собственно, здесь windows.h нужен, чтобы задать тип экспортируемых
  // функций в соответствии с требованиями операционной системы
  # if defined( WIN32 ) || defined( WIN16 )
  #   define WIN32_LEAN_AND_MEAN
  #   include <windows.h>
  # endif

# if defined( LIBMORPH_NAMESPACE )
} // end namespace
# endif   // LIBMORPH_NAMESPACE

# endif // _mlmadefs_h_
