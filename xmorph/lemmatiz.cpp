# include "lemmatiz.h"
# include "flexmake.h"
# include "grammap.h"
# include "moonycode/codes.h"

# if defined( NO_ASSERT )
#   if defined( NDEBUG )
#     define  assert( expr )
#   else
#     define  assert( expr )  if ( !(expr) )  exit( -1 )
#   endif
# else
#   include <assert.h>
# endif

namespace LIBMORPH_NAMESPACE
{
  static int  CreateFormFlex( byte_t*         pszout,
                              const steminfo& stinfo,
                              word16_t        grInfo,
                              byte_t          bflags );

  static int  CreateDictForm( byte_t*         lpDest,
                              const steminfo& stinfo,
                              word16_t        grInfo,
                              byte_t          bflags );

  inline  static  void  set_flexinfo( SGramInfo& o, const steminfo& s, const SGramInfo& g )
  {
    o.idForm = MapWordInfo( o.wdInfo = s.wdinfo & ~(wfPostSt + wfFlexes + wfMixTab + 0x1800),
                            o.grInfo = g.grInfo,                                  /* ^^^ mix type  */
                            o.bFlags = g.bFlags );
  }

  inline  static  void  set_0xffinfo( SGramInfo& o, const steminfo& s, const SGramInfo& g )
  {
    o = { (word16_t)(s.wdinfo & ~0xe000), 0xff, g.grInfo, g.bFlags };
  }

  inline  static  bool  has_gramform( const SLemmInfoA& lemm, byte_t form )
  {
    for ( auto ptr = lemm.pgrams, end = ptr + lemm.ngrams; ptr != end; ++ptr )
      if ( ptr->idForm == form )  return true;
    return false;
  }

//=========================================================================================
// doLemmatize::InsertStem
// the real safe lemmatization output creation method implemented as a class member
// to make code more readable
//=========================================================================================
  int   doLemmatize::InsertStem( lexeme_t         nlexid,
                                 const steminfo&  stinfo,
                                 const byte_t*    szpost,
                                 const byte_t*    pszstr,
                                 const SGramInfo* flexes,
                                 unsigned         fcount )
  {
    auto  set_graminfo = stinfo.tfoffs != 0 ? set_flexinfo : set_0xffinfo;
    bool  merge_lexeme = false;

    assert( nerror == 0 );

  // check if overflow will occur in this call;
  // check if last lemma exists and may be merged to current
    if ( plemma != nullptr )
    {
      if ( nlemma == 0 || plemma[-1].nlexid != nlexid )
      {
        if ( plemma >= elemma )
          return (nerror = LIDSBUFF_FAILED);

        plemma->nlexid = nlexid;
        plemma->plemma = pforms;
        plemma->pgrams = pgrams;
        plemma->ngrams = 0;
      } else merge_lexeme = true;
    }

  // check if there is a buffer for normal forms; create normal forms
  // and store single form to the buffer selected for forms
    if ( !merge_lexeme && pforms != nullptr )
    {
      char    fmbuff[256];
      char*   outptr = fmbuff;

    // Построить основу из словаря без изменяемой ее части
      for ( auto src = szstem; src != pszstr; )
        *outptr++ = *src++;

    // Построить окончания, соответствующие нормальной форме слова.
    // Если слово не флективно, то окончание будет нулевым
      if ( stinfo.tfoffs != 0 )
      {
        unsigned  nfInfo = GetNormalInfo( stinfo.wdinfo, flexes->grInfo, dwsets );

      // Построить нормальную форму слова. Если она не строится, проверить, почему.
        if ( CreateDictForm( (byte_t*)outptr, stinfo, nfInfo, afAnimated|afNotAlive ) )
          while ( *outptr != '\0' ) ++outptr;
      }

      *outptr = '\0';

    // Слово может иметь текст в постпозиции
      if ( szpost != nullptr )
      {
        auto  ccpost = *szpost++;
        auto  endptr = ccpost + szpost;

        while ( szpost != endptr )
          *outptr++ = *szpost++;

        *outptr = '\0';
      }

    // Привести слово к минимальной возможной капитализации
      SetCapScheme( fmbuff, GetMinScheme( stinfo.MinCapScheme(), fmbuff, scheme >> 8 ) );

    // create output string
    // check for overflow
      if ( encode != codepages::codepage_1251 )
      {
        size_t  cchout;

        if ( (cchout = codepages::mbcstombcs( encode, pforms, eforms - pforms, codepages::codepage_1251, fmbuff, outptr - fmbuff + 1 )) == (size_t)-1 )
          return (nerror = LEMMBUFF_FAILED);

        pforms += cchout;
      }
        else
      // register normal form
      {
        for ( auto src = fmbuff; pforms != eforms && src != outptr; )
          *pforms++ = *src++;
        if ( pforms == eforms )  return (nerror = LEMMBUFF_FAILED);
          else *pforms++ = '\0';
      }
    }

  // Проверить, надо ли восстанавливать грамматические описания
    if ( plemma != nullptr && pgrams != NULL )
    {
      if ( merge_lexeme )
      {
        for ( ; fcount > 0 && pgrams < egrams; --fcount )
          if ( !has_gramform( plemma[-1], flexes->idForm ) )  set_graminfo( *pgrams++, stinfo, *flexes++ );
            else ++flexes;
      }
        else
      {
        for ( ; fcount > 0 && pgrams < egrams; --fcount )
          set_graminfo( *pgrams++, stinfo, *flexes++ );
      }

      if ( fcount == 0 )  plemma->ngrams = (unsigned)(pgrams - plemma->pgrams);
        else return (nerror = GRAMBUFF_FAILED);
    }

    plemma += (plemma != nullptr && !merge_lexeme ? 1 : 0);
    nlemma += (!merge_lexeme ? 1 : 0);

    return 0;
  }

// doBuildForm implementation

  int   doBuildForm::InsertStem( lexeme_t         nlexid,
                                 const steminfo&  stinfo,
                                 const byte_t*    szpost,
                                 const byte_t*    pszstr,
                                 const SGramInfo* flexes,
                                 unsigned         fcount )
  {
    char      sbuild[64];
    char*     pbuild = sbuild;
    byte_t    stails[256];
    byte_t*   lptail = stails;
    int       ntails;
    unsigned  tfoffs = stinfo.tfoffs;
    word16_t  grInfo;
    byte_t    bFlags;

    (void)nlexid;
    (void)flexes;
    (void)fcount;

  // check the arguments
    assert( nerror == 0 );

  // check function call
    if ( output == nullptr || output >= outend )
      return (nerror = LEMMBUFF_FAILED);

  // Построить описание нужной формы, если она задана идентификатором
    if ( idform != (unsigned)-1 )
    {
      if ( (stinfo.wdinfo & 0x3f) == 51 )
      {
        if ( idform != 0x000000ff && tfoffs != idform )  return 0;
          else  tfoffs = 0;
        grinfo = 0;
        idform = 0xff;
      }
        else
      {
        if ( tfoffs == 0 && idform != 0x000000ff )
          return 0;
        if ( tfoffs != 0 && !GetWordInfo( (byte_t)stinfo.wdinfo, idform, grInfo, bFlags ) )
          return 0;
      }
    }
      else
    {
      grInfo = grinfo;
      bFlags = bflags;
    }

  // Отсеять случаи, когда разрешено только множественное число,
  // а требуется наоборот единственное
    if ( idform != 0x000000ff && (stinfo.wdinfo & 0x3F) != 52 )
      if ( (stinfo.wdinfo & wfMultiple) != 0 && (grInfo & gfMultiple) == 0 )
        return 0;

  // Построить основу из словаря без изменяемой ее части
    for ( auto src = szstem; src != pszstr; )
      *pbuild++ = *src++;

  // Построить окончания, соответствующие нормальной форме слова.
  // Если слово не флективно, то окончание будет нулевым
    if ( tfoffs == 0 )  {  stails[0] = '\0';  ntails = 1;  }
      else  ntails = CreateFormFlex( stails, stinfo, grInfo, bFlags );

  // the loop creates forms directly in the output buffer to prevent
  // copying of data twice, to the local buffer and to the output
  // buffer; the buffer size check is performed online; strings
  // are appended to the buffer including '\0' character of the
  // flexion
    while ( ntails-- > 0 )
    {
      auto    totail = pbuild;
      size_t  cchout;

    // append next tail
      while ( (*totail = *lptail++) != '\0' )
        ++totail;

    // check if a word has a postfix; append the postfix; check if overflow
      if ( szpost != NULL )
        totail = *szpost + (char*)memcpy( totail, szpost + 1, *szpost );

    // set the zero
      *totail++ = '\0';

    // set capitalization scheme
      SetCapScheme( (char*)sbuild, GetMinScheme( stinfo.MinCapScheme(), sbuild, scheme >> 8 ) );

    // output the string
      if ( (cchout = codepages::mbcstombcs( encode, output, outend - output, codepages::codepage_1251, sbuild, totail - sbuild )) == (size_t)-1 )
        return (nerror = LEMMBUFF_FAILED);  else  output += cchout;

    // correct buffer length
      ++rcount;
    }
    return 0;
  }

  //=====================================================================
  // Meth: CreateFormFlex
  // Функция синтезирует варианты флективной части слова, исходя из его
  // части речи, чередований, окончаний и грамматической информации
  // о форме.
  // Возвращает количество построенных вариантов.
  //=====================================================================
  static int  CreateFormFlex( byte_t* lpDest, const steminfo& stinfo, word16_t  grInfo, byte_t  bflags )
  {
    byte_t    szfrag[0x20];
    unsigned  ccfrag = 0;

  // Вычислить нужную ступень чередования
    if ( stinfo.mtoffs != 0 )
    {
      const byte_t* mixtab = stinfo.GetSwapTable();
      int           mpmask = 0x08 << stinfo.GetSwapLevel( grInfo, bflags );
      int           mcount = *mixtab++;
      const byte_t* thestr = nullptr;

      while ( thestr == nullptr && mcount-- > 0 )
      {
        if ( (*mixtab & mpmask) != 0 )  thestr = mixtab;
          else {  auto mixlen = *mixtab++ & 0x0f;  mixtab += mixlen;  }
      }

      if ( thestr != nullptr )
        memcpy( szfrag, thestr + 1, ccfrag = 0x0f & *thestr );
    }

  // Теперь построить возможные окончания
    return BuildFlexSet( lpDest, stinfo.GetFlexTable(), { grInfo, bflags }, { szfrag, ccfrag } );
  }

  //=====================================================================
  // Meth: CreateDictForm
  // Функция синтезирует варианты флективной части слова, исходя из его
  // части речи, чередований, окончаний и грамматической информации
  // о форме.
  // Возвращает количество построенных вариантов.
  //=====================================================================
  static int  CreateDictForm( byte_t*   lpDest, const steminfo& stinfo,
                              word16_t  grInfo, byte_t          bflags )
  {
    int   nforms;

  // try create the exact form
    if ( (nforms = CreateFormFlex( lpDest, stinfo, grInfo, bflags )) != 0 )
      return nforms;

  // check if is inreverce - nothing to do on reverce forms
    if ( (grInfo & gfRetForms) != 0 )
      return 0;
    
  // check if verb or adjective
    if ( !IsVerb( stinfo.wdinfo ) && !IsAdjective( stinfo.wdinfo ) )
      return 0;

  // try build reverce
    return CreateFormFlex( lpDest, stinfo, grInfo | gfRetForms, bflags );
  }

}  // namespace

