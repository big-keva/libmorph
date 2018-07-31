# include "references.h"
# include "serialize.h"
# include <stdexcept>

namespace libmorph
{

  template <class S>
  S*  LoadRef( S* s, std::string& n, uint32_t& o )
    {
      size_t  l;

      if ( (s = ::FetchFrom( ::FetchFrom( s, o ), l )) != nullptr )
      {
        n.assign( l, ' ' );
        s = ::FetchFrom( s, (char*)n.c_str(), l );
      }
      return s;
    }

  template <class S>
  S*  LoadTab( S* s, TableIndex::maptype& m )
    {
      size_t      toload;
      std::string newkey;
      uint32_t    newofs;

      for ( s = ::FetchFrom( s, toload ); toload-- > 0 && (s = LoadRef( s, newkey, newofs )) != nullptr; )
        m.insert( { newkey, newofs } );
        
      return s;
    }

  FILE* TableIndex::Load( FILE* s )
    {
      return LoadTab( s, *this );
    }

}
