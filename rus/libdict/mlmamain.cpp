/******************************************************************************

    libmorphrus - dictiorary-based morphological analyser for Russian.

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
      email: keva@meta.ua, keva@rambler.ru
      Phone: +7(495)648-4058, +7(926)513-2991, +7(707)129-1418

******************************************************************************/
# include "xmorph/wildscan.h"
# include "xmorph/charlist.h"
# include "../include/mlma1049.h"
# include "../codepages.hpp"
# include "../chartype.h"
# include "mlmadefs.h"
# include "scanClass.hpp"
# include "wildClass.hpp"
# include "lemmatize.hpp"
# include "buildform.hpp"
# include <cstdlib>
# include <cstring>
# include <errno.h>

# if !defined( _WIN32_WCE )
  # define  CATCH_ALL         try {
  # define  ON_ERRORS( code ) } catch ( ... ) { return (code); }
# else
  # define  CATCH_ALL
  # define  ON_ERRORS( code )
# endif  // ! _WIN32_WCE

namespace libmorph {
namespace rus {

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
  struct  CMlmaMb: public IMlmaMb
  {
    int MLMAPROC  Attach() override {  return 0;  }
    int MLMAPROC  Detach() override {  return 0;  }

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
    int MLMAPROC  CheckHelp( char*          output, size_t    cchout,
                             const char*    pszstr, size_t    cchstr ) override;
    int MLMAPROC  GetWdInfo( unsigned char* pwindo, lexeme_t  nlexid ) override;
    int MLMAPROC  FindMatch( IMlmaMatch*    pienum,
                             const char*    pszstr, size_t    cchstr ) override;

  public:     // construction
    CMlmaMb( unsigned cp = codepages::codepage_1251 ): codepage( cp ) {}

  protected:
    template <size_t N>
    int           GetJocker( uint8_t (&)[N], const char*, size_t ) const;
    template <size_t N>
    unsigned      ToCanonic( uint8_t (&)[N], const char*, size_t ) const;

  protected:  // codepage
    unsigned  codepage;

  };

  struct  CMlmaCp final: public CMlmaMb
  {
    virtual int MLMAPROC  Attach();
    virtual int MLMAPROC  Detach();

  public:     // construction
    CMlmaCp( unsigned cp ): CMlmaMb( cp ), refcount( 0 ) {}

  protected:  // codepage
    long      refcount;

  };

  struct  CMlmaWc: public IMlmaWc
  {
    int MLMAPROC  Attach() override {  return 0;  }
    int MLMAPROC  Detach() override {  return 0;  }

    int MLMAPROC  CheckWord( const widechar*  pszstr, size_t    cchstr,
                             unsigned         dwsets )  override;
    int MLMAPROC  Lemmatize( const widechar*  pszstr, size_t    cchstr,
                             SLemmInfoW*      output, size_t    cchout,
                             widechar*        plemma, size_t    clemma,
                             SGramInfo*       pgrams, size_t    ngrams,
                             unsigned         dwsets )  override;
    int MLMAPROC  BuildForm( widechar*        output, size_t    cchout,
                             lexeme_t         nlexid, formid_t  idform )  override;
    int MLMAPROC  FindForms( widechar*        output, size_t    cchout,
                             const widechar*  pwsstr, size_t    cchstr,
                             unsigned char    idform, unsigned  dwsets )  override;
    int MLMAPROC  CheckHelp( widechar*        output, size_t    cchout,
                             const widechar*  pwsstr, size_t    cchstr )  override;
    int MLMAPROC  GetWdInfo( unsigned char*   pwindo, lexeme_t  nlexid )  override;
    int MLMAPROC  FindMatch( IMlmaMatch*      pmatch,
                             const widechar*  pszstr, size_t    cchstr )  override;
  };

  CMlmaMb mlmaMbInstance;
  CMlmaWc mlmaWcInstance;

  // CMlmaMb implementation

  int   CMlmaMb::CheckWord( const char* pszstr, size_t  cchstr, unsigned  dwsets )
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
      libmorphrus::stemtree, { locase, scheme >> 16 } );
    ON_ERRORS( -1 )
  }

  int   CMlmaMb::Lemmatize( const char* pszstr, size_t  cchstr,
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
      libmorphrus::stemtree, { locase, scheme >> 16 } );

      return nerror < 0 ? nerror : lemmatize;
    ON_ERRORS( -1 )
  }

  int   CMlmaMb::BuildForm( char* output, size_t cchout, lexeme_t nlexid, formid_t idform )
  {
    CATCH_ALL
      uint8_t         lidkey[0x10];
      const uint8_t*  ofsptr;
      auto            getofs = []( const byte_t* thedic, const fragment& str ) {  return str.empty() ? thedic : nullptr;  };

    // check the arguments
      if ( output == nullptr || cchout == 0 )
        return ARGUMENT_FAILED;

    // No original word form; algo jumps to lexeme block dictionary point by lexeme id
      if ( (ofsptr = Flat::ScanTree<uint16_t>( getofs, libmorphrus::lidstree, { lidkey, lexkeylen( lidkey, nlexid ) } )) != nullptr )
      {
        auto    dicpos = libmorphrus::stemtree + getserial( ofsptr );
        byte_t  szstem[maximal::form_length] = {};
        int     nerror;

      // fill other fields
        auto  buildform = FormBuild( { output, cchout, codepage }, CapScheme( charTypeMatrix, toLoCaseMatrix, toUpCaseMatrix ),
          nlexid, idform, szstem );

        nerror = Flat::GetTrack<uint8_t>( Flat::ViewList( MakeBuildClass( buildform ), dicpos ),
          libmorphrus::stemtree, szstem, 0, dicpos );

        return nerror < 0 ? nerror : buildform;
      }
      return 0;
    ON_ERRORS( -1 )
  }

  int   CMlmaMb::FindForms(
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
      libmorphrus::stemtree, { locase, scheme >> 16 } );

      return nerror < 0 ? nerror : buildform;
    ON_ERRORS( -1 )
  }

 /*
  * CheckHelp( output, cchout, pszstr, cchstr )
  *
  * Реализация через FindMatch( ... ) требует предварительной обработки шаблона к форме
  * 'либо одна звёздочка в конце, либо один знак вопроса где угодно'
  */
  int   CMlmaMb::CheckHelp( char* output, size_t  cchout, const char* pszstr, size_t  cchstr )
  {
    CATCH_ALL
      uint8_t locase[maximal::form_length];
      uint8_t smatch[maximal::form_length];
      auto    scheme = ToCanonic( locase, pszstr, cchstr );
      size_t  qtrpos;       // позиция квантора в строке
      size_t  keylen;       // длина поискового ключа
      Charset achars;
      int     nerror;

    // check source string and length
      if ( scheme == (unsigned)-1 || scheme == 0 )
        return scheme == (unsigned)-1 ? WORDBUFF_FAILED : 0;

    // check output buffer
      if ( output == nullptr || cchout == 0 )
        return ARGUMENT_FAILED;

    // провериьт наличие и рассчитать позицию квантора
      for ( auto  hasqtr = keylen = (qtrpos = size_t(-1)) + 1; keylen != (scheme >> 16); )
      {
        auto  chnext = locase[keylen++];

        if ( chnext == '*' )
        {
          qtrpos = keylen - 1;
          break;
        }
          else
        if ( chnext == '?' )
        {
          if ( hasqtr++ == 0 )  qtrpos = keylen - 1;
            else return ARGUMENT_FAILED;
        }
      }

    // проверить наличие кванторов
      if ( qtrpos != size_t(-1) && keylen != 0 )  locase[keylen] = '\0';
        else return 0;

    // scan the dictionary
      if ( (nerror = Wild::ScanTree<uint8_t>(
        MakeModelMatch( [&]( lexeme_t, const uint8_t* str, size_t len, const SGramInfo& )
          {  return achars( qtrpos < len ? str[qtrpos] : uint8_t(0) ), 0;  } ),
        libmorphrus::stemtree, { locase, scheme >> 16 }, smatch, 0 )) != 0 )
      return nerror;

      return achars( MbcsCoder( output, cchout, codepage ).object() );
    ON_ERRORS( -1 )
  }

  int   CMlmaMb::GetWdInfo( unsigned char* pwinfo, lexeme_t lexkey )
  {
    CATCH_ALL
      byte_t        lidkey[0x10];
      const byte_t* ofsptr;
      auto          getofs = []( const byte_t* thedic, const fragment& str ){  return str.empty() ? thedic : nullptr;  };

    // No original word form; algo jumps to lexeme block dictionary point by lexeme id
      if ( (ofsptr = Flat::ScanTree<word16_t>( getofs, libmorphrus::lidstree, { lidkey, lexkeylen( lidkey, lexkey ) } )) != nullptr )
      {
        const byte_t* dicpos = libmorphrus::stemtree + getserial( ofsptr ) + 2; /* 2 => clower && cupper */
        lexeme_t      nlexid = getserial( dicpos );
        word16_t      oclass = getword16( dicpos );
        steminfo      stinfo;

        if ( nlexid != lexkey )
          return LIDSBUFF_FAILED;

        *pwinfo = stinfo.Load( libmorphrus::classmap + (oclass & 0x7fff) ).wdinfo & 0x3f;
          return 1;
      }
      return 0;
    ON_ERRORS( -1 )
  }

  int   CMlmaMb::FindMatch( IMlmaMatch*  pmatch, const char* pszstr, size_t cchstr )
  {
    CATCH_ALL
      char      cpsstr[maximal::form_length];
      uint8_t   locase[maximal::form_length];
      uint8_t   sforms[maximal::buffer_size];
      SStrMatch amatch[maximal::forms_count];
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
        if ( nlexid != lastId )
        {
          if ( lastId != 0 )
            pmatch->RegisterLexeme( lastId, nmatch, amatch );
          lastId = nlexid;
          pforms = sforms;
          nmatch = 0;
        }

        for ( auto prev = amatch, last = amatch + nmatch; prev != last; ++prev )
          if ( prev->id == grinfo.idForm && prev->cc == length && memcmp( prev->sz, string, length ) == 0 )
            return 0;

        amatch[nmatch++] = { strncpy( (char*)pforms, (const char*)string, length ), length, grinfo.idForm };
          pforms += length + 1;

        return 0;
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
        libmorphrus::stemtree, { locase, cchstr }, (uint8_t*)cpsstr, 0 )) != 0 )
      return nerror;

      return lastId != 0 ? pmatch->RegisterLexeme( lastId, nmatch, amatch ) : 0;
    ON_ERRORS( -1 )
  }

  template <size_t N>
  auto  CMlmaMb::ToCanonic( uint8_t (& output)[N], const char* pszstr, size_t cchstr ) const -> unsigned
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

  // CMlmaCp implementation

  int   CMlmaCp::Attach()
  {
    return ++refcount;
  }

  int   CMlmaCp::Detach()
  {
    long  rcount;
    
    if ( (rcount = --refcount) == 0 )
    {
      this->~CMlmaCp();
      free( this );
    }
    return rcount;
  }

  // CMlmaWc wrapper implementation

  int   CMlmaWc::CheckWord( const widechar* pwsstr, size_t  cchstr, unsigned  dwsets )
  {
    char    szword[maximal::form_length];
    size_t  ccword;

  // get string length and convert to ansi
    if ( (ccword = codepages::widetombcs( codepages::codepage_1251, szword, sizeof(szword), (const widechar*)pwsstr, cchstr )) == (size_t)-1 )
      return WORDBUFF_FAILED;
    return mlmaMbInstance.CheckWord( szword, ccword, dwsets );
  }

  int   CMlmaWc::Lemmatize( const widechar* pwsstr, size_t  cchstr,
                            SLemmInfoW*     output, size_t  cchout,
                            widechar*       plemma, size_t  clemma,
                            SGramInfo*      pgrams, size_t  ngrams,
                            unsigned        dwsets )
  {
    SLemmInfoA  lmbuff[0x20];
    char        szlemm[0xf0];
    char        szword[0xf0];
    char*       lplemm;
    size_t      ccword;
    int         lcount;
    int         lindex;

  // get string length and convert to ansi
    if ( (ccword = codepages::widetombcs( codepages::codepage_1251, szword, 0xf0, (const widechar*)pwsstr, cchstr )) == (size_t)-1 )
      return WORDBUFF_FAILED;

  // call default lemmatizer
    if ( (lcount = mlmaMbInstance.Lemmatize( szword, ccword,
      output != nullptr ? lmbuff : nullptr, cchout <= 32 ? cchout : 32,
      plemma != nullptr ? szlemm : nullptr, clemma <= sizeof(szlemm) ? clemma : sizeof(szlemm),
      pgrams, ngrams, dwsets )) <= 0 )
        return lcount;

  // fill output data
    if ( output != nullptr )
      for ( lindex = 0; lindex < lcount; lindex++, output++ )
      {
        output->nlexid = lmbuff[lindex].nlexid;
        output->pgrams = lmbuff[lindex].pgrams;
        output->ngrams = lmbuff[lindex].ngrams;
        output->plemma = lmbuff[lindex].plemma == nullptr ? nullptr :
            plemma + (lmbuff[lindex].plemma - szlemm);
      }
    if ( plemma != nullptr )
      for ( lindex = 0, lplemm = szlemm; lindex < lcount; ++lindex )
      {
        size_t    nccstr = codepages::mbcstowide( codepages::codepage_1251, (widechar*)plemma, clemma, lplemm ) + 1;
          
        plemma += nccstr;
        clemma -= nccstr;
        lplemm += nccstr;
      }
    return lcount;
  }

  int   CMlmaWc::BuildForm( widechar* output, size_t    cchout,
                            lexeme_t  nlexid, formid_t  idform )
  {
    char        szform[98];
    char*       lpform;
    int         fcount;
    int         findex;

  // build the form
    if ( (fcount = mlmaMbInstance.BuildForm( szform, cchout <= sizeof(szform) - 1 ?
      cchout : sizeof(szform) - 1, nlexid, idform )) <= 0 )
        return fcount;

  // convert
    for ( findex = 0, lpform = szform; findex < fcount; ++findex )
    {
      size_t    nccstr = codepages::mbcstowide( codepages::codepage_1251, output, cchout, lpform ) + 1;

      output += nccstr;
      cchout -= nccstr;
      lpform += nccstr;
    }

    return fcount;
  }

  int   CMlmaWc::FindForms(
    widechar*       output, size_t    cchout,
    const widechar* pwsstr, size_t    cchstr,
    formid_t        idform, unsigned  dwsets )
  {
    char    szword[98];
    char    szform[98];
    char*   lpform;
    size_t  ccword;
    int     fcount;
    int     findex;

  // get string length and convert to ansi
    if ( (ccword = codepages::widetombcs( codepages::codepage_1251, szword, sizeof(szword) - 1, pwsstr, cchstr )) == (size_t)-1 )
      return WORDBUFF_FAILED;

  // build the form
    if ( (fcount = mlmaMbInstance.FindForms( szform, cchout <= sizeof(szform) - 1 ?
      cchout : sizeof(szform) - 1, szword, ccword, idform, dwsets )) <= 0 )
        return fcount;

  // convert
    for ( findex = 0, lpform = szform; findex < fcount; ++findex )
    {
      size_t    nccstr = codepages::mbcstowide( codepages::codepage_1251, output, cchout, lpform ) + 1;

      output += nccstr;
      cchout -= nccstr;
      lpform += nccstr;
    }

    return fcount;
  }

  int   CMlmaWc::CheckHelp(       widechar* output, size_t  cchout,
                            const widechar* pwsstr, size_t  cchstr )
  {
    char    szword[98];
    char    chhelp[98];
    size_t  ccword;
    int     ccount;

  // get string length and convert to native codepage
    if ( (ccword = codepages::widetombcs( codepages::codepage_1251, szword, sizeof(szword) - 1, pwsstr, cchstr )) == (size_t)-1 )
      return WORDBUFF_FAILED;

  // build the form
    if ( (ccount = mlmaMbInstance.CheckHelp( chhelp, cchout <= sizeof(chhelp) ?
      cchout : sizeof(chhelp), szword, ccword )) <= 0 )
        return ccount;

  // convert
    codepages::mbcstowide( codepages::codepage_1251, output, cchout, chhelp, ccount );
    return ccount;
  }

  int   CMlmaWc::GetWdInfo( unsigned char* pwinfo, lexeme_t nlexid )
  {
    return mlmaMbInstance.GetWdInfo( pwinfo, nlexid );
  }

  int   CMlmaWc::FindMatch( IMlmaMatch* pmatch, const widechar* pwsstr, size_t cchstr )
  {
    char    szword[maximal::form_length];
    size_t  ccword;

  // get string length and convert to native codepage
    if ( (ccword = codepages::widetombcs( codepages::codepage_1251, szword, sizeof(szword) - 1, pwsstr, cchstr )) == (size_t)-1 )
      return WORDBUFF_FAILED;

    return mlmaMbInstance.FindMatch( pmatch, szword, ccword );
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

int   MLMAPROC        mlmaruLoadMbAPI( IMlmaMb**  ptrAPI )
{
  if ( ptrAPI == nullptr )
    return -1;
  *ptrAPI = (IMlmaMb*)&mlmaMbInstance;
    return 0;
}

int   MLMAPROC        mlmaruLoadCpAPI( IMlmaMb**  ptrAPI, const char* codepage )
{
  CMlmaMb*  palloc;
  unsigned  pageid = (unsigned)-1;

  for ( auto& page: codepageList )
    if ( strcasecmp( page.szcodepage, codepage ) == 0 )
      {  pageid = page.idcodepage;  break;  }

  if ( pageid == (unsigned)-1 || ptrAPI == nullptr )
    return EINVAL;

  if ( pageid == codepages::codepage_1251 )
    return mlmaruLoadMbAPI( ptrAPI );

  if ( (palloc = (CMlmaMb*)malloc( sizeof(*palloc) )) == nullptr )
    return ENOMEM;

  (*ptrAPI = (IMlmaMb*)new( palloc ) CMlmaMb( pageid ))->Attach();
    return 0;
}

int   MLMAPROC        mlmaruLoadWcAPI( IMlmaWc**  ptrAPI )
{
  if ( ptrAPI == nullptr )
    return -1;
  *ptrAPI = (IMlmaWc*)&mlmaWcInstance;
    return 0;
}

