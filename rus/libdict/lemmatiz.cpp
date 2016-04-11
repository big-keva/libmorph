# include "lemmatiz.h"
# include "capsheme.h"
# include "flexmake.h"
# include "grammap.h"

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
//
// the strings uniquelizer
// provides unique output for forms list
//
  struct  uniqlist
  {
    char*       lpbuff;       // the output buffer
    size_t      ccbuff;       // buffer size
    const char* pforms[256];  // the forms cache
    int         nforms;

  public:
                uniqlist( char* lpdest, size_t ccdest ): lpbuff( lpdest ), ccbuff( ccdest ), nforms( 0 )
                  {
                  }
    int         Append();
    char*       GetBuf( size_t );
  protected:
    bool        Search( const char* pszstr, int& sindex );
  };

//
// the list maker - fills the specified list with partial-parsed forms
// on grammatical descriptions
//
  struct  FormsBuilder
  {
    const steminfo& rfstem;
    uniqlist        output;
    const byte08_t* szstem;
    size_t          ccstem;
    const byte08_t* mixtab;

  public:     // construction
                    FormsBuilder( char*           lpdest,
                                  size_t          ccdest,
                                  const byte08_t* szbase,
                                  size_t          ccbase,
                                  const steminfo& stinfo ): rfstem( stinfo ),
                                                            output( lpdest, ccdest ),
                                                            szstem( szbase ),
                                                            ccstem( ccbase )
                      {
                      }
    int             RegisterFlex( const byte08_t* pszstr,
                                  unsigned        cchstr,
                                  word16_t        grinfo,
                                  byte08_t        bflags );
  };

// uniqlist inline implementation

  inline  int     uniqlist::Append()
    {
      int       search;

    // check if string is present
      if ( Search( lpbuff, search ) )
        return 0;

    // check for overflow
      if ( nforms >= (int)(sizeof(pforms) / sizeof(pforms[0])) )
        return WORDBUFF_FAILED;

    // register in the output list; check for the overflow
      if ( search < nforms )
        memmove( pforms + search + 1, pforms + search,
          (nforms - search) * sizeof(const char*) );
      pforms[search] = lpbuff;
        ++nforms;
      do  --ccbuff;  while ( *lpbuff++ != '\0' );
      return 0;
    }

  inline  char*   uniqlist::GetBuf( size_t length )
    {
      return length >= ccbuff ? NULL : lpbuff;
    }

  inline  bool    uniqlist::Search( const char* pszstr, int& sindex )
    {
      int   lo = 0;
      int   hi = (int)nforms - 1;
      bool  ok = false;

      while ( lo <= hi )
      {
        int   md = (lo + hi) >> 1;
        int   rc = strcmp( pforms[md], pszstr );

        if ( rc < 0 ) lo = md + 1;
          else
        {
          hi = md - 1;
          ok |= (rc == 0);
        }
      }
      sindex = lo;
      return ok;
    }

  // FormsBuilder inline implementation

    inline  int       FormsBuilder::RegisterFlex( const byte08_t* pszstr,
                                                  unsigned        cchstr,
                                                  word16_t        grinfo,
                                                  byte08_t        bflags )
    {
      const byte08_t* pszmix = NULL;
      unsigned        cchmix = 0;
      size_t          ccform;
      char*           lptail;
      char*           buforg;
      int             mpower;

    // detect mix power
      if ( mixtab != NULL )
      {
        if ( (mpower = GetMixPower( rfstem.wdinfo, grinfo, bflags )) > *mixtab )
          mpower = 1;
        for ( pszmix = mixtab + 1, mpower = 0x10 < (mpower - 1); (*pszmix & mpower) == 0;
          pszmix += 1 + (0x0f & *pszmix) )  (void)NULL;
        cchmix = 0x0f & *pszmix++;
      }

    // access the buffer
      if ( (lptail = buforg = output.GetBuf( (ccform = ccstem + cchmix + cchstr + rfstem.ccpost) + 1 )) == NULL )
        return WORDBUFF_FAILED;

    // append the string
      lptail = (char*)memcpy( lptail, szstem, ccstem ) + ccstem;
      lptail = (char*)memcpy( lptail, pszmix, cchmix ) + cchmix;
      lptail = (char*)memcpy( lptail, pszstr, cchstr ) + cchstr;
      lptail = (char*)memcpy( lptail, rfstem.szpost, rfstem.ccpost ) + rfstem.ccpost;
        *lptail++ = '\0';

    // set capitalization scheme
      SetCapScheme( buforg, GetMinScheme( pspMinCapValue[rfstem.wdinfo & 0x3F], buforg ) );

    // set next form
      return output.Append();
    }

  static int  CreateFormList( FormsBuilder& rfmake, const byte08_t* ptable,
                              byte08_t*     curstr, unsigned        curlen );

  static int  CreateFormFlex( byte08_t*     lpDest, word16_t        wdinfo,
                              unsigned      mtoffs, unsigned        tfoffs,
                              word16_t      grInfo, byte08_t        bflags );

  static int  CreateDictForm( byte08_t*     lpDest, const steminfo& stinfo,
                              word16_t      grInfo, byte08_t        bflags );

//=========================================================================================
// doLemmatize::Register
// the real safe lemmatization output creation method implemented as a class member
// to make code more readable
//=========================================================================================
  int   doLemmatize::InsertStem( lexeme_t         nlexid,
                                 const byte08_t*  pszstr,
                                 const steminfo&  stinfo,
                                 const SGramInfo* flexes,
                                 unsigned         fcount )
  {
    assert( nerror == 0 );
    assert( plemma != 0 );

  // check if overflow will occur in this call
    if ( clemma == 0 )
      return (nerror = LIDSBUFF_FAILED);

  // output lexeme id
    plemma->nlexid = nlexid;

  // check if there is a buffer for normal forms; create normal forms
  // and store single form to the buffer selected for forms
    if ( (plemma->plemma = pforms) != NULL )
    {
      byte08_t  fmbuff[256];
      size_t    ccstem;

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
      if ( stinfo.szpost != NULL )
      {
        memcpy( fmbuff + ccstem, stinfo.szpost, stinfo.ccpost );
          fmbuff[ccstem = ccstem + stinfo.ccpost] = '\0';
      }

    // ѕривести слово к минимальной возможной капитализации
      SetCapScheme( (char*)fmbuff, GetMinScheme( pspMinCapValue[stinfo.wdinfo & 0x3F], (const char*)fmbuff, scheme >> 8 ) );

    // check for overflow
      if ( ccstem >= cforms )
        return (nerror = LEMMBUFF_FAILED);

    // register normal form
      memcpy( pforms, fmbuff, ccstem + 1 );
        pforms += ccstem + 1;
        cforms -= ccstem + 1;
    }

  // ѕроверить, надо ли восстанавливать грамматические описани€
    if ( (plemma->pgrams = pgrams) != NULL )
    {
    // check possible overflow
      if ( cgrams < fcount )
        return (nerror = GRAMBUFF_FAILED);
      
    // copy...
      for ( ; fcount-- > 0; ++pgrams, ++flexes )
        pgrams->iForm = (byte08_t)MapWordInfo( pgrams->wInfo = (byte08_t)stinfo.wdinfo,
          pgrams->gInfo = flexes->gInfo, pgrams->other = flexes->other );
    }
    ++plemma;
      --clemma;
    ++rcount;
    return 0;
  }

// doBuildForm implementation

  int   doBuildForm::InsertStem( lexeme_t         nlexid,
                                 const byte08_t*  pszstr,
                                 const steminfo&  stinfo,
                                 const SGramInfo* flexes,
                                 unsigned         fcount )
  {
    char*       outend = output + cchout;
    byte08_t    szform[64];
    byte08_t    stails[256];
    byte08_t*   lptail = stails;
    size_t      ccstem;
    int         ntails;
    unsigned    tfoffs = stinfo.tfoffs;
    word16_t    grInfo;
    byte08_t    bFlags;

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
        if ( tfoffs != 0 && !GetWordInfo( (byte08_t)stinfo.wdinfo, idform, grInfo, bFlags ) )
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
      if ( (ntails = CreateFormFlex( stails, stinfo.wdinfo, stinfo.mtoffs, tfoffs, grInfo, bFlags )) == 0 )
        return 0;
    }

  // the loop creates forms directly in the output buffer to prevent
  // copying of data twice, to the local buffer and to the output
  // buffer; the buffer size check is performed online; strings
  // are appended to the buffer including '\0' character of the
  // flexion
    while ( ntails-- > 0 )
    {
      const unsigned char*  srctop = szform;
      const unsigned char*  srcend = srctop + ccstem;
      char*                 outorg = output;

    // check the length of the output buffer
      if ( cchout <= ccstem )
        return (nerror = LEMMBUFF_FAILED);
      while ( srctop < srcend )
        *output++ = (char)*srctop++;

    // append next tail
      while ( output < outend && *lptail != '\0' )
        *output++ = (char)*lptail++;
      if ( output >= outend )
        return (nerror = LEMMBUFF_FAILED);
      else ++lptail;

    // check if a word has a postfix; append the postfix; check if overflow
      if ( (srctop = stinfo.szpost) != NULL )
      {
        for ( srcend = srctop + stinfo.ccpost; output < outend && srctop < srcend; )
          *output++ = (char)*srctop++;
        if ( output >= outend )
          return (nerror = LEMMBUFF_FAILED);
      }

    // set the zero
      *output++ = '\0';

    // set capitalization scheme
      SetCapScheme( outorg, GetMinScheme( pspMinCapValue[stinfo.wdinfo & 0x3F], outorg, scheme >> 8 ) );

    // correct buffer length
      cchout -= (output - outorg);
        ++rcount;
    }
    return rcount;
  }

// doListForms implementation

  int   doListForms::InsertStem( lexeme_t         nlexid,
                                 const byte08_t*  pszstr,
                                 const steminfo&  stinfo,
                                 const SGramInfo* flexes,
                                 unsigned         fcount )
  {
    FormsBuilder  mklist( output, cchout, szstem, pszstr - szstem, stinfo );
    byte08_t      strbuf[0x20];

  // check for faults
    assert( nerror == 0 );
    assert( output != 0 );

  // check if the word is NOT flective
    if ( stinfo.tfoffs == 0 )
    {
    // register null string
      if ( mklist.RegisterFlex( (const byte08_t*)"", 0, 0, 0 ) != 0 )
        return (nerror = WORDBUFF_FAILED);
    }
      else
    if ( CreateFormList( mklist, flexTree + (stinfo.tfoffs << 4), strbuf, 0 ) != 0 )
      return (nerror = WORDBUFF_FAILED);

  // fill output structure
    output  = mklist.output.lpbuff;
    cchout  = mklist.output.ccbuff;
    rcount += mklist.output.nforms;

    return 0;
  }

  static int  CreateFormList( FormsBuilder&   rfmake,
                              const byte08_t* ptable,
                              byte08_t*       curstr,
                              unsigned        curlen = 0 )
  {
    byte08_t        bflags;
    int             ccount = (bflags = *ptable++) & 0x7f;
    int             nerror;

    while ( ccount-- > 0 )
    {
      const byte08_t* subdic;
      int             sublen;

      curstr[curlen] = *ptable++;
        sublen = getserial( ptable );
      ptable = sublen + (subdic = ptable);

      if ( (nerror = CreateFormList( rfmake, subdic, curstr, curlen )) != 0 )
        return nerror;
    }

    if ( (bflags & 0x80) != 0 )
    {
      int   ngrams = *ptable++;

      while ( ngrams-- > 0 )
        if ( (nerror = rfmake.RegisterFlex( curstr, curlen, getword16( ptable ), *ptable++ )) != 0 )
          return nerror;
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
  static int  CreateFormFlex( byte08_t* lpDest, word16_t  wdinfo,
                              unsigned  mtoffs, unsigned  tfoffs,
                              word16_t  grInfo, byte08_t  bflags )
  {
    byte08_t  szfrag[0x20];
    unsigned  ccfrag = 0;

  // ¬ычислить нужную ступень чередовани€
    if ( mtoffs != 0 )
    {
      const byte08_t* mixtab = mxTables + mtoffs;
      const byte08_t* thestr;
      int             mpower;

      if ( (mpower = GetMixPower( wdinfo, grInfo, bflags )) > *mixtab++ )
        mpower = 1;
      for ( thestr = mixtab, mpower = 0x10 << (mpower - 1); (*thestr & mpower) == 0;
        thestr += 1 + (0x0f & *thestr) )  (void)NULL;
      memcpy( szfrag, thestr, ccfrag = 0x0f & *thestr++ );
    }

  // “еперь построить возможные окончани€
    return CreateDestStringsOnTable( lpDest, szfrag, ccfrag, flexTree + (tfoffs << 4), grInfo, bflags );  
  }

  //=====================================================================
  // Meth: CreateDictForm
  // ‘ункци€ синтезирует варианты флективной части слова, исход€ из его
  // части речи, чередований, окончаний и грамматической информации
  // о форме.
  // ¬озвращает количество построенных вариантов.
  //=====================================================================
  static int  CreateDictForm( byte08_t* lpDest, const steminfo& stinfo,
                              word16_t  grInfo, byte08_t        bflags )
  {
    int   nforms;

  // try create the exact form
    if ( (nforms = CreateFormFlex( lpDest, stinfo.wdinfo, stinfo.mtoffs, stinfo.tfoffs, grInfo, bflags )) != 0 )
      return nforms;

  // check if is inreverce - nothing to do on reverce forms
    if ( (grInfo & gfRetForms) != 0 )
      return 0;
    
  // check if verb or adjective
    if ( !IsVerb( stinfo.wdinfo ) && !IsAdjective( stinfo.wdinfo ) )
      return 0;

  // try build reverce
    return CreateFormFlex( lpDest, stinfo.wdinfo, stinfo.mtoffs, stinfo.tfoffs, grInfo | gfRetForms, bflags );
  }

}  // namespace

