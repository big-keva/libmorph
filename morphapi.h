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
# if !defined( _libmorph_morphapi_h_ )
# define _libmorph_morphapi_h_

# include <limits.h>
# include <stdint.h>
# include <stddef.h>

# if defined( __cplusplus )
#   include <string>
#   include <vector>
#   include <stdexcept>
#   include <functional>
# else
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
    MLMA_METHOD( CheckHelp )( MLMA_THIS
      char*           output, size_t    cchout,
      const char*     pszstr, size_t    cchstr ) MLMA_PURE;
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
    MLMA_METHOD( CheckHelp )( MLMA_THIS
      widechar*       output, size_t    cchout,
      const widechar* pwsstr, size_t    cchstr ) MLMA_PURE;
    MLMA_METHOD( GetWdInfo )( MLMA_THIS
      unsigned char*  pwinfo, lexeme_t  nlexid ) MLMA_PURE;
    MLMA_METHOD( FindMatch )( MLMA_THIS
      IMlmaMatch*     pienum,
      const widechar* pszstr, size_t    cchstr ) MLMA_PURE;
  MLMA_END;

# if defined( __cplusplus )
  template <class Mlma, class Char>
  struct IMlmaXX: Mlma
  {
    using CharType = Char;
    using LemmType = typename std::conditional<std::is_same<CharType, char>::value,
      SLemmInfoA, SLemmInfoW>::type;
    using GramType = SGramInfo;

    using   string = std::basic_string<CharType>;

    struct  inword;
  template <class T>
    struct  outbuf;
    struct  lexeme;

    using Mlma::GetWdInfo;
    using Mlma::CheckWord;
    using Mlma::Lemmatize;
    using Mlma::BuildForm;
    using Mlma::FindForms;
    using Mlma::CheckHelp;
    using Mlma::FindMatch;

    auto  GetWdInfo( lexeme_t ) -> uint8_t;
    int   CheckWord( const inword&, unsigned dwsets = 0 );
    int   Lemmatize( const inword&,
      const outbuf<LemmType>&  lxbuff,
      const outbuf<CharType>&  stbuff = {},
      const outbuf<GramType>&  grbuff = {}, unsigned dwsets = 0 );
    auto  Lemmatize( const inword&, unsigned dwsets = 0 ) -> std::vector<lexeme>;
    int   BuildForm( const outbuf<CharType>&, lexeme_t nlexid, formid_t idform );
    auto  BuildForm( lexeme_t nlexid, formid_t idform ) -> std::vector<string>;
    int   FindForms( const outbuf<CharType>&, const inword&, formid_t, unsigned dwsets = 0 );
    auto  FindForms( const inword&, formid_t, unsigned dwsets = 0 ) -> std::vector<string>;
    int   CheckHelp( const outbuf<CharType>&, const inword& );
    auto  CheckHelp( const inword& ) -> string;
    int   FindMatch( IMlmaMatch*, const inword& );
    template <class FMatch>
    int   FindMatch( const inword&, FMatch );

  };

  using IMlmaMbXX = IMlmaXX<IMlmaMb, char>;
  using IMlmaWcXX = IMlmaXX<IMlmaWc, widechar>;

  template <class Mlma, class Char>
  struct  IMlmaXX<Mlma, Char>::inword
  {
    const CharType* t;
    size_t          l;

  public:
    inword( const CharType* s, size_t n = (size_t)-1 ): t( s ), l( n )  {}
    template <class Allocator>
    inword( const std::basic_string<CharType, Allocator>& s ): inword( s.c_str(), s.length() ) {}
  };

  template <class Mlma, class Char>
  template <class T>
  struct  IMlmaXX<Mlma, Char>::outbuf
  {
    T*      t;
    size_t  l;

  public:
    outbuf(): t( nullptr ), l( 0 ) {}
    template <size_t N>
    outbuf( T (&p)[N] ): t( p ), l( N ) {}
    outbuf( T* p, size_t n ): t( p ), l( n ) {}
  };

  template <class Mlma, class Char>
  struct  IMlmaXX<Mlma, Char>::lexeme: LemmType
  {
    lexeme( const LemmType& lx ): LemmType( lx )
    {
      if ( lx.plemma != nullptr )
        this->plemma = (normal = string( lx.plemma )).c_str();
      if ( lx.ngrams != 0 )
        this->pgrams = (agrams = std::vector<SGramInfo>( lx.pgrams, lx.pgrams + lx.ngrams )).data();
    }
    lexeme( lexeme&& lx ): LemmType( std::move( lx ) ),
      normal( std::move( lx.normal ) ),
      agrams( std::move( lx.agrams ) )
    {
      this->plemma = normal.c_str();
      this->pgrams = agrams.data();
    }

    string                 normal;
    std::vector<SGramInfo> agrams;

  };

  template <class Mlma, class Char>
  auto  IMlmaXX<Mlma, Char>::GetWdInfo( lexeme_t nlexid ) -> uint8_t
  {
    uint8_t wdinfo;

    return this->GetWdInfo( &wdinfo, nlexid ) > 0 ? wdinfo : 0;
  }

  template <class Mlma, class Char>
  int   IMlmaXX<Mlma, Char>::CheckWord( const inword&  szword, unsigned dwsets )
  {
    return this->CheckWord( szword.t, szword.l, dwsets );
  }

  template <class Mlma, class Char>
  int   IMlmaXX<Mlma, Char>::Lemmatize( const inword&  szword,
    const outbuf<LemmType>&  lxbuff,
    const outbuf<CharType>&  stbuff,
    const outbuf<GramType>&  grbuff, unsigned  dwsets )
  {
    return this->Lemmatize(
      szword.t, szword.l,
      lxbuff.t, lxbuff.l,
      stbuff.t, stbuff.l,
      grbuff.t, grbuff.l, dwsets );
  }

  template <class Mlma, class Char>
  auto  IMlmaXX<Mlma, Char>::Lemmatize( const inword&  szword, unsigned  dwsets ) -> std::vector<lexeme>
  {
    using GramType = SGramInfo;

    LemmType  lxbuff[32];
    GramType  grbuff[64];
    CharType  stbuff[256];
    int       nbuilt = this->Lemmatize( szword, lxbuff, stbuff, grbuff, dwsets );

    if ( nbuilt >= 0 )
    {
      auto  output = std::vector<lexeme>();

      for ( auto p = lxbuff, e = p + nbuilt; p != e; )
        output.push_back( *p++ );

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

  template <class Mlma, class Char>
  int   IMlmaXX<Mlma, Char>::BuildForm( const outbuf<CharType>& output,
    lexeme_t nlexid, formid_t idform )
  {
    return this->BuildForm( output.t, output.l, nlexid, idform );
  }

  template <class Mlma, class Char>
  auto  IMlmaXX<Mlma, Char>::BuildForm(
    lexeme_t nlexid, formid_t idform ) -> std::vector<string>
  {
    CharType  buffer[0x100];
    auto      output = std::vector<string>();
    int       nforms = this->BuildForm( buffer, nlexid, idform );

    for ( auto strptr = buffer; nforms-- > 0; strptr += output.back().length() + 1 )
      output.emplace_back( strptr );

    return output;
  }

  template <class Mlma, class Char>
  int   IMlmaXX<Mlma, Char>::FindForms( const outbuf<CharType>& output,
    const inword& szword, formid_t idform, unsigned dwsets )
  {
    return this->FindForms( output.t, output.l, szword.t, szword.l, idform, dwsets );
  }

  template <class Mlma, class Char>
  auto  IMlmaXX<Mlma, Char>::FindForms(
    const inword& szword, formid_t idform, unsigned dwsets ) -> std::vector<string>
  {
    CharType  buffer[0x100];
    auto      output = std::vector<string>();
    int       nforms = this->FindForms( buffer, szword, idform, dwsets );

    for ( auto strptr = buffer; nforms-- > 0; strptr += output.back().length() + 1 )
      output.emplace_back( strptr );

    return output;
  }

  template <class Mlma, class Char>
  int   IMlmaXX<Mlma, Char>::CheckHelp( const outbuf<CharType>& output, const inword& szword )
  {
    return this->CheckHelp( output.t, output.l, szword.t, szword.l );
  }

  template <class Mlma, class Char>
  auto  IMlmaXX<Mlma, Char>::CheckHelp( const inword& szword ) -> string
  {
    widechar  output[256];
    int       length = this->CheckHelp( output, 256, szword.t, szword.l );

    if ( length < 0 )
      throw std::runtime_error( "internal error" );

    return { output, (size_t)length };
  }

  template <class Mlma, class Char>
  int   IMlmaXX<Mlma, Char>::FindMatch( IMlmaMatch* pmatch, const inword& szword )
  {
    return this->FindMatch( pmatch, szword.t, szword.l );
  }

  template <class Mlma, class Char>
  template <class FMatch>
  int   IMlmaXX<Mlma, Char>::FindMatch( const inword&  szword, FMatch fmatch )
  {
    static_assert( std::is_constructible<std::function<int(lexeme_t, int, const SStrMatch*)>, FMatch>::value,
      "invalid callback type, int( lexeme_t, int, const SStrMatch* ) expected" );

    struct MatchAPI: IMlmaMatch
    {
      FMatch& match;

      MatchAPI( FMatch& fm ): match( fm )
        {}
      int   MLMAPROC  Attach() override
        {  return 1;  }
      int   MLMAPROC  Detach() override
        {  return 1;  }
      int   MLMAPROC  AddLexeme( lexeme_t lex, int cnt, const SStrMatch* ptr ) override
        {  return match( lex, cnt, ptr );  }
    };
    auto  pmatch = MatchAPI( fmatch );

    return this->FindMatch( &pmatch, szword.t, szword.l );
  }

# endif  /* __cplusplus */
# endif  /* !mlma_interface_defined */

# if !defined( mlma_getapi_defined )
# define mlma_getapi_defined

typedef int (MLMAPROC *libmorphGetAPI)( const char* apiKey, void** ppvAPI );

# endif  /* !mlma_getapi_defined */

# endif /* _libmorph_morphapi_h_ */
