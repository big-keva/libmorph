/******************************************************************************

    libfuzzyrus - fuzzy morphological analyser for Russian.

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

  MLMA_INTERFACE( IMlfaMatch )
    MLMA_METHOD( Attach )( MLMA_VOID ) MLMA_PURE;
    MLMA_METHOD( Detach )( MLMA_VOID ) MLMA_PURE;

    MLMA_METHOD( AddLexeme )( MLMA_THIS
      const void* plemma,
      size_t      clemma,
      size_t      cbytes,
      unsigned    uclass,
      int         nforms, const SStrMatch* pforms ) MLMA_PURE;
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
      IMlfaMatch*     pmatch,
      const char*     pszstr, size_t    cchstr ) MLMA_PURE;
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
      IMlfaMatch*     pmatch,
      const widechar* pszstr, size_t    cchstr ) MLMA_PURE;
  MLMA_END;

# if defined( __cplusplus )
  template <class Mlfa, class CharType>
  struct IMlfaXX: Mlfa
  {
    using StemType = typename std::conditional<std::is_same<CharType, char>::value,
      SStemInfoA, SStemInfoW>::type;
    using GramType = SGramInfo;

    using   string = std::basic_string<CharType>;

    struct  inword;
    template <class T>
      struct  outbuf;
    struct  lexeme;

    using Mlfa::GetWdInfo;
    using Mlfa::GetSample;
    using Mlfa::Lemmatize;
    using Mlfa::BuildForm;
    using Mlfa::FindMatch;

    auto  GetWdInfo( unsigned ) -> uint8_t;
    int   GetSample( const outbuf<CharType>&, unsigned uclass );
    auto  GetSample( unsigned uclass ) -> string;
    int   Lemmatize( const inword&,
      const outbuf<StemType>&,
      const outbuf<CharType>& = {},
      const outbuf<GramType>& = {}, unsigned dwsets = 0 );
    auto  Lemmatize( const inword& pszstr, unsigned dwsets = 0 ) -> std::vector<lexeme>;
    int   BuildForm( const outbuf<CharType>&,
      const inword&, unsigned nclass, formid_t idform );
    auto  BuildForm(
      const inword&, unsigned nclass, formid_t idform ) -> std::vector<string>;
    int   BuildForm( const outbuf<CharType>&, const StemType&, formid_t );
    auto  BuildForm( const StemType&, formid_t idform ) -> std::vector<string>;
    int   FindMatch( IMlfaMatch* pmatch, const inword& pszstr );
    template <class FMatch>
    int   FindMatch( const inword&, FMatch );

  };

  using IMlfaMbXX = IMlfaXX<IMlfaMb, char>;
  using IMlfaWcXX = IMlfaXX<IMlfaWc, widechar>;

  template <class Mlma, class CharType>
  struct  IMlfaXX<Mlma, CharType>::inword
  {
    const CharType* t;
    size_t          l;

  public:
    inword( const CharType* s, size_t n = (size_t)-1 ): t( s ), l( n )  {}
    template <class Allocator>
    inword( const std::basic_string<CharType, Allocator>& s ): inword( s.c_str(), s.length() ) {}
  };

  template <class Mlfa, class CharType>
  template <class T>
  struct  IMlfaXX<Mlfa, CharType>::outbuf
  {
    T*      t;
    size_t  l;

  public:
    outbuf(): t( nullptr ), l( 0 ) {}
    template <size_t N>
    outbuf( T (&p)[N] ): t( p ), l( N ) {}
    outbuf( T* p, size_t n ): t( p ), l( n ) {}
  };

  template <class Mlfa, class CharType>
  struct  IMlfaXX<Mlfa, CharType>::lexeme: StemType
  {
    lexeme( const StemType& lx ): StemType( lx )
    {
      if ( lx.plemma != nullptr )
        this->plemma = (normal = string( lx.plemma, lx.ccstem )).c_str();
      if ( lx.ngrams != 0 )
        this->pgrams = (agrams = std::vector<SGramInfo>( lx.pgrams, lx.pgrams + lx.ngrams )).data();
    }
    lexeme( lexeme&& lx ): StemType( std::move( lx ) ),
      normal( std::move( lx.normal ) ),
      agrams( std::move( lx.agrams ) )
    {
      this->plemma = normal.c_str();
      this->pgrams = agrams.data();
    }

    string                 normal;
    std::vector<SGramInfo> agrams;

  };

  template <class Mlfa, class CharType>
  auto  IMlfaXX<Mlfa, CharType>::GetWdInfo( lexeme_t nlexid ) -> uint8_t
  {
    uint8_t wdinfo;

    return GetWdInfo( &wdinfo, nlexid ) > 0 ? wdinfo : 0;
  }

  template <class Mlfa, class CharType>
  int   IMlfaXX<Mlfa, CharType>::GetSample( const outbuf<CharType>& sample, unsigned uclass )
  {
    return GetSample( sample.t, sample.l, uclass );
  }

  template <class Mlfa, class CharType>
  auto  IMlfaXX<Mlfa, CharType>::GetSample( unsigned uclass ) -> string
  {
    CharType  sample[64];

    return GetSample( sample, uclass ) > 0 ? sample : string{};
  }

  template <class Mlfa, class CharType>
  int   IMlfaXX<Mlfa, CharType>::Lemmatize(
    const inword&             pszstr,
    const outbuf<StemType>&   alemms,
    const outbuf<CharType>&   aforms,
    const outbuf<SGramInfo>&  agrams, unsigned dwsets )
  {
    return Lemmatize(
      pszstr.t, pszstr.l,
      alemms.t, alemms.l,
      aforms.t, aforms.l,
      agrams.t, agrams.l, dwsets );
  }

  template <class Mlfa, class CharType>
  auto  IMlfaXX<Mlfa, CharType>::Lemmatize( const inword& pszstr, unsigned dwsets ) -> std::vector<lexeme>
  {
    StemType  astems[0x20];
    CharType  aforms[0x200];
    SGramInfo agrams[0x50];
    int       nbuilt = Lemmatize( pszstr, astems, aforms, agrams, dwsets );

    if ( nbuilt >= 0 )
    {
      auto  output = std::vector<lexeme>();

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

  template <class Mlfa, class CharType>
  int   IMlfaXX<Mlfa, CharType>::BuildForm(
    const outbuf<CharType>& output,
    const inword&           plemma,
    unsigned                nclass,
    formid_t                idform )
  {
    return BuildForm(
      output.t, output.l,
      plemma.t, plemma.l, nclass, idform );
  }

  template <class Mlfa, class CharType>
  int   IMlfaXX<Mlfa, CharType>::BuildForm( const outbuf<CharType>& output,
    const StemType& stinfo, formid_t idform )
  {
    return BuildForm( output, { stinfo.plemma, stinfo.ccstem }, stinfo.nclass, idform );
  }

  template <class Mlfa, class CharType>
  auto  IMlfaXX<Mlfa, CharType>::BuildForm( const inword& plemma,
    unsigned nclass, formid_t idform ) -> std::vector<string>
  {
    CharType  buffer[0x100];
    auto      output = std::vector<string>();
    int       nforms = BuildForm( buffer, plemma, nclass, idform );

    for ( auto strptr = buffer; nforms-- > 0; strptr += output.back().length() + 1 )
      output.emplace_back( strptr );

    return output;
  }

  template <class Mlfa, class CharType>
  auto  IMlfaXX<Mlfa, CharType>::BuildForm( const StemType& stinfo, formid_t idform ) -> std::vector<string>
  {
    return BuildForm( { stinfo.plemma, stinfo.ccstem }, stinfo.nclass, idform );
  }

  template <class Mlfa, class CharType>
  int   IMlfaXX<Mlfa, CharType>::FindMatch( IMlfaMatch* pmatch, const inword& pszstr )
  {
    return FindMatch( pmatch, pszstr.t, pszstr.l );
  }

  template <class Mlfa, class CharType>
  template <class FMatch>
  int   IMlfaXX<Mlfa, CharType>::FindMatch( const inword&  szword, FMatch fmatch )
  {
    static_assert( std::is_constructible<std::function<int(const void*, size_t, size_t, int, const SStrMatch*)>, FMatch>::value,
      "invalid callback type, int( lexeme_t, int, const SStrMatch* ) expected" );

    struct MatchAPI: IMlfaMatch
    {
      FMatch& match;

      MatchAPI( FMatch& fm ): match( fm )
        {}
      int   MLMAPROC  Attach() override
        {  return 1;  }
      int   MLMAPROC  Detach() override
        {  return 1;  }
      int   MLMAPROC  AddLexeme( const void* str, size_t len, size_t cch, unsigned cls, int cnt, const SStrMatch* ptr ) override
        {  return match( str, len, cch, cls, cnt, ptr );  }
    };
    auto  pmatch = MatchAPI( fmatch );

    return this->FindMatch( &pmatch, szword.t, szword.l );
  }

# endif // __cplusplus

# endif  /* !mlfa_interface_defined */

# if defined( __cplusplus )
extern "C" {
# endif /* __cplusplus */

  int   MLMAPROC        mlfaruLoadMbAPI( IMlfaMb** );
  int   MLMAPROC        mlfaruLoadCpAPI( IMlfaMb**, const char* codepage );
  int   MLMAPROC        mlfaruLoadWcAPI( IMlfaWc** );

# if defined( __cplusplus )
}
  template <class T>  int   mlfaruLoadMbAPI( T** pp )
    {  return mlfaruLoadMbAPI( (IMlfaMb**)pp );  }
  template <class T>  int   mlfaruLoadCpAPI( T** pp, const char* cp )
    {  return mlfaruLoadCpAPI( (IMlfaMb**)pp, cp );  }
  template <class T>  int   mlfaruLoadWcAPI( T** pp )
    {  return mlfaruLoadWcAPI( (IMlfaWc**)pp );  }
# endif /* __cplusplus */

# endif /* _mlfa1049_h_ */
