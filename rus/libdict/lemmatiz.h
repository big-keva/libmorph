# if !defined( _lemmatiz_h_ )
# define _lemmatiz_h_

# include <namespace.h>
# include "mlmadefs.h"
# include "capsheme.h"

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
    const byte08_t* szstem;       // base string pointer
    unsigned        scheme;       // the capitalization scheme
    unsigned        dwsets;

  public:
    doCheckWord( const byte08_t* szbase, unsigned uflags ): szstem( szbase ),
                                                            scheme( 0 ), dwsets( uflags )
      {
      }
    int   InsertStem( lexeme_t, const byte08_t*, const steminfo&, const SGramInfo*, unsigned )
      {
        return 1;
      }
    bool  VerifyCaps( word16_t  wdinfo ) const
      {
        return (dwsets & sfIgnoreCapitals) != 0 || IsGoodSheme( scheme, pspMinCapValue[wdinfo & 0x3f] );
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
    size_t          clemma;   // the buffer length
    char*           pforms;   // the buffer for the forms
    size_t          cforms;   // the buffer size
    SGramInfo*      pgrams;   // the buffer for grammar descriptions
    size_t          cgrams;   // gramma descriptions length

    int             rcount;   // the count of built descriptions
    int             nerror;

  public: // the registration API
          doLemmatize( const byte08_t* szbase, unsigned uflags ): doCheckWord( szbase, uflags ),
                         plemma( NULL ),
                         pforms( NULL ),
                         pgrams( NULL ),
                         rcount( 0x00 ),
                         nerror( 0x00 )
            {
            }
    int   InsertStem( lexeme_t          nlexid,
                      const byte08_t*   pszstr,
                      const steminfo&   stinfo,
                      const SGramInfo*  flexes,
                      unsigned          fcount );

  };

  struct  doGetWdInfo: public doCheckWord
  {
          doGetWdInfo( const byte08_t* szbase, unsigned uflags ): doCheckWord( szbase, uflags )
            {
            }
    int   InsertStem( lexeme_t          /*nlexid*/,
                      const byte08_t*   /*strend*/,
                      const steminfo&   stinfo,
                      const SGramInfo*  /*flexes*/,
                      unsigned          /*fcount*/ )
            {
              *(byte08_t*)szstem = (byte08_t)stinfo.wdinfo;
              return 1;
            }

  };

  struct  doBuildForm: public doCheckWord
  {
    char*       output;
    size_t      cchout;

    word16_t    grinfo;
    byte08_t    bflags;
    unsigned    idform;

    int         rcount;
    int         nerror;

  public: // the registration API
    doBuildForm( const byte08_t* szbase, unsigned uflags ): doCheckWord( szbase, uflags ),
                    output( NULL ),
                    rcount( 0 ),
                    nerror( 0 )
      {
      }
    int   InsertStem( lexeme_t          nlexid,
                      const byte08_t*   strend,
                      const steminfo&   stinfo,
                      const SGramInfo*  flexes,
                      unsigned          fcount );

  };

  struct  doListForms: public doCheckWord
  {
    char*   output;
    size_t  cchout;

    int     rcount;
    int     nerror;

  public: // the registration API
    doListForms( const byte08_t* szbase, unsigned uflags ): doCheckWord( szbase, uflags ),
            rcount( 0 ),
            nerror( 0 )
      {
      }
    int   InsertStem( lexeme_t          nlexid,
                      const byte08_t*   strend,
                      const steminfo&   stinfo,
                      const SGramInfo*  flexes,
                      unsigned          fcount );

  };

} // end namespace

# endif // _lemmatiz_h_
