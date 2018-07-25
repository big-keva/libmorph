# if !defined( __references_h__ )
# define __references_h__
# include "serialize.h"
# include <cstdint>
# include <map>

namespace libmorph
{

  class TableIndex
  {
    std::map<std::string, uint32_t> refmap;

    struct ref
    {
      std::string key;
      uint32_t    ofs;

    public:
      template <class S>  S*  Load( S* s );
    };

  public:
    uint32_t                Find( const char* thekey ) const;
    template <class S>  S*  Load( S* s );
  };

  // TableIndex implementation

  template <class S>
  S*  TableIndex::ref::Load( S* s )
    {
      uint32_t  cchkey;

      if ( (s = ::FetchFrom( ::FetchFrom( s, ofs ), cchkey )) != nullptr )
      {
        key.assign( cchkey, ' ' );

        s = ::FetchFrom( s, (char*)key.c_str(), cchkey );
      }
      return s;
    }

  inline
  uint32_t  TableIndex::Find( const char* thekey ) const
    {
      auto  it = refmap.find( thekey );

      return it != refmap.end() ? it->second : 0;
    }

  template <class S>
  S*  TableIndex::Load( S* s )
    {
      size_t  toload;
      ref     newref;

      for ( s = ::FetchFrom( s, toload ); toload-- > 0 && (s = newref.Load( s )) != nullptr; )
        refmap.insert( { newref.key, newref.ofs } );
        
      return s;
    }

}

# endif  // __references_h__
