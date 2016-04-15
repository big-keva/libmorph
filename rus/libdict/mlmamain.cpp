# include "../include/mlma1049.h"
# include <namespace.h>
# include "mlmadefs.h"
# include "lemmatiz.h"
# include "wildscan.h"
# include "scandict.h"
# include "capsheme.h"
# include <string.h>
# include <libcodes/codes.h>

#if defined(_MSC_VER) 
#   if !defined( strncasecmp )
#     define strncasecmp memicmp
#   endif  // strncasecmp
#   if !defined( strcasecmp )
#     define strcasecmp  strcmpi
#   endif  // strcasecmp
# endif

# if !defined( _WIN32_WCE )
  # define  CATCH_ALL         try {
  # define  ON_ERRORS( code ) } catch ( ... ) { return (code); }
# else
  # define  CATCH_ALL
  # define  ON_ERRORS( code )
# endif  // ! _WIN32_WCE

namespace LIBMORPH_NAMESPACE
{
  struct getDictPos
  {
    const byte08_t* operator () ( const byte08_t* thedic, const byte08_t*, size_t cchstr ) const
      {
        return cchstr == 0 ? thedic : NULL;
      }
  };

  class doEnumList
  {
    TEnumWords  lpfunc;
    void*       vparam;

  public:     // construction
    doEnumList( TEnumWords f, void* v ): lpfunc( f ), vparam( v )
      {
      }
    int operator () ( const byte08_t* thedic )
      {
        unsigned  ucount = getserial( thedic );

        while ( ucount-- > 0 )
        {
/*
          steminfo  stinfo( thedic );
          lexeme_t  nlexid = getserial( thedic );

          if ( !lpfunc( nlexid, vparam ) )
            return -1;
*/
        }
        return 0;
      }
  };

  //
  // the new api - IMlma interface class
  //
  struct  CMlmaMb: public IMlmaMb
  {
    virtual int MLMAPROC  Attach();
    virtual int MLMAPROC  Detach();

    virtual int MLMAPROC  SetLoCase( char*            pszstr, size_t    cchstr );
    virtual int MLMAPROC  SetUpCase( char*            pszstr, size_t    cchstr );
    virtual int MLMAPROC  CheckWord( const char*      pszstr, size_t    cchstr,
                                     unsigned         dwsets );
    virtual int MLMAPROC  Lemmatize( const char*      pszstr, size_t    cchstr,
                                     SLemmInfoA*      output, size_t    cchout,
                                     char*            plemma, size_t    clemma,
                                     SGramInfo*       pgrams, size_t    ngrams,
                                     unsigned         dwsets );
    virtual int MLMAPROC  BuildForm( char*            output, size_t    cchout,
                                     unsigned         nlexid, byte08_t  idform );
    virtual int MLMAPROC  FindForms( char*            output, size_t    cchout,
                                     const char*      pszstr, size_t    cchstr,
                                     unsigned char    idform );
    virtual int MLMAPROC  CheckHelp( char*            output, size_t    cchout,
                                     const char*      pszstr, size_t    cchstr );

  public:     // construction
    CMlmaMb( unsigned cp = codepages::codepage_1251 ): codepage( cp ), refcount( 0 )
      {
      }

  protected:  // codepage
    unsigned  codepage;
    long      refcount;

  };

  struct  CMlmaCp: public CMlmaMb
  {
    virtual int MLMAPROC  Attach();
    virtual int MLMAPROC  Detach();

  public:     // construction
    CMlmaCp( unsigned cp ): CMlmaMb( cp ), refcount( 0 )
      {
      }

  protected:  // codepage
    long      refcount;

  };

  struct  CMlmaWc
  {
    virtual int MLMAPROC  Attach();
    virtual int MLMAPROC  Detach();

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
                                     unsigned         nlexid, byte08_t  idform );
    virtual int MLMAPROC  FindForms( widechar*        output, size_t    cchout,
                                     const widechar*  pwsstr, size_t    cchstr,
                                     unsigned char    idform );
    virtual int MLMAPROC  CheckHelp( widechar*        output, size_t    cchout,
                                     const widechar*  pwsstr, size_t    cchstr );
  };

  CMlmaMb mlmaMbInstance;
  CMlmaWc mlmaWcInstance;

  using namespace codepages;

  // CMlmaMb implementation

  int   CMlmaMb::Attach()
  {
    return 0;
  }

  int   CMlmaMb::Detach()
  {
    return 0;
  }

  int   CMlmaMb::SetLoCase( char* pszstr, size_t  cchstr )
  {
    CATCH_ALL
      strtolower( codepage, pszstr, cchstr, pszstr, cchstr );
      return 0;
    ON_ERRORS( -1 )
  }

  int   CMlmaMb::SetUpCase( char* pszstr, size_t  cchstr )
  {
    CATCH_ALL
      strtoupper( codepage, pszstr, cchstr, pszstr, cchstr );
      return 0;
    ON_ERRORS( -1 )
  }

  int   CMlmaMb::CheckWord( const char* pszstr, size_t  cchstr, unsigned  dwsets )
  {
    CATCH_ALL
      byte08_t    locase[256];
      char        cpsstr[256];
      doCheckWord scheck( locase, dwsets );

    // check string length
      if ( cchstr == (size_t)-1 )
        cchstr = strlen( pszstr );

    // check for overflow
      if ( cchstr >= sizeof(locase) )
        return WORDBUFF_FAILED;

    // modify the codepage
      if ( codepage != codepage_1251 )
        if ( (cchstr = mbcstombcs( codepage_1251, cpsstr, sizeof(cpsstr), codepage, pszstr, cchstr )) != (size_t)-1 ) pszstr = cpsstr;
          else  return WORDBUFF_FAILED;

    // get capitalization scheme
      scheck.scheme = GetCapScheme( locase, sizeof(locase), pszstr, cchstr ) & 0x0000ffff;

      // fill scheck structure
      return ScanDict<byte08_t, int>( listLookup<doCheckWord>( scheck ), stemtree, locase, cchstr );
    ON_ERRORS( -1 )
  }

  int   CMlmaMb::Lemmatize( const char* pszstr, size_t  cchstr,
                            SLemmInfoA* output, size_t  cchout,
                            char*       plemma, size_t  clemma,
                            SGramInfo*  pgrams, size_t  ngrams, unsigned  dwsets )
  {
    CATCH_ALL
      byte08_t    locase[256];
      char        cpsstr[256];
      doLemmatize lemact( locase, dwsets, codepage );

    // check string length
      if ( cchstr == (size_t)-1 )
        cchstr = strlen( pszstr );

    // check for overflow
      if ( cchstr >= sizeof(locase) )
        return WORDBUFF_FAILED;

    // modify the codepage
      if ( codepage != codepage_1251 )
        if ( (cchstr = mbcstombcs( codepage_1251, cpsstr, sizeof(cpsstr), codepage, pszstr, cchstr )) != (size_t)-1 ) pszstr = cpsstr;
          else  return WORDBUFF_FAILED;

    // get capitalization scheme
      lemact.scheme = GetCapScheme( locase, sizeof(locase), pszstr, cchstr ) & 0x0000ffff;

    // fill other fields
      lemact.elemma = (lemact.plemma = output) + cchout;
      lemact.eforms = (lemact.pforms = plemma) + clemma;
      lemact.egrams = (lemact.pgrams = pgrams) + ngrams;

    // call dictionary scanner
      return ScanDict<byte08_t, int>( listLookup<doLemmatize>( lemact ), stemtree, locase, cchstr ) < 0 ?
        lemact.nerror : (int)(lemact.plemma - output);
    ON_ERRORS( -1 )
  }

  int   CMlmaMb::BuildForm( char* output, size_t  cchout, lexeme_t  nlexid, formid_t  idform )
  {
    CATCH_ALL
      byte08_t        lidkey[0x10];
      const byte08_t* ofsptr;

    // Оригинальная форма слова не задана, следует применять модификацию алгоритма, "прыгающую"
    // по словарю идентификаторов лексем сразу в нужную точку на странице.
      if ( (ofsptr = ScanDict<word16_t, const byte08_t*>( getDictPos(), lidstree, lidkey, lexkeylen( lidkey, nlexid ) )) != NULL )
      {
        const byte08_t* dicpos = stemtree + getserial( ofsptr );
        byte08_t        szstem[0x80];
        doBuildForm     abuild( szstem, 0, codepage );

        abuild.outend = (abuild.output = output) + cchout;
        abuild.grinfo = 0;
        abuild.bflags = 0;
        abuild.idform = idform;

        return FindStem( abuild, stemtree, szstem, dicpos ) >= 0 ? abuild.rcount : abuild.nerror;
      }
      return 0;
    ON_ERRORS( -1 )
  }

  int   CMlmaMb::FindForms( char* output, size_t  cchout, const char* pszstr, size_t  cchstr, formid_t idform )
  {
    CATCH_ALL
      byte08_t    locase[256];
      char        cpsstr[256];
      doBuildForm abuild( locase, 0, codepage );

    // check string length
      if ( cchstr == (size_t)-1 )
        cchstr = strlen( pszstr );

    // check for overflow
      if ( cchstr >= sizeof(locase) )
        return WORDBUFF_FAILED;

    // modify the codepage
      if ( codepage != codepage_1251 )
        if ( (cchstr = mbcstombcs( codepage_1251, cpsstr, sizeof(cpsstr), codepage, pszstr, cchstr )) != (size_t)-1 ) pszstr = cpsstr;
          else  return WORDBUFF_FAILED;

      abuild.scheme = GetCapScheme( locase, sizeof(locase), pszstr, cchstr ) & 0x0000ffff;
      abuild.outend = (abuild.output = output) + cchout;
      abuild.grinfo = 0;
      abuild.bflags = 0;
      abuild.idform = idform;

      return ScanDict<byte08_t, int>( listLookup<doBuildForm>( abuild ), stemtree, locase, cchstr ) < 0 ? abuild.nerror : abuild.rcount;
    ON_ERRORS( -1 )
  }

  int   CMlmaMb::CheckHelp( char* output, size_t  cchout, const char* pszstr, size_t  cchstr )
  {
    CATCH_ALL
      byte08_t  locase[256];
      char      cpsstr[256];
      int       nchars;

    // check string length
      if ( cchstr == (size_t)-1 )
        cchstr = strlen( pszstr );

    // check for overflow
      if ( cchstr >= sizeof(locase) )
        return WORDBUFF_FAILED;

    // modify the codepage
      if ( codepage != codepage_1251 )
        if ( (cchstr = mbcstombcs( codepage_1251, cpsstr, sizeof(cpsstr), codepage, pszstr, cchstr )) != (size_t)-1 ) pszstr = cpsstr;
          else  return WORDBUFF_FAILED;

    // change the word to the lower case
      memcpy( locase, pszstr, cchstr );
        locase[cchstr] = '\0';
      SetLowerCase( locase );

    // scan the dictionary
      if ( (nchars = (int)WildScan( (byte08_t*)cpsstr, cchout, locase, cchstr )) <= 0 )
        return nchars;

      return mbcstombcs( codepage, output, cchout, codepage_1251, cpsstr, nchars );
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
      delete this;
    return rcount;
  }

  // CMlmaWc wrapper implementation

  int   CMlmaWc::Attach()
  {
    return 0;
  }

  int   CMlmaWc::Detach()
  {
    return 0;
  }

  int   CMlmaWc::SetLoCase( widechar* pwsstr, size_t  cchstr )
  {
    CATCH_ALL
      strtolower( (widechar*)pwsstr, cchstr, (const widechar*)pwsstr, cchstr );
      return 0;
    ON_ERRORS( -1 )
  }
                               
  int   CMlmaWc::SetUpCase( widechar* pwsstr, size_t  cchstr )
  {
    CATCH_ALL
      strtoupper( (widechar*)pwsstr, cchstr, (const widechar*)pwsstr, cchstr );
      return 0;
    ON_ERRORS( -1 )
  }
                               
  int   CMlmaWc::CheckWord( const widechar* pwsstr, size_t  cchstr, unsigned  dwsets )
  {
    char    szword[0x100];
    size_t  ccword;

  // get string length and convert to ansi
    if ( (ccword = widetombcs( codepage_1251, szword, 0x100, (const widechar*)pwsstr, cchstr )) == (size_t)-1 )
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
    if ( (ccword = widetombcs( codepage_1251, szword, 0xf0, (const widechar*)pwsstr, cchstr )) == (size_t)-1 )
      return WORDBUFF_FAILED;

  // call default lemmatizer
    if ( (lcount = mlmaMbInstance.Lemmatize( szword, ccword,
      output != NULL ? lmbuff : NULL, cchout <= 32 ? cchout : 32,
      plemma != NULL ? szlemm : NULL, clemma <= sizeof(szlemm) ? clemma : sizeof(szlemm),
      pgrams, ngrams, dwsets )) <= 0 )
        return lcount;

  // fill output data
    if ( output != NULL )
      for ( lindex = 0; lindex < lcount; lindex++, output++ )
      {
        output->nlexid = lmbuff[lindex].nlexid;
        output->pgrams = lmbuff[lindex].pgrams;
        output->ngrams = lmbuff[lindex].ngrams;
        output->plemma = lmbuff[lindex].plemma == NULL ? NULL :
            plemma + (lmbuff[lindex].plemma - szlemm);
      }
    if ( plemma != NULL )
      for ( lindex = 0, lplemm = szlemm; lindex < lcount; ++lindex )
      {
        size_t    nccstr = mbcstowide( codepage_1251, (widechar*)plemma, clemma, lplemm ) + 1;
          
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
      size_t    nccstr = mbcstowide( codepage_1251, output, cchout, lpform ) + 1;

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
    if ( (ccword = widetombcs( codepage_1251, szword, sizeof(szword) - 1, pwsstr, cchstr )) == (size_t)-1 )
      return WORDBUFF_FAILED;

  // build the form
    if ( (fcount = mlmaMbInstance.FindForms( szform, cchout <= sizeof(szform) - 1 ?
      cchout : sizeof(szform) - 1, szword, ccword, idform )) <= 0 )
        return fcount;

  // convert
    for ( findex = 0, lpform = szform; findex < fcount; ++findex )
    {
      size_t    nccstr = mbcstowide( codepage_1251, output, cchout, lpform ) + 1;

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
// get string length and convert to ansi
    if ( (ccword = widetombcs( codepage_1251, szword, sizeof(szword) - 1, pwsstr, cchstr )) == (size_t)-1 )
      return WORDBUFF_FAILED;

  // build the form
    if ( (ccount = mlmaMbInstance.CheckHelp( chhelp, cchout <= sizeof(chhelp) ?
      cchout : sizeof(chhelp), szword, ccword )) <= 0 )
        return ccount;

  // convert
    mbcstowide( codepage_1251, output, cchout, chhelp, ccount );
    return ccount;
  }

  struct
  {
    unsigned    idcodepage;
    const char* szcodepage;
  } codepageList[] =
  {
    { codepage_1251, "Windows-1251" },
    { codepage_1251, "Windows" },
    { codepage_1251, "1251" },
    { codepage_1251, "Win-1251" },
    { codepage_1251, "Win" },
    { codepage_1251, "Windows 1251" },
    { codepage_1251, "Win 1251" },
    { codepage_1251, "ansi" },
    { codepage_koi8, "koi-8" },
    { codepage_koi8, "koi8" },
    { codepage_koi8, "20866" },
    { codepage_866,  "dos" },
    { codepage_866,  "oem" },
    { codepage_866,  "866" },
    { codepage_iso,  "28595" },
    { codepage_iso,  "iso-88595" },
    { codepage_iso,  "iso-8859-5" },
    { codepage_mac,  "10007" },
    { codepage_iso,  "mac" },
    { codepage_utf8, "65001" },
    { codepage_utf8, "utf-8" },
    { codepage_utf8, "utf8" }
  };
}

using namespace LIBMORPH_NAMESPACE;

int   MLMAPROC        mlmaruLoadMbAPI( IMlmaMb**  ptrAPI )
{
  if ( ptrAPI == NULL )
    return -1;
  *ptrAPI = (IMlmaMb*)&mlmaMbInstance;
    return 0;
}

int   MLMAPROC        mlmaruLoadCpAPI( IMlmaMb**  ptrAPI, const char* codepage )
{
  CMlmaMb*  palloc;
  unsigned  pageid = (unsigned)-1;
  int       nindex;

  for ( nindex = 0; nindex < sizeof(codepageList) / sizeof(codepageList[0]) && pageid == (unsigned)-1; ++nindex )
    if ( strcasecmp( codepageList[nindex].szcodepage, codepage ) == 0 )
      pageid = codepageList[nindex].idcodepage;

  if ( pageid == (unsigned)-1 || ptrAPI == NULL )
    return EINVAL;

  if ( pageid == codepage_1251 )
    return mlmaruLoadMbAPI( ptrAPI );

  if ( (palloc = new CMlmaMb( pageid )) == NULL )
    return ENOMEM;
  (*ptrAPI = (IMlmaMb*)palloc)->Attach();
    return 0;
}

int   MLMAPROC        mlmaruLoadWcAPI( IMlmaWc**  ptrAPI )
{
  if ( ptrAPI == NULL )
    return -1;
  *ptrAPI = (IMlmaWc*)&mlmaWcInstance;
    return 0;
}
