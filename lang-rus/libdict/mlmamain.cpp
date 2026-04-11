/******************************************************************************

    libmorphrus - dictionary-based morphological analyser for Russian.

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
# include "../../rus.h"
# include "../chartype.h"
# include "xmorph/codepages.hpp"
# include "xmorph/wildscan.h"
# include "xmorph/mlmawide.hpp"
# include "mlmadefs.h"
# include "scanClass.hpp"
# include "wildClass.hpp"
# include "lemmatize.hpp"
# include "buildform.hpp"
# include <cstdlib>
# include <cstring>

# if !defined( _WIN32_WCE )
  # define  CATCH_ALL         try {
  # define  ON_ERRORS( code ) } catch ( ... ) { return (code); }
# else
  # define  CATCH_ALL
  # define  ON_ERRORS( code )
# endif  // ! _WIN32_WCE

# if defined(_MSC_VER)
#   define strcasecmp _strcmpi
# endif   // MSVC

namespace NAMESPACE
{

  struct anyway_ok
  {
    template <class ... args>
    int  operator()( args... ) const  {  return 1;  }
  };

  enum  maximal: size_t
  {
    form_length = 48,               // максимальная длина слова в буквак
    utf8_length = form_length * 2,  // максимальная длина слова в байтах
    forms_count = 256,              // реально чуть меньше, но у глаголов много, до 352
    buffer_size = forms_count * utf8_length
  };

  //
  // the new api - IMlma interface class
  //
  template <unsigned codepage>
  struct  CMlmaMb: IMlmaMb
  {
    int MLMAPROC  Attach() override {  return 1;  }
    int MLMAPROC  Detach() override {  return 1;  }

    int MLMAPROC  CheckWord( const char*    pszstr, size_t    cchstr,
                             unsigned       dwsets )                   override;
    int MLMAPROC  Lemmatize( const char*    pszstr, size_t    cchstr,
                             SLemmInfoA*    output, size_t    cchout,
                             char*          plemma, size_t    clemma,
                             SGramInfo*     pgrams, size_t    ngrams,
                             unsigned       dwsets )                   override;
    int MLMAPROC  BuildForm( char*          output, size_t    cchout,
                             lexeme_t       nlexid, formid_t  idform ) override;
    int MLMAPROC  FindForms( char*          output, size_t    cchout,
                             const char*    pszstr, size_t    cchstr,
                             formid_t       idform, unsigned  dwsets ) override;
    int MLMAPROC  GetWdInfo( unsigned char* pwindo, lexeme_t  nlexid ) override;
    int MLMAPROC  FindMatch( IMlmaMatch*    pienum,
                             const char*    pszstr, size_t    cchstr ) override;

  protected:
    template <size_t N>
    int           GetJocker( uint8_t (&)[N], const char*, size_t ) const;
    template <size_t N>
    unsigned      ToCanonic( uint8_t (&)[N], const char*, size_t ) const;

  };

  CMlmaMb<codepages::codepage_1251>                           mbInstance1251;
  CMlmaMb<codepages::codepage_koi8>                           mbInstanceKoi8;
  CMlmaMb<codepages::codepage_866>                            mbInstance866;
  CMlmaMb<codepages::codepage_iso>                            mbInstanceISO;
  CMlmaMb<codepages::codepage_mac>                            mbInstanceMac;
  CMlmaMb<codepages::codepage_utf8>                           mbInstanceUtf8;
  CMlmaWc<CMlmaMb<codepages::codepage_1251>, mbInstance1251>  wcInstanceMlma;

  // CMlmaMb implementation

  template <unsigned codepage>
  int   CMlmaMb<codepage>::CheckWord( const char* pszstr, size_t  cchstr, unsigned  dwsets )
  {
    CATCH_ALL
      uint8_t locase[maximal::form_length];
      auto    scheme = ToCanonic( locase, pszstr, cchstr );

    // check source string and length
      if ( scheme == (unsigned)-1 || scheme == 0 )
        return scheme == (unsigned)-1 ? WORDBUFF_FAILED : 0;

    // get capitalization scheme; if it is invalid, do not pass time to analyses
      if ( uint8_t(scheme) == 0xff && (dwsets & sfIgnoreCapitals) == 0 )
        return 0;

    // fill scheck structure
      return Flat::ScanTree<uint8_t>( Flat::ScanList( MakeClassMatch( anyway_ok() )
        .SetCapitalization( uint16_t(scheme) )
        .SetSearchSettings( dwsets ) ),
      stemtree, { locase, scheme >> 16 } );
    ON_ERRORS( -1 )
  }

  template <unsigned codepage>
  int   CMlmaMb<codepage>::Lemmatize( const char* pszstr, size_t  cchstr,
                            SLemmInfoA* plemma, size_t  llemma,
                            char*       pforms, size_t  lforms,
                            SGramInfo*  pgrams, size_t  lgrams, unsigned  dwsets )
  {
    CATCH_ALL
      uint8_t locase[maximal::form_length];
      auto    scheme = ToCanonic( locase, pszstr, cchstr );
      int     nerror;

    // check source string and length
      if ( scheme == (unsigned)-1 || scheme == 0 )
        return scheme == (unsigned)-1 ? WORDBUFF_FAILED : 0;

    // get capitalization scheme; if it is invalid, do not pass time to analyses
      if ( uint8_t(scheme) == 0xff && (dwsets & sfIgnoreCapitals) == 0 )
        return 0;

    // create lemmatizer object
      auto  lemmatize = Lemmatizer( CapScheme( charTypeMatrix, toLoCaseMatrix, toUpCaseMatrix ),
        { plemma, llemma },
        { pforms, lforms, codepage },
        { pgrams, lgrams }, locase, dwsets );

      nerror = Flat::ScanTree<uint8_t>( Flat::ScanList( MakeClassMatch( lemmatize )
        .SetCapitalization( scheme & 0xffff )
        .SetSearchSettings( dwsets ) ),
      stemtree, { locase, scheme >> 16 } );

      return nerror < 0 ? nerror : lemmatize;
    ON_ERRORS( -1 )
  }

  template <unsigned codepage>
  int   CMlmaMb<codepage>::BuildForm( char* output, size_t cchout, lexeme_t nlexid, formid_t idform )
  {
    CATCH_ALL
      uint8_t         lidkey[0x10];
      const uint8_t*  ofsptr;
      auto            getofs = []( const byte_t* thedic, const fragment& str ) {  return str.empty() ? thedic : nullptr;  };

    // check the arguments
      if ( output == nullptr || cchout == 0 )
        return ARGUMENT_FAILED;

    // No original word form; algo jumps to lexeme block dictionary point by lexeme id
      if ( (ofsptr = Flat::ScanTree<uint16_t>( getofs, lidstree, { lidkey, lexkeylen( lidkey, nlexid ) } )) != nullptr )
      {
        auto    dicpos = stemtree + getserial( ofsptr );
        byte_t  szstem[maximal::form_length] = {};
        int     nerror;

      // fill other fields
        auto  buildform = FormBuild( { output, cchout, codepage }, CapScheme( charTypeMatrix, toLoCaseMatrix, toUpCaseMatrix ),
          nlexid, idform, szstem );

        nerror = Flat::GetTrack<uint8_t>( Flat::ViewList( MakeBuildClass( buildform ), dicpos ),
          stemtree, szstem, 0, dicpos );

        return nerror < 0 ? nerror : buildform;
      }
      return 0;
    ON_ERRORS( -1 )
  }

  template <unsigned codepage>
  int   CMlmaMb<codepage>::FindForms(
    char*       output, size_t    cchout,
    const char* pszstr, size_t    cchstr,
    formid_t    idform, unsigned  dwsets )
  {
    CATCH_ALL
      uint8_t locase[maximal::form_length];
      auto    scheme = ToCanonic( locase, pszstr, cchstr );
      int     nerror;

    // check the arguments
      if ( output == nullptr || cchout == 0 )
        return ARGUMENT_FAILED;

    // check source string and length
      if ( scheme == (unsigned)-1 || scheme == 0 )
        return scheme == (unsigned)-1 ? WORDBUFF_FAILED : 0;

    // get capitalization scheme; if it is invalid, do not pass time to analyses
      if ( uint8_t(scheme) == 0xff && (dwsets & sfIgnoreCapitals) == 0 )
        return 0;

    // create form builder
      auto  buildform = FormBuild( { output, cchout, codepage }, CapScheme( charTypeMatrix, toLoCaseMatrix, toUpCaseMatrix ),
        0x0000, idform, locase );

      nerror = Flat::ScanTree<uint8_t>( Flat::ScanList( MakeClassMatch( buildform )
        .SetCapitalization( uint16_t(scheme) )
        .SetSearchSettings( dwsets ) ),
      stemtree, { locase, scheme >> 16 } );

      return nerror < 0 ? nerror : buildform;
    ON_ERRORS( -1 )
  }

  template <unsigned codepage>
  int   CMlmaMb<codepage>::GetWdInfo( unsigned char* pwinfo, lexeme_t lexkey )
  {
    CATCH_ALL
      byte_t        lidkey[0x10];
      const byte_t* ofsptr;
      auto          getofs = []( const byte_t* thedic, const fragment& str ){  return str.empty() ? thedic : nullptr;  };

    // No original word form; algo jumps to lexeme block dictionary point by lexeme id
      if ( (ofsptr = Flat::ScanTree<word16_t>( getofs, lidstree, { lidkey, lexkeylen( lidkey, lexkey ) } )) != nullptr )
      {
        const byte_t* dicpos = stemtree + getserial( ofsptr ) + 2; /* 2 => clower && cupper */
        lexeme_t      nlexid = getserial( dicpos );
        word16_t      oclass = getword16( dicpos );
        steminfo      stinfo;

        if ( nlexid != lexkey )
          return LIDSBUFF_FAILED;

        *pwinfo = stinfo.Load( classmap + (oclass & 0x7fff) ).wdinfo & 0x3f;
          return 1;
      }
      return 0;
    ON_ERRORS( -1 )
  }

  template <unsigned codepage>
  int   CMlmaMb<codepage>::FindMatch( IMlmaMatch*  pmatch, const char* pszstr, size_t cchstr )
  {
    CATCH_ALL
      char      cpsstr[maximal::form_length];
      uint8_t   locase[maximal::form_length];
      uint8_t   sforms[maximal::buffer_size];
      SStrMatch amatch[maximal::forms_count * 2];
      uint8_t*  pforms;
      size_t    nmatch = 0;
      lexeme_t  lastId = 0;
      int       nerror;
      auto      reglex = [&](
        lexeme_t          nlexid,
        const uint8_t*    string,
        size_t            length,
        const SGramInfo&  grinfo )
      {
        char  strbuf[maximal::utf8_length];

        if ( nlexid != lastId )
        {
          if ( lastId != 0 )
            pmatch->AddLexeme( lastId, nmatch, amatch );
          lastId = nlexid;
          pforms = sforms;
          nmatch = 0;
        }
        /*
        for ( auto prev = amatch, last = amatch + nmatch; prev != last; ++prev )
          if ( prev->id == grinfo.idForm && prev->cc == length && memcmp( prev->sz, string, length ) == 0 )
            return 0;
        */
        if ( codepage != codepages::codepage_1251 )
        {
          length = codepages::mbcstombcs( codepage, strbuf, sizeof(strbuf),
            codepages::codepage_1251, (const char*)string, length );
          string = (const uint8_t*)strbuf;
        }

        for ( amatch[nmatch++] = { (const char*)pforms, length, grinfo.idForm };
          length-- > 0; ) *pforms++ = *string++;

        return *pforms++ = '\0';
      };

    // check output buffer
      if ( pmatch == nullptr )
        return ARGUMENT_FAILED;

    // check template defined
      if ( pszstr == nullptr )
        return ARGUMENT_FAILED;

    // check length defined
      if ( cchstr == (size_t)-1 )
        for ( cchstr = 0; pszstr[cchstr] != 0; ++cchstr ) (void)NULL;

    // check length defined
      if ( cchstr == 0 )
        return 0;

    // check for overflow
      if ( cchstr >= sizeof(locase) )
        return WORDBUFF_FAILED;

    // modify the codepage
      if ( codepage != codepages::codepage_1251 )
      {
        if ( (cchstr = ToInternal( cpsstr, codepage, pszstr, cchstr )) == (size_t)-1 )
          return WORDBUFF_FAILED;
        pszstr = cpsstr;
      }

    // change the word to the lower case
      CapScheme( charTypeMatrix, toLoCaseMatrix, toUpCaseMatrix ).Get(
        locase,
        pszstr,
        cchstr );
      locase[cchstr] = '\0';

    // scan the dictionary
      if ( (nerror = Wild::ScanTree<uint8_t>( MakeModelMatch( reglex ),
        stemtree, { locase, cchstr }, (uint8_t*)cpsstr, 0 )) != 0 )
      return nerror;

      return lastId != 0 ? pmatch->AddLexeme( lastId, nmatch, amatch ) : 0;
    ON_ERRORS( -1 )
  }

  template <unsigned codepage>
  template <size_t N>
  auto  CMlmaMb<codepage>::ToCanonic( uint8_t (& output)[N], const char* pszstr, size_t cchstr ) const -> unsigned
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

  static const struct
  {
    IMlmaMb*          instance;
    const char* const codepage;
  } codepageList[] =
  {
    { &mbInstance1251, "Windows-1251" },
    { &mbInstance1251, "Windows" },
    { &mbInstance1251, "1251" },
    { &mbInstance1251, "Win-1251" },
    { &mbInstance1251, "Win" },
    { &mbInstance1251, "Windows 1251" },
    { &mbInstance1251, "Win 1251" },
    { &mbInstance1251, "ansi" },
    { &mbInstanceKoi8, "koi-8" },
    { &mbInstanceKoi8, "koi8" },
    { &mbInstanceKoi8, "20866" },
    { &mbInstance866,  "dos" },
    { &mbInstance866,  "oem" },
    { &mbInstance866,  "866" },
    { &mbInstanceISO,  "28595" },
    { &mbInstanceISO,  "iso-88595" },
    { &mbInstanceISO,  "iso-8859-5" },
    { &mbInstanceMac,  "10007" },
    { &mbInstanceMac,  "mac" },
    { &mbInstanceUtf8, "65001" },
    { &mbInstanceUtf8, "utf-8" },
    { &mbInstanceUtf8, "utf8" },
    { (IMlmaMb*)&wcInstanceMlma, "utf-16" },
    { (IMlmaMb*)&wcInstanceMlma, "utf16" }
  };

}

/*
 * declare old-style api functions
 */
extern "C"  int   MLMAPROC  mlmaruLoadMbAPI( IMlmaMb** );
extern "C"  int   MLMAPROC  mlmaruLoadCpAPI( IMlmaMb**, const char* );
extern "C"  int   MLMAPROC  mlmaruLoadWcAPI( IMlmaWc** );

/*
 * declare public style api function
 */
extern "C"  int   MLMAPROC  mlmaruGetAPI( const char*, void** );

using namespace NAMESPACE;

extern "C"  int   MLMAPROC  mlmaruLoadMbAPI( IMlmaMb**  ptrAPI )
{
  if ( ptrAPI == nullptr )
    return EINVAL;
  (*ptrAPI = &mbInstance1251)->Attach();
    return 0;
}

extern "C"  int   MLMAPROC  mlmaruLoadCpAPI( IMlmaMb**  ptrAPI, const char* codepage )
{
// check API pointer
  if ( ptrAPI == nullptr || codepage == nullptr || *codepage == '\0' )
    return EINVAL;

// detect the codepage
  for ( auto& page: codepageList )
    if ( strcasecmp( page.codepage, codepage ) == 0 )
    {
      if ( page.instance == (IMlmaMb*)&wcInstanceMlma )
        return EINVAL;
      return (*ptrAPI = page.instance)->Attach(), 0;
    }

  return *ptrAPI = nullptr, EINVAL;
}

extern "C"  int   MLMAPROC  mlmaruLoadWcAPI( IMlmaWc**  ptrAPI )
{
  if ( ptrAPI == nullptr )
    return EINVAL;
  (*ptrAPI = &wcInstanceMlma)->Attach();
    return 0;
}

extern "C"  int   MLMAPROC  mlmaruGetAPI( const char* apiKey, void** ppvAPI )
{
  // check call parameters
  if ( ppvAPI == nullptr )
    return EINVAL;
  if ( apiKey == nullptr || memcmp( apiKey, LIBMORPH_API_4_MAGIC ":", sizeof(LIBMORPH_API_4_MAGIC) ) != 0 )
    return EINVAL;

  // detect the codepage
  for ( auto& page: codepageList )
    if ( strcasecmp( page.codepage, apiKey + sizeof(LIBMORPH_API_4_MAGIC) ) == 0 )
      return reinterpret_cast<IMlmaMb*>( *ppvAPI = page.instance )->Attach(), 0;

  return *ppvAPI = nullptr, EINVAL;
}

extern "C" void __stack_chk_fail(void) {}
