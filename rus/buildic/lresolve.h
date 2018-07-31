#if !defined( __lresolve_h__ )
#define __lresolve_h__
# include "../libdict/mlmadefs.h"
# include <tools/references.h>
# include "mtables.h"
# include <cassert>

template <class O>  O*  Serialize( O*, const void*, size_t );

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

struct  lexemeinfo: public morphclass
{
  std::string ststem;
  std::string stpost;

  byte_t      chrmin = 0;
  byte_t      chrmax = 0;

public:
  lexemeinfo()
    {}
  lexemeinfo( const lexemeinfo& li ): morphclass( li ),
    ststem( li.ststem ),
    stpost( li.stpost ),
    chrmin( li.chrmin ),
    chrmax( li.chrmax ) {}
  lexemeinfo( lexemeinfo&& li ): morphclass( std::move( li ) ),
    ststem( std::move( li.ststem ) ),
    stpost( std::move( li.stpost ) ),
    chrmin( li.chrmin ),
    chrmax( li.chrmax ) {}
  lexemeinfo& operator = ( lexemeinfo&& li )
    {
      morphclass::operator = ( std::move( li ) );
      ststem = std::move( li.ststem );
      stpost = std::move( li.stpost );
      chrmin = li.chrmin;
      chrmax = li.chrmax;
      return *this;
    }
};

lexemeinfo  ResolveClassInfo( 
  const char* sznorm, const char*   szdies, const char*   sztype, const char* zapart, const char*   szcomm,
  const char* ftable, const libmorph::TableIndex& findex,
  const char* mtable, const libmorph::rus::Alternator& mindex );

#endif // __lresolve_h__
