# if !defined( __plaintree_h__ )
# define  __plaintree_h__
# include <mtc/serialize.h>
# include <mtc/array.h>

# if defined( _MSC_VER )
#   pragma warning( push )
#   pragma warning( disable: 4291 4514 4786 4710 )
# endif // _MSC_VER

using namespace mtc;

template <class val>
struct  storecounter
{
  static  unsigned  GetBufLen()
    {  return sizeof(val);  }
  template <class O>
  static O*         Serialize( O* o, int ncount, bool extended )
    {
      val   acount = ncount | (extended ? (1 << (sizeof(val) * CHAR_BIT - 1)) : 0);

      return ::Serialize( o, &acount, sizeof(acount) );
    }
};

template <class element, class counter = unsigned char>
class wordtree: public array<wordtree<element, counter>>
{
  unsigned char chnode;
  element*      pstems;
  unsigned      length;

public:     // construction
            wordtree( unsigned char c = '\0' );
           ~wordtree();

public:     // expand
        element*  GetObject();
  const element*  GetObject() const;
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
wordtree<element, counter>::wordtree( unsigned char c ):
  array<wordtree<element, counter>>( 0x2 ), chnode( c ), pstems( nullptr )
{
}

template <class element, class counter>
wordtree<element, counter>::~wordtree()
{
  if ( pstems != NULL )
    delete pstems;
}

template <class element, class counter>
element*  wordtree<element, counter>::GetObject()
{
  return pstems;
}

template <class element, class counter>
const element*  wordtree<element, counter>::GetObject() const
{
  return pstems;
}

template <class element, class counter>
element*  wordtree<element, counter>::InsertStr( const char* pszstr, size_t cchstr )
{
  if ( cchstr > 0 )
  {
    wordtree* ptrtop = *this;
    wordtree* ptrend = *this + this->GetLen();

    while ( ptrtop < ptrend && ptrtop->chnode > (unsigned char)*pszstr )
      ++ptrtop;
    if ( ptrtop >= ptrend || ptrtop->chnode != (unsigned char)*pszstr )
    {
      wordtree  insert( (unsigned char)*pszstr );
      int       inspos;

      if ( Insert( inspos = (int)(ptrtop - *this), insert ) == 0 )  ptrtop = *this + inspos;
        else return NULL;
    }
    return ptrtop->InsertStr( pszstr + 1, cchstr - 1 );
  }

  if ( pstems == nullptr )
    pstems = allocate_with<element>( GetAllocator() );

  return pstems;
}

template <class element, class counter>
const element*  wordtree<element, counter>::SearchStr( const char* pszstr, size_t cchstr ) const
{
  if ( cchstr > 0 )
  {
    auto ptrtop = begin();
    auto ptrend = end();

    while ( ptrtop < ptrend && ptrtop->chnode > (unsigned char)*pszstr )
      ++ptrtop;
    if ( ptrtop >= ptrend || ptrtop->chnode != (unsigned char)*pszstr )
      return NULL;
    return ptrtop->SearchStr( pszstr + 1, cchstr - 1 );
  }

  return cchstr == 0 ? pstems : NULL;
}

template <class element, class counter>
size_t  wordtree<element, counter>::GetBufLen()
{
  size_t    buflen = storecounter<counter>::GetBufLen();
  wordtree* ptrtop;
  wordtree* ptrend;
  size_t    sublen;

  for ( ptrend = (ptrtop = *this) + this->GetLen(); ptrtop < ptrend; ++ptrtop )
    buflen += ::GetBufLen( sublen = ptrtop->GetBufLen() ) + sublen + 1;

  if ( pstems != NULL )
    buflen += pstems->GetBufLen();

  return (size_t)(length = (unsigned)buflen);
}

template <class element, class counter> template <class O>
inline  O*  wordtree<element, counter>::Serialize( O* o ) const
{
  const wordtree* p;

  if ( (o = storecounter<counter>::Serialize( o, (int)size(), pstems != NULL )) == NULL )
    return NULL;

  for ( p = begin(); p < end(); ++p )
    o = p->Serialize( ::Serialize( ::Serialize( o, p->chnode ), p->length ) );

  if ( pstems != NULL )
    o = pstems->Serialize( o );

  return o;
}

template <class element, class counter> template <class A>
size_t  wordtree<element, counter>::Enumerate( const A& action, size_t offset )
{
  wordtree* p;

  for ( offset += storecounter<counter>::GetBufLen(), p = begin(); p < end(); ++p )
    if ( (offset = p->Enumerate( action, offset + ::GetBufLen( p->chnode ) + ::GetBufLen( p->length ) )) == (size_t)-1 )
      return offset;

  if ( pstems != NULL )
    offset = pstems->Enumerate( action, offset );

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

  return pstems != NULL ? action( *pstems, k, l ) : 0;
}

# endif  // __plaintree_h__
