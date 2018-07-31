# pragma once
# if !defined( __libmorph_wordtree_h__ )
# define  __libmorph_wordtree_h__
# include "serialize.h"
# include <climits>
# include <cassert>
# include <memory>
# include <vector>

# if defined( _MSC_VER )
#   pragma warning( push )
#   pragma warning( disable: 4291 4514 4786 4710 )
# endif // _MSC_VER

namespace wordtree_impl
{
  template <size_t N>
  struct serializer
  {
    template <class O, class C> O*  operator ()( O* o, C c ) const
      {  return (o = ::Serialize( o, (uint8_t)c )) != nullptr ? serializer<N-1>()( o, (C)(c >> CHAR_BIT) ) : nullptr;  }
  };

  template <>
  struct serializer<1>
  {
    template <class O, class C> O* operator ()( O* o, C c ) const
      {  return ::Serialize( o, (uint8_t)c );  }
  };

  template <class counter>
  class storecount
  {
    enum
    {
      upperShift = sizeof(counter) * CHAR_BIT - 1,
      upperFlags = 1 << upperShift
    };
  public:
    storecount( size_t c, bool f ): cvalue( static_cast<counter>( c | (f ? upperFlags : 0) ) )
      {  assert( (c & upperFlags) == 0 );  }

  public:
    template <class O>  O*      Serialize( O* o ) const {  return serializer<sizeof(counter)>()( o, cvalue );  }
    constexpr static    size_t  GetBufLen(      )       {  return sizeof(counter);  }

  protected:
    counter cvalue;

  };

}

template <class element, class counter = uint8_t>
class wordtree
{
  std::vector<wordtree<element, counter>> nested;
  std::unique_ptr<element>                p_data;
  uint8_t                                 chnode;
  size_t                                  length;

public:     // construction
  wordtree( uint8_t c = 0 ): chnode( c ), length( 0 )
    {}

public:     // expand
        element*  Insert( const char*, size_t );
  const element*  Search( const char*, size_t ) const;

public:     // serialization
                      size_t  GetBufLen();
  template <class O>  O*      Serialize( O* ) const;
  template <class A>  size_t  Enumerate( A, size_t o = 0 );
};

// wordtree inline implementation

template <class element, class counter>
element*  wordtree<element, counter>::Insert( const char* pszstr, size_t cchstr )
{
  if ( cchstr > 0 )
  {
    auto  ptrtop = nested.begin();
    auto  ptrend = nested.end();

    while ( ptrtop != ptrend && ptrtop->chnode > (uint8_t)*pszstr )
      ++ptrtop;

    if ( ptrtop == ptrend || ptrtop->chnode != (uint8_t)*pszstr )
      ptrtop = nested.insert( ptrtop, wordtree( (uint8_t)*pszstr ) );

    return ptrtop->Insert( pszstr + 1, cchstr - 1 );
  }

  if ( p_data == nullptr )
    p_data = std::unique_ptr<element>( new element() );

  return p_data.get();
}

template <class element, class counter>
const element*  wordtree<element, counter>::Search( const char* pszstr, size_t cchstr ) const
{
  if ( cchstr > 0 )
  {
    auto  ptrtop = nested.begin();
    auto  ptrend = nested.end();

    while ( ptrtop != ptrend && ptrtop->chnode > (uint8_t)*pszstr )
      ++ptrtop;

    if ( ptrtop != ptrend || ptrtop->chnode != (uint8_t)*pszstr )
      return nullptr;

    return ptrtop->Search( pszstr + 1, cchstr - 1 );
  }

  assert( cchstr == 0 );

  return p_data.get();
}

template <class element, class counter>
size_t  wordtree<element, counter>::GetBufLen()
{
  size_t  buflen = wordtree_impl::storecount<counter>::GetBufLen();

  for ( auto& wt: nested )
  {
    size_t  sublen = wt.GetBufLen();

    buflen += ::GetBufLen( sublen ) + sublen + 1;
  }

  if ( p_data != nullptr )
    buflen += ::GetBufLen( *p_data.get() );

  return length = buflen;
}

template <class element, class counter> template <class O>
inline  O*  wordtree<element, counter>::Serialize( O* o ) const
{
  if ( (o = wordtree_impl::storecount<counter>( nested.size(), p_data != nullptr ).Serialize( o )) == nullptr )
    return nullptr;

  for ( auto& n: nested )
    o = n.Serialize( ::Serialize( ::Serialize( o, n.chnode ), n.length ) );

  if ( p_data != nullptr )
    o = ::Serialize( o, *p_data.get() );

  return o;
}

template <class element, class counter>
template <class A>
size_t  wordtree<element, counter>::Enumerate( A action, size_t offset )
{
  offset += wordtree_impl::storecount<counter>::GetBufLen();

  for ( auto& n: nested )
    if ( (offset = n.Enumerate( action, offset + ::GetBufLen( n.chnode ) + ::GetBufLen( n.length ) )) == (size_t)-1 )
      return offset;

  if ( p_data == nullptr )
    return offset;

  action( *p_data.get(), offset );

  return offset + ::GetBufLen( *p_data.get() );
}

# endif  // __libmorph_wordtree_h__
