# pragma once
# if !defined( __references_h__ )
# define __references_h__
# include "serialize.h"
# include <cstdint>
# include <cstdio>
# include <map>

namespace libmorph
{

  struct TableIndex: protected std::map<std::string, uint32_t>
  {
    uint32_t  Find( const char* thekey ) const
      {
        auto  it = find( thekey );

        return it != end() ? it->second : 0;
      }

    template <class S>
    S*        Load( S* s )
      {
        return ::FetchFrom( s, *(std::map<std::string, uint32_t>*)this );
      }

  };

}

# endif  // __references_h__
