# include <php.h>
# include <ext/standard/info.h>
# include "../../include/mlma1033.h"
# include "../../../unicode/utfapi.h"

/* All the functions that will be exported (available) must be declared */
PHP_MINIT_FUNCTION    ( morpheng );
PHP_MSHUTDOWN_FUNCTION( morpheng );
PHP_MINFO_FUNCTION    ( morpheng );

ZEND_FUNCTION         ( mlmaencheckword );
ZEND_FUNCTION         ( mlmaenlemmatize );
ZEND_FUNCTION         ( mlmaenbuildform );
ZEND_FUNCTION         ( mlmaencheckhelp );

/* function list so that the Zend engine will know what’s here */
static zend_function_entry php_morpheng_functions[] =
{
  ZEND_FE( mlmaencheckword, NULL )
  ZEND_FE( mlmaenlemmatize, NULL )
  ZEND_FE( mlmaenbuildform, NULL )
  ZEND_FE( mlmaencheckhelp, NULL )

  {NULL, NULL, NULL}
};

/* module information */
zend_module_entry morpheng_module_entry =
{
  STANDARD_MODULE_HEADER,
  "php_morpheng",
  php_morpheng_functions,
    PHP_MINIT     ( morpheng ),
    PHP_MSHUTDOWN ( morpheng ),
    NULL,
    NULL,
    PHP_MINFO     ( morpheng ),
  NO_VERSION_YET,
  STANDARD_MODULE_PROPERTIES
};

#if COMPILE_DL_PHPMORPHENG
  ZEND_GET_MODULE( morpheng )
#endif

# define  mlma_no_string  0x00010000
# define  mlma_no_lexeme  0x00020000
# define  mlma_no_grinfo  0x00040000

PHP_MINIT_FUNCTION    ( morpheng )
{
  REGISTER_LONG_CONSTANT( "sfIgnoreCapitals", sfIgnoreCapitals, CONST_PERSISTENT | CONST_CS );

  REGISTER_LONG_CONSTANT( "mlma_no_string",   mlma_no_string,   CONST_PERSISTENT | CONST_CS );
  REGISTER_LONG_CONSTANT( "mlma_no_lexeme",   mlma_no_lexeme,   CONST_PERSISTENT | CONST_CS );
  REGISTER_LONG_CONSTANT( "mlma_no_grinfo",   mlma_no_grinfo,   CONST_PERSISTENT | CONST_CS );

  return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION( morpheng )
{
  return SUCCESS;
}

PHP_MINFO_FUNCTION    ( morpheng )
{
  php_info_print_table_start();
  php_info_print_table_row( 2, "php_morpheng Extension", "Andrew Kovalenko aka Keva" );
  php_info_print_table_end();
}

/******************************************************************************/
/*  short MLMA_API EXPORT mlmaenCheckWord( const char*    lpword,             */
/*                                         unsigned short dwsets );           */
/******************************************************************************/
PHP_FUNCTION( mlmaencheckword )
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

/*  check for overflow        */
  if ( cchstr >= sizeof(winstr) )
  {
    RETURN_FALSE;
  }

/*  check if is utf-8 string  */
  if ( detect_utf8( pszstr, cchstr ) )
  {
    if ( decode_utf8( winstr, pszstr, cchstr ) != 0 )
    {
      RETURN_FALSE;
    }
    pszstr = winstr;
  }

/*  return check result       */
  if ( mlmaenCheckWord( pszstr, (unsigned short)dwsets ) > 0 )
  {
      RETURN_TRUE;
  }
    else
  {
      RETURN_FALSE;
  }
}

/******************************************************************************/
/*  short MLMA_API EXPORT mlmaenLemmatize( const char*    lpword,             */
/*                                         unsigned short dwsets,             */
/*                                         char*          lpLemm,             */
/*                                         lexeme_t*      lpLIDs,             */
/*                                         char*          lpGram,             */
/*                                         unsigned short ccLemm,             */
/*                                         unsigned short cdwLID,             */
/*                                         unsigned short cbGram );           */
/*========================== PHP representation ==============================*/
/*  function mlmaenLemmatize( string, dwsets )                                */
/*  returns the array of lemmatization results                                */
/******************************************************************************/
PHP_FUNCTION( mlmaenlemmatize )
{
  char      winstr[0x100];
  char      szlemm[0x100];
  lexeme_t  dwlids[0x20];
  char      szgram[0x200];
  char*     lplemm;
  lexeme_t* lplids;
  char*     lpgram;
  char*     pszstr;
  int       cchstr;
  long      dwsets;
  int       lcount;
  int       lindex;

/*  get string to be checked  */
  if ( zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "sl",
      &pszstr,
      &cchstr,
      &dwsets ) == FAILURE)
  {
      RETURN_FALSE;
  }

/*  check lemmatization flags */
  lplemm = (dwsets & mlma_no_string) != 0 ? NULL : szlemm;
  lplids = (dwsets & mlma_no_lexeme) != 0 ? NULL : dwlids;
  lpgram = (dwsets & mlma_no_grinfo) != 0 ? NULL : szgram;

/*  check for overflow        */
  if ( cchstr >= sizeof(winstr) )
  {
    RETURN_FALSE;
  }

/*  check if is utf-8 string  */
  if ( detect_utf8( pszstr, cchstr ) )
  {
    if ( decode_utf8( winstr, pszstr, cchstr ) != 0 )
    {
      RETURN_FALSE;
    }
    pszstr = winstr;
  }

/*  check lemmas count        */
  if ( (lcount = mlmaenLemmatize( pszstr, (unsigned short)dwsets,
    lplemm,
    lplids,
    lpgram,
    (unsigned short)(lplemm != NULL ? sizeof(szlemm) / sizeof(szlemm[0]) : 0),
    (unsigned short)(lplids != NULL ? sizeof(dwlids) / sizeof(dwlids[0]) : 0),
    (unsigned short)(lpgram != NULL ? sizeof(szgram) / sizeof(szgram[0]) : 0) )) <= 0 )
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
    if ( lplemm != NULL )
    {
    /*  if the string is utf8-decoded, encode the result  */
      if ( pszstr == winstr )
      {
        encode_utf8( winstr, lplemm );

        add_index_string( plemma, 0x0000, winstr, 1 );
      }
        else
    /*  set lemmatization string                          */
      {
        add_index_string( plemma, 0x0000, lplemm, 1 );
      }

      while ( *lplemm++ != '\0' ) (void)NULL;
    }

  /*  check if lexid is defined   */
    if ( lplids != NULL )
      add_index_long( plemma, 0x0001, *lplids++ );

  /*  check grammardescription    */
    if ( lpgram != NULL )
    {
      SGramInfo*  pginfo = (SGramInfo*)(lpgram + 1);
      int         ngrams = *lpgram;
      zval*       pgrams;
      int         findex;

    /*  fill grammatical descriptions */
      lpgram += ngrams * sizeof(SGramInfo) + 1;

    /*  register lemma description  */
      MAKE_STD_ZVAL ( pgrams );
        array_init  ( pgrams );

      add_index_zval( plemma, 0x0002, pgrams );

      for ( findex = 0; findex < ngrams; ++findex )
      {
        zval*   galloc;

        MAKE_STD_ZVAL ( galloc );
          array_init  ( galloc );

        add_index_zval( pgrams, findex, galloc );

        add_index_long( galloc, 0x0000, pginfo[findex].wInfo );
        add_index_long( galloc, 0x0001, pginfo[findex].gInfo );
        add_index_long( galloc, 0x0002, pginfo[findex].other );
        add_index_long( galloc, 0x0003, pginfo[findex].iForm );
      }
    }
  }
}

/******************************************************************************/
/*  short MLMA_API EXPORT mlmaenBuildForm( const char*    lpword,             */
/*                                         lexeme_t       nLexID,             */
/*                                         unsigned short dwsets,             */
/*                                         unsigned char  idForm,             */
/*                                         char*          lpDest,             */
/*                                         unsigned short ccDest );           */
/*========================== PHP representation ==============================*/
/*  function mlmaenBuildForm( string|nlexid, idform, sample )                 */
/*  returns the array of lemmatization results                                */
/******************************************************************************/
PHP_FUNCTION( mlmaenbuildform )
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
  int       findex;

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
  if ( (fcount = mlmaenBuildForm( szlemm, nlexid, 0, (unsigned char)idform,
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
      encode_utf8( winstr, lpform );

      add_index_string( return_value, findex, winstr, 1 );
    }
      else
    {
      add_index_string( return_value, findex, lpform, 1 );
    }

    while ( *lpform++ != '\0' ) (void)NULL;
  }
}

/******************************************************************************/
/*  short MLMA_API EXPORT mlmaenCheckHelp( const char* lpword,                */
/*                                         char*       lpList );              */
/*========================== PHP representation ==============================*/
/*  function mlmaenCheckHelp( szmask )                                        */
/******************************************************************************/
PHP_FUNCTION( mlmaencheckhelp )
{
  char  szhelp[0x80];
  char* szmask;
  int   ccmask;

/*  get parameter string      */
  if ( zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "s",
      &szmask,
      &ccmask ) == FAILURE)
  {
      RETURN_FALSE;
  }

/*  create checker help       */
  if ( (ccmask = mlmaenCheckHelp( szmask, szhelp )) <= 0 )
  {
      RETURN_FALSE;
  }

  RETURN_STRINGL( szhelp, ccmask, 1 );
}
