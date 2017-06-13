# if !defined( __plaintree_h__ )
# define  __plaintree_h__
# include <mtc/serialize.h>
# include <mtc/autoptr.h>
# include <mtc/array.h>

# if defined( _MSC_VER )
#   pragma warning( push )
#   pragma warning( disable: 4291 4514 4786 4710 )
# endif // _MSC_VER

using namespace mtc;

template <class counter>
class wordtree_counter
{
  template <size_t N>
  struct serial
  {
    template <class O, class C>
    static  O*  put( O* o, C c )
      {  return (o = ::Serialize( o, (byte_t)c )) != nullptr ? serial<N-1>::put( o, (C)(c >> CHAR_BIT) ) : nullptr;  }
    template <class O>
    static  O*  put( O* o, char c )   {  return ::Serialize( o, c );  }
    template <class O>
    static  O*  put( O* o, byte_t c ) {  return ::Serialize( o, c );  }
  };
  template <>
  struct serial<0>
  {
    template <class O, class C> static  O*  put( O* o, C c )
      {  (void)c;  return o;  }
  };

  enum
  {
    upperShift = sizeof(counter) * CHAR_BIT - 1,
    upperFlags = 1 << upperShift
  };
public:
  wordtree_counter( int c, bool f ): cvalue( static_cast<counter>( c | (f ? upperFlags : 0) ) )
    {
      assert( (c & upperFlags) == 0 );
    }

public:
  template <class O>
  O*      Serialize( O* o ) const
    {  return serial<sizeof(counter)>::put( o, cvalue );  }
  constexpr static
  size_t  GetBufLen()
    {  return sizeof(counter);  }

protected:
  counter cvalue;

};

template <class element, class counter = byte_t>
class wordtree: public array<wordtree<element, counter>>
{
  _auto_<element> pstems;
  byte_t          chnode;
  size_t          length;

public:     // construction
            wordtree( byte_t c = 0 ): array<wordtree<element, counter>>( 0x2 ), chnode( c ), length( 0 )  {}

public:     // expand
        element*  GetObject()         {  return pstems;  }
  const element*  GetObject() const   {  return pstems;  }
        element*  InsertStr( const char*, size_t );
  const element*  SearchStr( const char*, size_t ) const;

public:     // serialization
  size_t    GetBufLen();
  template <class O>
  O*        Serialize( O* ) const;

public:     // enum
  template <class A>
  size_t    Enumerate( const A& action, size_t offset = 0 );
  template <class A>
  int       EnumItems( const A& action, char* k, size_t l = 0 ) const;

};

// wordtree inline implementation

template <class element, class counter>
element*  wordtree<element, counter>::InsertStr( const char* pszstr, size_t cchstr )
{
  if ( cchstr > 0 )
  {
    auto  ptrtop = this->begin();
    auto  ptrend = this->end();

    while ( ptrtop < ptrend && ptrtop->chnode > (byte_t)*pszstr )
      ++ptrtop;

    if ( ptrtop >= ptrend || ptrtop->chnode != (byte_t)*pszstr )
    {
      wordtree  insert( (byte_t)*pszstr );
      int       inspos;

      if ( this->Insert( inspos = (int)(ptrtop - *this), insert ) == 0 )  ptrtop = *this + inspos;
        else return nullptr;
    }

    return ptrtop->InsertStr( pszstr + 1, cchstr - 1 );
  }

  if ( pstems == nullptr )
    pstems = allocate_with<element>( this->GetAllocator() );

  return pstems;
}

template <class element, class counter>
const element*  wordtree<element, counter>::SearchStr( const char* pszstr, size_t cchstr ) const
{
  if ( cchstr > 0 )
  {
    auto  ptrtop = this->begin();
    auto  ptrend = this->end();

    while ( ptrtop < ptrend && ptrtop->chnode > (byte_t)*pszstr )
      ++ptrtop;

    if ( ptrtop >= ptrend || ptrtop->chnode != (byte_t)*pszstr )
      return nullptr;

    return ptrtop->SearchStr( pszstr + 1, cchstr - 1 );
  }

  return cchstr == 0 ? pstems : nullptr;
}

template <class element, class counter>
size_t  wordtree<element, counter>::GetBufLen()
{
  size_t  buflen = wordtree_counter<counter>::GetBufLen();

  for ( auto p = this->begin(); p < this->end(); ++p )
  {
    size_t  sublen = p->GetBufLen();

    buflen += ::GetBufLen( sublen ) + sublen + 1;
  }

  if ( pstems != nullptr )
    buflen += ::GetBufLen( *pstems.ptr() );

  return length = buflen;
}

template <class element, class counter> template <class O>
inline  O*  wordtree<element, counter>::Serialize( O* o ) const
{
  if ( (o = wordtree_counter<counter>( size(), pstems != nullptr ).Serialize( o )) == nullptr )
    return nullptr;

  for ( auto p = this->begin(); p < this->end(); ++p )
    o = p->Serialize( ::Serialize( ::Serialize( o, p->chnode ), p->length ) );

  if ( pstems != nullptr )
    o = ::Serialize( o, *pstems.ptr() );

  return o;
}

template <class element, class counter>
template <class A>
size_t  wordtree<element, counter>::Enumerate( const A& action, size_t offset )
{
  offset += wordtree_counter<counter>::GetBufLen();

  for ( auto p = this->begin(); p < this->end(); ++p )
    if ( (offset = p->Enumerate( action, offset + ::GetBufLen( p->chnode ) + ::GetBufLen( p->length ) )) == (size_t)-1 )
      return offset;

  if ( pstems != nullptr )
    offset = ::Enumerate( *pstems.ptr(), action, offset );

  return offset;
}

template <class element, class counter> template <class A>
int       wordtree<element, counter>::EnumItems( const A& action, char* k, size_t  l ) const
{
  int   nerror;

  for ( auto p = begin(); p < end(); ++p )
  {
    k[l] = p->chnode;
    if ( (nerror = p->EnumItems( action, k, l + 1 )) != 0 )
      return nerror;
  }

  return pstems != nullptr ? action( *pstems.ptr(), k, l ) : 0;
}

# endif  // __plaintree_h__
