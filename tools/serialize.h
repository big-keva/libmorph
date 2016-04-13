# if !defined( __serialize_h__ )
# define  __serialize_h__
# include <string.h>
# include <stdlib.h>
# include <stdio.h>
# include <errno.h>

//[]=========================================================================[]

template <class T>

inline  unsigned  GetBufLen( T dwdata )
  {
    T         bitest = 0x007f;
    unsigned  ncount = 1;

    while ( (dwdata & ~bitest) != 0 )
    {
      bitest = (T)((bitest << 7) | 0x7f);
        ++ncount;
    }
    return ncount;
  }

inline  unsigned    GetBufLen(  char  )
  {
    return 1;
  }

inline  unsigned    GetBufLen( unsigned char )
  {
    return 1;
  }

inline  unsigned    GetBufLen(  bool  )
  {
    return 1;
  }

inline  unsigned    GetBufLen( float )
  {
    return sizeof(float);
  }

inline  unsigned    GetBufLen( double )
  {
    return sizeof(double);
  }

inline  unsigned  GetBufLen( const char*  string )
  {
    unsigned length = strlen( string );

    return sizeof(*string) * length + GetBufLen( length );
  }

inline  unsigned  GetBufLen( char*        string )
  {
    return GetBufLen( (const char*)string );
  }

//[]=========================================================================[]

inline  char* Serialize( char* o, char c )
  {
    if ( o != NULL )
      *o++ = c;
    return o;
  }

inline  char* Serialize( char* o, const void* p, unsigned l )
  {
    return o != NULL ? l + (char*)memcpy( o, p, l ) : NULL;
  }

inline  FILE* Serialize( FILE* o, char c )
  {
    return o != NULL && fwrite( &c, sizeof(char), 1, o ) == 1 ? o : NULL;
  }

inline  FILE* Serialize( FILE* o, const void* p, unsigned l )
  {
    return o != NULL && fwrite( p, sizeof(char), l, o ) == l ? o : NULL;
  }

template <class O>  inline  O* Serialize( O* o, unsigned char b )
  {
    return Serialize( o, (char)b );
  }

template <class O>  inline  O* Serialize( O* o, bool    b )
  {
    return Serialize( o, (char)(b ? 1 : 0) );
  }

template <class O>  inline  O* Serialize( O* o, float   f )
  {
    return Serialize( o, &f, sizeof(f) );
  }

template <class O>  inline  O* Serialize( O* o, double  d )
  {
    return Serialize( o, &d, sizeof(d) );
  }

template <class O, class T>
inline  O*  Serialize( O*  o, T t )
  {
    int   nshift = 0;
    char  bstore;
  
    do
    {
      unsigned  ushift = nshift++ * 7;

      bstore = (char)(((t & (((T)0x7f) << ushift)) >> ushift) & 0x7f);
        t &= ~(((T)0x7f) << ushift);
      if ( t != 0 )
        bstore |= 0x80;
      o = Serialize( o, bstore );
    } while ( o != NULL && (bstore & 0x80) != 0 );

    return o;
  }

template <class O>  inline  O* Serialize( O* o, const char* s )
  {
    unsigned length = strlen( s );

    return Serialize( Serialize( o, length ), (const void*)s, sizeof(*s) * length );
  }

template <class O>  inline  O*  Serialize( O* o, char* s )
  {
    return Serialize( o, (const char*)s );
  }

//[]=========================================================================[]

template <class T>
inline  int       FetchData( const char*& buffer, T& rfitem )
  {
    int   nshift = 0;
    char  bfetch;

    rfitem = 0;
    do  rfitem |= (((T)(bfetch = *buffer++) & 0x7f)) << (nshift++ * 7);
      while ( bfetch & 0x80 );
    return 0;
  }

inline  int       FetchData( const char*& buffer, char&   rfitem )
  {
    rfitem = *buffer++;
      return 0;
  }

inline  int       FetchData( const char*& buffer, unsigned char& rfitem )
  {
    rfitem = (unsigned char)*buffer++;
      return 0;
  }

inline  int       FetchData( const char*& buffer, bool&   rfitem )
  {
    rfitem = *buffer++ != 0;
      return 0;
  }

inline  int       FetchData( const char*& buffer, float&  rvalue )
  {
    memcpy( &rvalue, buffer, sizeof(rvalue) );
      buffer += sizeof(rvalue);
    return 0;
  }

inline  int       FetchData( const char*& buffer, double& rvalue )
  {
    memcpy( &rvalue, buffer, sizeof(rvalue) );
      buffer += sizeof(rvalue);
    return 0;
  }

inline  int       FetchData( const char*& buffer, const char*&  string )
  {
    unsigned  length;

    if ( (length = (unsigned)*buffer++) & 0x80 )
      length = (length & 0x7f) | ((unsigned)*buffer++ << 7);
    if ( (string = (char*)malloc( length + 1 )) == NULL )
      return ENOMEM;
    memcpy( (char*)string, buffer, length );
      buffer += length;
    ((char*)string)[length] = '\0';
      return 0;
  }

inline  int       FetchData( const char*& buffer, char*&  string )
  {
    return FetchData( buffer, (const char*&)string );
  }

//[]=========================================================================[]

inline  const char* FetchFrom( const char* s, char& c )
  {
    if ( s != NULL )
      c = *s++;
    return s;
  }

inline  const char* FetchFrom( const char* s, void* p, unsigned l )
  {
    if ( s == NULL )
      return NULL;
    memcpy( p, s, l );
      return s + l;
  }

inline  FILE*       FetchFrom( FILE* s, char& c )
  {
    return s != NULL && fread( &c, sizeof(char), 1, s ) == 1 ? s : NULL;
  }

inline  FILE* FetchFrom( FILE* s, void* p, unsigned l )
  {
    return s != NULL && fread( p, sizeof(char), l, s ) == l ? s : NULL;
  }

template <class S> inline  S* FetchFrom( S* s, unsigned char& b )
  {
    return FetchFrom( s, (char&)b );
  }

template <class S> inline  S* FetchFrom( S* s, bool& b )
  {
    char  c;

    if ( (s = FetchFrom( s, c )) != NULL )
      b = c != 0;
    return s;
  }

template <class S> inline  S* FetchFrom( S* s, float& f )
  {
    return FetchFrom( s, &f, sizeof(f) );
  }

template <class S> inline  S* FetchFrom( S* s, double& d )
  {
    return FetchFrom( s, &d, sizeof(d) );
  }

template <class S, class T>
inline  S*  FetchFrom( S* s, T& t )
  {
    int   nshift = 0;
    char  bfetch;

    t = 0;
    do  {
      if ( (s = FetchFrom( s, bfetch )) == NULL ) return NULL;
        else  t |= (((T)bfetch & 0x7f)) << (nshift++ * 7);
    } while ( bfetch & 0x80 );

    return s;
  }

template <class S> inline  S* FetchFrom( S* s, const char*& r )
  {
    unsigned  length;

    if ( (s = FetchFrom( s, length )) == NULL )
      return NULL;
    if ( (r = (char*)malloc( length + 1 )) == NULL )
      return NULL;
    if ( (s = FetchFrom( s, r, length )) == NULL ) free( r );
      else  r[length] = '\0';
    return s;
  }

template <class S> inline  S* FetchFrom( S* s, char*&  r )
  {
    return FetchFrom( s, (const char*&)r );
  }

# endif  // __serialize_h__
