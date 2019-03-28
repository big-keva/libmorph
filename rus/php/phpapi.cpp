# include <php.h>
# include <ext/standard/info.h>
# include "../include/mlma1049.h"
# include <moonycode/codes.h>

/* All the functions that will be exported (available) must be declared */
PHP_MINIT_FUNCTION    ( morphrus );
PHP_MSHUTDOWN_FUNCTION( morphrus );
PHP_MINFO_FUNCTION    ( morphrus );

PHP_FUNCTION         ( mlmarucheckword );
PHP_FUNCTION         ( mlmarulemmatize );
PHP_FUNCTION         ( mlmarugetwdinfo );
PHP_FUNCTION         ( mlmarubuildform );
PHP_FUNCTION         ( mlmarucheckhelp );

/* function list so that the Zend engine will know what’s here */
static zend_function_entry php_morphrus_functions[] =
{
  PHP_FE( mlmarucheckword, NULL )
  PHP_FE( mlmarulemmatize, NULL )
  PHP_FE( mlmarugetwdinfo, NULL )
  PHP_FE( mlmarubuildform, NULL )
  PHP_FE( mlmarucheckhelp, NULL )
};

/* module information */
zend_module_entry morphrus_module_entry =
{
  STANDARD_MODULE_HEADER,
  "php_morphrus",
  php_morphrus_functions,
    PHP_MINIT     ( morphrus ),
    PHP_MSHUTDOWN ( morphrus ),
    NULL,
    NULL,
    PHP_MINFO     ( morphrus ),
  NO_VERSION_YET,
  STANDARD_MODULE_PROPERTIES
};

#if COMPILE_DL_PHPMORPHRUS
  ZEND_GET_MODULE( morphrus )
#endif

# define  mlma_no_string  0x00010000
# define  mlma_no_grinfo  0x00040000

IMlmaMb*  mlma_api = NULL;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

PHP_MINIT_FUNCTION    ( morphrus )
{
  REGISTER_LONG_CONSTANT( "sfIgnoreCapitals", sfIgnoreCapitals, CONST_PERSISTENT | CONST_CS );

  REGISTER_LONG_CONSTANT( "mlma_no_string",   mlma_no_string,   CONST_PERSISTENT | CONST_CS );
  REGISTER_LONG_CONSTANT( "mlma_no_grinfo",   mlma_no_grinfo,   CONST_PERSISTENT | CONST_CS );

  mlmaruLoadMbAPI( &mlma_api );

  return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION( morphrus )
{
  mlma_api->Detach();

  return SUCCESS;
}

PHP_MINFO_FUNCTION    ( morphrus )
{
  php_info_print_table_start();
  php_info_print_table_row( 2, "php_morphrus Extension", "Andrew Kovalenko aka Keva" );
  php_info_print_table_end();
}

/******************************************************************************/
/*  short MLMA_API EXPORT mlmaruCheckWord( const char*    lpword,             */
/*                                         unsigned short dwsets );           */
/******************************************************************************/
PHP_FUNCTION( mlmarucheckword )
{
  char*   pszstr;
  int     cchstr;
  long    dwsets;
  char    winstr[0x100];

/*  get string to be checked  */
  if ( zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "sl",
      &pszstr,
      &cchstr,
      &dwsets ) == FAILURE)
  {
      RETURN_FALSE;
  }

/*  check if is utf-8 string  */
  if ( codepages::utf8::detect( pszstr, cchstr ) )
  {
    cchstr = codepages::mbcstombcs( codepages::codepage_1251, winstr, sizeof(winstr),
                                    codepages::codepage_utf8, pszstr, cchstr );
    if ( cchstr == (size_t)-1 )
    {
      RETURN_FALSE;
    }
      else
    pszstr = winstr;
  }

/*  return check result       */
  if ( mlma_api->CheckWord( pszstr, cchstr, (unsigned short)dwsets ) > 0 )
  {
      RETURN_TRUE;
  }
    else
  {
      RETURN_FALSE;
  }
}

/******************************************************************************/
/*  short MLMA_API EXPORT mlmaruLemmatize( const char*    lpword,             */
/*                                         unsigned short dwsets,             */
/*                                         char*          lpLemm,             */
/*                                         lexeme_t*      lpLIDs,             */
/*                                         char*          lpGram,             */
/*                                         unsigned short ccLemm,             */
/*                                         unsigned short cdwLID,             */
/*                                         unsigned short cbGram );           */
/*========================== PHP representation ==============================*/
/*  function mlmaruLemmatize( string, dwsets )                                */
/*  returns the array of lemmatization results                                */
/******************************************************************************/
PHP_FUNCTION( mlmarulemmatize )
{
  char        winstr[0x100];
  char        buffer[0x100];
  SLemmInfoA  lemmas[0x20];
  SGramInfo   ginfos[0x100];
  char*       pszstr;
  int         cchstr;
  long        dwsets;
  int         lcount;
  int         lindex;

/*  get string to be checked  */
  if ( zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "sl",
      &pszstr,
      &cchstr,
      &dwsets ) == FAILURE)
  {
      RETURN_FALSE;
  }

/*  check lemmatization flags */
  char*        pnorms = (dwsets & mlma_no_string) != 0 ? NULL : buffer;
  size_t       cnorms = (dwsets & mlma_no_string) != 0 ? 0x00 : sizeof(buffer);
  SGramInfo*   pgrams = (dwsets & mlma_no_grinfo) != 0 ? NULL : ginfos;
  size_t       cgrams = (dwsets & mlma_no_grinfo) != 0 ? 0x00 : sizeof(ginfos) / sizeof(ginfos[0]);

/*  check if is utf-8 string  */
  if ( codepages::utf8::detect( pszstr, cchstr ) )
  {
    cchstr = codepages::mbcstombcs( codepages::codepage_1251, winstr, sizeof(winstr),
                                    codepages::codepage_utf8, pszstr, cchstr );
    if ( cchstr == (size_t)-1 )
    {
      RETURN_FALSE;
    }
      else
    pszstr = winstr;
  }

/*  check lemmas count        */
  if ( (lcount = mlma_api->Lemmatize(
    pszstr, cchstr,
    lemmas, sizeof(lemmas) / sizeof(lemmas[0]),
    pnorms, cnorms,
    pgrams, cgrams, (unsigned short)dwsets )) <= 0 )
  {
    RETURN_FALSE;
  }

/*  build the output          */
  array_init( return_value );

  for ( lindex = 0; lindex < lcount; ++lindex )
  {
    zval* plemma;

  /*  register lemma description  */
    MAKE_STD_ZVAL ( plemma );
      array_init  ( plemma );

    add_index_zval( return_value, lindex, plemma );

  /*  check if string is defined  */
    if ( pnorms != NULL )
    {
    /*  if the string is utf8-decoded, encode the result  */
      if ( pszstr == winstr )
      {
        encode_utf8( winstr, pnorms, -1 );

        add_index_string( plemma, 0x0000, winstr, 1 );
      }
        else
    /*  set lemmatization string                          */
      {
        add_index_string( plemma, 0x0000, pnorms, 1 );
      }

      while ( *pnorms++ != '\0' ) (void)NULL;
    }

  /*  check if lexid is defined   */
    add_index_long( plemma, 0x0001, lemmas[lindex].nlexid );

  /*  check grammardescription    */
    if ( pgrams != NULL )
    {
      SGramInfo*  gr_beg = lemmas[lindex].pgrams;
      zval*       pgrams;

    /*  register lemma description  */
      MAKE_STD_ZVAL ( pgrams );
        array_init  ( pgrams );

      add_index_zval( plemma, 0x0002, pgrams );

      for ( size_t findex = 0; findex != lemmas[lindex].ngrams; ++findex, ++gr_beg )
      {
        zval*   galloc;

        MAKE_STD_ZVAL ( galloc );
          array_init  ( galloc );

        add_index_zval( pgrams, findex, galloc );

        add_index_long( galloc, 0x0000, gr_beg->wdInfo );
        add_index_long( galloc, 0x0001, gr_beg->grInfo );
        add_index_long( galloc, 0x0002, gr_beg->bFlags );
        add_index_long( galloc, 0x0003, gr_beg->idForm );
      }
    }
  }
}

/******************************************************************************/
/*  short MLMA_API EXPORT mlmaruGetWdInfo( lexeme_t       nLexID );           */
/*========================== PHP representation ==============================*/
/*  function mlmaruGetWdInfo( nlexid )                                        */
/*  returns the word information value                                        */
/******************************************************************************/
PHP_FUNCTION( mlmarugetwdinfo )
{
  lexeme_t      nlexid;
  unsigned char wdinfo;

  if ( zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l", &nlexid ) == FAILURE)
  {
    WRONG_PARAM_COUNT;
  }

  if ( !mlma_api->GetWdInfo( &wdinfo, nlexid ) )
  {
    RETURN_FALSE;
  }

  RETURN_LONG( wdinfo );
}

/******************************************************************************/
/*  short MLMA_API EXPORT mlmaruBuildForm( const char*    lpword,             */
/*                                         lexeme_t       nLexID,             */
/*                                         unsigned short dwsets,             */
/*                                         unsigned char  idForm,             */
/*                                         char*          lpDest,             */
/*                                         unsigned short ccDest );           */
/*========================== PHP representation ==============================*/
/*  function mlmaruBuildForm( string|nlexid, idform, sample )                 */
/*  returns the array of lemmatization results                                */
/******************************************************************************/
PHP_FUNCTION( mlmarubuildform )
{
  char      winstr[0x100];
  int       useutf = 0;
  zval**    zvword;
  zval**    zvform;
  zval**    zvsmpl = NULL;
  char      szform[0x100];
  char*     szlemm;
  char*     lpform;
  lexeme_t  nlexid;
  long      idform;
  int       fcount;

/*  check parameter count       */
  if ( ZEND_NUM_ARGS() < 2 || zend_get_parameters_ex( ZEND_NUM_ARGS(),
    &zvword, &zvform, &zvsmpl ) == FAILURE )
  {
    WRONG_PARAM_COUNT;
  }

/*  check nlexid defined        */
  convert_to_string_ex( zvword );

  if ( Z_STRLEN_PP( zvword ) <= 0 )
  {
    RETURN_FALSE;
  }

  szlemm = Z_STRVAL_PP( zvword );

  if ( (nlexid = strtoul( szlemm, &lpform, 0 )) != 0 && *lpform == '\0' )
  {
    szlemm = NULL;
  }

/*  check if source is string   */
  if ( szlemm != NULL )
  {
    unsigned  cchstr = strlen( szlemm );

  /*  check if is utf-8 string  */
    if ( detect_utf8( szlemm, cchstr ) )
    {
      if ( decode_utf8( winstr, szlemm, cchstr ) != 0 )
      {
        RETURN_FALSE;
      }
      szlemm = winstr;
      useutf = 1;
    }
  }

/*  check if sample is defined  */
  if ( szlemm == NULL && zvsmpl != NULL )
  {
    convert_to_string_ex( zvsmpl );

    useutf = detect_utf8( Z_STRVAL_PP( zvsmpl ), Z_STRLEN_PP( zvsmpl ) );
  }

/*  get form id                 */
  convert_to_long_ex( zvform );

  if ( (idform = Z_LVAL_PP( zvform )) < 0 )
    RETURN_FALSE;

  if ( idform >= 256 )
    RETURN_FALSE;

/*  build word forms          */
  RETURN_FALSE;
# if 0
  if ( (fcount = mlmaruBuildForm( szlemm, nlexid, 0, (unsigned char)idform,
    szform, sizeof(szform) )) <= 0 )
  {
    RETURN_FALSE;
  }

/*  add the results           */
  array_init( return_value );

  for ( lpform = szform, findex = 0; findex < fcount; ++findex )
  {
    if ( useutf )
    {
      encode_utf8( winstr, lpform, -1 );

      add_index_string( return_value, findex, winstr, 1 );
    }
      else
    {
      add_index_string( return_value, findex, lpform, 1 );
    }

    while ( *lpform++ != '\0' ) (void)NULL;
  }
# endif
}

/******************************************************************************/
/*  short MLMA_API EXPORT mlmaruCheckHelp( const char* lpword,                */
/*                                         char*       lpList );              */
/*========================== PHP representation ==============================*/
/*  function mlmaruCheckHelp( szmask )                                        */
/******************************************************************************/
PHP_FUNCTION( mlmarucheckhelp )
{
  char  szhelp[0x80];
  char* szmask;
  int   ccmask;
  int   cchelp;

/*  get parameter string      */
  if ( zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "s",
      &szmask,
      &ccmask ) == FAILURE)
  {
      RETURN_FALSE;
  }

/*  create checker help       */
  if ( (cchelp = mlma_api->CheckHelp( szhelp, sizeof(szhelp), szmask, ccmask )) <= 0 )
  {
      RETURN_FALSE;
  }

  RETURN_STRINGL( szhelp, cchelp, 1 );
}

#pragma GCC diagnostic pop

