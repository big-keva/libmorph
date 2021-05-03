# include "mlmadefs.h"
# include "lemmatiz.h"
# include "wildscan.h"
# include "scandict.h"
# include "capsheme.h"

# if !defined( _WIN32_WCE )
  # define  CATCH_ALL         try {
  # define  ON_ERRORS( code ) } catch ( ... ) { return (code); }
# else
  # define  CATCH_ALL
  # define  ON_ERRORS( code )
# endif  // ! _WIN32_WCE

namespace __libmorpheng__
{
  extern unsigned         page_table_size;
  extern unsigned char*   page_table_data[];

  extern unsigned 	lids_table_size;
  extern SLidsRef       lids_table_data[];
/*  extern struct
  {
    word32_t        minvalue;
    word32_t        maxvalue;
    unsigned char*  pagedata;
  } lids_table_data[]; */

  CPageMan  glossary( page_table_data, page_table_size );
  CLIDsMan  lidsdict( (SLidsRef*)lids_table_data, lids_table_size );
}

using namespace __libmorpheng__;

short MLMA_API EXPORT mlmaenCheckWord( const char* lpWord, unsigned short options )
{
  SScanPage scan;
  char      word[256];
  unsigned  size;

  CATCH_ALL
    if ( (size = GetCapScheme( lpWord, word, sizeof(word) - 1, scan.scheme ))
      == (unsigned)-1 )
        return WORDBUFF_FAILED;

    scan.lpWord = word;
    scan.nFlags = options | sfStopAfterFirst;
    scan.lpData = NULL;

    return ScanDictionary( glossary, word, size, scan, NULL );
  ON_ERRORS( -1 )
}

short MLMA_API EXPORT mlmaenLemmatize( const char*    lpWord,
                                       word16_t       dwsets,
                                       char*          lpLemm,
                                       lexeme_t*      lpLIDs,
                                       char*          lpGram,
                                       word16_t       ccLemm,
                                       word16_t       cdwLID,
                                       word16_t       cbGram )
{
  SScanPage scan;
  SLemmInfo lemm;
  char      word[256];
  unsigned  size;

  CATCH_ALL
    if ( (size = GetCapScheme( lpWord, word, sizeof(word) - 1, scan.scheme ))
      == (unsigned)-1 )
        return WORDBUFF_FAILED;

    lemm.lpDest = lpLemm;   // Указатель на массив нормальных форм слова
    lemm.ccDest = ccLemm;   // Количество символов, влезающее в lpDest
    lemm.lpLids = lpLIDs;   // Указатель на массив лексических номеров
    lemm.clLids = cdwLID;   // Количество элементов, влезающее в lpLids
    lemm.lpInfo = lpGram;   // Указатель на массив описаний отождествившихся форм
    lemm.cbInfo = cbGram;   // Количество байт, влезающих в lpInfo
    lemm.failed = 0;        // Номер слетевшего по переполнению параметра
    lemm.fBuilt = 0;        // Количество построенных нормальных форм

    scan.lpWord = word;
    scan.nFlags = dwsets;
    scan.lpData = &lemm;

    if ( !ScanDictionary( glossary, word, size, scan, actLemmatize ) )
      return 0;
    if ( lemm.failed != 0 ) return lemm.failed;
    return lemm.fBuilt;
  ON_ERRORS( -1 )
}

short   MLMA_API  EXPORT  mlmaenBuildForm( const char*    pszWord,
                                           lexeme_t       dwLexId,
                                           word16_t       options,
                                           unsigned char  nFormId,
                                           char*          pszDest,
                                           word16_t       nccDest )
{
  CATCH_ALL
    SScanPage scan;
    SMakeInfo lemm;

    lemm.lpDest = pszDest;
    lemm.ccDest = nccDest;
    lemm.idForm = nFormId;
    lemm.wrdLID = dwLexId;
    lemm.failed = 0;
    lemm.fBuilt = 0;

    scan.nFlags = options;
    scan.lpData = &lemm;
    scan.scheme = 0;

    if ( pszWord != NULL )
    {
      char      word[256];
      unsigned  size;

      if ( (size = GetCapScheme( pszWord, word, sizeof(word) - 1, scan.scheme ))
        == (unsigned)-1 )
          return WORDBUFF_FAILED;

      scan.lpWord = word;

      if ( !ScanDictionary( glossary, word, size, scan, actBuildForm ) )
        return 0;
    }
      else
    {
    // Оригинальная форма слова не задана, следует применять модификацию
    // алгоритма, "прыгающую" по словарю идентификаторов лексем сразу в
    // нужную точку на странице.
    
      scan.lpWord = NULL;

      if ( !DirectPageJump( lidsdict, glossary, dwLexId, scan, actBuildForm ) )
        return 0;
    }
    if ( lemm.failed != 0 )
      return lemm.failed;
    return lemm.fBuilt;
  ON_ERRORS( -1 )
}

short MLMA_API EXPORT mlmaenEnumWords( TEnumWords enumproc, void *lpv )
{
  unsigned        idPage;
  word32_t        idNode;
  SLidsRef*       lpPage;
  unsigned char*  lpData;

  CATCH_ALL
    for ( idPage = 0; idPage < lidsdict.GetCount(); idPage++ )
    {
      word32_t minLid;
      word32_t maxLid;

      lpPage = lidsdict[idPage];
      lpData = lpPage->pagedata;
      minLid = GetWord32( &lpPage->minvalue );
      maxLid = GetWord32( &lpPage->maxvalue );

      for ( idNode = 0; idNode <= maxLid - minLid; idNode++ )
        if ( lpData[idNode * 3] != 0 )
          if ( !enumproc( minLid + idNode, lpv ) )
            return 0;
    }
    return 1;
  ON_ERRORS( -1 )
}

short MLMA_API EXPORT mlmaenCheckHelp( const char*  lpWord,
                                       char*        lpList )
{
  CATCH_ALL
    int   size = strlen( lpWord );
    char  word[256];

    if ( size >= (int)sizeof(word) )
      return 0;
    SetLowerCase( (unsigned char*)memcpy( word, lpWord, size + 1 ) );
    return (short)WildScanDictionary( glossary, (byte08_t*)word, (byte08_t*)lpList );
  ON_ERRORS( -1 )
}

short MLMA_API EXPORT mlmaenListForms( lexeme_t       nlexid,
                                       char*          lpbuff,
                                       unsigned       ccbuff )
{
  CATCH_ALL
    SLemmInfo lemm;
    SScanPage scan;

    lemm.lpDest = lpbuff;
    lemm.ccDest = ccbuff;
    lemm.failed = 0;
    lemm.fBuilt = 0;
    scan.nFlags = 0;
    scan.lpData = (void*)&lemm;
    scan.scheme = 0;

    if ( !DirectPageJump( lidsdict, glossary, nlexid, scan, actListForms ) )
      return 0;
      
    if ( lemm.failed != 0 )
      return lemm.failed;
      
    return lemm.fBuilt;
  ON_ERRORS( -1 )
}

short MLMA_API EXPORT mlmaenGetWordInfo( lexeme_t       nLexId,
                                         unsigned char* wdinfo )
{
  CATCH_ALL
    SScanPage scan;

    scan.nFlags = 0;
    scan.lpData = (void*)wdinfo;
    scan.scheme = 0;

    return DirectPageJump( lidsdict, glossary, nLexId, scan, actGetWdType ) ? 1 : 0;
  ON_ERRORS( -1 )
}

short MLMA_API EXPORT mlmaenGetClassRef( lexeme_t         nLexId,
                                         unsigned short*  rclass )
{
  CATCH_ALL
    SScanPage scan;

    scan.nFlags = 0;
    scan.lpData = (void*)rclass;
    scan.scheme = 0;

    return DirectPageJump( lidsdict, glossary, nLexId, scan, actGetClass ) ? 1 : 0;
  ON_ERRORS( -1 )
}

//
// return word type information by the class
//
short MLMA_API EXPORT mlmaenGetTypeInfo( unsigned short   iclass,
                                         unsigned char*   lpinfo )
{
  CATCH_ALL
    char* pclass = (char*)GetInfoByID( iclass );

    if ( lpinfo != NULL ) *lpinfo = (byte08_t)GetWord16( pclass );
      else return -1;
     return 1;  
  ON_ERRORS( -1 )
}
