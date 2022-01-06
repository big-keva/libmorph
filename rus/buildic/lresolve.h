#if !defined( __lresolve_h__ )
#define __lresolve_h__
# include "../libdict/mlmadefs.h"
# include <tools/references.h>
# include "mtables.h"
# include <cassert>

struct  morphclass
{
  uint16_t  wdinfo = 0;
  uint16_t  tfoffs = 0;
  uint16_t  mtoffs = 0;

public:     // compare
  bool  operator == ( const morphclass& r ) const
    {
      return wdinfo == r.wdinfo
          && tfoffs == r.tfoffs
          && mtoffs == r.mtoffs;
    }
  bool  operator != ( const morphclass& r ) const
    {
      return !(*this == r);
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

constexpr morphclass nullclass;

struct  lexemeinfo
{
  std::string ststem;

  morphclass  mclass;
  byte_t      chrmin = 0;
  byte_t      chrmax = 0;

  std::string stpost;

  lexemeinfo( const lexemeinfo& ) = delete;
  lexemeinfo& operator = ( const lexemeinfo& ) = delete;
public:
  lexemeinfo() = default;
  lexemeinfo( lexemeinfo&& li ):
      ststem( std::move( li.ststem ) ),
      mclass( std::move( li.mclass ) ),
      chrmin( li.chrmin ),
      chrmax( li.chrmax ),
      stpost( std::move( li.stpost ) ) {}
  lexemeinfo& operator = ( lexemeinfo&& li )
    {
      ststem = std::move( li.ststem );
      mclass = std::move( li.mclass );
      chrmin = li.chrmin;
      chrmax = li.chrmax;
      stpost = std::move( li.stpost );
      return *this;
    }
};

lexemeinfo  ResolveClassInfo( 
  const char* sznorm, const char*   szdies, const char*   sztype, const char* zapart, const char*   szcomm,
  const char* ftable, const libmorph::TableIndex& findex,
  const char* mtable, const libmorph::rus::Alternator& mindex );

#endif // __lresolve_h__
