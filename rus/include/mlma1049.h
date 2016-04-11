# if !defined( _mlma1049_h_ )
# define _mlma1049_h_

# if defined( WIN32 )
#   include <pshpack1.h>
# endif

# include <limits.h>
# include <stdint.h>

# if !defined( __widechar_defined__ )
# define  __widechar_defined__
#   if defined(WCHAR_MAX) && (WCHAR_MAX >> 16) == 0
    typedef wchar_t         widechar;
#   else
    typedef unsigned short  widechar;
#   endif  // size
# endif  // __widechar_defined__

# if !defined( __lexeme_t_defined__ )
#   define  __lexeme_t_defined__
#   if    UINT_MAX  == 0xffffffff       // check if 32-bit platform
      typedef unsigned int  lexeme_t;
#   elif  ULONG_MAX == 0xffffffffUL
      typedef unsigned long lexeme_t;
#   else
#     error Cannot resolve the 32-bit unsigned int type!
#   endif
# endif

# if !defined( lemmatize_errors_defined )
#   define lemmatize_errors_defined
#   define LEMMBUFF_FAILED -1
#   define LIDSBUFF_FAILED -2
#   define GRAMBUFF_FAILED -3
#   define WORDBUFF_FAILED -4
# endif

# if !defined( PAGELOAD_FAILED )
#   define PAGELOAD_FAILED -5
# endif  // PAGELOAD_FAILED
# if !defined( PAGELOCK_FAILED )
#   define PAGELOCK_FAILED -6
# endif  // PAGELOCK_FAILED

# if !defined( mlma_search_flags_defined )
#   define mlma_search_flags_defined
#   define sfStopAfterFirst  0x0001    // Достаточно одного отождествления
#   define sfIgnoreCapitals  0x0002    // Плевать на схему капитализации
# endif

# if !defined( mlmaru_search_flags_defined )
#   define mlmaru_search_flags_defined
#   define sfHardForms       0x0004    // Затрудненные словоформы
#   define sfDefisWordTails  0x0008    // Обрабатывать соед. гласные
#   define nfAdjVerbs        0x0100    // Нормализация до причастия
# endif

# if !defined( mlma_grammarecord_defined )
#   define mlma_grammarecord_defined

  typedef struct
  {
    unsigned char   wInfo;
    unsigned char   iForm;
    unsigned short  gInfo;
    unsigned char   other;
  } SGramInfo;

# endif  /* mlma_grammarecord_defined */

# if !defined( mlma_lexemerecord_defined )
# define  mlma_lexemerecord_defined

  typedef struct
  {
    lexeme_t        nlexid;
    const char*     plemma;
    SGramInfo*      pgrams;
    unsigned        ngrams;
  } SLemmInfoA;

  typedef struct
  {
    lexeme_t        nlexid;
    const widechar* plemma;
    SGramInfo*      pgrams;
    unsigned        ngrams;
  } SLemmInfoW;

# endif  // ! mlma_lexemerecord_defined

/**************************************************************************/
/*            Грамматическая информация из таблиц окончаний               */
/*                                                                        */
/*                                *******                                 */
/*                                                                        */
/* Склоняющиеся слова, младшие 2 байта. Грамматическая информация.        */
/*                                                                        */
/* @+++++++ ++++++++ - возвратная форма (глаголы и прилагательные)        */
/* +@@@++++ ++++++++ - падеж                                              */
/* ++++@+++ ++++++++ - число                                              */
/* +++++@@+ ++++++++ - род                                                */
/* +++++++@ ++++++++ - краткая форма (для прил. и прич.)                  */
/* ++++++++ @+++++++ - сравнительная степень                              */
/* ++++++++ +@@+++++ - причастие, деепричастие, страдательное причастие   */
/* ++++++++ +++@@+++ - лицо (для глаголов и их форм)                      */
/* ++++++++ +++++@@@ - временн'ая характеристика глагольной формы         */
/*                                                                        */
/* Ниже документирован дополнительный "информационный" байт, сопутству-   */
/* ющий каждому элементу таблиц окончаний:                                */
/*                                                                        */
/*          +++++++@ - окончание для одушевленного имени                  */
/*          ++++++@+ - окончание для неодушевленного имени                */
/**************************************************************************/

# if !defined( russian_gram_info_defined )
#   define russian_gram_info_defined

#   define wfMultiple     0x40          /* Множественное число   */

#   define afAnimated     0x01          /*                       */
#   define afNotAlive     0x02          /*                       */
#   define afHardForm     0x04          /* Затрудненная форма    */
#   define afJoiningC     0x08          /* Соед. гласная         */

#   define gfRetForms     0x8000        /* Возвратная форма      */
#   define gfFormMask     0x7000        /* Маска для падежей     */
#   define gfMultiple     0x0800        /* Множественное число   */
#   define gfGendMask     0x0600        /* Род                   */
#   define gfShortOne     0x0100        /* Краткая форма         */
#   define gfCompared     0x0080        /* Сравнительная степень */
#   define gfVerbForm     0x0060        /* Причастная информация */
#   define gfAdverb       0x0040        /* Наречие от прил.      */
#   define gfVerbFace     0x0018        /* Лицо                  */
#   define gfVerbTime     0x0007        /* Время                 */

#   define vtInfinitiv    0x0001        /* Инфинитив             */
#   define vtImperativ    0x0002        /* Повелит. наклонение   */
#   define vtFuture       0x0003        /* Будущее время         */
#   define vtPresent      0x0004        /* Настоящее время       */
#   define vtPast         0x0005        /* Прошедшее время       */

#   define vbFirstFace    0x0008        /* Первое лицо           */
#   define vbSecondFace   0x0010        /* Второе лицо           */
#   define vbThirdFace    0x0018        /* Третье лицо           */

#   define vfVerb         0x0000        /* Глагольная форма      */
#   define vfVerbActive   0x0020        /* Действит. причастие   */
#   define vfVerbPassiv   0x0040        /* Страд. причастие      */
#   define vfVerbDoing    0x0060        /* Деепричастие          */

# endif  /* russian_gram_info_defined */

# if !defined( EXPORT )
#   if defined( WIN16 )
#     define EXPORT __export
#   else
#     define EXPORT
#   endif /* WIN16 */
# endif /* EXPORT */

# if !defined( MLMA_API )
#   if defined( WIN16 )
#     define MLMA_API __far __pascal
#   elif defined( _WIN32 )
#     define MLMA_API __stdcall
#   else
#     define MLMA_API
#   endif
# endif /* MLMA_API definition */

# if !defined( MLMAPROC )
#   if defined( WIN16 )
#     define MLMAPROC __far __pascal __export
#   elif defined( _WIN32 )
#     define MLMAPROC __stdcall
#   else
#     if defined( __GNUC__ ) && !defined( __LP64__ )
#       define MLMAPROC __attribute__((stdcall))
#     else
#       define MLMAPROC
#     endif
#   endif
# endif /* MLMA_API definition */

/*================ The new API for the morphological analyser ================*/
/* uses common a-la-COM API which would look in the same way on both C && C++ */
/* compilers.                                                                 */
/*============================================================================*/

# if defined( __cplusplus )

#   define  MLMA_INTERFACE( iface ) \
    struct  iface {

#   define  MLMA_METHOD( method )   \
    virtual int MLMAPROC  method

#   define  MLMA_THIS
#   define  MLMA_PURE = 0

#   define  MLMA_END  }

# else

#   define  MLMA_INTERFACE( iface )     \
    struct  iface##_vtbl;               \
                                        \
    typedef struct iface                \
    {                                   \
      const struct iface##_vtbl* vtbl;  \
    } iface;                            \
                                        \
    struct  iface##_vtbl                \
    {

#   define  MLMA_METHOD( method )   \
    int (MLMAPROC *method)

#   define  MLMA_THIS   void*,
#   define  MLMA_PURE

#   define  MLMA_END  }

# endif  // __cplusplus

# if !defined( mlma_interface_defined )
# define  mlma_interface_defined

  MLMA_INTERFACE( IMlmaMb )
    MLMA_METHOD( SetLoCase )( MLMA_THIS
                              char*           pszstr, unsigned  cchstr )  MLMA_PURE;
    MLMA_METHOD( SetUpCase )( MLMA_THIS
                              char*           pszstr, unsigned  cchstr )  MLMA_PURE;
    MLMA_METHOD( CheckWord )( MLMA_THIS
                              const char*     pszstr, unsigned  cchstr,
                              unsigned        dwsets ) MLMA_PURE;
    MLMA_METHOD( Lemmatize )( MLMA_THIS
                              const char*     pszstr, unsigned  cchstr,
                              SLemmInfoA*     plexid, unsigned  clexid,
                              char*           plemma, unsigned  clemma,
                              SGramInfo*      pgrams, unsigned  ngrams,
                              unsigned        dwsets ) MLMA_PURE;
    MLMA_METHOD( BuildForm )( MLMA_THIS
                              char*           output, unsigned      cchout,
                              lexeme_t        nlexid, unsigned char idform ) MLMA_PURE;
    MLMA_METHOD( FindForms )( MLMA_THIS
                              char*           output, unsigned      cchout,
                              const char*     pszstr, unsigned      cchstr,
                              unsigned char   idform ) MLMA_PURE;
    MLMA_METHOD( CheckHelp )( MLMA_THIS
                              char*           output, unsigned      cchout,
                              const char*     pszstr, unsigned      cchstr ) MLMA_PURE;
  MLMA_END;

  MLMA_INTERFACE( IMlmaWc )
    MLMA_METHOD( SetLoCase )( MLMA_THIS
                              widechar*       pszstr, unsigned  cchstr )  MLMA_PURE;
    MLMA_METHOD( SetUpCase )( MLMA_THIS
                              widechar*       pwsstr, unsigned  cchstr )  MLMA_PURE;
    MLMA_METHOD( CheckWord )( MLMA_THIS
                              const widechar* pszstr, unsigned  cchstr,
                              unsigned        dwsets ) MLMA_PURE;
    MLMA_METHOD( Lemmatize )( MLMA_THIS
                              const widechar* pszstr, unsigned  cchstr,
                              SLemmInfoW*     plexid, unsigned  clexid,
                              widechar*       plemma, unsigned  clemma,
                              SGramInfo*      pgrams, unsigned  ngrams,
                              unsigned        dwsets ) MLMA_PURE;
    MLMA_METHOD( BuildForm )( MLMA_THIS
                              widechar*       output, unsigned      cchout,
                              lexeme_t        nlexid, unsigned char idform ) MLMA_PURE;
    MLMA_METHOD( FindForms )( MLMA_THIS
                              widechar*       output, unsigned      cchout,
                              const widechar* pszstr, unsigned      cchstr,
                              unsigned char   idform ) MLMA_PURE;
    MLMA_METHOD( CheckHelp )( MLMA_THIS
                              widechar*       output, unsigned      cchout,
                              const widechar* pwsstr, unsigned      cchstr ) MLMA_PURE;
  MLMA_END;

# if !defined( __cplusplus )

#   define  mlma_CheckWord( module, pszstr, cchstr, dwsets )    \
      module->vtbl->CheckWord( (module), (pszstr), (cchstr), (dwsets) )

#   define  mlma_Lemmatize( module, pszstr, cchstr, plexid,     \
            clexid, plemma, clemma, pgrams, ngrams, dwsets )    \
      module->vtbl->Lemmatize( (module), (pszstr), (cchstr),    \
                                         (plexid), (clexid),    \
                                         (plemma), (clemma),    \
                                         (pgrams), (ngrams), (dwsets) )

#   define  mlma_BuildForm( module, output, cchout, nlexid, idform )  \
      module->vtbl->BuildForm( (module), (output), (cchout),          \
                                         (nlexid), (idform) )

#   define  mlma_FindForms( module, output, cchout, pszstr, cchstr, idform )  \
      module->vtbl->FindForms( (module), (output), (cchout),                  \
                                         (pszstr), (cchstr), (idform) )

#   define  mlma_CheckHelp( module, output, cchout, pszstr, cchstr )  \
      module->vtbl->CheckHelp( (module), (output), (cchout),          \
                                         (pszstr), (cchstr) )

# endif  // !__cplusplus

# endif  /* !mlma_interface_defined */

# if defined( __cplusplus )
extern "C" {
# endif /* __cplusplus */

# if !defined( __TEnumWords_prototype_defined__ )
#   define  __TEnumWords_prototype_defined__
      typedef short (MLMA_API* TEnumWords)( lexeme_t lid, void* lpv );
# endif

  int   MLMAPROC        mlmaruLoadMbAPI( IMlmaMb** );
  int   MLMAPROC        mlmaruLoadWcAPI( IMlmaWc** );

  short MLMA_API EXPORT mlmaruCheckWord( const char*    lpword,
                                         unsigned short dwsets );
  short MLMA_API EXPORT mlmaruLemmatize( const char*    lpword,
                                         unsigned short dwsets,
                                         char*          lpLemm,
                                         lexeme_t*      lpLIDs,
                                         char*          lpGram,
                                         unsigned short ccLemm,
                                         unsigned short cdwLID,
                                         unsigned short cbGram );
  short MLMA_API EXPORT mlmaruBuildForm( const char*    lpword,
                                         lexeme_t       nLexID,
                                         unsigned short dwsets,
                                         unsigned char  idForm,
                                         char*          lpDest,
                                         unsigned short ccDest );
  short MLMA_API EXPORT mlmaruBuildFormGI( const char*    lpword,
                                           lexeme_t       nLexID,
                                           unsigned short dwsets,
                                           unsigned short grInfo,
                                           unsigned char  bFlags,
                                           char*          lpDest,
                                           unsigned short ccDest );
  short MLMA_API EXPORT mlmaruEnumWords( TEnumWords enumproc,
                                         void*      lpvparam );
  short MLMA_API EXPORT mlmaruCheckHelp( const char* lpword,
                                         char*       lpList );
  short MLMA_API EXPORT mlmaruListForms( lexeme_t       nlexid,
                                         char*          lpbuff,
                                         unsigned       ccbuff );
/* toolhelper functions */
  short MLMA_API EXPORT mlmaruGetWordInfo( lexeme_t         nLexID,
                                           unsigned char*   lpinfo );

# if defined( __cplusplus )
}
# endif /* __cplusplus */

# if defined( WIN32 )
#   include <poppack.h>
# endif

# endif /* _mlma1049_h_ */
