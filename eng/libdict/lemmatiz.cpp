# include "lemmatiz.h"
# include "capsheme.h"
# include "flexmake.h"

namespace __libmorpheng__
{
//
// the strings uniquelizer
// provides unique output for forms list
//
  struct  uniqlist
  {
    char*       lpbuff;       // the output buffer
    unsigned    ccbuff;       // buffer size
    const char* pforms[256];  // the forms cache
    int         nforms;
  public:
                uniqlist( char* lpdest, unsigned ccdest );
    int         Append();
    char*       GetBuf( unsigned );
  protected:
    bool        Search( const char* pszstr, int& sindex );
  };

//
// the list maker - fills the specified list with partial-parsed forms
// on grammatical descriptions
//
  struct  makelist
  {
    uniqlist        output;
    unsigned        idstem;
    const char*     lpstem;
    unsigned        ccstem;
    const char*     lpsuff;
    unsigned        ccsuff;
    const char*     szpost;
    unsigned        ccpost;
    word16_t        wbinfo;
    word16_t        tfoffs;
  public:
                    makelist( char*       lpdest,
                              unsigned    ccdest,
                              const char* ptrorg,
                              unsigned    cchorg );
    void            InitStem( const byte08_t* lpdict );
    int             Register( const char*     pszstr,
                              unsigned        cchstr );
  };

// uniqlist inline implementation

  inline          uniqlist::uniqlist( char*     lpdest,
                                      unsigned  ccdest ): lpbuff( lpdest ),
                                                          ccbuff( ccdest ),
                                                          nforms( 0 )
    {
    }

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

  inline  char*   uniqlist::GetBuf( unsigned length )
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

// makelist inline implementation

  inline              makelist::makelist( char*       lpdest,
                                          unsigned    ccdest,
                                          const char* ptrorg,
                                          unsigned    cchorg ):
                                            output( lpdest, ccdest ),
                                            idstem( 0 ),
                                            lpstem( ptrorg ),
                                            ccstem( cchorg ),
                                            lpsuff( NULL ),
                                            ccsuff( 0 ),
                                            szpost( NULL ),
                                            ccpost( 0 ),
                                            wbinfo( 0 ),
                                            tfoffs( 0 )
                      {
                      }

  inline  void      makelist::InitStem( const byte08_t* lpdict )
    {
      const char* lpinfo;

    // get stem info
      lpinfo = (const char*)GetInfoByID( idstem = GetWord16( lpdict ) );

    // get stem description
      wbinfo = GetWord16( lpinfo );
        lpinfo += sizeof(word16_t);
        lpdict += sizeof(word16_t);

    // check for flex
      if ( (wbinfo & wfFlexes) != 0 )
        tfoffs = GetWord16( lpinfo );

    // check for suffix
      if ( (idstem & wfSuffix) != 0 )
      {
        const char* sufofs = (const char*)suffixes
          + *lpdict++ * sizeof(word16_t);
        lpsuff = (const char*)suffixes + GetWord16( sufofs );
        ccsuff = (unsigned char)*lpsuff++;
      }
    // Слово может иметь текст в постпозиции
      if ( (idstem & wfPostSt) != 0 )
      {
        szpost = (const char*)lpdict;
        ccpost = (unsigned char)*szpost++;
      }
    }

  inline  int       makelist::Register( const char*     pszstr,
                                        unsigned        cchstr )
  {
    unsigned    ccform;
    char*       lptail;
    char*       buforg;

  // get form length
    ccform = ccstem + ccsuff + cchstr + ccpost;

  // access the buffer
    if ( (lptail = buforg = output.GetBuf( ccform + 1 )) == NULL )
      return WORDBUFF_FAILED;

  // append the string
    lptail = (char*)memcpy( lptail, lpstem, ccstem ) + ccstem;
    lptail = (char*)memcpy( lptail, lpsuff, ccsuff ) + ccsuff;
    lptail = (char*)memcpy( lptail, pszstr, cchstr ) + cchstr;
    lptail = (char*)memcpy( lptail, szpost, ccpost ) + ccpost;
      *lptail++ = '\0';

  // set capitalization scheme
    SetCapScheme( buforg, GetMinScheme( 0, GetWordCaps( wbinfo ), buforg ) );

  // set next form
    return output.Append();
  }

  static int  CreateFormList( makelist&   rfmake,
                              word16_t    tfoffs,
                              const char* szpref,
                              unsigned    ccpref );

  static int  CreateFormFlex( byte08_t* lpDest,
                              word16_t  tfOffs,
                              byte08_t  idform );

  void  actGetWdType( const byte08_t* lpStem,
                      const byte08_t* /* lpWord */,
                      SGramInfo*      /* grInfo */,
                      int             /* cgInfo */,
                      SScanPage&      rfScan )
  {
    word16_t  idStem = GetWord16( lpStem );       /* Номер класса                     */
    byte08_t* wdInfo = (byte08_t*)rfScan.lpData;  /* Извлечь указатель на переменную  */
    char*     lpInfo = (char*)GetInfoByID( idStem );

    *wdInfo = (byte08_t)GetWord16( lpInfo );
  }

  void  actGetClass( const byte08_t* lpStem,
                     const byte08_t* /* lpWord */,
                     SGramInfo*      /* grInfo */,
                     int             /* cgInfo */,
                     SScanPage&      rfScan )
  {
    word16_t* lpInfo = (word16_t*)rfScan.lpData;  /* Извлечь указатель на переменную  */

    *lpInfo = GetWord16( lpStem ) & 0x3fff;
  }

  void  actLemmatize( const byte08_t* lpStem,
                      const byte08_t* lpWord,
                      SGramInfo*      grInfo,
                      int             cgInfo,
                      SScanPage&      rfScan )
  {
    SLemmInfo&      rfLemm = *(SLemmInfo*)rfScan.lpData;
    word16_t        idstem = GetWord16( lpStem );     // Номер класса
    const byte08_t* lpinfo = (const byte08_t*)GetInfoByID( idstem );
    word16_t        wbinfo = GetWord16( lpinfo );
    word16_t        tfOffs = 0;

  // Проверить, нет ли переполнения выходных массивов и надо ли
  // вообще строить эту нормальную форму
    if ( rfLemm.failed != 0 )
      return;

  // Если указатель на массив нормальных форм ненулевой, требуется
  // построить текст нормальной формы
    if ( rfLemm.lpDest != NULL )
    {
      const byte08_t* psinfo = lpStem + sizeof(word16_t);
      int             ccstem = lpWord - (byte08_t*)rfScan.lpWord; // Длина основы
      char            szText[256];                                // Текст(ы) форм

    // Пропустить описание типа слова
      lpinfo += sizeof(word16_t);

    // Если есть окончания, то извлечь ссылку на них
      if ( wbinfo & wfFlexes )
        tfOffs = GetWord16( lpinfo );

    // Построить основу из словаря без изменяемой ее части
      memcpy( szText, rfScan.lpWord, ccstem );

    // Если есть отщепленный фрагмент, добавить его к слову
      if ( idstem & wfSuffix )
      {
        char *lpFrag = (char*)suffixes + *(((word16_t*)suffixes) + *psinfo++);
    
        memcpy( szText + ccstem, lpFrag + 1, lpFrag[0] );
        ccstem += lpFrag[0];
      }

    // Построить окончания, соответствующие нормальной форме слова.
    // Если слово не флективно, то окончание будет нулевым
      if ( tfOffs != 0 )
      {
        byte08_t  idform = GetNormalInfo( tfOffs );

      // check if form is built
        if ( CreateFormFlex( (byte08_t*)szText + ccstem, tfOffs, idform ) == 0 )
          strcpy( szText, rfScan.lpWord );
        ccstem = strlen( szText );
      }
        else
      szText[ccstem] = 0;

    // Слово может иметь текст в постпозиции
      if ( idstem & wfPostSt )
      {
        memcpy( szText + ccstem, psinfo + 1, *psinfo );
          ccstem += *lpinfo;
        szText[ccstem] = 0;
      }

    // Привести слово к минимальной возможной капитализации
      SetCapScheme( szText, GetMinScheme( rfScan.scheme >> 8, GetWordCaps( wbinfo ), szText ) );

    // Итак, нормальная форма построена. Теперь следует зарегистрировать
    // ее в выдаче.
      if ( rfLemm.ccDest > ccstem )
      {
        strcpy( rfLemm.lpDest, szText );
        rfLemm.lpDest += ccstem + 1;
        rfLemm.ccDest -= ccstem + 1;
      }
        else
      {
        rfLemm.failed = LEMMBUFF_FAILED;
        return;
      }
    }

  // Пропустить смещение грамматического класса и всю сопутствующую
  // информацию
    lpStem += sizeof(word16_t);
    if ( idstem & wfSuffix )
      lpStem++;
    if ( idstem & wfPostSt )
      lpStem += (byte08_t)lpStem[0] + 1;

  // Проверить, надо ли восстанавливать лексический номер
    if ( rfLemm.lpLids != NULL )
    {
      if ( rfLemm.clLids > 0 )
      {
        word32_t  curLID = 0;

        memcpy( &curLID, lpStem, ((idstem & 0x3000) >> 12) + 1 );
        Swap32( curLID );
        *rfLemm.lpLids++ = curLID;
        rfLemm.clLids--;
      }
        else
      {
        rfLemm.failed = LIDSBUFF_FAILED;
        return;
      }
    }

  // Проверить, надо ли восстанавливать грамматические описания
    if ( rfLemm.lpInfo != NULL )
    {
      if ( rfLemm.cbInfo >= cgInfo * sizeof(SGramInfo) + 1 )
      {
      // Восстановить количество грамматических описаний
        *rfLemm.lpInfo++ = (char)cgInfo;
        rfLemm.cbInfo--;

      // Восстановить собственно грамматические описания
        memcpy( rfLemm.lpInfo, grInfo, cgInfo * sizeof(SGramInfo) );
        rfLemm.lpInfo += cgInfo * sizeof(SGramInfo);
        rfLemm.cbInfo -= cgInfo * sizeof(SGramInfo);
      }
        else
      {
        rfLemm.failed = GRAMBUFF_FAILED;
        return;
      }
    }
    rfLemm.fBuilt++;
  }

  void  actBuildForm( const byte08_t* lpStem,
                      const byte08_t* lpWord,
                      SGramInfo*  /* lpInfo */,
                      int         /* cgInfo */,
                      SScanPage&      rfScan )
  {
    word16_t    idStem = GetWord16( lpStem );     // Номер класса
    SMakeInfo&  rfMake = *(SMakeInfo*)rfScan.lpData;
    char*       lpInfo = (char*)GetInfoByID( idStem );
    word16_t    wbInfo = GetWord16( lpInfo );
    word16_t    tfOffs = 0;
    int         ccStem = lpWord - (byte08_t*)rfScan.lpWord; // Длина основы
    byte08_t    szStem[64];
    byte08_t    szText[256];                                // Текст(ы) форм
    int         cTails;
    byte08_t*   lpTail = szText;

  // Проверить, нет ли переполнения выходных массивов и надо ли
  // вообще строить эту нормальную форму
    if ( rfMake.failed != 0 )
      return;

  // Пропустить описание типа слова и номер его класса
    lpStem += sizeof(word16_t);
    lpInfo += sizeof(word16_t);

  // Если есть окончания, то извлечь ссылку на них
    if ( wbInfo & wfFlexes )
      tfOffs = GetWord16( lpInfo );

  // Построить основу из словаря без изменяемой ее части
    memcpy( szStem, rfScan.lpWord, ccStem );

  // Если есть отщепленный фрагмент, добавить его к слову
    if ( idStem & wfSuffix )
    {
      char *lpFrag = (char*)suffixes + *(((word16_t*)suffixes) + *lpStem++);
  
      memcpy( szStem + ccStem, lpFrag + 1, lpFrag[0] );
      ccStem += lpFrag[0];
    }

  // Построить окончания, соответствующие нормальной форме слова.
  // Если слово не флективно, то окончание будет нулевым
    if ( tfOffs == 0 )
    {
      szText[0] = 0;
      cTails = 1;
    }
      else
    {
      if ( ( cTails = CreateFormFlex( szText, tfOffs, rfMake.idForm ) ) == 0 )
        return;
    }

  // Теперь - полный цикл по построенным окончаниям с образованием
  // цельных форм
    while ( cTails-- > 0 )
    {
    // Добавить текст окончания к построенной основе
      int   ccTail = strlen( (char*)lpTail );
      int   ccWord = ccStem + ccTail;

      memcpy( szStem + ccStem, lpTail, ccTail + 1 );

    // Слово может иметь текст в постпозиции
      if ( idStem & wfPostSt )
      {
        memcpy( szStem + ccStem + ccTail, lpStem + 1, lpStem[0] );
        ccWord += lpStem[0];
        szStem[ccWord] = 0;
      }

    // Итак, форма построена. Теперь следует зарегистрировать ее
      if ( rfMake.ccDest > ccWord )
      {
        unsigned  scheme = GetMinScheme( rfScan.scheme >> 8, GetWordCaps( wbInfo ),
          strcpy( rfMake.lpDest, (char*)szStem ) );

      // Привести слово к минимальной схеме капитализации
        SetCapScheme( rfMake.lpDest, scheme );

      // Пропустить слово
        rfMake.lpDest += ccWord + 1;
        rfMake.ccDest -= ccWord + 1;
        rfMake.fBuilt++;
      }
        else
      {
        rfMake.failed = LEMMBUFF_FAILED;
        return;
      }
      lpTail += ccTail + 1;
    }
  }

  void  actListForms( const byte08_t* lpdict,
                      const byte08_t* pwtail,
                      SGramInfo*    /* lpInfo */,
                      int           /* cgInfo */,
                      SScanPage&      rfScan )
  {
    SLemmInfo&  rflemm = *(SLemmInfo*)rfScan.lpData;
    makelist    mklist( rflemm.lpDest, rflemm.ccDest, rfScan.lpWord,
      (char*)pwtail - (char*)rfScan.lpWord );

  // check for faults
    if ( rflemm.failed != 0 )
      return;

  // initialize form builder
    mklist.InitStem( lpdict );

  // check if the word is NOT flective
    if ( mklist.tfoffs == 0 )
    {
    // register null string
      if ( mklist.Register( "", 0 ) != 0 )
        rflemm.failed = WORDBUFF_FAILED;
    }
      else
    if ( CreateFormList( mklist, mklist.tfoffs, NULL, 0 ) != 0 )
    {
      rflemm.failed = WORDBUFF_FAILED;
    }

  // fill output structure
    rflemm.lpDest =   mklist.output.lpbuff;
    rflemm.ccDest =   mklist.output.ccbuff;
    rflemm.fBuilt +=  mklist.output.nforms;
  }

  static int  CreateFormList( makelist&   rfmake,
                              word16_t    tfoffs,
                              const char* szpref,
                              unsigned    ccpref )
  {
    const byte08_t* ftable = fxTables + tfoffs;
    int             fcount = *ftable++;
    int             nerror;

  // scan table itself
    while ( fcount-- > 0 )
    {
      word16_t        stoffs = GetWord16( ftable + 1 );
      const byte08_t* tftext = (const byte08_t*)fxString + stoffs;
      char            sztail[64];
      char*           lptail = sztail;

    // skip to the next item
      ftable += sizeof(word16_t) + sizeof(byte08_t);

    // create tail text
      lptail = (char*)memcpy( lptail, szpref, ccpref ) + ccpref;
      lptail = (char*)memcpy( lptail, tftext + 1, *tftext ) + *tftext;

    // else nested reference is 'absent' because optional is already precessed
      if ( (nerror = rfmake.Register( sztail, lptail - sztail )) != 0 )
        return nerror;
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
  static int  CreateFormFlex( byte08_t* lpDest,
                              word16_t  tfOffs,
                              byte08_t  idform )
  {
    const byte08_t* ptable = fxTables + tfOffs;
    int             nitems = *ptable++;
    int             nforms = 0;

    while ( nitems-- > 0 )
    {
      byte08_t        formid = *ptable++;
      word16_t        stoffs = GetWord16( ptable );
      const byte08_t* sztail = (const byte08_t*)fxString + stoffs;
        ptable += sizeof(word16_t);
      int             cctail;

    // check if form is found
      if ( formid != idform )
        continue;

    // create output form
      for ( cctail = *sztail++; cctail > 0; --cctail )
        *lpDest++ = *sztail++;
      *lpDest++ = '\0';
      ++nforms;
    }
    return nforms;
  }

}
