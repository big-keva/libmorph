# include "lemmatiz.h"
# include <xmorph/flexmake.h>
# include <xmorph/grammap.h>
# include <libcodes/codes.h>

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
    o.iForm = (byte_t)MapWordInfo( o.wInfo = (byte_t)s.wdinfo,
                                    o.gInfo =         g.gInfo,
                                    o.other =         g.other );
  }

  inline  static  void  set_0xffinfo( SGramInfo& o, const steminfo& s, const SGramInfo& g )
  {
    o.wInfo = (byte_t)s.wdinfo;
    o.iForm = 0xff;
    o.gInfo = g.gInfo;
    o.other = g.other;
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

    assert( nerror == 0 );
    assert( plemma != 0 );

  // check if overflow will occur in this call
    if ( plemma < elemma )  {  plemma->nlexid = nlexid;  plemma->ngrams = 0;  }
      else  return (nerror = LIDSBUFF_FAILED);

  // check if there is a buffer for normal forms; create normal forms
  // and store single form to the buffer selected for forms
    if ( (plemma->plemma = pforms) != nullptr )
    {
      byte_t  fmbuff[256];
      size_t  ccstem;

    // ѕостроить основу из словар€ без измен€емой ее части
      memcpy( fmbuff, szstem, ccstem = (pszstr - szstem) );

    // ѕостроить окончани€, соответствующие нормальной форме слова.
    // ≈сли слово не флективно, то окончание будет нулевым
      if ( stinfo.tfoffs != 0 )
      {
        unsigned  nfInfo = GetNormalInfo( stinfo.wdinfo, flexes->gInfo, dwsets );

      // ѕостроить нормальную форму слова. ≈сли она не строитс€, проверить, почему.
        if ( !CreateDictForm( fmbuff + ccstem, stinfo, nfInfo, afAnimated|afNotAlive ) )
          strncpy( (char*)fmbuff, (const char*)szstem, ccstem = pszstr - szstem );
        else ccstem = strlen( (const char*)fmbuff );
      }
        else
      fmbuff[ccstem] = 0;

    // —лово может иметь текст в постпозиции
      if ( szpost != nullptr )
      {
        memcpy( fmbuff + ccstem, szpost + 1, *szpost );
          fmbuff[ccstem = ccstem + *szpost] = '\0';
      }

    // ѕривести слово к минимальной возможной капитализации
      SetCapScheme( (char*)fmbuff, GetMinScheme( stinfo.MinCapScheme(), (const char*)fmbuff, scheme >> 8 ) );

    // create output string
    // check for overflow
      if ( encode != codepages::codepage_1251 )
      {
        size_t  cchout;

        if ( (cchout = codepages::mbcstombcs( encode, pforms, eforms - pforms, codepages::codepage_1251,
          (const char*)fmbuff, ccstem + 1 )) == (size_t)-1 )  return LEMMBUFF_FAILED;
        else pforms += cchout;
      }
        else
      {
        if ( pforms + ccstem >= eforms )
          return (nerror = LEMMBUFF_FAILED);

      // register normal form
        memcpy( pforms, fmbuff, ccstem + 1 );
          pforms += ccstem + 1;
      }
    }

  // ѕроверить, надо ли восстанавливать грамматические описани€
    if ( (plemma->pgrams = pgrams) != NULL )
    {
      for ( ; fcount > 0 && pgrams < egrams; --fcount )
        set_graminfo( *pgrams++, stinfo, *flexes++ );

      if ( fcount == 0 )  plemma->ngrams = pgrams - plemma->pgrams;
        else return (nerror = GRAMBUFF_FAILED);
    }
    ++plemma;
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
    byte_t    szform[64];
    byte_t    stails[256];
    byte_t*   lptail = stails;
    size_t    ccstem;
    int       ntails;
    unsigned  tfoffs = stinfo.tfoffs;
    word16_t  grInfo;
    byte_t    bFlags;

  // check the arguments
    assert( output != NULL );
    assert( nerror == 0 );

  // ѕостроить описание нужной формы, если она задана идентификатором
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

  // ќтсе€ть случаи, когда разрешено только множественное число,
  // а требуетс€ наоборот единственное
    if ( idform != 0x000000ff && (stinfo.wdinfo & 0x3F) != 52 )
      if ( (stinfo.wdinfo & wfMultiple) != 0 && (grInfo & gfMultiple) == 0 )
        return 0;

  // ѕостроить основу из словар€ без измен€емой ее части
    memcpy( szform, szstem, ccstem = (pszstr - szstem) );

  // ѕостроить окончани€, соответствующие нормальной форме слова.
  // ≈сли слово не флективно, то окончание будет нулевым
    if ( tfoffs == 0 )
    {
      stails[0] = '\0';
        ntails = 1;
    }
      else
    {
      if ( (ntails = CreateFormFlex( stails, stinfo, grInfo, bFlags )) == 0 )
        return 0;
    }

  // the loop creates forms directly in the output buffer to prevent
  // copying of data twice, to the local buffer and to the output
  // buffer; the buffer size check is performed online; strings
  // are appended to the buffer including '\0' character of the
  // flexion
    while ( ntails-- > 0 )
    {
      byte_t* outptr;
      size_t  cbcopy;

    // append next tail
      for ( outptr = szform + ccstem; (*outptr = *lptail++) != '\0'; ++outptr )
        (void)NULL;

    // check if a word has a postfix; append the postfix; check if overflow
      if ( szpost != NULL )
        outptr = *szpost + (byte_t*)memcpy( outptr, szpost + 1, *szpost );

    // set the zero
      *outptr++ = '\0';

    // set capitalization scheme
      SetCapScheme( (char*)szform, GetMinScheme( stinfo.MinCapScheme(), (char*)szform, scheme >> 8 ) );

    // output the string
      if ( (cbcopy = codepages::mbcstombcs( encode, output, outend - output, codepages::codepage_1251, (const char*)szform, outptr - szform )) == (size_t)-1 )
        return (nerror = LEMMBUFF_FAILED);  else  output += cbcopy;

    // correct buffer length
      ++rcount;
    }
    return 0;
  }

  //=====================================================================
  // Meth: CreateFormFlex
  // ‘ункци€ синтезирует варианты флективной части слова, исход€ из его
  // части речи, чередований, окончаний и грамматической информации
  // о форме.
  // ¬озвращает количество построенных вариантов.
  //=====================================================================
  static int  CreateFormFlex( byte_t* lpDest, const steminfo& stinfo, word16_t  grInfo, byte_t  bflags )
  {
    byte_t    szfrag[0x20];
    unsigned  ccfrag = 0;

  // ¬ычислить нужную ступень чередовани€
    if ( stinfo.mtoffs != 0 )
    {
      const byte_t* mixtab = stinfo.GetSwapTable();
      int           mpmask = 0x08 << stinfo.GetSwapLevel( grInfo, bflags );
      int           mcount = *mixtab++;
      const byte_t* thestr;

      for ( thestr = NULL; thestr == NULL && mcount-- > 0; mixtab += (*mixtab++ & 0x0f) )
        if ( (*mixtab & mpmask) != 0 )  thestr = mixtab;

      if ( thestr != NULL )
        memcpy( szfrag, thestr + 1, ccfrag = 0x0f & *thestr );
    }

  // “еперь построить возможные окончани€
    return BuildFlexSet( lpDest, stinfo.GetFlexTable(), szfrag, ccfrag, grInfo, bflags );  
  }

  //=====================================================================
  // Meth: CreateDictForm
  // ‘ункци€ синтезирует варианты флективной части слова, исход€ из его
  // части речи, чередований, окончаний и грамматической информации
  // о форме.
  // ¬озвращает количество построенных вариантов.
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
