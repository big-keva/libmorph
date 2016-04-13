# if !defined( __libmorph_array_h__ )
# define  __libmorph_array_h__
//   09/16/05 - (danila) - warnings
//[] 2003-05-04 Kaa: PushBack function was added.

# include <stdlib.h>
# include <string.h>
# include <assert.h>
# include <errno.h>

# if defined( _MSC_VER )
#   pragma warning( push )
#   pragma warning( disable: 4291 4514 4786 4710 )
# endif // _MSC_VER

struct  __the_array_element_ptr
{
};

inline  void* operator new ( size_t, __the_array_element_ptr* lpdata )
  {  return lpdata;  }

// Common destruction methods
template <class A>
inline  void  __safe_array_destruct( A* lplist, int  lcount )
  {  for ( ; lcount-- > 0; lplist++ ) lplist->~A(); }
template <>
inline  void  __safe_array_destruct( char*, int ) {}
template <>
inline  void  __safe_array_destruct( unsigned char*, int ) {}
template <>
inline  void  __safe_array_destruct( short*, int ) {}
template <>
inline  void  __safe_array_destruct( unsigned short*, int ) {}
template <>
inline  void  __safe_array_destruct( int*, int ) {}
template <>
inline  void  __safe_array_destruct( unsigned int*, int ) {}
template <>
inline  void  __safe_array_destruct( long*, int ) {}
template <>
inline  void  __safe_array_destruct( unsigned long*, int ) {}
template <>
inline  void  __safe_array_destruct( float*, int ) {}
template <>
inline  void  __safe_array_destruct( double*, int ) {}
template <>
inline  void  __safe_array_destruct( char**, int ) {}
template <>
inline  void  __safe_array_destruct( unsigned char**, int ) {}
template <>
inline  void  __safe_array_destruct( short**, int ) {}
template <>
inline  void  __safe_array_destruct( unsigned short**, int ) {}
template <>
inline  void  __safe_array_destruct( int**, int ) {}
template <>
inline  void  __safe_array_destruct( unsigned int**, int ) {}
template <>
inline  void  __safe_array_destruct( long**, int ) {}
template <>
inline  void  __safe_array_destruct( unsigned long**, int ) {}
template <>
inline  void  __safe_array_destruct( float**, int ) {}
template <>
inline  void  __safe_array_destruct( double**, int ) {}

// Common construction methods
template <class A>
inline  void  __safe_array_construct_def( A* p, int c )
  {
    memset( p, 0, c * sizeof(A) );
    while ( c-- > 0 )
      new( (__the_array_element_ptr*)(p++) )A;
  }
template <>
inline  void  __safe_array_construct_def( unsigned long* p, int c )
  {  memset( p, 0, c * sizeof(unsigned long) );  }
template <>
inline  void  __safe_array_construct_def( unsigned int* p, int c )
  {  memset( p, 0, c * sizeof(unsigned int) );  }
template <>
inline  void  __safe_array_construct_def( unsigned short* p, int c )
  {  memset( p, 0, c * sizeof(unsigned short) );  }
template <>
inline  void  __safe_array_construct_def( unsigned char* p, int c )
  {  memset( p, 0, c * sizeof(unsigned char) );  }
template <>
inline  void  __safe_array_construct_def( long* p, int c )
  {  memset( p, 0, c * sizeof(long) );  }
template <>
inline  void  __safe_array_construct_def( int* p, int c )
  {  memset( p, 0, c * sizeof(int) );  }
template <>
inline  void  __safe_array_construct_def( short* p, int c )
  {  memset( p, 0, c * sizeof(short) );  }
template <>
inline  void  __safe_array_construct_def( char* p, int c )
  {  memset( p, 0, c * sizeof(char) );  }
template <>
inline  void  __safe_array_construct_def( double* p, int c )
  {  memset( p, 0, c * sizeof(double) );  }
template <>
inline  void  __safe_array_construct_def( float* p, int c )
  {  memset( p, 0, c * sizeof(float) );  }

// Copy construction methods
template <class A>
inline  void  __safe_array_construct_cpy( A* p, const A& r )
  {
    new( (__the_array_element_ptr*)p )A( r );
  }
inline  void  __safe_array_construct_cpy( unsigned long* p, unsigned long r )
  {  *p = r;  }
inline  void  __safe_array_construct_cpy( unsigned int* p, unsigned int r )
  {  *p = r;  }
inline  void  __safe_array_construct_cpy( unsigned short* p, unsigned short r )
  {  *p = r;  }
inline  void  __safe_array_construct_cpy( unsigned char* p, unsigned char r )
  {  *p = r;  }
inline  void  __safe_array_construct_cpy( long* p, long r )
  {  *p = r;  }
inline  void  __safe_array_construct_cpy( int* p, int r )
  {  *p = r;  }
inline  void  __safe_array_construct_cpy( short* p, short r )
  {  *p = r;  }
inline  void  __safe_array_construct_cpy( char* p, char r )
  {  *p = r;  }
inline  void  __safe_array_construct_cpy( double* p, double r )
  {  *p = r;  }
inline  void  __safe_array_construct_cpy( float* p, float r )
  {  *p = r;  }

// Swap method
template <class T>
void mtCommon_Swap( T& r1, T& r2 )
{
  T t(r1); r1 = r2; r2 = t;
}

template <class T, class R>
class array
{
public:
  typedef T ArrayDataTypeT;

        array( int adelta = 0x10 );
        array( const array<T,R>& );
  array<T,R>& operator =( const array<T,R>& );
        ~array();
  int   Append( R );
  int   Append( int, T* );
  int   Append( array<T,R>& );  
  int   Insert( int, R );
  int   Insert( int, int, T* );
  int   Delete( int );
  int   GetLen() const;
  int   SetLen( int );
  void  DelAll();
  int&  Length();
  int   Lookup( R refone );
  int   Lookup( R refone, int (*compare)( T&, T& ) );
  bool  Search( R refone, int& );
  bool  Search( R refone, int&, int (*compare)( T&, T& ) );
  void  Resort( int (*compare)( T&, T& ) );
  operator        T* ();
  operator const  T* () const;
  T&    operator [] ( int );
  const T&  operator [] ( int ) const;
  void  Swap   ( array<T, R>& );
  int   GetLimit(            ) const;
  int   GetDelta(            ) const;
  void  SetDelta( int nDelta );

// std::container compatibility
  T*        begin()           {  return this->pitems;  }
  const T*  begin()     const {  return this->pitems;  }
  T*        end()             {  return this->pitems + this->GetLen();  }
  const T*  end()       const {  return this->pitems + this->GetLen();  }
  int       size()      const {  return this->ncount;  }
  int       max_size()  const {  return this->nlimit;  }
  bool      empty()     const {  return this->ncount == 0;  }

protected:
  T*    pitems;
  int   ncount;
  int   nlimit;
  int   ndelta;

};

template <class T, class R>
inline  array<T, R>::array( int adelta ):
                                pitems( 0 ),
                                ncount( 0 ),
                                nlimit( 0 ),
                                ndelta( adelta <= 0 ? 0x10 : adelta )
{
}

template <class T, class R>
inline array<T, R>::array( const array<T,R>& r )
  : pitems( r.pitems ),
    ncount( r.ncount ), 
    nlimit( r.nlimit ),
    ndelta( r.ndelta )
{
  if ( r.nlimit > 0 )
    ::new( (__the_array_element_ptr*)&r ) array();
}

template <class T, class R>
inline array<T,R>& array<T, R>::operator =( const array<T,R>& r )
{
  if ( nlimit != 0 )
    this->~array();

  pitems = r.pitems;
  ncount = r.ncount;
  nlimit = r.nlimit;
  ndelta = r.ndelta;

  if ( r.nlimit > 0 )
    ::new( (__the_array_element_ptr*)&r ) array();

  return *this;
}


template <class T, class R>
inline  array<T, R>::~array()
{
  if ( pitems )
  {
    if ( ncount )
      __safe_array_destruct( pitems, ncount );
    free( pitems );
  }
}

template <class T, class R>
inline  int   array<T, R>::Append( R  refone )
{
  return Insert( ncount, refone );
}

template <class T, class R>
inline  int   array<T, R>::Append( int acount, T* lplist )
{
  return Insert( ncount, acount, lplist );
}

template <class T, class R>
inline int    array<T, R>::Append( array<T, R>&   refarr )  
{
  return Insert( ncount, refarr.ncount, refarr.pitems );
}

template <class T, class R>
inline  int   array<T, R>::Insert( int nindex, R  refone )
{
// Check if valid arguments passed
  if ( nindex < 0 || nindex > ncount )
    return EINVAL;

  assert( ncount <= nlimit );

// Ensure enough space
  if ( ncount == nlimit )
  {
    int   newlimit = nlimit + ndelta;
    T*    newitems;

    assert( newlimit > nindex );

  // Allocate new space
    if ( (newitems = (T*)malloc( newlimit * sizeof(T) )) == NULL )
      return ENOMEM;

  // Copy the data
    if ( ncount > 0 )
      memcpy( newitems, pitems, ncount * sizeof(T) );

  // Set new buffer
    if ( pitems != NULL )
      free( pitems );
    pitems = newitems;
    nlimit = newlimit;
  }

// Check if the space would be prepared
  if ( nindex < ncount )
    memmove( pitems + nindex + 1, pitems + nindex, (ncount - nindex)
      * sizeof(T) );

// Create the element with the copy constructor
  __safe_array_construct_cpy( pitems + nindex, (R)refone );
  ++ncount;
  return 0;
}

template <class T, class R>
inline  int   array<T, R>::Insert( int nindex, int  acount, T*  lplist )
{
  int   nerror;

  while ( acount-- > 0 )
    if ( (nerror = Insert( nindex++, *lplist++ )) != 0 )
      return nerror;
  return 0;
}

template <class T, class R>
inline  int   array<T, R>::Delete( int nindex )
{
  if ( nindex < 0 || nindex >= ncount )
    return EINVAL;
  else __safe_array_destruct( pitems + nindex, 1 );
  if ( nindex < --ncount )
    memmove( pitems + nindex, pitems + nindex + 1, (ncount - nindex)
      * sizeof(T) );
  return 0;
}

template <class T, class R>
inline  int   array<T, R>::GetLen() const
{
  return ncount;
}

template <class T, class R>
inline  int   array<T, R>::SetLen( int length )
{
  if ( length < 0 )
    return EINVAL;
  if ( length < ncount )
    __safe_array_destruct( pitems + length, ncount - length );
  if ( length > ncount )
  {
    if ( length > nlimit )
    {
      int   newlimit = nlimit + ndelta;
      T*    newitems;

      if ( newlimit < length )
        newlimit = length;

    // Allocate new space
      if ( (newitems = (T*)malloc( newlimit * sizeof(T) )) == NULL )
        return ENOMEM;

    // Copy the data
      if ( ncount > 0 )
        memcpy( newitems, pitems, ncount * sizeof(T) );

    // Set new buffer
      if ( pitems != NULL )
        free( pitems );
      pitems = newitems;
      nlimit = newlimit;
    }
    __safe_array_construct_def( pitems + ncount, length - ncount );
  }
    else
  if ( length == 0 )
  {
    free( pitems );
    pitems = 0;
    nlimit = 0;
  }
  ncount = length;
  return 0;
}

template <class T, class R>
inline  void   array<T, R>::DelAll()
{
  __safe_array_destruct( pitems, ncount );
  free( pitems );
  pitems = 0;
  nlimit = 0;
  ncount = 0;
}

template <class T, class R>
inline  int&  array<T, R>::Length()
{
  return ncount;
}

template <class T, class R>
inline  int   array<T, R>::Lookup( R  refone )
{
  int   nindex;

  for ( nindex = 0; nindex < ncount; ++nindex )
    if ( refone == pitems[nindex] )
      return nindex;

  return -1;
}

template <class T, class R>
inline  int   array<T, R>::Lookup( R  refone, int (*compare)( T&, T& ) )
{
  int   nindex;

  for ( nindex = 0; nindex < ncount; ++nindex )
    if ( compare( refone, pitems[nindex] ) == 0 )
      return nindex;

  return -1;
}

template <class T, class R>
inline  bool  array<T, R>::Search( R  refone, int&  refidx )
{
  int   l = 0;
  int   h = ncount - 1;
  int   m;
  bool  s = false;

  while ( l <= h )
  {
    m = ( l + h ) >> 1;
    if ( pitems[m] < refone ) l = m + 1;
      else
    {
      h = m - 1;
      s |= (pitems[m] == refone);
    }
  }
  refidx = (int)l;
  return s;
}

template <class T, class R>
inline  bool  array<T, R>::Search( R  refone, int&  refidx, int (*compare)( T& r1, T& r2 ) )
{
  int   l = 0;
  int   h = ncount - 1;
  int   m;
  bool  s = false;

  while ( l <= h )
  {
    int   r;

    m = ( l + h ) >> 1;
    r = compare( pitems[m], refone );

    if ( r < 0 ) l = m + 1;
      else
    {
      h = m - 1;
      s |= (r == 0);
    }
  }
  refidx = (int)l;
  return s;
}

template <class T, class R>
inline  void  array<T, R>::Resort( int (*compare)( T& r1, T& r2 ) )
{
  if ( pitems && ncount )
    qsort( pitems, ncount, sizeof(T), (int (*)( const void*, const void* ))compare );
}

template <class T, class R>
inline  array<T, R>::operator T* ()
{
  return pitems;
}

template <class T, class R>
inline  array<T, R>::operator const T* () const
{
  return pitems;
}

template <class T, class R>
inline  T&    array<T, R>::operator [] ( int nindex )
{
  assert( nindex < ncount && nindex >= 0 );
  return pitems[nindex];
}

template <class T, class R>
inline  const T&  array<T, R>::operator [] ( int nindex ) const
{
  assert( nindex < ncount && nindex >= 0 );
  return pitems[nindex];
}

template <class T, class R>
int   array<T,R>::GetLimit() const
{
  return nlimit;
}

template <class T, class R>
int   array<T, R>::GetDelta() const
{
  return ndelta;
}

template <class T, class R>
void  array<T, R>::SetDelta( int nDelta )
{
  ndelta = nDelta;
}

# if defined( _MSC_VER )
#   pragma warning( pop )
# endif // _MSC_VER

# endif  // __libmorph_array_h__
