# include <api.h>

namespace NAMESPACE
{

  template <class Mlma, Mlma& mlma>
  class CMlmaWc final: public IMlmaWc
  {
    class WcMatch;

  public:
    int MLMAPROC  Attach() override {  return 0;  }
    int MLMAPROC  Detach() override {  return 0;  }

    int MLMAPROC  CheckWord(
      const widechar*,  size_t, unsigned )  override;

    int MLMAPROC  Lemmatize(
      const widechar*,  size_t,
      SLemmInfoW*,      size_t,
      widechar*,        size_t,
      SGramInfo*,       size_t, unsigned )  override;
    int MLMAPROC  BuildForm(
      widechar*,        size_t,
      lexeme_t,         formid_t )  override;
    int MLMAPROC  FindForms(
      widechar*,        size_t,
      const widechar*,  size_t,
      unsigned char,    unsigned )  override;
    int MLMAPROC  GetWdInfo(
      unsigned char*,   lexeme_t )  override;
    int MLMAPROC  FindMatch( IMlmaMatch*,
      const widechar*,  size_t   )  override;
  };

  template <class Mlma, Mlma& mlma>
  class CMlmaWc<Mlma, mlma>::WcMatch: public IMlmaMatch
  {
    IMlmaMatch*   client;

  public:
    WcMatch( IMlmaMatch* pc ): client( pc ) {}

    int MLMAPROC  Attach() override {  return 1;  }
    int MLMAPROC  Detach() override {  return 1;  }
    int MLMAPROC  AddLexeme( lexeme_t, int, const SStrMatch* ) override;
  };

  // CMlmaWc template implementation

  template <class Mlma, Mlma& mlma>
  int   CMlmaWc<Mlma, mlma>::CheckWord( const widechar* pwsstr, size_t  cchstr, unsigned  dwsets )
  {
    char    szword[0x30];
    size_t  ccword;

    // get string length and convert to ansi
    if ( (ccword = codepages::widetombcs( codepages::codepage_1251, szword, sizeof(szword), pwsstr, cchstr )) == (size_t)-1 )
      return WORDBUFF_FAILED;
    return mlma.CheckWord( szword, ccword, dwsets );
  }

  template <class Mlma, Mlma& mlma>
  int   CMlmaWc<Mlma, mlma>::Lemmatize(
    const widechar*  pwsstr, size_t  cchstr,
    SLemmInfoW*      output, size_t  cchout,
    widechar*        plemma, size_t  clemma,
    SGramInfo*       pgrams, size_t  ngrams, unsigned dwsets )
  {
    SLemmInfoA  lmbuff[0x20];
    char        szlemm[0xf0];
    char        szword[0xf0];
    char*       lplemm;
    size_t      ccword;
    int         lcount;
    int         lindex;

  // get string length and convert to ansi
    if ( (ccword = codepages::widetombcs( codepages::codepage_1251, szword, 0xf0, pwsstr, cchstr )) == (size_t)-1 )
      return WORDBUFF_FAILED;

  // call default lemmatizer
    if ( (lcount = mlma.Lemmatize(
      szword,                               ccword,
      output != nullptr ? lmbuff : nullptr, std::min( cchout, std::size(lmbuff) ),
      plemma != nullptr ? szlemm : nullptr, std::min( clemma, std::size(szlemm) ),
      pgrams,                               ngrams, dwsets )) <= 0 )
    return lcount;

  // fill output data
    for ( lindex = 0; output != nullptr && lindex < lcount; ++lindex, ++output )
    {
      output->nlexid = lmbuff[lindex].nlexid;
      output->pgrams = lmbuff[lindex].pgrams;
      output->ngrams = lmbuff[lindex].ngrams;
      output->plemma = lmbuff[lindex].plemma == nullptr ? nullptr : plemma + (lmbuff[lindex].plemma - szlemm);
    }

    for ( lindex = 0, lplemm = szlemm; plemma != nullptr && lindex < lcount; ++lindex )
    {
      size_t    nccstr = codepages::mbcstowide( codepages::codepage_1251, (widechar*)plemma, clemma, lplemm ) + 1;

      plemma += nccstr;
      clemma -= nccstr;
      lplemm += nccstr;
    }
    return lcount;
  }

  template <class Mlma, Mlma& mlma>
  int   CMlmaWc<Mlma, mlma>::BuildForm(
    widechar* output, size_t    cchout,
    lexeme_t  nlexid, formid_t  idform )
  {
    char        szform[98];
    char*       lpform;
    int         fcount;
    int         findex;

  // check the arguments
    if ( output == nullptr || cchout == 0 )
      return ARGUMENT_FAILED;

  // build the form
    if ( (fcount = mlma.BuildForm( szform, std::min( cchout, std::size(szform) ) - 1, nlexid, idform )) <= 0 )
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

  template <class Mlma, Mlma& mlma>
  int   CMlmaWc<Mlma, mlma>::FindForms(
    widechar*        output, size_t    cchout,
    const widechar*  pwsstr, size_t    cchstr,
    unsigned char    idform, unsigned  dwsets )
  {
    char    szword[98];
    char    szform[98];
    char*   lpform;
    size_t  ccword;
    int     fcount;
    int     findex;

  // check output buffer
    if ( output == nullptr || cchout == 0 || pwsstr == nullptr || cchstr == 0 )
      return ARGUMENT_FAILED;

  // get string length and convert to ansi
    if ( (ccword = codepages::widetombcs( codepages::codepage_1251, szword, std::min( cchstr, std::size(szword) ), pwsstr, cchstr )) == (size_t)-1 )
      return WORDBUFF_FAILED;

  // build the form
    if ( (fcount = mlma.FindForms( szform, std::min( cchout, std::size(szform) ) - 1, szword, ccword, idform, dwsets )) <= 0 )
      return fcount;

  // convert
    for ( findex = 0, lpform = szform; findex < fcount; ++findex )
    {
      size_t  nccstr = codepages::mbcstowide( codepages::codepage_1251, output, cchout, lpform ) + 1;

      output += nccstr;
      cchout -= nccstr;
      lpform += nccstr;
    }

    return fcount;
  }

  template <class Mlma, Mlma& mlma>
  int   CMlmaWc<Mlma, mlma>::GetWdInfo( unsigned char*  pwinfo, lexeme_t  nlexid )
  {
    return mlma.GetWdInfo( pwinfo, nlexid );
  }

  template <class Mlma, Mlma& mlma>
  int   CMlmaWc<Mlma, mlma>::FindMatch( IMlmaMatch* pmatch, const widechar* pwsstr, size_t cchstr )
  {
    WcMatch client( pmatch );
    char    szword[0x30];
    size_t  ccword;

  // check the args
    if ( pmatch == nullptr || pwsstr == nullptr || cchstr == 0 )
      return ARGUMENT_FAILED;

  // get string length and convert to native codepage
    if ( (ccword = codepages::widetombcs( codepages::codepage_1251, szword, sizeof(szword), pwsstr, cchstr )) == (size_t)-1 )
      return WORDBUFF_FAILED;

    return mlma.FindMatch( &client, szword, ccword );
  }

  // IMlmaWc::WcMatch implementation

  template <class Mlma, Mlma& mlma>
  int   CMlmaWc<Mlma, mlma>::WcMatch::AddLexeme(
    lexeme_t          nlexid,
    int               nforms,
    const SStrMatch*  pmatch )
  {
    SStrMatch amatch[0x100];
    widechar  strbuf[0x100 * 0x30];
    widechar* strptr = strbuf;

    for ( int idform = 0; idform < nforms; ++idform )
    {
      amatch[idform] = { (const char*)strptr, pmatch[idform].cc, pmatch[idform].id };

      codepages::mbcstowide( codepages::codepage_1251, strptr, std::end(strbuf) - strptr,
        pmatch[idform].sz, pmatch[idform].cc );
      strptr += pmatch[idform].cc;
      *strptr++ = 0;
    }
    return client->AddLexeme( nlexid, nforms, amatch );
  }

}
