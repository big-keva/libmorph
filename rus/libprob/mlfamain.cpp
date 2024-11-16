/******************************************************************************

    libmorphrus - dictiorary-based morphological analyser for Russian.
    Copyright (C) 1994-2016 Andrew Kovalenko aka Keva

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

    Commercial license is available upon request.

    Contacts:
      email: hige.keva@gmail.com, keva@rambler.ru
      Skype: big_keva
      Phone: +7(495)648-4058, +7(926)513-2991

******************************************************************************/
# include "scandict.hpp"
# include "buildforms.hpp"
# include "lemmatizer.hpp"
# include <cstring>

# if !defined( _WIN32_WCE )
  # define  CATCH_ALL         try {
  # define  ON_ERRORS( code ) } catch ( ... ) { return (code); }
# else
  # define  CATCH_ALL
  # define  ON_ERRORS( code )
# endif  // ! _WIN32_WCE

namespace libmorph {
namespace rus {

  //
  // the new api - IMlma interface class
  //
  class  CMlfaMb: public IMlfaMb
  {
    static const auto CPS_1251 = codepages::codepage_1251;
    static const auto CPS_UTF8 = codepages::codepage_utf8;

    const unsigned  codepage;

  public:     // construction
    CMlfaMb( unsigned cp = CPS_1251 ): codepage( cp ) {}

  public:
    int MLMAPROC  Attach() override {  return 0;  }
    int MLMAPROC  Detach() override {  return 1;  }

  public:     // overridables
    int MLMAPROC  GetWdInfo(
      unsigned char*  pwinfo, unsigned uclass ) override;
    int MLMAPROC  GetSample(
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
    int MLMAPROC  FindMatch(
      IMatchStemMb* pmatch,
      const char*   pszstr, size_t    cchstr ) override;

  protected:
    template <size_t N>
    unsigned      ToCanonic( uint8_t (&)[N], const char*, size_t ) const;

  };

  class  CMlfaCp final: public CMlfaMb
  {
    using CMlfaMb::CMlfaMb;

  public:
    int MLMAPROC  Attach()  override;
    int MLMAPROC  Detach()  override;

  protected:  // codepage
    long      refcount = 0;

  };

  class CMlfaWc: public IMlfaWc
  {
  public:     // lifetime control
    int MLMAPROC  Attach()  override  {  return 0;  }
    int MLMAPROC  Detach()  override  {  return 1;  }

  public:     // overridables
    int MLMAPROC  GetWdInfo(
      unsigned char*  pwinfo, unsigned uclass ) override;
    int MLMAPROC  GetSample(
      widechar*     sample, size_t      samlen,
      unsigned      uclass ) override;
    int MLMAPROC  Lemmatize(
      const widechar* pszstr, size_t    cchstr,
      SStemInfoW*     output, size_t    cchout,
      widechar*       plemma, size_t    clemma,
      SGramInfo*      pgrams, size_t    ngrams, unsigned  dwsets )  override;
    int MLMAPROC  BuildForm(
      widechar*       output, size_t    cchout,
      const widechar* lpstem, size_t    ccstem,
      unsigned        nclass, formid_t  idform ) override;
    int MLMAPROC  FindMatch(
      IMlmaMatch* pmatch,
      const widechar* pszstr, size_t    cchstr ) override;

  };

  CMlfaMb mlfaMbInstance;
  CMlfaWc mlfaWcInstance;

  int   CMlfaMb::GetWdInfo(
    unsigned char*  pwinfo, unsigned uclass )
  {
    try
    {
      if ( pwinfo == nullptr )
        return ARGUMENT_FAILED;
      ::FetchFrom( libfuzzy::rus::GetClass( uclass ), *pwinfo );
        return 1;
    }
    catch ( const std::invalid_argument& )
    {
      return ARGUMENT_FAILED;
    }
  }

  int   CMlfaMb::GetSample(
    char*     sample,
    size_t    samlen,
    unsigned  uclass )
  {
    if ( sample == nullptr || samlen == 0 )
      return ARGUMENT_FAILED;

    try
    {
      uint8_t partSp;
      int     fcount;
      auto    pclass =
        ::FetchFrom(
        ::FetchFrom( libfuzzy::rus::GetClass( uclass ), partSp ), fcount );

      while ( fcount-- > 0 )
      {
        uint8_t idform;
        uint8_t ccflex;

        pclass = ::FetchFrom( ::FetchFrom( pclass, idform ), ccflex );
          pclass += ccflex;
      }

      pclass = ::FetchFrom( pclass, fcount );

      return ToCodepage( codepage, sample, samlen, pclass, fcount ) == (size_t)-1 ?
        LEMMBUFF_FAILED : 1;
    }
    catch ( const std::invalid_argument& )
    {
      return ARGUMENT_FAILED;
    }
  }

  int   CMlfaMb::Lemmatize(
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
      auto  lemmatizer = libfuzzy::rus::Lemmatizer( CapScheme(
          charTypeMatrix, toLoCaseMatrix,
          toUpCaseMatrix, pspMinCapValue ),
        { plemma, clemma },
        { pforms, cforms, codepage },
        { pgrams, cgrams },
      locase, scheme >> 16, scheme, dwsets );

      if ( (nerror = libfuzzy::rus::patricia::ScanTree( lemmatizer, (const char*)libfuzzyrus::ReverseDict,
        (const char*)locase + (scheme >> 16) - 1, scheme >> 16 )) != 0 )
          return nerror;

      return lemmatizer;
    ON_ERRORS( -1 )
  }

  int   CMlfaMb::BuildForm(
    char*       output, size_t    cchout,
    const char* pszstr, size_t    cchstr,
    unsigned    nclass, formid_t  idform )
  {
    try
    {
      uint8_t locase[256];
      auto    scheme = ToCanonic( locase, pszstr, cchstr );

    // check the args
      if ( output == nullptr || cchout == 0 || nclass >= libfuzzyrus::ClassNumber )
        return ARGUMENT_FAILED;

      if ( cchstr != 0 && pszstr == nullptr )
        return ARGUMENT_FAILED;

    // check source string and length
      if ( scheme == (unsigned)-1 || scheme == 0 )
        return scheme == (unsigned)-1 ? WORDBUFF_FAILED : 0;

      return libfuzzy::rus::Buildform( CapScheme(
          charTypeMatrix, toLoCaseMatrix,
          toUpCaseMatrix, pspMinCapValue ), { output, cchout, codepage } )
        ( locase, scheme >> 8, nclass, idform );
    }
    catch ( const std::invalid_argument& ) {  return ARGUMENT_FAILED;  }
    catch ( ... ) {  return -1;  }
  }

  int   CMlfaMb::FindMatch( IMatchStemMb* pmatch, const char* pszstr, size_t  cchstr )
  {
    CATCH_ALL
      uint8_t   locase[256];
      uint8_t   strbuf[256];
      auto      scheme = ToCanonic( locase, pszstr, cchstr );
      auto      aforms = std::vector<char>( 0x100 * 0x100 * 2 );
      char*     pforms = aforms.data();

      char      slemma[256];
      size_t    clemma = (size_t)-1;

      unsigned  cclass = 0;

      SStrMatch amatch[256];
      size_t    cmatch = 0;

      int       nerror;
      auto      fflush = [&]()
      {
        char    encode[256];
        size_t  cbytes = 0;
        uint8_t partsp;

        for ( auto p = slemma, e = slemma + clemma; p != e; ++p )
          cbytes += libmorph::rus::ToCodepage( codepage, encode + cbytes, sizeof(encode) - cbytes, p, 1 );

        pmatch->Add( encode, clemma, cbytes, cclass, cmatch, amatch );
      };
      auto      reglex = [&](
        const fragment& szstem,
        const fragment& szflex,
        unsigned        uclass,
        unsigned        uoccur,
        uint8_t         idform )
      {
        char  szform[256];
        auto  ccstem = (size_t)(szstem.size() + 2);     // длина текущей основы
        auto  outptr = std::copy( szstem.begin(), szstem.end(), szform );
//        auto  flprob =

        for ( auto  srcbeg = szflex.end() - 1; srcbeg >= szflex.begin(); )
          *outptr++ = *srcbeg--;

      // check stem and class match to previous call
        if ( uclass != cclass || ccstem != clemma || memcmp( szform, slemma, ccstem ) != 0 )
        {
          if ( cclass != 0 )
            fflush();

          memcpy( slemma, szform,
            clemma = ccstem );
          cclass = uclass;
          pforms = aforms.data();
          cmatch = 0;
        }

      // append stem form
        amatch[cmatch] =
          {
            sz: pforms,
            cc: ToCodepage( codepage, pforms, aforms.data() + aforms.size() - pforms, szform, outptr - szform ),
            id: idform,
          };
        pforms += amatch[cmatch++].cc + 1;
        return 0;
      };

    // check output buffer
      if ( pmatch == nullptr )
        return ARGUMENT_FAILED;

    // check template defined
      if ( pszstr == nullptr )
        return ARGUMENT_FAILED;

    // check source string and length
      if ( scheme == (unsigned)-1 || scheme == 0 )
        return scheme == (unsigned)-1 ? WORDBUFF_FAILED : 0;

    // scan the dictionary
      if ( (nerror = libfuzzy::rus::patricia::WildScan( reglex, (const char*)libfuzzyrus::ReverseDict, strbuf, 0,
        locase + (scheme >> 16) - 1, scheme >> 16 )) != 0 )
          return nerror;

      if ( cclass != 0 )
        fflush();

      return 0;
    ON_ERRORS( -1 )
  }

  template <size_t N>
  auto  CMlfaMb::ToCanonic( uint8_t (& output)[N], const char* pszstr, size_t cchstr ) const -> unsigned
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

    return CapScheme(
      charTypeMatrix, toLoCaseMatrix,
      toUpCaseMatrix, pspMinCapValue ).Get( output, pszstr, cchstr );
  }

  // CMlmaCp implementation

  int   CMlfaCp::Attach()
  {
    return ++refcount;
  }

  int   CMlfaCp::Detach()
  {
    long  rcount;
    
    if ( (rcount = --refcount) == 0 )
    {
      this->~CMlfaCp();
      free( this );
    }
    return rcount;
  }

  // CMlfaWc wrapper implementation

  int   CMlfaWc::GetWdInfo(
    unsigned char*  pwinfo, unsigned uclass )
  {
    return mlfaMbInstance.GetWdInfo( pwinfo, uclass );
  }

  int   CMlfaWc::GetSample(
    widechar*       sample, size_t  samlen,
    unsigned        uclass )
  {
    char  buffer[256];
    int   nerror;

    if ( (nerror = mlfaMbInstance.GetSample( buffer, samlen, uclass )) <= 0 )
      return nerror;

    return ToWidechar( sample, samlen, buffer ) == (size_t)-1 ?
      LEMMBUFF_FAILED : 1;
  }

  int   CMlfaWc::Lemmatize(
    const widechar* pwsstr, size_t  cchstr,
    SStemInfoW*     output, size_t  outlim,
    widechar*       plemma, size_t  clemma,
    SGramInfo*      pgrams, size_t  ngrams, unsigned  dwsets )
  {
    SStemInfoA  stbuff[0x20];
    char        szlemm[0x200];
    char        szword[0xf0];
    char*       lplemm;
    size_t      ccword;
    int         lcount;
    int         lindex;

  // get string length and convert to ansi
    if ( (ccword = ToInternal( szword, pwsstr, cchstr )) == (size_t)-1 )
      return WORDBUFF_FAILED;

  // call default lemmatizer
    if ( (lcount = mlfaMbInstance.Lemmatize( szword, ccword,
      output != nullptr ? stbuff : nullptr, outlim <= 32 ? outlim : 32,
      plemma != nullptr ? szlemm : nullptr, clemma <= sizeof(szlemm) ? clemma : sizeof(szlemm),
      pgrams, ngrams, dwsets )) <= 0 )
        return lcount;

  // fill output data
    if ( output != nullptr )
      for ( auto mbstem = stbuff; mbstem != stbuff + lcount; ++mbstem )
      {
        *output++ = {
          mbstem->ccstem, mbstem->nclass,
          mbstem->plemma != nullptr ? plemma + (mbstem->plemma - szlemm) : nullptr,
          mbstem->pgrams, mbstem->ngrams, mbstem->weight };
      }

  // fill lemmaized strings
    if ( plemma != nullptr )
      for ( lindex = 0, lplemm = szlemm; lindex < lcount; ++lindex )
      {
        auto  nccstr = ToWidechar( plemma, clemma, lplemm ) + 1;
          plemma += nccstr;
          clemma -= nccstr;
          lplemm += nccstr;
      }

    return lcount;
  }

  int   CMlfaWc::BuildForm(
    widechar*       output, size_t    cchout,
    const widechar* plemma, size_t    clemma,
    unsigned        nclass, formid_t  idform )
  {
    char  slemma[0x40];
    char  szform[0x60];
    char* lpform;
    int   fcount;
    int   findex;

  // check form buffer
    if ( output == nullptr )
      return WORDBUFF_FAILED;

  // create stem string
    if ( plemma == nullptr )
      return LEMMBUFF_FAILED;

    if ( ToInternal( slemma, plemma, clemma ) == (size_t)-1 )
      return WORDBUFF_FAILED;

  // build the form
    fcount = mlfaMbInstance.BuildForm( szform, std::min( cchout, sizeof(szform) - 1 ),
      slemma, clemma, nclass, idform );

  // convert created strings if available
    for ( findex = 0, lpform = szform; findex < fcount; ++findex )
    {
      auto  cchstr = ToWidechar( output, cchout, lpform ) + 1;
        output += cchstr;
        cchout -= cchstr;
        lpform += cchstr;
    }

    return fcount;
  }

  int   CMlfaWc::FindMatch( IMlmaMatch* pmatch,
    const widechar* pwsstr, size_t  cchstr )
  {
    /*
    char        szword[0xf0];
    size_t      ccword;

  // get string length and convert to native codepage
    if ( (ccword = ToInternal( szword, pwsstr, cchstr )) == (size_t)-1 )
      return WORDBUFF_FAILED;

    */return 0;//mlfaMbInstance.FindMatch( pmatch, szword, ccword );
  }

  struct
  {
    unsigned    idcodepage;
    const char* szcodepage;
  } codepageList[] =
  {
    { codepages::codepage_1251, "Windows-1251" },
    { codepages::codepage_1251, "Windows" },
    { codepages::codepage_1251, "1251" },
    { codepages::codepage_1251, "Win-1251" },
    { codepages::codepage_1251, "Win" },
    { codepages::codepage_1251, "Windows 1251" },
    { codepages::codepage_1251, "Win 1251" },
    { codepages::codepage_1251, "ansi" },
    { codepages::codepage_koi8, "koi-8" },
    { codepages::codepage_koi8, "koi8" },
    { codepages::codepage_koi8, "20866" },
    { codepages::codepage_866,  "dos" },
    { codepages::codepage_866,  "oem" },
    { codepages::codepage_866,  "866" },
    { codepages::codepage_iso,  "28595" },
    { codepages::codepage_iso,  "iso-88595" },
    { codepages::codepage_iso,  "iso-8859-5" },
    { codepages::codepage_mac,  "10007" },
    { codepages::codepage_iso,  "mac" },
    { codepages::codepage_utf8, "65001" },
    { codepages::codepage_utf8, "utf-8" },
    { codepages::codepage_utf8, "utf8" }
  };

}}

using namespace libmorph::rus;

int   MLMAPROC        mlfaruLoadMbAPI( IMlfaMb**  ptrAPI )
{
  if ( ptrAPI == nullptr )
    return -1;
  *ptrAPI = (IMlfaMb*)&mlfaMbInstance;
    return 0;
}

int   MLMAPROC        mlfaruLoadCpAPI( IMlfaMb**  ptrAPI, const char* codepage )
{
  CMlfaMb*  palloc;
  unsigned  pageid = (unsigned)-1;

  for ( auto& page: codepageList )
    if ( strcasecmp( page.szcodepage, codepage ) == 0 )
      {  pageid = page.idcodepage;  break;  }

  if ( pageid == (unsigned)-1 || ptrAPI == nullptr )
    return EINVAL;

  if ( pageid == codepages::codepage_1251 )
    return mlfaruLoadMbAPI( ptrAPI );

  if ( (palloc = new( std::nothrow ) CMlfaMb( pageid )) == nullptr )
    return ENOMEM;
  (*ptrAPI = (IMlfaMb*)palloc)->Attach();
    return 0;
}

int   MLMAPROC  mlfaruLoadWcAPI( IMlfaWc**  ptrAPI )
{
  if ( ptrAPI == nullptr )
    return -1;
  *ptrAPI = (IMlfaWc*)&mlfaWcInstance;
    return 0;
}
