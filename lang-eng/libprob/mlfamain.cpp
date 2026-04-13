/******************************************************************************

    libfuzzyrus - fuzzy morphological analyser for Russian.

    Copyright (C) 1994-2025 Andrew Kovalenko aka Keva

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
      Phone: +7(495)648-4058, +7(926)513-2991

******************************************************************************/
# include "scandict.hpp"
# include "buildforms.hpp"
# include "lemmatizer.hpp"
# include "xmorph/mlfawide.hpp"
# include <cstring>

# if !defined( _WIN32_WCE )
  # define  CATCH_ALL         try {
  # define  ON_ERRORS( code ) } catch ( ... ) { return (code); }
# else
  # define  CATCH_ALL
  # define  ON_ERRORS( code )
# endif  // ! _WIN32_WCE

namespace NAMESPACE
{

  const float  inflexProbTable[16] = { -1,
    0.0,    // нулевое окончание, 1 символ в конце основы
    0.05,   // -?й
    0.17,   // -?ть, ?ой
    0.55,   // -?ому
    0.85,   // -?ющем
    0.97,   // -?иться
    0.99,   // -?ющийся
    0.99,   // -?ировать
    1.0,    // -?ировался
    1.0,    // -?ироваться
    1.0,
    1.0,
    1.0,     // -?ироваться
    1.0,     // -?ироваться
    1.0 };
    
  //
  // the new api - IMlma interface class
  //
  template <unsigned codepage>
  class  CMlfaMb: public IMlfaMb
  {
    static const auto CPS_1251 = codepages::codepage_1251;
    static const auto CPS_UTF8 = codepages::codepage_utf8;

  public:
    int MLMAPROC  Attach() override {  return 1;  }
    int MLMAPROC  Detach() override {  return 1;  }

  public:     // overridables
    int MLMAPROC  GetWdInfo(
      unsigned char*  pwinfo, unsigned uclass ) override;
    int MLMAPROC  GetModels(
      char*         sample, size_t    samlen,
      unsigned      uclass ) override;
    int MLMAPROC  Lemmatize(
      const char*   pszstr, size_t    cchstr,
      SStemInfoA*   output, size_t    cchout,
      char*         plemma, size_t    clemma,
      SGramInfo*    pgrams, size_t    ngrams, unsigned  dwsets )  override;
    int MLMAPROC  BuildForm(
      char*         output, size_t    cchout,
      const char*   lpstem, size_t    ccstem,
      unsigned      nclass, formid_t  idform ) override;

  protected:
    template <size_t N>
    unsigned      ToCanonic( uint8_t (&)[N], const char*, size_t ) const;

  };

  CMlfaMb<codepages::codepage_1251>                           mbInstance1251;
  CMlfaMb<codepages::codepage_koi8>                           mbInstanceKoi8;
  CMlfaMb<codepages::codepage_866>                            mbInstance866;
  CMlfaMb<codepages::codepage_iso>                            mbInstanceIso;
  CMlfaMb<codepages::codepage_mac>                            mbInstanceMac;
  CMlfaMb<codepages::codepage_utf8>                           mbInstanceUtf8;
  CMlfaWc<CMlfaMb<codepages::codepage_1251>, mbInstance1251>  wcInstanceMlfa;

  // CmlfaMb template implementation

  template <unsigned codepage>
  int   CMlfaMb<codepage>::GetWdInfo( unsigned char*  pwinfo, unsigned uclass )
  {
    try
    {
      if ( pwinfo == nullptr )
        return ARGUMENT_FAILED;
      ::FetchFrom( GetClass( uclass ), *pwinfo );
        return 1;
    }
    catch ( const std::invalid_argument& )
    {
      return ARGUMENT_FAILED;
    }
  }

  template <unsigned codepage>
  int   CMlfaMb<codepage>::GetModels( char* models, size_t  buflen, unsigned  nclass )
  {
    if ( models == nullptr || buflen == 0 )
      return ARGUMENT_FAILED;

    try
    {
      uint8_t   partSp;
      uint8_t   clSets;
      int       fcount;
      auto      pclass =
        ::FetchFrom( ::FetchFrom( ::FetchFrom( GetClass( nclass ),
          partSp ),
          clSets ),
          fcount );
      int     mcount;
      auto    modend = models + buflen;

      while ( fcount-- > 0 )
      {
        uint8_t idform;
        uint8_t ccflex;

        pclass = ::FetchFrom( ::FetchFrom( pclass, idform ), ccflex );
          pclass += ccflex;
      }

      pclass = ::FetchFrom( pclass, fcount );

      for ( mcount = 0; fcount-- > 0; ++mcount )
      {
        char      suffix[2];
        unsigned  mPower;
        unsigned  mdSize;
        size_t    cchmod;

        pclass = ::FetchFrom( ::FetchFrom( ::FetchFrom( pclass,
          suffix, 2 ),
          mPower ),
          mdSize );

        if ( (cchmod = ToCodepage( codepage, models, modend - models, pclass, mdSize )) == (size_t)-1 )
          return mcount == 0 ? LEMMBUFF_FAILED : mcount;

        models += cchmod + 1;
        pclass += mdSize;
      }
      return mcount;
    }
    catch ( const std::invalid_argument& )
    {
      return ARGUMENT_FAILED;
    }
  }

  template <unsigned codepage>
  int   CMlfaMb<codepage>::Lemmatize(
    const char* pszstr, size_t  cchstr,
    SStemInfoA* plemma, size_t  clemma,
    char*       pforms, size_t  cforms,
    SGramInfo*  pgrams, size_t  cgrams, unsigned  dwsets )
  {
    CATCH_ALL
      uint8_t locase[256];
      auto    scheme = ToCanonic( locase, pszstr, cchstr );
      int     nerror;

    // check source string and length
      if ( scheme == (unsigned)-1 || scheme == 0 )
        return scheme == (unsigned)-1 ? WORDBUFF_FAILED : 0;

    // get capitalization scheme; if it is invalid, do not pass time to analyses
      if ( uint8_t(scheme) == 0xff && (dwsets & sfIgnoreCapitals) == 0 )
        return 0;

    // create lemmatize collector
      auto  lemmatizer = Lemmatizer( CapScheme( charTypeMatrix, toLoCaseMatrix, toUpCaseMatrix ),
        { plemma, clemma },
        { pforms, cforms, codepage },
        { pgrams, cgrams },
      locase, scheme >> 16, scheme, dwsets );

      if ( (nerror = patricia::ScanTree( lemmatizer, (const char*)ReverseDict,
        (const char*)locase + (scheme >> 16) - 1, scheme >> 16 )) != 0 )
          return nerror;

      return lemmatizer;
    ON_ERRORS( -1 )
  }

  template <unsigned codepage>
  int   CMlfaMb<codepage>::BuildForm(
    char*       output, size_t    cchout,
    const char* pszstr, size_t    cchstr,
    unsigned    nclass, formid_t  idform )
  {
    try
    {
      uint8_t locase[256];
      auto    scheme = ToCanonic( locase, pszstr, cchstr );

    // check the args
      if ( output == nullptr || cchout == 0 || nclass >= ClassNumber )
        return ARGUMENT_FAILED;

      if ( cchstr != 0 && pszstr == nullptr )
        return ARGUMENT_FAILED;

    // check source string and length
      if ( scheme == (unsigned)-1 || scheme == 0 )
        return scheme == (unsigned)-1 ? WORDBUFF_FAILED : 0;

      return Buildform( CapScheme( charTypeMatrix, toLoCaseMatrix, toUpCaseMatrix ),
        { output, cchout, codepage } ) ( locase, scheme >> 16, nclass, idform );
    }
    catch ( const std::invalid_argument& ) {  return ARGUMENT_FAILED;  }
    catch ( ... ) {  return -1;  }
  }

  template <unsigned codepage>
  template <size_t N>
  auto  CMlfaMb<codepage>::ToCanonic( uint8_t (& output)[N], const char* pszstr, size_t cchstr ) const -> unsigned
  {
    char  mbsstr[N];

    if ( pszstr == nullptr )
      return 0;

    if ( cchstr == (size_t)-1 )
      for ( cchstr = 0; pszstr[cchstr] != 0; ++cchstr ) (void)NULL;

    if ( cchstr == 0 )
      return 0;

    if ( codepage != codepages::codepage_1251 )
    {
      if ( (cchstr = ToInternal( mbsstr, codepage, pszstr, cchstr )) != (size_t)-1 )  pszstr = mbsstr;
        else return (unsigned)-1;
    }

    return CapScheme( charTypeMatrix, toLoCaseMatrix, toUpCaseMatrix ).Get(
      output,
      pszstr,
      cchstr );
  }

}

/*
 * declare old-style api functions
 */
extern "C"  int   MLMAPROC  mlfaenLoadMbAPI( IMlfaMb** );
extern "C"  int   MLMAPROC  mlfaenLoadCpAPI( IMlfaMb**, const char* );
extern "C"  int   MLMAPROC  mlfaenLoadWcAPI( IMlfaWc** );

/*
 * declare public style api function
 */
extern "C"  int   MLMAPROC  mlfaenGetAPI( const char*, void** );

using namespace NAMESPACE;

extern "C"  int   MLMAPROC  mlfaenLoadMbAPI( IMlfaMb**  ptrAPI )
{
  if ( ptrAPI == nullptr )
    return -1;
  (*ptrAPI = &mbInstance1251)->Attach();
    return 0;
}

extern "C"  int   MLMAPROC  mlfaenLoadCpAPI( IMlfaMb**  ptrAPI, const char* codepage )
{
  (void)codepage;
  return mlfaenLoadMbAPI( ptrAPI );
}

extern "C"  int   MLMAPROC  mlfaenLoadWcAPI( IMlfaWc**  ptrAPI )
{
  if ( ptrAPI == nullptr )
    return -1;
  (*ptrAPI = &wcInstanceMlfa)->Attach();
  return 0;
}

extern "C"  int   MLMAPROC  mlfaenGetAPI( const char* apiKey, void** ppvAPI )
{
  // check call parameters
  if ( ppvAPI == nullptr )
    return EINVAL;
  if ( apiKey == nullptr || memcmp( apiKey, LIBFUZZY_API_4_MAGIC ":", sizeof(LIBFUZZY_API_4_MAGIC) ) != 0 )
    return EINVAL;

  // detect the codepage
  if ( strcasecmp( "utf-16", apiKey + sizeof(LIBMORPH_API_4_MAGIC) ) == 0
    || strcasecmp( "utf16",  apiKey + sizeof(LIBMORPH_API_4_MAGIC) ) == 0 )
  {
    return mlfaenLoadWcAPI( (IMlfaWc**)ppvAPI );
  }

  return mlfaenLoadMbAPI( (IMlfaMb**)ppvAPI );
}
