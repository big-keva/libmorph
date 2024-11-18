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

  MLMA_INTERFACE( IMatchStemMb )
    MLMA_METHOD( Attach )( MLMA_VOID ) MLMA_PURE;
    MLMA_METHOD( Detach )( MLMA_VOID ) MLMA_PURE;

    MLMA_METHOD( Add )( MLMA_THIS
      const char* plemma,
      size_t      clemma,
      size_t      cchstr,
      unsigned    uclass,
      size_t      nforms, const SStrMatch* pforms ) MLMA_PURE;
  MLMA_END;

  MLMA_INTERFACE( IMatchStemWc )
    MLMA_METHOD( Attach )( MLMA_VOID ) MLMA_PURE;
    MLMA_METHOD( Detach )( MLMA_VOID ) MLMA_PURE;

    MLMA_METHOD( Add )( MLMA_THIS
      const widechar* plemma,
      size_t          clemma,
      size_t          cchstr,
      unsigned        uclass,
      size_t          nforms, const SStrMatch* pforms ) MLMA_PURE;
  MLMA_END;

  MLMA_INTERFACE( IMlfaMb )
    MLMA_METHOD( Attach )( MLMA_VOID ) MLMA_PURE;
    MLMA_METHOD( Detach )( MLMA_VOID ) MLMA_PURE;

    MLMA_METHOD( GetWdInfo )( MLMA_THIS
      unsigned char*  pwinfo, unsigned  uclass ) MLMA_PURE;
    MLMA_METHOD( GetSample )( MLMA_THIS
      char*           sample, size_t    buflen,
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
    MLMA_METHOD( FindMatch )( MLMA_THIS
      IMatchStemMb*   pienum,
      const char*     pszstr, size_t    cchstr ) MLMA_PURE;

# if defined( __cplusplus )
    template <class T>
    using outbuf = MlmaTraits<char>::outbuf<T>;
    using inword = MlmaTraits<char>::inword;
    using string = MlmaTraits<char>::string;

  public:

    auto  GetWdInfo( lexeme_t nlexid ) -> uint8_t
    {
      uint8_t wdinfo;

      return GetWdInfo( &wdinfo, nlexid ) > 0 ? wdinfo : (uint8_t)-1;
    }

    int   GetSample( const outbuf<char>& sample, unsigned uclass )
    {
      return GetSample( sample.t, sample.l, uclass );
    }

    auto  GetSample( unsigned uclass ) -> string
    {
      char  sample[64];

      return GetSample( sample, uclass ) > 0 ? sample : "";
    }

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

    auto  Lemmatize( const inword& pszstr, unsigned dwsets = 0 ) -> std::vector<SLemmInfo<SStemInfoA>>
    {
      SStemInfoA  astems[0x20];
      char        aforms[0x200];
      SGramInfo   agrams[0x50];
      int         nbuilt = Lemmatize( pszstr, astems, aforms, agrams, dwsets );

      if ( nbuilt >= 0 )
      {
        auto  output = std::vector<SLemmInfo<SStemInfoA>>();

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
      const inword& plemma, unsigned nclass, formid_t idform ) -> std::vector<std::basic_string<char>>
    {
      char  buffer[0x100];
      auto  output = std::vector<std::basic_string<char>>();
      int   nforms = BuildForm( buffer, plemma, nclass, idform );

      for ( auto strptr = buffer; nforms-- > 0; strptr += output.back().length() + 1 )
        output.emplace_back( strptr );

      return output;
    }

    auto  BuildForm( const SStemInfoA& stinfo, formid_t idform ) -> std::vector<std::basic_string<char>>
    {
      return BuildForm( { stinfo.plemma, stinfo.ccstem }, stinfo.nclass, idform );
    }

    int   FindMatch( IMatchStemMb* pmatch, const inword& pszstr )
    {
      return FindMatch( pmatch, pszstr.t, pszstr.l );
    }
# endif
  MLMA_END;

  MLMA_INTERFACE( IMlfaWc )
    MLMA_METHOD( Attach )( MLMA_VOID ) MLMA_PURE;
    MLMA_METHOD( Detach )( MLMA_VOID ) MLMA_PURE;

    MLMA_METHOD( GetWdInfo )( MLMA_THIS
      unsigned char*  pwinfo, unsigned  uclass ) MLMA_PURE;
    MLMA_METHOD( GetSample )( MLMA_THIS
      widechar*       sample, size_t    buflen,
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
    MLMA_METHOD( FindMatch )( MLMA_THIS
      IMatchStemWc*   pienum,
      const widechar* pszstr, size_t    cchstr ) MLMA_PURE;
# if defined( __cplusplus )
    template <class T>
    using outbuf = MlmaTraits<char>::outbuf<T>;
    using inword = MlmaTraits<widechar>::inword;
    using string = MlmaTraits<widechar>::string;

  public:

    auto  GetWdInfo( lexeme_t nlexid ) -> uint8_t
    {
      uint8_t wdinfo;

      return GetWdInfo( &wdinfo, nlexid ) > 0 ? wdinfo : 0;
    }

    int   GetSample( const outbuf<widechar>& sample, unsigned uclass )
    {
      return GetSample( sample.t, sample.l, uclass );
    }

    auto  GetSample( unsigned uclass ) -> string
    {
      widechar  sample[64];

      return GetSample( sample, uclass ) > 0 ? sample : string{};
    }

    int   Lemmatize(
      const inword&             pszstr,
      const outbuf<SStemInfoW>& alemms,
      const outbuf<widechar>&   aforms,
      const outbuf<SGramInfo>&  agrams, unsigned  dwsets = 0 )
    {
      return Lemmatize(
        pszstr.t, pszstr.l,
        alemms.t, alemms.l,
        aforms.t, aforms.l,
        agrams.t, agrams.l, dwsets );
    }

    auto  Lemmatize( const inword& pszstr, unsigned dwsets = 0 ) -> std::vector<SLemmInfo<SStemInfoW>>
    {
      SStemInfoW  astems[0x20];
      widechar    aforms[0x200];
      SGramInfo   agrams[0x50];
      int         nbuilt = Lemmatize( pszstr, astems, aforms, agrams, dwsets );

      if ( nbuilt >= 0 )
      {
        auto  output = std::vector<SLemmInfo<SStemInfoW>>();

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
      const outbuf<widechar>& output,
      const inword&           plemma,
      unsigned                nclass,
      formid_t                idform )
    {
      return BuildForm(
        output.t, output.l,
        plemma.t, plemma.l, nclass, idform );
    }

    int   BuildForm(
      const outbuf<widechar>& output,
      const SStemInfoW&       stinfo, formid_t  idform )
    {
      return BuildForm( output, { stinfo.plemma, stinfo.ccstem }, stinfo.nclass, idform );
    }

    auto  BuildForm(
      const inword& plemma, unsigned nclass, formid_t idform ) -> std::vector<string>
    {
      widechar  buffer[0x100];
      auto      output = std::vector<string>();
      int       nforms = BuildForm( buffer, plemma, nclass, idform );

      for ( auto strptr = buffer; nforms-- > 0; strptr += output.back().length() + 1 )
        output.emplace_back( strptr );

      return output;
    }

    auto  BuildForm( const SStemInfoW& stinfo, formid_t idform ) -> std::vector<string>
    {
      return BuildForm( { stinfo.plemma, stinfo.ccstem }, stinfo.nclass, idform );
    }

    int   FindMatch( IMatchStemWc* pmatch, const inword& pszstr )
    {
      return FindMatch( pmatch, pszstr.t, pszstr.l );
    }

# endif // __cplusplus

  MLMA_END;

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
