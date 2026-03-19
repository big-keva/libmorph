/******************************************************************************

    libmorph - dictionary-based morphological analysers.

    Copyright (c) 1994-2026 Andrew Kovalenko aka Keva

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
      email: keva@rambler.ru
      Phone: +7(495)648-4058, +7(926)513-2991, +7(707)129-1418

******************************************************************************/
# if !defined( _libmorph_api_h_ )
# define _libmorph_api_h_
# include <limits.h>
# include <stdint.h>
# include <stddef.h>

# if !defined( __cplusplus )
#   include <uchar.h>
# endif

# if !defined( __widechar_defined__ )
# define  __widechar_defined__
#   if defined(WCHAR_MAX) && (WCHAR_MAX >> 16) == 0
    typedef wchar_t   widechar;
#   else
    typedef char16_t  widechar;
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

# if !defined( __formid_t_defined__ )
#   define  __formid_t_defined__
    typedef unsigned char   formid_t;
# endif // __formid_t_defined__

# if !defined( mlma_strmatch_defined )
#   define  mlma_strmatch_defined
  typedef struct
  {
    union
    {
      const char*     sz;
      const widechar* ws;
    };
    size_t            cc;
    formid_t          id;
  } SStrMatch;
# endif // mlma_strmatch_defined

# if !defined( lemmatize_errors_defined )
#   define lemmatize_errors_defined
#   define LEMMBUFF_FAILED -1
#   define LIDSBUFF_FAILED -2
#   define GRAMBUFF_FAILED -3
#   define WORDBUFF_FAILED -4
#   define ARGUMENT_FAILED -5
#   define BUFFER_OVERFLOW -6
# endif

# if !defined( mlma_search_flags_defined )
#   define mlma_search_flags_defined
#   define sfStopAfterFirst  0x0001    // Достаточно одного отождествления
#   define sfIgnoreCapitals  0x0002    // Плевать на схему капитализации
# endif

# if !defined( mlma_grammarecord_defined )
#   define mlma_grammarecord_defined

  typedef struct
  {
    uint16_t  wdInfo;
    formid_t  idForm;
    uint16_t  grInfo;
    uint8_t   bFlags;
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

/*
 * The main interface structure declaration - describes SStemInfo structure
 * similar to SLemmInfo but providing stem length instead of lexeme
 */
# if !defined( mlfa_stemrecord_defined )
# define  mlfa_stemrecord_defined

  typedef struct
  {
    const char*     plemma;
    unsigned        ccstem;
    unsigned        nclass;
    SGramInfo*      pgrams;
    unsigned        ngrams;
    float           weight;
  } SStemInfoA;

  typedef struct
  {
    const widechar* plemma;
    unsigned        ccstem;
    unsigned        nclass;
    SGramInfo*      pgrams;
    unsigned        ngrams;
    float           weight;
  } SStemInfoW;

# endif  // ! mlfa_lexemerecord_defined

# if !defined( EXPORT )
#   if defined( WIN16 )
#     define EXPORT __export
#   else
#     define EXPORT
#   endif /* WIN16 */
# endif /* EXPORT */

# if !defined( MLMAPROC )
#   if defined( WIN16 )
#     define MLMAPROC __far __pascal __export
#   elif defined( _WIN32 )
#     define MLMAPROC __stdcall
#   else
#     define MLMAPROC
#   endif
# endif

/*================ The new API for the morphological analyser ================*/
/* uses common a-la-COM API which would look in the same way on both C && C++ */
/* compilers.                                                                 */
/*============================================================================*/

# if !defined( MLMA_INTERFACE )
#   if defined( __cplusplus )
#     define  MLMA_INTERFACE( iface ) \
      struct  iface {

#     define  MLMA_METHOD( method )   \
      virtual int MLMAPROC  method

#     define  MLMA_THIS
#     define  MLMA_VOID
#     define  MLMA_PURE = 0

#     define  MLMA_END  }

#   else

#     define  MLMA_INTERFACE( iface )     \
      struct  iface##_vtbl;               \
                                          \
      typedef struct iface                \
      {                                   \
        const struct iface##_vtbl* vtbl;  \
      } iface;                            \
                                          \
      struct  iface##_vtbl                \
      {

#     define  MLMA_METHOD( method )   \
      int (MLMAPROC *(method))

#     define  MLMA_THIS   void*,
#     define  MLMA_VOID   void*
#     define  MLMA_PURE

#     define  MLMA_END  }

#   endif  // __cplusplus
# endif  // MLMA_INTERFACE

# if !defined( mlma_interface_defined )
# define  mlma_interface_defined

  MLMA_INTERFACE( IMlmaMatch )
    MLMA_METHOD( Attach )( MLMA_VOID ) MLMA_PURE;
    MLMA_METHOD( Detach )( MLMA_VOID ) MLMA_PURE;

    MLMA_METHOD( AddLexeme )( MLMA_THIS
      lexeme_t  nlexid,
      int       nforms, const SStrMatch* pforms ) MLMA_PURE;
  MLMA_END;

  MLMA_INTERFACE( IMlmaMb )
    MLMA_METHOD( Attach )( MLMA_VOID ) MLMA_PURE;
    MLMA_METHOD( Detach )( MLMA_VOID ) MLMA_PURE;

    MLMA_METHOD( CheckWord )( MLMA_THIS
      const char*     pszstr, size_t  cchstr,
      unsigned        dwsets                  ) MLMA_PURE;
    MLMA_METHOD( Lemmatize )( MLMA_THIS
      const char*     pszstr, size_t  cchstr,
      SLemmInfoA*     plexid, size_t  clexid,
      char*           plemma, size_t  clemma,
      SGramInfo*      pgrams, size_t  ngrams,
      unsigned        dwsets                  ) MLMA_PURE;
    MLMA_METHOD( BuildForm )( MLMA_THIS
      char*           output, size_t    cchout,
      lexeme_t        nlexid, formid_t  idform ) MLMA_PURE;
    MLMA_METHOD( FindForms )( MLMA_THIS
      char*           output, size_t    cchout,
      const char*     pszstr, size_t    cchstr,
      formid_t        idform, unsigned  dwsets ) MLMA_PURE;
    MLMA_METHOD( GetWdInfo )( MLMA_THIS
      unsigned char*  pwinfo, lexeme_t  nlexid ) MLMA_PURE;
    MLMA_METHOD( FindMatch )( MLMA_THIS
      IMlmaMatch*     pienum,
      const char*     pszstr, size_t    cchstr ) MLMA_PURE;
  MLMA_END;

  MLMA_INTERFACE( IMlmaWc )
    MLMA_METHOD( Attach )( MLMA_VOID ) MLMA_PURE;
    MLMA_METHOD( Detach )( MLMA_VOID ) MLMA_PURE;

    MLMA_METHOD( CheckWord )( MLMA_THIS
      const widechar* pszstr, size_t    cchstr,
      unsigned        dwsets                    ) MLMA_PURE;
    MLMA_METHOD( Lemmatize )( MLMA_THIS
      const widechar* pszstr, size_t    cchstr,
      SLemmInfoW*     plexid, size_t    clexid,
      widechar*       plemma, size_t    clemma,
      SGramInfo*      pgrams, size_t    ngrams,
      unsigned        dwsets                    ) MLMA_PURE;
    MLMA_METHOD( BuildForm )( MLMA_THIS
      widechar*       output, size_t    cchout,
      lexeme_t        nlexid, formid_t  idform ) MLMA_PURE;
    MLMA_METHOD( FindForms )( MLMA_THIS
      widechar*       output, size_t    cchout,
      const widechar* pszstr, size_t    cchstr,
      formid_t        idform, unsigned  dwsets ) MLMA_PURE;
    MLMA_METHOD( GetWdInfo )( MLMA_THIS
      unsigned char*  pwinfo, lexeme_t  nlexid ) MLMA_PURE;
    MLMA_METHOD( FindMatch )( MLMA_THIS
      IMlmaMatch*     pienum,
      const widechar* pszstr, size_t    cchstr ) MLMA_PURE;
  MLMA_END;

  MLMA_INTERFACE( IMlfaMb )
    MLMA_METHOD( Attach )( MLMA_VOID ) MLMA_PURE;
    MLMA_METHOD( Detach )( MLMA_VOID ) MLMA_PURE;

    MLMA_METHOD( GetWdInfo )( MLMA_THIS
      unsigned char*  pwinfo, unsigned  uclass ) MLMA_PURE;
    MLMA_METHOD( GetModels )( MLMA_THIS
      char*           models, size_t    buflen,
      unsigned        uclass ) MLMA_PURE;
    MLMA_METHOD( Lemmatize )( MLMA_THIS
      const char*     pszstr, size_t    cchstr,
      SStemInfoA*     pstems, size_t    clexid,
      char*           plemma, size_t    clemma,
      SGramInfo*      pgrams, size_t    ngrams,
      unsigned        dwsets ) MLMA_PURE;
    MLMA_METHOD( BuildForm )( MLMA_THIS
      char*           output, size_t    cchout,
      const char*     lpstem, size_t    ccstem,
      unsigned        nclass, formid_t  idform ) MLMA_PURE;
  MLMA_END;

  MLMA_INTERFACE( IMlfaWc )
    MLMA_METHOD( Attach )( MLMA_VOID ) MLMA_PURE;
    MLMA_METHOD( Detach )( MLMA_VOID ) MLMA_PURE;

    MLMA_METHOD( GetWdInfo )( MLMA_THIS
      unsigned char*  pwinfo, unsigned  uclass ) MLMA_PURE;
    MLMA_METHOD( GetModels )( MLMA_THIS
      widechar*       models, size_t    buflen,
      unsigned        uclass ) MLMA_PURE;
    MLMA_METHOD( Lemmatize )( MLMA_THIS
      const widechar* pszstr, size_t    cchstr,
      SStemInfoW*     pstems, size_t    clexid,
      widechar*       plemma, size_t    clemma,
      SGramInfo*      pgrams, size_t    ngrams,
      unsigned        dwsets ) MLMA_PURE;
    MLMA_METHOD( BuildForm )( MLMA_THIS
      widechar*       output, size_t    cchout,
      const widechar* lpstem, size_t    ccstem,
      unsigned        nclass, formid_t  idform ) MLMA_PURE;
  MLMA_END;

# endif  /* !mlma_interface_defined */

# define LIBMORPH_API_4_MAGIC   "libmorph.api.v4"
# define LIBFUZZY_API_4_MAGIC   "libfuzzy.api.v4"

# if !defined( mlma_getapi_defined )
# define mlma_getapi_defined

typedef int (MLMAPROC *libmorphGetAPI)( const char* apiMagic, void** ppvAPI );

# endif  /* !mlma_getapi_defined */

# endif /* _libmorph_api_h_ */
