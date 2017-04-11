# if !defined( _mlfa1049_h_ )
# define _mlfa1049_h_
# include "mlma1049.h"

// the main interface structure declaration - describes SStemInfo structure
// similar to SLemmInfo but providing stem length instead of lexeme
//
# if !defined( mlfa_lexemerecord_defined )
# define  mlfa_lexemerecord_defined

  typedef struct
  {
    unsigned        ccstem;
    unsigned        nclass;
    const char*     plemma;
    SGramInfo*      pgrams;
    unsigned        ngrams;
    float           weight;
  } SStemInfoA;

  typedef struct
  {
    unsigned        ccstem;
    unsigned        nclass;
    const widechar* plemma;
    SGramInfo*      pgrams;
    unsigned        ngrams;
    float           weight;
  } SStemInfoW;

# endif  // ! mlfa_lexemerecord_defined

# if !defined( mlfa_interface_defined )
# define  mlfa_interface_defined

  MLMA_INTERFACE( IMlfaMb )
    MLMA_METHOD( Attach )( MLMA_VOID ) MLMA_PURE;
    MLMA_METHOD( Detach )( MLMA_VOID ) MLMA_PURE;

    MLMA_METHOD( Lemmatize )( MLMA_THIS
                              const char*   pszstr, size_t    cchstr,
                              SStemInfoA*   pstems, size_t    clexid,
                              char*         plemma, size_t    clemma,
                              SGramInfo*    pgrams, size_t    ngrams,
                              unsigned      dwsets ) MLMA_PURE;
    MLMA_METHOD( BuildForm )( MLMA_THIS
                              char*         output, size_t    cchout,
                              const char*   lpstem, size_t    ccstem,
                              unsigned      nclass, formid_t  idform ) MLMA_PURE;
  MLMA_END;

  MLMA_INTERFACE( IMlfaWc )
    MLMA_METHOD( Attach )( MLMA_VOID ) MLMA_PURE;
    MLMA_METHOD( Detach )( MLMA_VOID ) MLMA_PURE;

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

# if !defined( __cplusplus )

#   define  mlfa_Lemmatize( module, pszstr, cchstr, plexid,   \
            clexid, plemma, clemma, pgrams, ngrams, dwsets )  \
    module->vtbl->Lemmatize( (module), (pszstr), (cchstr),    \
                                       (plexid), (clexid),    \
                                       (plemma), (clemma),    \
                                       (pgrams), (ngrams), (dwsets) )

#   define  mlfa_BuildForm( module, output, cchout, lpstem,   \
            ccstem, nclass, idform )                          \
    module->vtbl->BuildForm( (module), (output), (cchout),    \
                                       (lpstem), (ccstem),    \
                                       (nclass), (idform) )

# endif  // !__cplusplus

# endif  /* !mlfa_interface_defined */

# if defined( __cplusplus )
extern "C" {
# endif /* __cplusplus */

  int   MLMAPROC        mlfaruLoadMbAPI( IMlfaMb** );
  int   MLMAPROC        mlfaruLoadWcAPI( IMlfaWc** );

  short MLMA_API EXPORT mlfaruLemmatize( const char*    lpword,
                                         unsigned short dwsets,
                                         char*          lpLemm,
                                         SStemInfoA*    lpstem,
                                         char*          lpGram,
                                         unsigned short ccLemm,
                                         unsigned short csStem,
                                         unsigned short cbGram );
  short MLMA_API EXPORT mlfaruBuildForm( const char*    lpword,
                                         unsigned       ccstem,
                                         unsigned       nclass,
                                         unsigned short dwsets,
                                         unsigned char  idForm,
                                         char*          lpDest,
                                         unsigned short ccDest );

# if defined( __cplusplus )
}
# endif /* __cplusplus */

# endif /* _mlfa1049_h_ */
