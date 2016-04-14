# if !defined( __plaintree_h__ )
# define  __plaintree_h__
# include "serialize.h"
# include "array.h"

# if defined( _MSC_VER )
#   pragma warning( push )
#   pragma warning( disable: 4291 4514 4786 4710 )
# endif // _MSC_VER

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
class wordtree: protected array<wordtree<element, counter>, wordtree<element, counter>& >
{
  unsigned char chnode;
  element*      pstems;
  unsigned      length;

public:     // construction
            wordtree( unsigned char c = '\0' );
           ~wordtree();

public:     // expand
  element*  InsertStr( const char*, unsigned );

public:     // serialization
  unsigned  GetBufLen();
  template <class O>
  O*        Serialize( O* ) const;

public:     // enum
  template <class A>
  unsigned  Enumerate( A action, unsigned offset = 0 );

};

// wordtree inline implementation

template <class element, class counter>
wordtree<element, counter>::wordtree( unsigned char c ): array<wordtree<element, counter>, wordtree<element, counter>& >( 0x2 ),
  chnode( c ), pstems( NULL )
{
}

template <class element, class counter>
wordtree<element, counter>::~wordtree()
{
  if ( pstems != NULL )
    delete pstems;
}

template <class element, class counter>
element*  wordtree<element, counter>::InsertStr( const char* pszstr, unsigned cchstr )
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

      if ( Insert( inspos = ptrtop - *this, insert ) == 0 )  ptrtop = *this + inspos;
        else return NULL;
    }
    return ptrtop->InsertStr( pszstr + 1, cchstr - 1 );
  }

  if ( pstems == NULL && (pstems = (element*)malloc( sizeof(element) )) != NULL )
    new( (__the_array_element_ptr*)pstems ) element();

  return pstems;
}

template <class element, class counter>
unsigned  wordtree<element, counter>::GetBufLen()
{
  unsigned        buflen = storecounter<counter>::GetBufLen();
  wordtree*       ptrtop;
  wordtree*       ptrend;
  unsigned        sublen;

  for ( ptrend = (ptrtop = *this) + this->GetLen(); ptrtop < ptrend; ++ptrtop )
    buflen += ::GetBufLen( sublen = ptrtop->GetBufLen() ) + sublen + 1;

  if ( pstems != NULL )
    buflen += pstems->GetBufLen();

  return length = buflen;
}

template <class element, class counter> template <class O>
inline  O*        wordtree<element, counter>::Serialize( O* o ) const
{
  const wordtree* p;

  if ( (o = storecounter<counter>::Serialize( o, size(), pstems != NULL )) == NULL )
    return NULL;

  for ( p = begin(); p < end(); ++p )
    o = p->Serialize( ::Serialize( ::Serialize( o, p->chnode ), p->length ) );

  if ( pstems != NULL )
    o = pstems->Serialize( o );

  return o;
}

template <class element, class counter> template <class A>
unsigned  wordtree<element, counter>::Enumerate( A action, unsigned offset )
{
  wordtree* p;

  for ( offset += storecounter<counter>::GetBufLen(), p = begin(); p < end(); ++p )
    if ( (offset = p->Enumerate( action, offset + ::GetBufLen( p->chnode ) + ::GetBufLen( p->length ) )) == (unsigned)-1 )
      return offset;

  if ( pstems != NULL )
    offset = pstems->Enumerate( action, offset );

  return offset;
}

# endif  // __plaintree_h__
