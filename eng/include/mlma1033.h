#if !defined( _mlma1033_h_ )
#define _mlma1033_h_

# if defined( WIN32 )
#   include <pshpack1.h>
# endif

# include <limits.h>

#if !defined( __lexeme_t_defined__ )
#  define  __lexeme_t_defined__
#  if    UINT_MAX  == 0xffffffff       // check if 32-bit platform
     typedef unsigned int  lexeme_t;
#  elif  ULONG_MAX == 0xffffffffUL
     typedef unsigned long lexeme_t;
#  else
#    error Cannot resolve the 32-bit unsigned int type!
#  endif
#endif

#if !defined( lemmatize_errors_defined )
#define lemmatize_errors_defined
  #define LEMMBUFF_FAILED -1  
  #define LIDSBUFF_FAILED -2  
  #define GRAMBUFF_FAILED -3  
  #define WORDBUFF_FAILED -4  
#endif

# if !defined( PAGELOAD_FAILED )
#   define PAGELOAD_FAILED -5       // Error occurs during loading page
# endif  // PAGELOAD_FAILED
# if !defined( PAGELOCK_FAILED )
#   define PAGELOCK_FAILED -6       // Error occurs during locking page
# endif  // PAGELOCK_FAILED

#if !defined( mlma_search_flags_defined )
#define mlma_search_flags_defined
  #define   sfStopAfterFirst  0x0001    // Достаточно одного отождествления
  #define   sfIgnoreCapitals  0x0002    // Плевать на схему капитализации
#endif

#if !defined( mlma_grammarecord_defined )
  #define mlma_grammarecord_defined

  typedef struct
  {
    unsigned char  wInfo;
    unsigned char  iForm;
    unsigned short gInfo;
    unsigned char  other;
  } SGramInfo;

# endif  /* mlma_grammarecord_defined */

#if !defined( EXPORT )
#  if defined( WIN16 )
#    define EXPORT __export
#  else
#    define EXPORT
#  endif // WIN16
#endif // EXPORT

# if defined( WIN16 )
#   define MLMA_API __far __pascal
# elif defined( WIN32 )
#   define MLMA_API __stdcall
# else
#   define MLMA_API
# endif /* MLMA_API definition */

# if defined( __cplusplus )
extern "C" {
# endif /* __cplusplus */

# if !defined( __TEnumWords_prototype_defined__ )
#   define  __TEnumWords_prototype_defined__
      typedef short (MLMA_API* TEnumWords)( lexeme_t lid, void* lpv );
# endif

  short MLMA_API EXPORT mlmaenCheckWord( const char*    lpword,
                                         unsigned short dwsets );
  short MLMA_API EXPORT mlmaenLemmatize( const char*    lpword,
                                         unsigned short dwsets,
                                         char*          lpLemm,
                                         lexeme_t*      lpLIDs,
                                         char*          lpGram,
                                         unsigned short ccLemm,
                                         unsigned short cdwLID,
                                         unsigned short cbGram );
  short MLMA_API EXPORT mlmaenBuildForm( const char*    lpword,
                                         lexeme_t       nLexID,
                                         unsigned short dwsets,
                                         unsigned char  idForm,
                                         char*          lpDest,
                                         unsigned short ccDest );
  short MLMA_API EXPORT mlmaenEnumWords( TEnumWords enumproc,
                                         void*      lpvparam );
  short MLMA_API EXPORT mlmaenCheckHelp( const char* lpword,
                                         char*       lpList );
  short MLMA_API EXPORT mlmaenListForms( lexeme_t       nlexid,
                                         char*          lpbuff,
                                         unsigned       ccbuff );
/* toolhelper functions */
  short MLMA_API EXPORT mlmaenGetWordInfo( lexeme_t         nLexID,
                                           unsigned char*   lpinfo );
  short MLMA_API EXPORT mlmaenGetClassRef( lexeme_t         nLexID,
                                           unsigned short*  rclass );
  short MLMA_API EXPORT mlmaenGetTypeInfo( unsigned short   iclass,
                                           unsigned char*   lpinfo );

#if defined( __cplusplus )
}
#endif // __cplusplus

# if defined( WIN32 )
#   include <poppack.h>
# endif

# endif /* _mlma1033_h_ */
