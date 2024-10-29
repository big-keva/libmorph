# if !defined( _mlfa1049_h_ )
# define _mlfa1049_h_
# include "mlma1049.h"
# if defined( __cplusplus )
#   include <stdexcept>
# endif

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
# if defined( __cplusplus )
    using string_t = std::basic_string<char>;

  protected:
    template <class T>
    struct  outbuf
    {
      T*      t;
      size_t  l;

    public:
      outbuf(): t( nullptr ), l( 0 ) {}
      template <size_t N>
      outbuf( T (&p)[N] ): t( p ), l( N ) {}
      outbuf( T* p, size_t n ): t( p ), l( n ) {}
    };

    struct  inword
    {
      const char* t;
      size_t      l;

    public:
      inword( const string_t& s ):
        t( s.c_str() ), l( s.length() ) {}
      inword( const char* s, size_t n = (size_t)-1 ):
        t( s ), l( n )  {}
    };

  public:
    struct  SStemInfo: public SStemInfoA
    {
      SStemInfo( const SStemInfoA& s ): SStemInfoA( s )
      {
        if ( plemma != nullptr )
          plemma = (normal = plemma).c_str();
        if ( pgrams != nullptr && ngrams != 0 )
          pgrams = (gramma = { pgrams, pgrams + ngrams }).data();
      }

    protected:
      string_t                normal;   // the normal form
      std::vector<SGramInfo>  gramma;   // the grams buffer
    };

  public:
    int   Lemmatize(
      const inword&             pszstr,
      const outbuf<SStemInfoA>& alemms,
      const outbuf<char>&       aforms,
      const outbuf<SGramInfo>&  agrams, unsigned  dwsets = 0 )
    {
      return Lemmatize(
        pszstr.t, pszstr.l,
        alemms.t, alemms.l,
        aforms.t, aforms.l,
        agrams.t, agrams.l, dwsets );
    }

    auto  Lemmatize( const inword& pszstr, unsigned dwsets = 0 ) -> std::vector<SStemInfo>
    {
      SStemInfoA  astems[0x20];
      char        aforms[0x200];
      SGramInfo   agrams[0x50];
      int         nbuilt = Lemmatize( pszstr, astems, aforms, agrams, dwsets );

      if ( nbuilt >= 0 )
      {
        auto  output = std::vector<SStemInfo>();

        while ( nbuilt-- > 0 )
          output.push_back( astems[output.size()] );

        return output;
      }
      switch ( nbuilt )
      {
        case WORDBUFF_FAILED: throw std::out_of_range( "invalid string passed" );
        case LEMMBUFF_FAILED: throw std::out_of_range( "not enough space in forms buffer" );
        case LIDSBUFF_FAILED: throw std::out_of_range( "not enough space in stems buffer" );
        case GRAMBUFF_FAILED: throw std::out_of_range( "not enough space in grams buffer" );
        default:              throw std::runtime_error( "unknown error" );
      }
    }

    int   BuildForm(
      const outbuf<char>& output,
      const inword&       plemma,
      unsigned            nclass,
      formid_t            idform )
    {
      return BuildForm(
        output.t, output.l,
        plemma.t, plemma.l, nclass, idform );
    }

    int   BuildForm(
      const outbuf<char>& output,
      const SStemInfoA&   stinfo, formid_t  idform )
    {
      return BuildForm( output, { stinfo.plemma, stinfo.ccstem }, stinfo.nclass, idform );
    }

    auto  BuildForm(
      const inword& plemma, unsigned nclass, formid_t idform ) -> std::vector<string_t>
    {
      char  buffer[0x100];
      auto  output = std::vector<string_t>();
      int   nforms = BuildForm( buffer, plemma, nclass, idform );

      for ( auto strptr = buffer; nforms-- > 0; strptr += output.back().length() + 1 )
        output.emplace_back( strptr );

      return output;
    }

    auto  BuildForm( const SStemInfoA& stinfo, formid_t idform ) -> std::vector<string_t>
    {
      return BuildForm( { stinfo.plemma, stinfo.ccstem }, stinfo.nclass, idform );
    }

# endif
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
  int   MLMAPROC        mlfaruLoadCpAPI( IMlfaMb**, const char* codepage );
  int   MLMAPROC        mlfaruLoadWcAPI( IMlfaWc** );

# if defined( __cplusplus )
}
# endif /* __cplusplus */

# endif /* _mlfa1049_h_ */
