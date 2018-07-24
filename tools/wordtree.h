# pragma once
# if !defined( __libmorph_wordtree_h__ )
# define  __libmorph_wordtree_h__
# include <tools/serialize.h>
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
class wordtree: public std::vector<wordtree<element, counter>>
{
  std::unique_ptr<element>  pstems;
  uint8_t                   chnode;
  size_t                    length;

public:     // construction
  wordtree( uint8_t c = 0 ): std::vector<wordtree<element, counter>>(), chnode( c ), length( 0 )
    {}

public:     // expand
        element*  Insert( const char*, size_t );
  const element*  Search( const char*, size_t ) const;

public:     // serialization
                      size_t  GetBufLen();
  template <class O>  O*      Serialize( O* ) const;

public:     // enum
  template <class A>
  size_t    Enumerate( const A& action, size_t offset = 0 );
  template <class A>
  void      EnumItems( const A& action, char* k, size_t l = 0 ) const;

};

// wordtree inline implementation

template <class element, class counter>
element*  wordtree<element, counter>::Insert( const char* pszstr, size_t cchstr )
{
  if ( cchstr > 0 )
  {
    auto  ptrtop = this->begin();
    auto  ptrend = this->end();

    while ( ptrtop != ptrend && ptrtop->chnode > (uint8_t)*pszstr )
      ++ptrtop;

    if ( ptrtop == ptrend || ptrtop->chnode != (uint8_t)*pszstr )
      ptrtop = this->insert( ptrtop, wordtree( (uint8_t)*pszstr ) );

    return ptrtop->Insert( pszstr + 1, cchstr - 1 );
  }

  if ( pstems == nullptr )
    pstems = std::unique_ptr<element>( new element() );

  return pstems.get();
}

template <class element, class counter>
const element*  wordtree<element, counter>::Search( const char* pszstr, size_t cchstr ) const
{
  if ( cchstr > 0 )
  {
    auto  ptrtop = this->begin();
    auto  ptrend = this->end();

    while ( ptrtop != ptrend && ptrtop->chnode > (uint8_t)*pszstr )
      ++ptrtop;

    if ( ptrtop != ptrend || ptrtop->chnode != (uint8_t)*pszstr )
      return nullptr;

    return ptrtop->SearchStr( pszstr + 1, cchstr - 1 );
  }

  assert( cchstr == 0 );

  return pstems.get();
}

template <class element, class counter>
size_t  wordtree<element, counter>::GetBufLen()
{
  size_t  buflen = wordtree_impl::storecount<counter>::GetBufLen();

  for ( auto p = this->begin(); p != this->end(); ++p )
  {
    size_t  sublen = p->GetBufLen();

    buflen += ::GetBufLen( sublen ) + sublen + 1;
  }

  if ( pstems != nullptr )
    buflen += ::GetBufLen( *pstems.get() );

  return length = buflen;
}

template <class element, class counter> template <class O>
inline  O*  wordtree<element, counter>::Serialize( O* o ) const
{
  if ( (o = wordtree_impl::storecount<counter>( size(), pstems != nullptr ).Serialize( o )) == nullptr )
    return nullptr;

  for ( auto& n: *this )
    o = n.Serialize( ::Serialize( ::Serialize( o, n.chnode ), n.length ) );

  if ( pstems != nullptr )
    o = ::Serialize( o, *pstems.get() );

  return o;
}

template <class element, class counter>
template <class A>
size_t  wordtree<element, counter>::Enumerate( const A& action, size_t offset )
{
  offset += wordtree_impl::storecount<counter>::GetBufLen();

  for ( auto& n: *this )
    if ( (offset = n.Enumerate( action, offset + ::GetBufLen( n.chnode ) + ::GetBufLen( n.length ) )) == (size_t)-1 )
      return offset;

  if ( pstems != nullptr )
    offset = ::Enumerate( *pstems.get(), action, offset );

  return offset;
}

template <class element, class counter> template <class A>
void    wordtree<element, counter>::EnumItems( const A& action, char* k, size_t  l ) const
{
  for ( auto& n: *this )
  {
    k[l] = p->chnode;

    n.EnumItems( action, k, l + 1 );
  }

  if ( pstems != nullptr )
    action( *pstems.ptr(), k, l );
}

# endif  // __libmorph_wordtree_h__
