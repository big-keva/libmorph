# if !defined( _lemmatiz_h_ )
# define _lemmatiz_h_

# include <namespace.h>
# include "mlmadefs.h"
# include <xmorph/capsheme.h>

namespace LIBMORPH_NAMESPACE
{
  #if !defined( lemmatize_errors_defined )
    #define lemmatize_errors_defined
    #define LEMMBUFF_FAILED -1
    #define LIDSBUFF_FAILED -2
    #define GRAMBUFF_FAILED -3
    #define WORDBUFF_FAILED -4
  #endif

  //
  // doCheckWord - empty actor, reports no error on stem action, and the code
  // will return correct result itself
  //
  struct  doCheckWord
  {
    const byte_t* szstem;       // base string pointer
    unsigned      scheme;       // the capitalization scheme
    unsigned      dwsets;

  public:
    doCheckWord( const byte_t* szbase, unsigned uflags ):
      szstem( szbase ), scheme( 0 ), dwsets( uflags )
      {
      }
    int   InsertStem( lexeme_t, const steminfo&, const byte_t*, const byte_t*, const SGramInfo*, unsigned )
      {
        return 1;
      }
    bool  VerifyCaps( const steminfo& s ) const
      {
        return (dwsets & sfIgnoreCapitals) != 0 || IsGoodSheme( scheme, s.MinCapScheme() );
      }
  };

  //
  // doLemmatize - create lemmatization result, build normal (dictionary) form for a stem
  // passed to function
  // the output data - the references to the arrays to be filled by the lemmatizer
  //
  struct  doLemmatize: public doCheckWord
  {
    SLemmInfoA*     plemma;   // the output buffer for descriptions
    SLemmInfoA*     elemma;
    char*           pforms;   // the buffer for the forms
    char*           eforms;   // the buffer size
    SGramInfo*      pgrams;   // the buffer for grammar descriptions
    SGramInfo*      egrams;   // gramma descriptions length
    unsigned        encode;   // the character encoding

    int             nerror;

  public: // the registration API
          doLemmatize( const byte_t* szbase, unsigned uflags, unsigned cp ): doCheckWord( szbase, uflags ),
                         plemma( nullptr ),
                         pforms( nullptr ),
                         pgrams( nullptr ),
                         encode( cp ),
                         nerror( 0x00 )
            {
            }
    int   InsertStem( lexeme_t          nlexid,
                      const steminfo&   stinfo,
                      const byte_t*     szpost,
                      const byte_t*     pszstr,
                      const SGramInfo*  flexes,
                      unsigned          fcount );

  };

  struct  doGetWdInfo: public doCheckWord
  {
          doGetWdInfo( const byte_t* szbase, unsigned uflags ): doCheckWord( szbase, uflags )
            {
            }
    int   InsertStem( lexeme_t          /*nlexid*/,
                      const byte_t*     /*strend*/,
                      const steminfo&   stinfo,
                      const SGramInfo*  /*flexes*/,
                      unsigned          /*fcount*/ )
            {
              *(byte_t*)szstem = (byte_t)stinfo.wdinfo;
              return 1;
            }

  };

  struct  doBuildForm: public doCheckWord
  {
    char*       output;
    char*       outend;

    word16_t    grinfo;
    byte_t      bflags;
    unsigned    idform;
    unsigned    encode;

    int         rcount;
    int         nerror;

  public: // the registration API
    doBuildForm( const byte_t* szbase, unsigned dwsets, unsigned cp ): doCheckWord( szbase, dwsets ),
                    output( NULL ),
                    encode( cp ),
                    rcount( 0 ),
                    nerror( 0 )
      {
      }
    int   InsertStem( lexeme_t          nlexid,
                      const steminfo&   stinfo,
                      const byte_t*     szpost,
                      const byte_t*     strend,
                      const SGramInfo*  flexes,
                      unsigned          fcount );

  };

} // end namespace

# endif // _lemmatiz_h_
