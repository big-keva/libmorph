# include <api.h>

namespace NAMESPACE
{

  template <class Mlfa, Mlfa& mlma>
  class CMlfaWc final: public IMlfaWc
  {
    class WcMatch;

  public:
    int MLMAPROC  Attach() override {  return 1;  }
    int MLMAPROC  Detach() override {  return 1;  }

    int MLMAPROC  GetWdInfo(
      unsigned char*,   lexeme_t )  override;
    int MLMAPROC  GetModels(
      widechar*,        size_t, unsigned ) override;
    int MLMAPROC  Lemmatize(
      const widechar*,  size_t,
      SStemInfoW*,      size_t,
      widechar*,        size_t,
      SGramInfo*,       size_t, unsigned ) override;
    int MLMAPROC  BuildForm(
      widechar*,        size_t,
      const widechar*,  size_t,
      unsigned,         formid_t ) override;
  };

  // CMlmaWc template implementation

  template <class Mlfa, Mlfa& mlfa>
  int   CMlfaWc<Mlfa, mlfa>::GetWdInfo( unsigned char*  pwinfo, lexeme_t  nlexid )
  {
    return mlfa.GetWdInfo( pwinfo, nlexid );
  }

  template <class Mlfa, Mlfa& mlfa>
  int   CMlfaWc<Mlfa, mlfa>::GetModels( widechar* models, size_t  modlen, unsigned  uclass )
  {
    char  buffer[0x400];
    auto  bufptr = buffer;
    auto  modend = models + modlen;
    int   mcount;

    if ( (mcount = mlfa.GetModels( buffer, std::min(sizeof(buffer), modlen), uclass )) <= 0 )
      return mcount;

    for ( int imodel = 0; imodel < mcount; ++imodel )
    {
      auto  lmodel = ToWidechar( models, modend - models, bufptr );

      if ( lmodel == (size_t)-1 )
        return LEMMBUFF_FAILED;

      models += lmodel + 1;
      bufptr += lmodel + 1;
    }
    return mcount;
  }

  template <class Mlfa, Mlfa& mlfa>
  int   CMlfaWc<Mlfa, mlfa>::Lemmatize(
    const widechar*  pwsstr, size_t  cchstr,
    SStemInfoW*      output, size_t  cchout,
    widechar*        plemma, size_t  clemma,
    SGramInfo*       pgrams, size_t  ngrams, unsigned dwsets )
  {
    SStemInfoA  stbuff[0x20];
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
    if ( (lcount = mlfa.Lemmatize(
      szword,                               ccword,
      output != nullptr ? stbuff : nullptr, std::min( cchout, std::size(stbuff) ),
      plemma != nullptr ? szlemm : nullptr, std::min( clemma, std::size(szlemm) ),
      pgrams,                               ngrams, dwsets )) <= 0 )
    return lcount;

  // fill output data
    for ( lindex = 0; output != nullptr && lindex < lcount; ++lindex, ++output )
    {
      output->plemma = stbuff[lindex].plemma != nullptr ? plemma + (stbuff[lindex].plemma - szlemm) : nullptr,
      output->ccstem = stbuff[lindex].ccstem;
      output->nclass = stbuff[lindex].nclass;
      output->pgrams = stbuff[lindex].pgrams;
      output->ngrams = stbuff[lindex].ngrams;
      output->weight = stbuff[lindex].weight;
    }

    for ( lindex = 0, lplemm = szlemm; plemma != nullptr && lindex < lcount; ++lindex )
    {
      size_t    nccstr = codepages::mbcstowide( codepages::codepage_1251, plemma, clemma, lplemm ) + 1;
        plemma += nccstr;
        clemma -= nccstr;
        lplemm += nccstr;
    }
    return lcount;
  }

  template <class Mlfa, Mlfa& mlfa>
  int   CMlfaWc<Mlfa, mlfa>::BuildForm(
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
    fcount = mlfa.BuildForm( szform, std::min( cchout, sizeof(szform) - 1 ),
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

}
