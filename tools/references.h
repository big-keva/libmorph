# pragma once
# if !defined( __references_h__ )
# define __references_h__
# include <cstdint>
# include <cstdio>
# include <map>

namespace libmorph
{

  class TableIndex: protected std::map<std::string, uint32_t>
  {
  public:
    using maptype = std::map<std::string, uint32_t>;

  public:     // API
    uint32_t  Find( const char* ) const;
    FILE*     Load( FILE* );

  };

  inline
  uint32_t  TableIndex::Find( const char* thekey ) const
    {
      auto  it = find( thekey );

      return it != end() ? it->second : 0;
    }

}

# endif  // __references_h__
