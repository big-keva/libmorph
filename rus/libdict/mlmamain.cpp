# include <namespace.h>
# include "mlmadefs.h"
# include "lemmatiz.h"
# include "wildscan.h"
# include "scandict.h"
# include "capsheme.h"
# include "../include/mlma1049.h"
# include <string.h>
# include "../../unicode/libmorph_unicode.h"

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
    const byte08_t* operator () ( const byte08_t* thedic, const byte08_t*, unsigned cchstr ) const
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
          steminfo  stinfo( thedic );
          lexeme_t  nlexid = getserial( thedic );

          if ( !lpfunc( nlexid, vparam ) )
            return -1;
        }
        return 0;
      }
  };
  //
  // the new api - IMlma interface class
  //
  struct  CMlmaMb: public IMlmaMb
  {
    virtual int MLMAPROC  SetLoCase( char*                  pszstr, unsigned  cchstr );
    virtual int MLMAPROC  SetUpCase( char*                  pszstr, unsigned  cchstr );
    virtual int MLMAPROC  CheckWord( const char*            pszstr, unsigned  cchstr,
                                     unsigned               dwsets );
    virtual int MLMAPROC  Lemmatize( const char*            pszstr, unsigned  cchstr,
                                     SLemmInfoA*            output, unsigned  cchout,
                                     char*                  plemma, unsigned  clemma,
                                     SGramInfo*             pgrams, unsigned  ngrams,
                                     unsigned               dwsets );
    virtual int MLMAPROC  BuildForm( char*          output, unsigned      cchout,
                                     unsigned       nlexid, unsigned char idform );
    virtual int MLMAPROC  FindForms( char*                  output, unsigned      cchout,
                                     const char*            pszstr, unsigned      cchstr,
                                     unsigned char          idform );
    virtual int MLMAPROC  CheckHelp( char*                  output, unsigned      cchout,
                                     const char*            pszstr, unsigned      cchstr );
  };

  struct  CMlmaWc
  {
    virtual int MLMAPROC  SetLoCase( unsigned short*        pwsstr, unsigned  cchstr );
    virtual int MLMAPROC  SetUpCase( unsigned short*        pwsstr, unsigned  cchstr );
    virtual int MLMAPROC  CheckWord( const unsigned short*  pszstr, unsigned  cchstr,
                                     unsigned               dwsets );
    virtual int MLMAPROC  Lemmatize( const unsigned short*  pszstr, unsigned  cchstr,
                                     SLemmInfoW*            output, unsigned  cchout,
                                     unsigned short*        plemma, unsigned  clemma,
                                     SGramInfo*             pgrams, unsigned  ngrams,
                                     unsigned       dwsets );
    virtual int MLMAPROC  BuildForm( unsigned short*        output, unsigned      cchout,
                                     unsigned               nlexid, unsigned char idform );
    virtual int MLMAPROC  FindForms( unsigned short*        output, unsigned      cchout,
                                     const unsigned short*  pwsstr, unsigned      cchstr,
                                     unsigned char          idform );
    virtual int MLMAPROC  CheckHelp( unsigned short*        output, unsigned      cchout,
                                     const unsigned short*  pwsstr, unsigned      cchstr );
  };

  CMlmaMb mlmaMbInstance;
  CMlmaWc mlmaWcInstance;

  // CMlmaMb implementation

  int   CMlmaMb::SetLoCase( char*       pszstr,
                            unsigned    cchstr )
  {
    CATCH_ALL

    if ( cchstr == (unsigned)-1 )
      cchstr = strlen( pszstr );

    while ( cchstr-- > 0 )
    {
      *pszstr = (char)toLoCaseMatrix[(unsigned char)*pszstr];
        ++pszstr;
    }
    return 0;

    ON_ERRORS( -1 )
  }

  int   CMlmaMb::SetUpCase( char*       pszstr,
                            unsigned    cchstr )
  {
    CATCH_ALL

    if ( cchstr == (unsigned)-1 )
      cchstr = strlen( pszstr );

    while ( cchstr-- > 0 )
    {
      *pszstr = (char)toUpCaseMatrix[(unsigned char)*pszstr];
        ++pszstr;
    }
    return 0;

    ON_ERRORS( -1 )
  }

  int   CMlmaMb::CheckWord( const char* pszstr,
                            unsigned    cchstr,
                            unsigned    dwsets )
  {
    CATCH_ALL
      byte08_t    locase[256];
      doCheckWord scheck( locase, dwsets );

    // check string length
      if ( cchstr == (unsigned)-1 )
        cchstr = strlen( pszstr );

    // check for overflow
      if ( cchstr >= sizeof(locase) )
        return WORDBUFF_FAILED;

    // get capitalization scheme
      scheck.scheme = GetCapScheme( locase, sizeof(locase), pszstr, cchstr ) & 0x0000ffff;

      // fill scheck structure
      return ScanDict<byte08_t, int>( listLookup<doCheckWord>( scheck ), stemtree, locase, cchstr );
    ON_ERRORS( -1 )
  }

  int   CMlmaMb::Lemmatize( const char*   pszstr,
                            unsigned      cchstr,
                            SLemmInfoA*   output,
                            unsigned      cchout,
                            char*         plemma,
                            unsigned      clemma,
                            SGramInfo*    pgrams,
                            unsigned      ngrams,
                            unsigned      dwsets )
  {
    CATCH_ALL
      byte08_t    locase[256];
      doLemmatize lemact( locase, dwsets );

    // check string length
      if ( cchstr == (unsigned)-1 )
        cchstr = strlen( pszstr );

    // check for overflow
      if ( cchstr >= sizeof(locase) )
        return WORDBUFF_FAILED;

    // get capitalization scheme
      lemact.scheme = GetCapScheme( locase, sizeof(locase), pszstr, cchstr ) & 0x0000ffff;

    // fill other fields
      lemact.plemma = output;
        lemact.clemma = cchout;
      lemact.pforms = plemma;
        lemact.cforms = clemma;
      lemact.pgrams = pgrams;
        lemact.cgrams = ngrams;

    // call dictionary scanner
      return ScanDict<byte08_t, int>( listLookup<doLemmatize>( lemact ), stemtree, locase, cchstr ) < 0 ? lemact.nerror : lemact.rcount;
    ON_ERRORS( -1 )
  }

  int   CMlmaMb::BuildForm( char*         output,
                            unsigned      cchout,
                            lexeme_t      nlexid,
                            unsigned char idform )
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
        doBuildForm     abuild( szstem, 0 );

        abuild.output = output;
        abuild.cchout = cchout;
        abuild.grinfo = 0;
        abuild.bflags = 0;
        abuild.idform = idform;

        return FindStem( abuild, stemtree, szstem, dicpos ) == 0 || abuild.nerror != 0 ?
          abuild.nerror : abuild.rcount;
      }
      return 0;
    ON_ERRORS( -1 )
  }

  int   CMlmaMb::FindForms( char*           output,
                            unsigned        cchout,
                            const char*     pszstr,
                            unsigned        cchstr,
                            unsigned char   idform )
  {
    CATCH_ALL
      byte08_t    locase[256];
      doBuildForm abuild( locase, 0 );

    // check string length
      if ( cchstr == (unsigned)-1 )
        cchstr = strlen( pszstr );

    // check for overflow
      if ( cchstr >= sizeof(locase) )
        return WORDBUFF_FAILED;

      abuild.scheme = GetCapScheme( locase, sizeof(locase), pszstr, cchstr ) & 0x0000ffff;
      abuild.output = output;
      abuild.cchout = cchout;
      abuild.grinfo = 0;
      abuild.bflags = 0;
      abuild.idform = idform;

      if ( !ScanDict<byte08_t, int>( listLookup<doBuildForm>( abuild ), stemtree, locase, cchstr ) )
        return 0;

      return abuild.nerror != 0 ? abuild.nerror : abuild.rcount;
    ON_ERRORS( -1 )
  }

  int   CMlmaMb::CheckHelp( char*                  output,
                            unsigned               cchout,
                            const char*            pszstr,
                            unsigned               cchstr )
  {
    CATCH_ALL
      byte08_t  szbuff[256];
      byte08_t  locase[256];
      int       nchars;

    // check string length
      if ( cchstr == (unsigned)-1 )
        cchstr = strlen( pszstr );

    // check for overflow
      if ( cchstr >= sizeof(locase) )
        return WORDBUFF_FAILED;

    // change the word to the lower case
      memcpy( locase, pszstr, cchstr );
        locase[cchstr] = '\0';
      SetLowerCase( locase );

    // scan the dictionary
      if ( (nchars = WildScan( szbuff, locase, cchstr )) <= 0 )
        return nchars;

    // check if overflow
      if ( (unsigned)nchars > cchout )
        return WORDBUFF_FAILED;

      memcpy( output, szbuff, nchars );
        return nchars;
    ON_ERRORS( -1 )
  }

  // CMlmaWc wrapper implementation

  int   CMlmaWc::SetLoCase( unsigned short*        pwsstr,
                            unsigned               cchstr )
  {
    CATCH_ALL
      char      szword[0x400];
      unsigned  ccword;

    // get string length and convert to ansi
      if ( (ccword = wctoansi( szword, sizeof(szword) - 1, pwsstr, cchstr )) == (unsigned)-1 )
        return WORDBUFF_FAILED;
      mlmaMbInstance.SetLoCase( szword, ccword );
        ansitowc( pwsstr, cchstr, szword, ccword );
      return 0;
    ON_ERRORS( -1 )
  }
                               
  int   CMlmaWc::SetUpCase( unsigned short*        pwsstr,
                            unsigned               cchstr )
  {
    CATCH_ALL
      char      szword[0x400];
      unsigned  ccword;

    // get string length and convert to ansi
      if ( (ccword = wctoansi( szword, sizeof(szword) - 1, pwsstr, cchstr )) == (unsigned)-1 )
        return WORDBUFF_FAILED;
      mlmaMbInstance.SetUpCase( szword, ccword );
        ansitowc( pwsstr, cchstr, szword, ccword );
      return 0;
    ON_ERRORS( -1 )
  }
                               
  int   CMlmaWc::CheckWord( const unsigned short*  pwsstr,
                            unsigned               cchstr,
                            unsigned               dwsets )
  {
    char      szword[98];
    unsigned  ccword;

  // get string length and convert to ansi
    if ( (ccword = wctoansi( szword, sizeof(szword) - 1, pwsstr, cchstr )) == (unsigned)-1 )
      return WORDBUFF_FAILED;
    return mlmaMbInstance.CheckWord( szword, ccword, dwsets );
  }

  int   CMlmaWc::Lemmatize( const unsigned short*  pwsstr,
                            unsigned               cchstr,
                            SLemmInfoW*            output,
                            unsigned               cchout,
                            unsigned short*        plemma,
                            unsigned               clemma,
                            SGramInfo*             pgrams,
                            unsigned               ngrams,
                            unsigned               dwsets )
  {
    SLemmInfoA  lmbuff[32];
    char        szlemm[98];
    char        szword[98];
    char*       lplemm;
    unsigned    ccword;
    int         lcount;
    int         lindex;

  // get string length and convert to ansi
    if ( (ccword = wctoansi( szword, sizeof(szword) - 1, pwsstr, cchstr )) == (unsigned)-1 )
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
        unsigned  nccstr = ansitowc( plemma, clemma, lplemm ) + 1;
          
        plemma += nccstr;
        clemma -= nccstr;
        lplemm += nccstr;
      }
    return lcount;
  }

  int   CMlmaWc::BuildForm( unsigned short*      output,
                            unsigned             cchout,
                            lexeme_t             nlexid,
                            unsigned char        idform )
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
      unsigned  nccstr = ansitowc( output, cchout, lpform ) + 1;

      output += nccstr;
      cchout -= nccstr;
      lpform += nccstr;
    }

    return fcount;
  }

  int   CMlmaWc::FindForms( unsigned short*        output,
                            unsigned               cchout,
                            const unsigned short*  pwsstr,
                            unsigned               cchstr,
                            unsigned char          idform )
  {
    char        szword[98];
    char        szform[98];
    char*       lpform;
    unsigned    ccword;
    int         fcount;
    int         findex;

  // get string length and convert to ansi
    if ( (ccword = wctoansi( szword, sizeof(szword) - 1, pwsstr, cchstr )) == (unsigned)-1 )
      return WORDBUFF_FAILED;

  // build the form
    if ( (fcount = mlmaMbInstance.FindForms( szform, cchout <= sizeof(szform) - 1 ?
      cchout : sizeof(szform) - 1, szword, ccword, idform )) <= 0 )
        return fcount;

  // convert
    for ( findex = 0, lpform = szform; findex < fcount; ++findex )
    {
      unsigned  nccstr = ansitowc( output, cchout, lpform ) + 1;

      output += nccstr;
      cchout -= nccstr;
      lpform += nccstr;
    }

    return fcount;
  }

  int   CMlmaWc::CheckHelp( unsigned short*        output,
                            unsigned               cchout,
                            const unsigned short*  pwsstr,
                            unsigned               cchstr )
  {
    char        szword[98];
    char        chhelp[98];
    unsigned    ccword;
    int         ccount;

  // get string length and convert to ansi
    if ( (ccword = wctoansi( szword, sizeof(szword) - 1, pwsstr, cchstr )) == (unsigned)-1 )
      return WORDBUFF_FAILED;

  // build the form
    if ( (ccount = mlmaMbInstance.CheckHelp( chhelp, cchout <= sizeof(chhelp) ?
      cchout : sizeof(chhelp), szword, ccword )) <= 0 )
        return ccount;

  // convert
    ansitowc( output, cchout, chhelp, ccount );
    return ccount;
  }
}

using namespace LIBMORPH_NAMESPACE;

short MLMA_API EXPORT mlmaruCheckWord( const char* lpWord, unsigned short options )
{
  return mlmaMbInstance.CheckWord( lpWord, (unsigned)-1, options );
}

short MLMA_API EXPORT mlmaruLemmatize( const char*    lpWord,
                                       word16_t       dwsets,
                                       char*          lpLemm,
                                       lexeme_t*      lpLIDs,
                                       char*          lpGram,
                                       word16_t       ccLemm,
                                       word16_t       cdwLID,
                                       word16_t       cbGram )
{
  SLemmInfoA  lmbuff[32];   // the lemmatization buffer
  SGramInfo   grbuff[64];
  int         lcount;       // the lemmatization result
  int         lindex;

// call the lemmatizer
  if ( (lcount = mlmaMbInstance.Lemmatize( lpWord, (unsigned)-1,
                                           lmbuff, sizeof(lmbuff) / sizeof(lmbuff[0]),
                                           lpLemm, ccLemm,
                                           grbuff, sizeof(grbuff) / sizeof(grbuff[0]),
                                           dwsets )) <= 0 )
    return lcount;

// check if the fields needed
  if ( lpGram == NULL && lpLIDs == NULL )
    return lcount;

// fill selected fields
  for ( lindex = 0; lindex < lcount; lindex++ )
  {
    if ( lpGram != NULL )
    {
      unsigned  gcount = lmbuff[lindex].ngrams;

      if ( cbGram < 1 + gcount * sizeof(SGramInfo) )
        return GRAMBUFF_FAILED;
      else *lpGram++ = gcount;
        memcpy( lpGram, lmbuff[lindex].pgrams, gcount * sizeof(SGramInfo) );
      lpGram += gcount * sizeof(SGramInfo);
      cbGram -= gcount * sizeof(SGramInfo);
    }
    if ( lpLIDs != NULL )
    {
      if ( cdwLID == 0 )
        return LIDSBUFF_FAILED;
      *lpLIDs++ = lmbuff[lindex].nlexid;
        --cdwLID;
    }
  }
  return lcount;
}

short   MLMA_API  EXPORT  mlmaruBuildForm( const char*    pszstr,
                                           lexeme_t       nLexId,
                                           word16_t       dwsets,
                                           unsigned char  formId,
                                           char*          output,
                                           word16_t       cchout )
{
  return pszstr != NULL ? mlmaMbInstance.FindForms( output, cchout,
    pszstr, (unsigned)-1, formId ) : mlmaMbInstance.BuildForm( output, cchout, nLexId, formId );
}

short   MLMA_API  EXPORT  mlmaruBuildFormGI( const char*    pszstr,
                                             lexeme_t       nlexid,
                                             word16_t       dwsets,
                                             word16_t       grinfo,
                                             unsigned char  bflags,
                                             char*          output,
                                             word16_t       cchout )
{
  CATCH_ALL
    byte08_t    locase[256];
    unsigned    cchstr = 0;
    doBuildForm abuild( locase, 0 );

    abuild.output = output;
    abuild.cchout = cchout;
    abuild.grinfo = grinfo;
    abuild.bflags = bflags;
    abuild.idform = (unsigned)-1;

  // check for overflow
    if ( pszstr != NULL )
    {
      if ( (cchstr = strlen( pszstr )) >= sizeof(locase) )
        return WORDBUFF_FAILED;

      abuild.scheme = GetCapScheme( locase, sizeof(locase), pszstr, cchstr ) & 0x0000ffff;

      if ( !ScanDict<byte08_t, int>( listLookup<doBuildForm>( abuild ), stemtree, locase, cchstr ) )
        return 0;
    }
      else
    {
      byte08_t        lidkey[0x10];
      const byte08_t* ofsptr;

    // Оригинальная форма слова не задана, следует применять модификацию алгоритма, "прыгающую"
    // по словарю идентификаторов лексем сразу в нужную точку на странице.
      if ( (ofsptr = ScanDict<word16_t, const byte08_t*>( getDictPos(), lidstree, lidkey, lexkeylen( lidkey, nlexid ) )) != NULL )
      {
        const byte08_t* dicpos = stemtree + getserial( ofsptr );

        if ( FindStem( abuild, stemtree, locase, dicpos ) == 0 )
          return 0;
      }
    }

    return abuild.nerror != 0 ? abuild.nerror : abuild.rcount;
  ON_ERRORS( -1 )
}

short MLMA_API EXPORT mlmaruEnumWords( TEnumWords enumfn, void *vparam )
{
  CATCH_ALL
    doEnumList  doenum( enumfn, vparam );

    return EnumDict<byte08_t, int>( doenum, stemtree ) >= 0 ? 1 : 0;
  ON_ERRORS( -1 )
}

short MLMA_API EXPORT mlmaruCheckHelp( const char*  lpWord,
                                       char*        lpList )
{
  return mlmaMbInstance.CheckHelp( lpList, 32, lpWord, (unsigned)-1 );
}

short MLMA_API EXPORT mlmaruListForms( lexeme_t       nlexid,
                                       char*          output,
                                       unsigned       cchout )
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
      doListForms     aforms( szstem, 0 );

      aforms.output = output;
      aforms.cchout = cchout;

      return FindStem( aforms, stemtree, szstem, dicpos ) == 0 || aforms.nerror != 0 ?
        aforms.nerror : aforms.rcount;
    }
    return 0;
  ON_ERRORS( -1 )
}

short MLMA_API EXPORT mlmaruGetWordInfo( lexeme_t       nlexid,
                                         unsigned char* wdinfo )
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
      doGetWdInfo     getwdi( wdinfo, 0 );

      return FindStem( getwdi, stemtree, szstem, dicpos );
    }
    return 0;
  ON_ERRORS( -1 )
}

int   MLMAPROC        mlmaruLoadMbAPI( IMlmaMb**  ptrAPI )
{
  if ( ptrAPI == NULL )
    return -1;
  *ptrAPI = (IMlmaMb*)&mlmaMbInstance;
    return 0;
}

int   MLMAPROC        mlmaruLoadWcAPI( IMlmaWc**  ptrAPI )
{
  if ( ptrAPI == NULL )
    return -1;
  *ptrAPI = (IMlmaWc*)&mlmaWcInstance;
    return 0;
}
