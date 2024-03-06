# pragma once
# if !defined( __references_h__ )
# define __references_h__
# include "serialize.h"
# include <cstdint>
# include <cstdio>
# include <string>
# include <map>

namespace libmorph
{

  struct TableIndex: protected std::map<std::string, uint32_t>
  {
    auto  operator []( const char* key ) const -> uint32_t
    {
      auto  it = find( key );

      return it != end() ? it->second : 0;
    }
    auto  operator []( const std::string& key ) const -> uint32_t
    {
      auto  it = find( key );

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
