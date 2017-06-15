#if !defined( __lresolve_h__ )
#define __lresolve_h__
# include "../libdict/mlmadefs.h"
# include <mtc/zarray.h>
# include <tools/references.h>
# include "mtables.h"
# include <assert.h>

#if defined( _MSC_VER )
  #pragma warning( disable: 4237 )
#endif

struct  rusclassinfo
{
  word16_t  wdinfo;
  word16_t  tfoffs;
  word16_t  mtoffs;
  byte_t    chrmin;
  byte_t    chrmax;
  char      szpost[16];             // Текст пост-части слова

public:     // construction
  rusclassinfo(): wdinfo( 0 ), tfoffs( 0 ), mtoffs( 0 )
    {
      szpost[0] = '\0';
    }
  bool  operator == ( const rusclassinfo& r ) const
    {
      return wdinfo == r.wdinfo
          && tfoffs == r.tfoffs
          && mtoffs == r.mtoffs;
    }

public:     // serialization
  unsigned  GetBufLen() const
    {
      return sizeof(wdinfo)
        + (tfoffs != 0 ? sizeof(tfoffs) : 0)
        + (mtoffs != 0 ? sizeof(mtoffs) : 0);
    }
  template <class O>
  O*        Serialize( O* o ) const
    {
      unsigned short  wdInfo = wdinfo | (tfoffs    != 0 ? wfFlexes : 0)
                                      | (mtoffs    != 0 ? wfMixTab : 0);

                     o = ::Serialize( o, &wdInfo, sizeof(wdInfo) );
      o = (tfoffs != 0 ? ::Serialize( o, &tfoffs, sizeof(tfoffs) ) : o);
      o = (mtoffs != 0 ? ::Serialize( o, &mtoffs, sizeof(mtoffs) ) : o);

      return o;
    }

};

bool  ResolveClassInfo( const zarray<>& ztypes,
                        char*           pstems,
                        rusclassinfo&   rclass,
                        const char*     sznorm, const char*   szdies, const char*   sztype, const char* zapart, const char*   szcomm,
                        const char*     ftable, CReferences&  findex,
                        const char*     mtable, mixfiles&     mindex );

#if defined( _MSC_VER )
  #pragma warning( default: 4237 )
#endif

#endif // __lresolve_h__
