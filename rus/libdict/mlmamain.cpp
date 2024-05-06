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
      email: keva@meta.ua, keva@rambler.ru
      Skype: big_keva
      Phone: +7(495)648-4058, +7(926)513-2991

******************************************************************************/
# include "../include/mlma1049.h"
# include <namespace.h>
# include "mlmadefs.h"
# include "../../xmorph/scandict.h"
# include "../../xmorph/capsheme.h"
# include "../../xmorph/scanlist.h"
# include "../../xmorph/wildscan.h"
# include "../../xmorph/lemmatiz.h"
# include "moonycode/codes.h"
# include <stdlib.h>
# include <string.h>
# include <errno.h>

# if !defined( _WIN32_WCE )
  # define  CATCH_ALL         try {
  # define  ON_ERRORS( code ) } catch ( ... ) { return (code); }
# else
  # define  CATCH_ALL
  # define  ON_ERRORS( code )
# endif  // ! _WIN32_WCE

namespace LIBMORPH_NAMESPACE
{

  inline  int     igncasecmp( const char* s1, const char* s2 )
  {
    const auto  lc = []( char ch ){  return codepages::chartolower( codepages::codepage_1251, ch );  };
    int         rc;

    while ( (rc = lc( *s1 ) - lc( *s2++ )) == 0 && *s1++ != '\0' )
      (void)NULL;
    return rc;
  }

  //
  // the new api - IMlma interface class
  //
  struct  CMlmaMb: public IMlmaMb
  {
    int MLMAPROC  Attach() override {  return 0;  }
    int MLMAPROC  Detach() override {  return 0;  }

    int MLMAPROC  SetLoCase( char*            outstr, size_t    cchout,
                             const char*      srcstr, size_t    cchsrc ) override;
    int MLMAPROC  SetUpCase( char*            outstr, size_t    cchout,
                             const char*      srcstr, size_t    cchsrc ) override;
    int MLMAPROC  CheckWord( const char*      pszstr, size_t    cchstr,
                             unsigned         dwsets )                   override;
    int MLMAPROC  Lemmatize( const char*      pszstr, size_t    cchstr,
                             SLemmInfoA*      output, size_t    cchout,
                             char*            plemma, size_t    clemma,
                             SGramInfo*       pgrams, size_t    ngrams,
                             unsigned         dwsets )                   override;
    int MLMAPROC  BuildForm( char*            output, size_t    cchout,
                             lexeme_t         nlexid, formid_t  idform ) override;
    int MLMAPROC  FindForms( char*            output, size_t    cchout,
                             const char*      pszstr, size_t    cchstr,
                             formid_t         idform )                   override;
    int MLMAPROC  CheckHelp( char*            output, size_t    cchout,
                             const char*      pszstr, size_t    cchstr ) override;
    int MLMAPROC  GetWdInfo( unsigned char*   pwindo, lexeme_t  nlexid ) override;
    int MLMAPROC  EnumWords( IMlmaEnum*       pienum,
                             const char*      pszstr, size_t    cchstr ) override;
    int MLMAPROC  FindMatch( IMlmaMatch*      pienum,
                             const char*      pszstr, size_t    cchstr ) override;

  public:     // construction
    CMlmaMb( unsigned cp = codepages::codepage_1251 ): codepage( cp ) {}

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
    virtual int MLMAPROC  Attach() {  return 0;  }
    virtual int MLMAPROC  Detach() {  return 0;  }

    virtual int MLMAPROC  SetLoCase( widechar*        pwsstr, size_t    cchstr );
    virtual int MLMAPROC  SetUpCase( widechar*        pwsstr, size_t    cchstr );
    virtual int MLMAPROC  CheckWord( const widechar*  pszstr, size_t    cchstr,
                                     unsigned         dwsets );
    virtual int MLMAPROC  Lemmatize( const widechar*  pszstr, size_t    cchstr,
                                     SLemmInfoW*      output, size_t    cchout,
                                     widechar*        plemma, size_t    clemma,
                                     SGramInfo*       pgrams, size_t    ngrams,
                                     unsigned         dwsets );
    virtual int MLMAPROC  BuildForm( widechar*        output, size_t    cchout,
                                     lexeme_t         nlexid, byte_t    idform );
    virtual int MLMAPROC  FindForms( widechar*        output, size_t    cchout,
                                     const widechar*  pwsstr, size_t    cchstr,
                                     unsigned char    idform );
    virtual int MLMAPROC  CheckHelp( widechar*        output, size_t    cchout,
                                     const widechar*  pwsstr, size_t    cchstr );
    virtual int MLMAPROC  GetWdInfo( unsigned char*   pwindo, lexeme_t  nlexid );
    virtual int MLMAPROC  EnumWords( IMlmaEnum*       pienum,
                                     const widechar*  pszstr, size_t    cchstr );
    virtual int MLMAPROC  FindMatch( IMlmaMatch*      pmatch,
                                     const widechar*  pszstr, size_t    cchstr );
  };

  CMlmaMb mlmaMbInstance;
  CMlmaWc mlmaWcInstance;

  // CMlmaMb implementation

  int   CMlmaMb::SetLoCase( char*       outstr, size_t  cchout,
                            const char* srcstr, size_t  cchsrc )
  {
    CATCH_ALL
      return codepages::strtolower( codepage, outstr, cchout, srcstr, cchsrc );
    ON_ERRORS( -1 )
  }

  int   CMlmaMb::SetUpCase( char*       outstr, size_t  cchout,
                            const char* srcstr, size_t  cchsrc )
  {
    CATCH_ALL
      return codepages::strtoupper( codepage, outstr, cchout, srcstr, cchsrc );
    ON_ERRORS( -1 )
  }

  int   CMlmaMb::CheckWord( const char* pszstr, size_t  cchstr, unsigned  dwsets )
  {
    CATCH_ALL
      byte_t      locase[256];
      char        cpsstr[256];
      doCheckWord scheck( locase, dwsets );

    // check source string and length
      if ( pszstr == nullptr )
        return 0;

      if ( cchstr == (size_t)-1 )
        for ( cchstr = 0; pszstr[cchstr] != 0; ++cchstr ) (void)NULL;

    // check for overflow
      if ( cchstr >= sizeof(locase) )
        return WORDBUFF_FAILED;

    // modify the codepage
      if ( codepage != codepages::codepage_1251 )
      {
        if ( (cchstr = codepages::mbcstombcs( codepages::codepage_1251, cpsstr, sizeof(cpsstr), codepage, pszstr, cchstr )) == (size_t)-1 )
          return WORDBUFF_FAILED;
        pszstr = cpsstr;
      }

    // get capitalization scheme
      scheck.scheme = GetCapScheme( locase, sizeof(locase), pszstr, cchstr ) & 0x0000ffff;

    // fill scheck structure
      return LinearScanDict<byte_t, int>( listLookup<doCheckWord, steminfo>( scheck ),
        stemtree, { locase, cchstr } );
    ON_ERRORS( -1 )
  }

  int   CMlmaMb::Lemmatize( const char* pszstr, size_t  cchstr,
                            SLemmInfoA* plemma, size_t  clemma,
                            char*       pforms, size_t  cforms,
                            SGramInfo*  pgrams, size_t  cgrams, unsigned  dwsets )
  {
    CATCH_ALL
      byte_t      locase[256];
      char        cpsstr[256];
      unsigned    scheme;
      doLemmatize lemact( locase, dwsets, codepage );

    // check source string and length
      if ( pszstr == nullptr || cchstr == 0 )
        return 0;

      if ( cchstr == (size_t)-1 )
        for ( cchstr = 0; pszstr[cchstr] != 0; ++cchstr ) (void)NULL;

    // check for utf8 or multibyte
      if ( codepage != codepages::codepage_1251 )
      {
        if ( (cchstr = codepages::mbcstombcs( codepages::codepage_1251, cpsstr, sizeof(cpsstr), codepage, pszstr, cchstr )) == (size_t)-1 )
          return WORDBUFF_FAILED;
        if ( (scheme = GetCapScheme( locase, sizeof(locase), cpsstr, cchstr )) == (unsigned)-1 )
          return WORDBUFF_FAILED;
      }
        else
      {
        if ( cchstr > sizeof(locase) )
          return WORDBUFF_FAILED;
        if ( (scheme = GetCapScheme( locase, sizeof(locase), pszstr, cchstr )) == (unsigned)-1 )
          return WORDBUFF_FAILED;
      }

    // fill other fields
      lemact.elemma = (lemact.plemma = plemma) + clemma;
      lemact.eforms = (lemact.pforms = pforms) + cforms;
      lemact.egrams = (lemact.pgrams = pgrams) + cgrams;
      lemact.scheme = scheme & 0x0000ffff;

    // call dictionary scanner
      return LinearScanDict<byte_t, int>( listLookup<doLemmatize, steminfo>( lemact ), stemtree, { locase, cchstr } ) < 0 ?
        lemact.nerror : (int)lemact.nlemma;
    ON_ERRORS( -1 )
  }

  int   CMlmaMb::BuildForm( char* output, size_t  cchout, lexeme_t  nlexid, formid_t  idform )
  {
    CATCH_ALL
      byte_t        lidkey[0x10];
      const byte_t* ofsptr;
      auto          getofs = []( const byte_t* thedic, const fragment& str ) {  return str.empty() ? thedic : nullptr;  };

    // No original word form; algo jumps to lexeme block dictionary point by lexeme id
      if ( (ofsptr = LinearScanDict<word16_t, const byte_t*>( getofs, lidstree, { lidkey, lexkeylen( lidkey, nlexid ) } )) != nullptr )
      {
        const byte_t* dicpos = stemtree + getserial( ofsptr );
        byte_t        szstem[0x80];
        doBuildForm   abuild( szstem, 0, codepage );
        listTracer<doBuildForm, steminfo> tracer( abuild, szstem, dicpos );

        abuild.outend = (abuild.output = output) + cchout;
        abuild.grinfo = 0;
        abuild.bflags = 0;
        abuild.idform = idform;

        return RecursGetTrack<byte_t, int>( tracer, stemtree, szstem, 0, dicpos ) >= 0 ?
          abuild.rcount : abuild.nerror;
      }
      return 0;
    ON_ERRORS( -1 )
  }

  int   CMlmaMb::FindForms( char* output, size_t  cchout, const char* pszstr, size_t  cchstr, formid_t idform )
  {
    CATCH_ALL
      byte_t      locase[256];
      char        cpsstr[256];
      unsigned    scheme;
      doBuildForm abuild( locase, 0, codepage );

    // check source string and length
      if ( pszstr == nullptr || cchstr == 0 )
        return 0;

      if ( cchstr == (size_t)-1 )
        for ( cchstr = 0; pszstr[cchstr] != 0; ++cchstr ) (void)NULL;

    // check for utf8 or multibyte
      if ( codepage != codepages::codepage_1251 )
      {
        if ( (cchstr = codepages::mbcstombcs( codepages::codepage_1251, cpsstr, sizeof(cpsstr), codepage, pszstr, cchstr )) == (size_t)-1 )
          return WORDBUFF_FAILED;
        if ( (scheme = GetCapScheme( locase, sizeof(locase), cpsstr, cchstr )) == (unsigned)-1 )
          return WORDBUFF_FAILED;
      }
        else
      {
        if ( cchstr > sizeof(locase) )
          return WORDBUFF_FAILED;
        if ( (scheme = GetCapScheme( locase, sizeof(locase), pszstr, cchstr )) == (unsigned)-1 )
          return WORDBUFF_FAILED;
      }

      abuild.outend = (abuild.output = output) + cchout;
      abuild.grinfo = 0;
      abuild.bflags = 0;
      abuild.idform = idform;
      abuild.scheme = scheme & 0x0000ffff;

      return LinearScanDict<byte_t, int>( listLookup<doBuildForm, steminfo>( abuild ), stemtree, { locase, cchstr } ) < 0 ?
        abuild.nerror : (int)abuild.rcount;
    ON_ERRORS( -1 )
  }

  int   CMlmaMb::CheckHelp( char* output, size_t  cchout, const char* pszstr, size_t  cchstr )
  {
    CATCH_ALL
      byte_t  locase[256];
      char    cpsstr[256];
      int     nchars;

    // check source string and length
      if ( pszstr == nullptr )
        return 0;

      if ( cchstr == (size_t)-1 )
        for ( cchstr = 0; pszstr[cchstr] != 0; ++cchstr ) (void)NULL;

    // check for overflow
      if ( cchstr >= sizeof(locase) )
        return WORDBUFF_FAILED;

    // modify the codepage
      if ( codepage != codepages::codepage_1251 )
      {
        if ( (cchstr = codepages::mbcstombcs( codepages::codepage_1251, cpsstr, sizeof(cpsstr), codepage, pszstr, cchstr )) == (size_t)-1 )
          return WORDBUFF_FAILED;
        pszstr = cpsstr;
      }

    // change the word to the lower case
      memcpy( locase, pszstr, cchstr );
        locase[cchstr] = '\0';
      SetLowerCase( locase );

    // scan the dictionary
      if ( (nchars = (int)WildScan( (byte_t*)cpsstr, cchout, locase, cchstr )) <= 0 )
        return nchars;

      return (int)codepages::mbcstombcs( codepage, output, cchout, codepages::codepage_1251, cpsstr, nchars );
    ON_ERRORS( -1 )
  }

  int   CMlmaMb::GetWdInfo( unsigned char* pwinfo, lexeme_t lexkey )
  {
    CATCH_ALL
      byte_t        lidkey[0x10];
      const byte_t* ofsptr;
      auto          getofs = []( const byte_t* thedic, const fragment& str ){  return str.empty() ? thedic : nullptr;  };

    // No original word form; algo jumps to lexeme block dictionary point by lexeme id
      if ( (ofsptr = LinearScanDict<word16_t, const byte_t*>( getofs, lidstree, { lidkey, lexkeylen( lidkey, lexkey ) } )) != nullptr )
      {
        const byte_t* dicpos = stemtree + getserial( ofsptr ) + 2; /* 2 => clower && cupper */
        lexeme_t      nlexid = getserial( dicpos );
        word16_t      oclass = getword16( dicpos );
        steminfo      stinfo;

        if ( nlexid != lexkey )
          return -2;

        *pwinfo = stinfo.Load( classmap + (oclass & 0x7fff) ).wdinfo & 0x3f;
          return 1;
      }
      return 0;
    ON_ERRORS( -1 )
  }

  int   CMlmaMb::EnumWords( IMlmaEnum*  pienum, const char* pszstr, size_t cchstr )
  {
    CATCH_ALL
      byte_t  locase[0x100];
      char    cpsstr[0x100];
      auto    reglex = [pienum]( lexeme_t x, int n, const SStrMatch* f )
        {
          byte_t  aforms[0x100];

          for ( int i = 0; i != n; ++i )
            aforms[i] = f[i].id;

          return pienum->RegisterLexeme( x, n, aforms );
        };

    // check string length && check for overflow
      if ( cchstr == (size_t)-1 )
        for ( cchstr = 0; pszstr[cchstr] != 0; ++cchstr ) (void)NULL;
      if ( cchstr >= sizeof(locase) )
        return WORDBUFF_FAILED;

    // modify the codepage
      if ( codepage != codepages::codepage_1251 )
      {
        if ( (cchstr = codepages::mbcstombcs( codepages::codepage_1251, cpsstr, sizeof(cpsstr), codepage, pszstr, cchstr )) == (size_t)-1 )
          return WORDBUFF_FAILED;
        pszstr = cpsstr;
      }

    // change the word to the lower case
      SetLowerCase( (byte_t*)strncpy( (char*)locase, pszstr, cchstr ), cchstr )[cchstr] = 0;

    // scan the dictionary
      return GetWordMatch( locase, cchstr, reglex );
    ON_ERRORS( -1 )
  }

  int   CMlmaMb::FindMatch( IMlmaMatch*  pienum, const char* pszstr, size_t cchstr )
  {
    CATCH_ALL
      byte_t  locase[0x100];
      char    cpsstr[0x100];
      auto    reglex = [pienum]( lexeme_t x, int n, const SStrMatch* f )
        {  return pienum->RegisterLexeme( x, n, f );  };

    // check string length && check for overflow
      if ( cchstr == (size_t)-1 )
        for ( cchstr = 0; pszstr[cchstr] != 0; ++cchstr ) (void)NULL;
      if ( cchstr >= sizeof(locase) )
        return WORDBUFF_FAILED;

    // modify the codepage
      if ( codepage != codepages::codepage_1251 )
      {
        if ( (cchstr = codepages::mbcstombcs( codepages::codepage_1251, cpsstr, sizeof(cpsstr), codepage, pszstr, cchstr )) == (size_t)-1 )
          return WORDBUFF_FAILED;
        pszstr = cpsstr;
      }

    // change the word to the lower case
      SetLowerCase( (byte_t*)strncpy( (char*)locase, pszstr, cchstr ), cchstr )[cchstr] = 0;

    // scan the dictionary
      return GetWordMatch( locase, cchstr, reglex );
    ON_ERRORS( -1 )
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

  int   CMlmaWc::SetLoCase( widechar* pwsstr, size_t  cchstr )
  {
    if ( cchstr == (size_t)-1 )
      for ( cchstr = 0; pwsstr[cchstr] != 0; ++cchstr ) (void)NULL;

    CATCH_ALL
      return codepages::strtolower( (widechar*)pwsstr, cchstr, (const widechar*)pwsstr, cchstr ), 0;
    ON_ERRORS( -1 )
  }
                               
  int   CMlmaWc::SetUpCase( widechar* pwsstr, size_t  cchstr )
  {
    if ( cchstr == (size_t)-1 )
      for ( cchstr = 0; pwsstr[cchstr] != 0; ++cchstr ) (void)NULL;

    CATCH_ALL
      return codepages::strtoupper( (widechar*)pwsstr, cchstr, (const widechar*)pwsstr, cchstr ), 0;
    ON_ERRORS( -1 )
  }
                               
  int   CMlmaWc::CheckWord( const widechar* pwsstr, size_t  cchstr, unsigned  dwsets )
  {
    char    szword[0x100];
    size_t  ccword;

  // get string length and convert to ansi
    if ( (ccword = codepages::widetombcs( codepages::codepage_1251, szword, 0x100, (const widechar*)pwsstr, cchstr )) == (size_t)-1 )
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

  int   CMlmaWc::FindForms(       widechar* output, size_t  cchout,
                            const widechar* pwsstr, size_t  cchstr, formid_t  idform )
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
      cchout : sizeof(szform) - 1, szword, ccword, idform )) <= 0 )
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

  /*
    CMlmaWc::EnumWords()

    Convert unicode string to 1251 && pass to ultibyte native interface
  */
  int   CMlmaWc::EnumWords( IMlmaEnum*  pienum, const widechar* pwsstr, size_t cchstr )
  {
    char    szword[0x100];
    size_t  ccword;

  // get string length and convert to native codepage
    if ( (ccword = codepages::widetombcs( codepages::codepage_1251, szword, sizeof(szword) - 1, pwsstr, cchstr )) == (size_t)-1 )
      return WORDBUFF_FAILED;

    return mlmaMbInstance.EnumWords( pienum, szword, ccword );
  }

  int   CMlmaWc::FindMatch( IMlmaMatch* pmatch, const widechar* pwsstr, size_t cchstr )
  {
    char    szword[0x100];
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

}

using namespace LIBMORPH_NAMESPACE;

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
    if ( igncasecmp( page.szcodepage, codepage ) == 0 )
      {  pageid = page.idcodepage;  break;  }

  if ( pageid == (unsigned)-1 || ptrAPI == nullptr )
    return EINVAL;

  if ( pageid == codepages::codepage_1251 )
    return mlmaruLoadMbAPI( ptrAPI );

  if ( (palloc = new CMlmaMb( pageid )) == nullptr )
    return ENOMEM;
  (*ptrAPI = (IMlmaMb*)palloc)->Attach();
    return 0;
}

int   MLMAPROC        mlmaruLoadWcAPI( IMlmaWc**  ptrAPI )
{
  if ( ptrAPI == nullptr )
    return -1;
  *ptrAPI = (IMlmaWc*)&mlmaWcInstance;
    return 0;
}

