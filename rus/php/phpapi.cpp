# include <php.h>
# include <ext/standard/info.h>
# include "../include/mlma1049.h"

# define  mlma_no_string  0x00010000
# define  mlma_no_grinfo  0x00040000

/* C++/object-style API */
zend_class_entry*     morph_rus_class_entry;

//
// \morph\rus object implementation
//
struct morph_rus_object
{
  IMlmaMb*    mlma;
  zend_object zend;

public:     // handlers
  static  zend_object_handlers  object_handlers;

public:     // helpers
  static inline morph_rus_object* get( zend_object *obj )
    {
      return (morph_rus_object*)((char*)(obj) - XtOffsetOf(morph_rus_object, zend));
    }

public:     // construct/destruct
  static  auto  object_new( zend_class_entry *ce TSRMLS_DC ) -> zend_object*
    {
      auto  palloc = (morph_rus_object*)ecalloc( 1, sizeof(morph_rus_object) + zend_object_properties_size(ce) );

      zend_object_std_init  ( &palloc->zend, ce TSRMLS_CC );
      object_properties_init( &palloc->zend, ce );

      palloc->zend.handlers = &object_handlers;
      return &palloc->zend;
    }
  static  void  object_destroy( zend_object *object )
    {
      auto  intern = get( object );

      // Call __destruct() from user-land.
      zend_objects_destroy_object( object );
    }
  static  void  object_free( zend_object *object )
    {
      auto  intern = get( object );

      intern->mlma->Detach();
      zend_object_std_dtor( object ); 
    }
};

zend_object_handlers  morph_rus_object::object_handlers;

#define Z_GETOBJ_P( zv )  morph_rus_object::get( Z_OBJ_P( (zv) ) )

/*
  class Rus
  {
    ...
    public function __construct( $codepage = null );
  }
*/
PHP_METHOD(Rus, __construct)
{
  auto  zendId = getThis();
  auto  morpho = Z_GETOBJ_P( zendId );
  auto  argcnt = ZEND_NUM_ARGS();
  auto  c_page = (const char*)"Windows-1251";
  zval  z_page;

  if ( argcnt != 0 )
  {
    if ( argcnt != 1 || zend_get_parameters_array_ex( argcnt TSRMLS_CC, &z_page ) == FAILURE )
      WRONG_PARAM_COUNT;

    convert_to_string_ex( &z_page );
      c_page = Z_STRVAL_P( &z_page );
  }

  if ( morpho != nullptr && mlmaruLoadCpAPI( &morpho->mlma, c_page ) != 0 )
    WRONG_PARAM_COUNT;
}

/*
  class Rus
  {
    ...
    public function SetLoCase( $str, $len = 0 ) -> string;
  }
*/
PHP_METHOD(Rus, SetLoCase )
{
  auto  zendId = getThis();
  auto  morpho = Z_GETOBJ_P( zendId );
  auto  argcnt = ZEND_NUM_ARGS();
  char* in_str;
  long  cc_str;

  if ( argcnt == 2 )
  {
    long  length;

    if ( zend_parse_parameters( argcnt TSRMLS_CC, "sl", &in_str, &cc_str, &length ) != SUCCESS )
      RETURN_FALSE;
    cc_str = cc_str <= length ? cc_str : length;
  }
    else
  if ( argcnt == 1 )
  {
    if ( zend_parse_parameters( argcnt TSRMLS_CC, "s", &in_str, &cc_str ) != SUCCESS )
      RETURN_FALSE;
  }
    else
  {
    WRONG_PARAM_COUNT;
  }

  ZVAL_STRINGL( return_value, in_str, cc_str );

  morpho->mlma->SetLoCase( Z_STRVAL_P( return_value ), Z_STRLEN_P( return_value ) );
}

PHP_METHOD( Rus, SetUpCase )
{
  auto  zendId = getThis();
  auto  morpho = Z_GETOBJ_P( zendId );
  auto  argcnt = ZEND_NUM_ARGS();
  char* in_str;
  long  cc_str;

  if ( argcnt == 2 )
  {
    long  length;

    if ( zend_parse_parameters( argcnt TSRMLS_CC, "sl", &in_str, &cc_str, &length ) != SUCCESS )
      RETURN_FALSE;
    cc_str = cc_str <= length ? cc_str : length;
  }
    else
  if ( argcnt == 1 )
  {
    if ( zend_parse_parameters( argcnt TSRMLS_CC, "s", &in_str, &cc_str ) != SUCCESS )
      RETURN_FALSE;
  }
    else
  {
    WRONG_PARAM_COUNT;
  }

  ZVAL_STRINGL( return_value, in_str, cc_str );

  morpho->mlma->SetUpCase( Z_STRVAL_P( return_value ), Z_STRLEN_P( return_value ) );
}

//
// morph\Rus -> CheckWord( $string [, $options] )
//
PHP_METHOD( Rus, CheckWord )
{
  auto  zendId = getThis();
  auto  morpho = Z_GETOBJ_P( zendId );
  auto  argcnt = ZEND_NUM_ARGS();
  zval  argset[2];
  char* in_str;
  long  cc_str;
  long  dwsets = 0;

  if ( argcnt == 1 )
  {
    if ( zend_get_parameters_array_ex( argcnt TSRMLS_CC, argset ) == FAILURE )
      WRONG_PARAM_COUNT;
  }
    else
  if ( argcnt == 2 )
  {
    if ( zend_get_parameters_array_ex( argcnt TSRMLS_CC, argset ) == FAILURE )
      WRONG_PARAM_COUNT;
    convert_to_long_ex ( argset + 1 );
      dwsets = Z_LVAL_P( argset + 1 );
  }
    else
  WRONG_PARAM_COUNT;

  convert_to_string_ex ( argset + 0 );
    in_str = Z_STRVAL_P( argset + 0 );
    cc_str = Z_STRLEN_P( argset + 0 );

  RETURN_LONG( morpho->mlma->CheckWord( in_str, cc_str, dwsets ) );
}

//
// morph\Rus::Lemmatize( $string, $dwsets = 0 )
//    -> array( lemma_info )
//
PHP_METHOD( Rus, Lemmatize)
{
  auto        zendId = getThis();
  auto        morpho = Z_GETOBJ_P( zendId );
  auto        argcnt = ZEND_NUM_ARGS();
  char*       pszstr;
  long        cchstr;
  long        dwsets;
  SLemmInfoA  lemmas[0x20];
  SGramInfo   grinfo[0x40];
  char        normal[0x100];
  int         nlemma;

  switch ( argcnt )
  {
    case 1:   zend_parse_parameters( argcnt TSRMLS_CC, "s", &pszstr, &cchstr );
              dwsets = 0;
              break;
    case 2:   zend_parse_parameters( argcnt TSRMLS_CC, "sl", &pszstr, &cchstr, &dwsets );
              break;
    default:  WRONG_PARAM_COUNT;
  }

  if ( (nlemma = morpho->mlma->Lemmatize( pszstr, cchstr,
                                          lemmas, sizeof(lemmas) / sizeof(lemmas[0]),
                                          (dwsets & mlma_no_string) != 0 ? nullptr : normal, sizeof(normal),
                                          (dwsets & mlma_no_grinfo) != 0 ? nullptr : grinfo, sizeof(grinfo) / sizeof(grinfo[0]),
                                          dwsets & 0x0000ffff )) <= 0 )
    RETURN_FALSE;

  array_init( return_value );

  for ( auto plemma = lemmas; nlemma-- > 0; ++plemma )
  {
    zval  alemma;   array_init( &alemma );

    add_assoc_long( &alemma, "lexid", plemma->nlexid );

    if ( plemma->plemma != nullptr )
      add_assoc_str ( &alemma, "lemma", zend_string_init( plemma->plemma, strlen( plemma->plemma ), 1 ) );

    if ( plemma->pgrams != nullptr )
    {
      zval  agrams;   array_init( &agrams );

      for ( auto beg = plemma->pgrams, end = plemma->pgrams + plemma->ngrams; beg != end; ++beg )
      {
        zval  grinfo;   array_init( &grinfo );

        add_assoc_long( &grinfo, "wInfo", beg->wdInfo );
        add_assoc_long( &grinfo, "gInfo", beg->grInfo );
        add_assoc_long( &grinfo, "flags", beg->bFlags );
        add_assoc_long( &grinfo, "iForm", beg->idForm );

        add_next_index_zval( &agrams, &grinfo );
      }

      add_assoc_zval( &alemma, "gInfo", &agrams );
    }

    add_next_index_zval( return_value, &alemma );
  }
}

//
// morph\Rus -> BuildForm( $lexeme, $idform )
//    -> array( string )
//    -> FALSE
//
PHP_METHOD( Rus, BuildForm )
{
  auto  zendId = getThis();
  auto  morpho = Z_GETOBJ_P( zendId );
  auto  argcnt = ZEND_NUM_ARGS();
  long  lexeme;
  long  formid;
  char  aforms[256];
  int   nforms;

  if ( argcnt != 2 || zend_parse_parameters( argcnt TSRMLS_CC, "ll", &lexeme, &formid ) == FAILURE )
    WRONG_PARAM_COUNT;

  if ( lexeme == 0 )
  {
    php_error( E_WARNING, "Invalid '$lexeme' parameter to %s(...)", get_active_function_name( TSRMLS_C ) );
    RETURN_FALSE;
  }
  if ( formid < 0 || formid > 255 )
  {
    php_error( E_WARNING, "Invalid '$formid' parameter to %s(...), form identified must be between 0 and 255", get_active_function_name( TSRMLS_C ) );
    RETURN_FALSE;
  }

  if ( (nforms = morpho->mlma->BuildForm( aforms, sizeof(aforms), lexeme, (unsigned char)formid )) <= 0 )
    RETURN_FALSE;

/*  add the results           */
  array_init( return_value );

  for ( auto pforms = aforms; nforms-- > 0; pforms += strlen( pforms ) + 1 )
    add_next_index_str( return_value, zend_string_init( pforms, strlen( pforms ), 1 ) );
}

//
// morph\Rus -> FindForms( $lemma, $idform )
//    -> array( string )
//    -> FALSE
//
PHP_METHOD( Rus, FindForms )
{
  auto  zendId = getThis();
  auto  morpho = Z_GETOBJ_P( zendId );
  auto  argcnt = ZEND_NUM_ARGS();
  char* lexstr;
  long  lexlen;
  long  formid;
  char  aforms[256];
  int   nforms;

  if ( argcnt != 2 || zend_parse_parameters( argcnt TSRMLS_CC, "sl", &lexstr, &lexlen, &formid ) == FAILURE )
    WRONG_PARAM_COUNT;

  if ( lexlen == 0 )
  {
    php_error( E_WARNING, "Invalid '$lemma' parameter to %s(...), string must be passed", get_active_function_name( TSRMLS_C ) );
    RETURN_FALSE;
  }
  if ( formid < 0 || formid > 255 )
  {
    php_error( E_WARNING, "Invalid '$formid' parameter to %s(...), form identified must be between 0 and 255", get_active_function_name( TSRMLS_C ) );
    RETURN_FALSE;
  }

  if ( (nforms = morpho->mlma->FindForms( aforms, sizeof(aforms), lexstr, lexlen, (unsigned char)formid )) <= 0 )
    RETURN_FALSE;

/*  add the results           */
  array_init( return_value );

  for ( auto pforms = aforms; nforms-- > 0; pforms += strlen( pforms ) + 1 )
    add_next_index_str( return_value, zend_string_init( pforms, strlen( pforms ), 1 ) );
}

//
// morph\Rus -> CheckHelp( $wildcard )
//   -> string
//
PHP_METHOD( Rus, CheckHelp )
{
  auto  zendId = getThis();
  auto  morpho = Z_GETOBJ_P( zendId );
  auto  argcnt = ZEND_NUM_ARGS();
  char* szmask;
  int   ccmask;

/*  get parameter string      */
  if ( zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "s", &szmask, &ccmask ) == SUCCESS )
  {
    char  szhelp[256];
    int   cchelp;

    if ( (cchelp = morpho->mlma->CheckHelp( szhelp, sizeof(szhelp), szmask, ccmask )) <= 0 )
      {  RETURN_FALSE;  }

    RETURN_STR( zend_string_init( szhelp, cchelp, 1 ) );
  }
}

//
// morph\Rus -> GetWdInfo( $lexeme )
//   -> form_info != 0;
//   -> FALSE
//
PHP_METHOD( Rus, GetWdInfo )
{
  auto  zendId = getThis();
  auto  morpho = Z_GETOBJ_P( zendId );
  auto  argcnt = ZEND_NUM_ARGS();
  long  lexeme;

  if ( argcnt != 1 || zend_parse_parameters( argcnt TSRMLS_CC, "l", &lexeme ) == SUCCESS )
  {
    unsigned char wdinfo;

    if ( lexeme == 0 )
    {
      php_error( E_WARNING, "Invalid '$lexeme' parameter to %s(...)", get_active_function_name( TSRMLS_C ) );
      RETURN_FALSE;
    }

    if ( morpho->mlma->GetWdInfo( &wdinfo, lexeme ) ) { RETURN_LONG( wdinfo ); }
      else { RETURN_FALSE; }
  }
}

class EnumWords: protected IMlmaEnum
{
  zend_fcall_info       f_call;
  zend_fcall_info_cache f_data;

protected:  // overriden API
  MLMA_METHOD( Attach )() override {  return 1;  }
  MLMA_METHOD( Detach )() override {  return 1;  }
  MLMA_METHOD( RegisterLexeme )( lexeme_t nlexid, int nforms, const formid_t* pforms ) override
    {
      zval  argset[3];

      ZVAL_LONG ( argset + 0, nlexid );
      array_init( argset + 1 );

      for ( auto f = pforms, e = pforms + nforms; f != e; ++f )
        add_next_index_long( argset + 1, *f );

      f_call.param_count = 2;
      f_call.params      = argset;
      f_call.retval      = argset + 2;

      if ( zend_call_function( &f_call, &f_data ) != SUCCESS )
        return EFAULT;

      convert_to_long_ex  ( argset + 2 );

      return Z_LVAL_P( argset + 2 );
    }

public:     // construction and call
  EnumWords( zend_fcall_info& c, zend_fcall_info_cache& d ):
      f_call( reinterpret_cast<zend_fcall_info&&>( c ) ),
      f_data( reinterpret_cast<zend_fcall_info_cache&&>( d ) )
    {}
  auto  ptr() -> IMlmaEnum*
    {  return (IMlmaEnum*)this;  }
};

//
// morph\Rus -> EnumWords( $wildcard, $callable )
//   -> 0;
//   -> FALSE
//
PHP_METHOD( Rus, EnumWords )
{
  auto                  zendId = getThis();
  auto                  morpho = Z_GETOBJ_P( zendId );
  auto                  argcnt = ZEND_NUM_ARGS();
  char*                 stempl;
  long                  ltempl;
  zend_fcall_info       f_call;
  zend_fcall_info_cache f_data;

  zend_parse_parameters( argcnt TSRMLS_CC, "sf", &stempl, &ltempl, &f_call, &f_data );

  RETURN_LONG( morpho->mlma->EnumWords( EnumWords( f_call, f_data ).ptr(), stempl, ltempl ) );
}

/* All the functions that will be exported (available) must be declared */
PHP_MINIT_FUNCTION    ( morphrus );
PHP_MSHUTDOWN_FUNCTION( morphrus );
PHP_MINFO_FUNCTION    ( morphrus );

static zend_function_entry morph_rus_class_methods[] =
{
  PHP_ME( Rus, __construct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR )
  PHP_ME( Rus, SetLoCase,   NULL, ZEND_ACC_PUBLIC )
  PHP_ME( Rus, SetUpCase,   NULL, ZEND_ACC_PUBLIC )
  PHP_ME( Rus, CheckWord,   NULL, ZEND_ACC_PUBLIC )
  PHP_ME( Rus, Lemmatize,   NULL, ZEND_ACC_PUBLIC )
  PHP_ME( Rus, BuildForm,   NULL, ZEND_ACC_PUBLIC )
  PHP_ME( Rus, FindForms,   NULL, ZEND_ACC_PUBLIC )
  PHP_ME( Rus, GetWdInfo,   NULL, ZEND_ACC_PUBLIC )
  PHP_ME( Rus, CheckHelp,   NULL, ZEND_ACC_PUBLIC )
  PHP_ME( Rus, EnumWords,   NULL, ZEND_ACC_PUBLIC )

  PHP_FE_END
};

/* module information */
zend_module_entry morphrus_module_entry =
{
  STANDARD_MODULE_HEADER,
  "php_morphrus",
    NULL,
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

IMlmaMb*  mlma_api = NULL;

# if !defined(_MSC_VER)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wunused-parameter"
# endif // !_MSC_VER

PHP_MINIT_FUNCTION    ( morphrus )
{
  zend_class_entry  centry;

  INIT_CLASS_ENTRY( centry, "morph\\Rus", morph_rus_class_methods );
    morph_rus_class_entry = zend_register_internal_class( &centry TSRMLS_CC );
    morph_rus_class_entry->create_object = morph_rus_object::object_new;

  memcpy( &morph_rus_object::object_handlers, zend_get_std_object_handlers(), sizeof(morph_rus_object::object_handlers) );
    morph_rus_object::object_handlers.free_obj = morph_rus_object::object_free;
    morph_rus_object::object_handlers.dtor_obj = morph_rus_object::object_destroy; 
    morph_rus_object::object_handlers.offset = XtOffsetOf( morph_rus_object, zend ); 

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

# if !defined(_MSC_VER)
#   pragma GCC diagnostic pop
# endif // !_MSC_VER
