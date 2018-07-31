# pragma once
# if !defined( __references_h__ )
# define __references_h__
# include <tools/serialize.decl.h>
# include <cstdint>
# include <cstdio>
# include <map>

namespace libmorph
{

  class TableIndex: protected std::map<std::string, uint32_t>
  {

  public:     // API
    uint32_t  Find( const char* thekey ) const
      {
        auto  it = find( thekey );

        return it != end() ? it->second : 0;
      }

    template <class S>
    S*        Load( S* s )
      {
        size_t      toload;
        std::string newkey;
        uint32_t    newofs;

        for ( s = ::FetchFrom( s, toload ); toload-- > 0
          && (s = ::FetchFrom( ::FetchFrom( s, newofs ), newkey )) != nullptr; )
            this->insert( { newkey, newofs } );
        
        return s;
      }

  };

}

# endif  // __references_h__
