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
# if !defined( _libmorph_api_hpp_ )
# define _libmorph_api_hpp_
# if !defined( __cplusplus )
#   error( "api.hpp is C++-only header!" )
# endif
# include "api.h"
# include <string>
# include <vector>
# include <stdexcept>
# include <functional>

template <class Char>
class MlmaTraits
{
  template <class T, class C> friend class IMlmaXX;
  template <class T, class C> friend class IMlfaXX;

  struct  inword;
template <class T>
  struct  outbuf;
template <class T>
  struct  lexeme;
  using   string = std::basic_string<Char>;
};

template <class Mlma, class Char>
class IMlmaXX: public Mlma
{
  using CharType = Char;
  using LemmType = typename std::conditional<std::is_same<CharType, char>::value,
    SLemmInfoA, SLemmInfoW>::type;
  using GramType = SGramInfo;

  using Traits = MlmaTraits<CharType>;

public:
  using Mlma::GetWdInfo;
  using Mlma::CheckWord;
  using Mlma::Lemmatize;
  using Mlma::BuildForm;
  using Mlma::FindForms;
  using Mlma::CheckHelp;
  using Mlma::FindMatch;

  using inword = typename Traits::inword;
  using string = typename Traits::string;
template <class T>
  using outbuf = typename Traits::outbuf<T>;
  using lexeme = typename Traits::lexeme<LemmType>;

  auto  GetWdInfo( lexeme_t ) -> uint8_t;
  int   CheckWord( const inword&, unsigned dwsets = 0 );
  int   Lemmatize( const inword&,
    const outbuf<LemmType>&  lxbuff,
    const outbuf<CharType>&  stbuff = {},
    const outbuf<GramType>&  grbuff = {}, unsigned dwsets = 0 );
  auto  Lemmatize(
    const inword&, unsigned dwsets = 0 ) -> std::vector<lexeme>;
  int   BuildForm( const outbuf<CharType>&, lexeme_t nlexid, formid_t idform );
  auto  BuildForm( lexeme_t nlexid, formid_t idform ) -> std::vector<string>;
  int   FindForms( const outbuf<CharType>&,
    const inword&, formid_t, unsigned dwsets = 0 );
  auto  FindForms(
    const inword&, formid_t, unsigned dwsets = 0 ) -> std::vector<string>;
  int   CheckHelp( const outbuf<CharType>&, const inword& );
  auto  CheckHelp( const inword& ) -> string;
  int   FindMatch( IMlmaMatch*, const inword& );
  template <class FMatch>
  int   FindMatch( const inword&, FMatch );

};

using IMlmaMbXX = IMlmaXX<IMlmaMb, char>;
using IMlmaWcXX = IMlmaXX<IMlmaWc, widechar>;

template <class Mlfa, class Char>
class IMlfaXX: public Mlfa
{
  using CharType = Char;
  using StemType = typename std::conditional<std::is_same<CharType, char>::value,
    SStemInfoA, SStemInfoW>::type;
  using GramType = SGramInfo;

  using Traits = MlmaTraits<CharType>;

public:
  using Mlfa::GetWdInfo;
  using Mlfa::GetModels;
  using Mlfa::Lemmatize;
  using Mlfa::BuildForm;

  using inword = typename Traits::inword;
  using string = typename Traits::string;
template <class T>
  using outbuf = typename Traits::outbuf<T>;
  using lexeme = typename Traits::lexeme<StemType>;

  auto  GetWdInfo( unsigned ) -> uint8_t;
  int   GetModels( const outbuf<CharType>&, unsigned uclass );
  auto  GetModels( unsigned uclass ) -> std::vector<string>;
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

};

using IMlfaMbXX = IMlfaXX<IMlfaMb, char>;
using IMlfaWcXX = IMlfaXX<IMlfaWc, widechar>;

template <class CharType>
struct  MlmaTraits<CharType>::inword
{
  const CharType* t;
  size_t          l;

public:
  inword( const CharType* s, size_t n = (size_t)-1 ): t( s ), l( n )  {}
  template <class Allocator>
  inword( const std::basic_string<CharType, Allocator>& s ): inword( s.c_str(), s.length() ) {}
};

template <class CharType>
template <class T>
struct  MlmaTraits<CharType>::outbuf
{
  T*      t;
  size_t  l;

  outbuf(): t( nullptr ), l( 0 ) {}
  template <size_t N>
  outbuf( T (&p)[N] ): t( p ), l( N ) {}
  outbuf( T* p, size_t n ): t( p ), l( n ) {}
};

template <class CharType>
template <class LemmType>
struct  MlmaTraits<CharType>::lexeme: LemmType
{
  lexeme( const LemmType& lx ): LemmType( lx )
  {
    if ( lx.plemma != nullptr )
      this->plemma = (lemma = string( lx.plemma )).c_str();
    if ( lx.ngrams != 0 )
      this->pgrams = (grams = std::vector<SGramInfo>( lx.pgrams, lx.pgrams + lx.ngrams )).data();
  }
  lexeme( lexeme&& lx ): LemmType( std::move( lx ) ),
    lemma( std::move( lx.lemma ) ),
    grams( std::move( lx.grams ) )
  {
    this->plemma = lemma.c_str();
    this->pgrams = grams.data();
  }

  string                 lemma;
  std::vector<SGramInfo> grams;

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

template <class Mlfa, class CharType>
auto  IMlfaXX<Mlfa, CharType>::GetWdInfo( lexeme_t nlexid ) -> uint8_t
{
  uint8_t wdinfo;

  return GetWdInfo( &wdinfo, nlexid ) > 0 ? wdinfo : 0;
}

template <class Mlfa, class CharType>
int   IMlfaXX<Mlfa, CharType>::GetModels( const outbuf<CharType>& sample, unsigned uclass )
{
  return GetModels( sample.t, sample.l, uclass );
}

template <class Mlfa, class CharType>
auto  IMlfaXX<Mlfa, CharType>::GetModels( unsigned uclass ) -> std::vector<string>
{
  CharType  models[0x400];
  auto      mcount = GetModels( models, uclass );

  if ( mcount > 0 )
  {
    auto  outvec = std::vector<string>();

    for ( auto modstr = models; mcount-- > 0; )
    {
      outvec.push_back( string( modstr ) );
        modstr += outvec.back().length() + 1;
    }
    return outvec;
  }
  return {};
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

// creation helpers

template <class IMlma>
class Morpho
{
  IMlma*  ptrAPI = nullptr;

public:
  Morpho() = default;
  Morpho( IMlma* p )                    {  Assign( p );                }
  Morpho( const Morpho& m )             {  Assign( m.ptrAPI );         }
 ~Morpho()                              {  Assign( nullptr );          }
  Morpho& operator=( const Morpho& m )  {  return Assign( m.ptrAPI );  }
  auto operator -> () const -> IMlma*   {  return ptrAPI;              }
  auto operator * () const -> IMlma&    {  return *ptrAPI;             }
  operator void** ()                    {  return &ptrAPI;             }

protected:
  auto  Assign( IMlma* p ) -> Morpho&
  {
    if ( ptrAPI != nullptr )
      ptrAPI->Detach();
    if ( (ptrAPI = p) != nullptr )
      ptrAPI->Attach();
    return *this;
  }
};

# endif /* _libmorph_api_hpp_ */
