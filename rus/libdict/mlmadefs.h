# if !defined( _mlmadefs_h_ )
# define _mlmadefs_h_

// Определить кроссплатформенные типы данных
# include "../include/mlma1049.h"

# if defined( LIBMORPH_NAMESPACE )
namespace LIBMORPH_NAMESPACE
{
# endif   // LIBMORPH_NAMESPACE

  // Define common types used in the analyser
  typedef unsigned char   byte08_t;
  typedef unsigned short  word16_t;
  typedef lexeme_t        word32_t;

  # if defined( _MSC_VER )
  #   if !defined( INTEL_SYSTEM ) && defined( _M_IX86 )
  #     define  INTEL_SYSTEM
  #   endif  // _M_IX86
  #endif  // _MSC_VER

  # if defined( INTEL_SYSTEM )
  #   if !defined( LIBMORPH_ANYALIGN )
  #     define  LIBMORPH_ANYALIGN
  #   endif  // !LIBMORPH_ANYALIGN
  # endif  // LIBMORPH_ANYALIGN

  // Define data access macros
  # if defined( LIBMORPH_ANYALIGN )

  #   if !defined( GetWord16 )
  #     define GetWord16( pv )     *(word16_t*)(pv)
  #   endif  // GetWord16
  #   if !defined( GetWord32 )
  #     define GetWord32( pv )     *(word32_t*)(pv)
  #   endif  // GetWord32

  # else

  #   if !defined( GetWord16 )
  #     define GetWord16( pv )  (word16_t)( ((byte08_t*)(pv))[0] | (((byte08_t*)(pv))[1] << 8) )
  #   endif  // GetWord16
  #   if !defined( GetWord32 )
  #     define GetWord32( pv )  (word32_t)( ((byte08_t*)(pv))[0] | (((byte08_t*)(pv))[1] << 8)  \
                                         | (((byte08_t*)(pv))[2] << 16) | (((byte08_t*)(pv))[3] << 24))
  #   endif  // GetWord32

  # endif // macros definitions

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

  # define wfPostSt   0x8000       /* Stem has post-text definition          */
  # define wfMixTab   0x4000       /* Stem has mix-table reference           */
  # define wfFlexes   0x2000       /* Stem has flex-table reference          */
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

  inline  word32_t  getserial( const byte08_t*& p )
  {
    byte08_t  bfetch = *p++;
    word32_t  serial = bfetch & ~0x80;
    int       nshift = 1;

    while ( (bfetch & 0x80) != 0 )
      serial |= (((word32_t)(bfetch = *p++) & ~0x80) << (nshift++ * 7));
    return serial;
  }

  template <class T>
  inline  word16_t  getword16( const T*& p )
  {
    word16_t  v = *(word16_t*)p;
      p = (T*)(sizeof(word16_t) + (char*)p);
    return v;
  }

  inline  bool      haslevels( byte08_t b )
  {
    return (b & (ffNNext | ffONext)) != 0;
  }

  inline  bool      haslevels( const byte08_t* p )
  {
    return (*p & (ffNNext | ffONext)) != 0;
  }

  inline  void      invertstr( byte08_t* t, byte08_t* e )
  {
    while ( t < e ) {  byte08_t c = *t;  *t++ = *--e;  *e = c;  }
  }

  inline  size_t    lexkeylen( byte08_t* p, unsigned nlexid )
  {
    byte08_t* o = p;

    do *p++ = nlexid & 0xff;
      while ( (nlexid >>= 8) != 0 );
    invertstr( o, p );
      return p - o;
  }

  // Функции доступа к таблицам окончаний

  inline  word16_t  GetFlexInfo( const void* item )   // extract grammatical information;
    {  return GetWord16( ((char*)item) + 1 ); }

  inline  word16_t  GetFlexNext( const void* item )
    {  return GetWord16( ((char*)item) + 4 + ((char*)item)[3]  ); }

  // Макроопределения для вычмсления легальной ступени чередования

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
    int   mixType = (stemInfo & 0x0300) >> 8;

    return ( mixType == 0 ? GetVerbMixPowerType1( gramInfo ) :
           ( mixType == 1 ? GetVerbMixPowerType2( gramInfo ) :
           ( mixType == 2 ? GetVerbMixPowerType3( gramInfo ) : 1 ) ) );
  }

    #define MascNotAnimMixPower(GramInfo)   \
      ((((GramInfo & (gfFormMask | gfMultiple)) == 0) || ((GramInfo & (gfFormMask | gfMultiple)) == (3 << 12))) ? 1 : 2)

    #define MascAnimateMixPower(GramInfo)   \
      (((GramInfo & (gfFormMask | gfMultiple)) == 0) ? 1 : 2)

    #define MascMixedMixPower(GramInfo, AFlags)   \
      ((((AFlags) & afAnimated) != 0) ? MascAnimateMixPower((GramInfo)) :  \
        MascNotAnimMixPower((GramInfo)))

    #define FemnNotAnimMixPower(GramInfo)   \
      (((GramInfo & (gfFormMask | gfMultiple)) == ((1 << 12) | gfMultiple)) ? 2 : 1)

    #define FemnAnimateMixPower(GramInfo)   \
      ((((GramInfo & (gfFormMask | gfMultiple)) == ((1 << 12) | gfMultiple)) || \
        ((GramInfo & (gfFormMask | gfMultiple)) == ((3 << 12) | gfMultiple))) ? 2 : 1)

    #define FemnMixedMixPower(GramInfo, AFlags)   \
      ((((AFlags) & afAnimated) != 0) ? FemnAnimateMixPower((GramInfo)) :  \
        FemnNotAnimMixPower((GramInfo)))

    #define GetAdjectivMixPower(GramInfo)   \
      ((GramInfo == gfCompared) ? 3 :       \
       ((GramInfo == ((1 << 9) | gfShortOne)) ? 2 : 1))

  inline int  GetMixPower( word16_t stemInfo,
                           word16_t gramInfo,
                           byte08_t aniflags )
  {
    int   idtype = mixTypes[stemInfo & 0x3F];

    return ( idtype == 1 ? GetVerbMixPower( stemInfo, gramInfo ) :
           ( idtype == 2 ? MascNotAnimMixPower( gramInfo ) :
           ( idtype == 3 ? MascAnimateMixPower( gramInfo ) :
           ( idtype == 4 ? MascMixedMixPower( gramInfo, aniflags ) :
           ( idtype == 5 ? FemnNotAnimMixPower( gramInfo ) :
           ( idtype == 6 ? FemnAnimateMixPower( gramInfo ) :
           ( idtype == 7 ? FemnMixedMixPower( gramInfo, aniflags ) :
           ( idtype == 8 ? GetAdjectivMixPower( gramInfo ) : 1 ) ) ) ) ) ) ) );
  }

  // stem access class

  struct  steminfo
  {
    word16_t        wdinfo;
    word16_t        tfoffs;
    word16_t        mtoffs;

  public:     // init
    steminfo& Load( const byte08_t* pclass )
      {
        wdinfo = getword16( pclass );
        tfoffs = (wdinfo & wfFlexes) != 0 || (wdinfo & 0x3f) == 51 ? getword16( pclass ) : 0;
        mtoffs = (wdinfo & wfMixTab) != 0 ? getword16( pclass ) : 0;
        return *this;
      }
  };

  // Некоторые полезные функции, выясняющие часть речи

  inline  bool  IsVerb( word16_t wbInfo )
  {
    return (wbInfo & 0x3F) <= 6;
  }

  inline  bool  IsAdjective( word16_t wbInfo )
  {
    wbInfo &= 0x3F;

    return ( ( wbInfo >= 25 ) && ( wbInfo <= 28 ) )
      || ( wbInfo == 34 ) || ( wbInfo == 36 ) || ( wbInfo == 42 );
  }

  # define MAX_WORD_FORMS  32

# if defined( LIBMORPH_NAMESPACE )
} // end namespace
# endif   // LIBMORPH_NAMESPACE

# endif // _mlmadefs_h_
