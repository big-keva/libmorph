/*

The MIT License (MIT)

Copyright (c) 2000-2016 Андрей Коваленко aka Keva
  keva@meta.ua
  keva@rambler.ru
  skype: big_keva
  phone: +7(495)648-4058, +7(916)015-5592

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

=============================================================================

Данная лицензия разрешает лицам, получившим копию данного программного обеспечения
и сопутствующей документации (в дальнейшем именуемыми «Программное Обеспечение»),
безвозмездно использовать Программное Обеспечение без ограничений, включая неограниченное
право на использование, копирование, изменение, слияние, публикацию, распространение,
сублицензирование и/или продажу копий Программного Обеспечения, а также лицам, которым
предоставляется данное Программное Обеспечение, при соблюдении следующих условий:

Указанное выше уведомление об авторском праве и данные условия должны быть включены во
все копии или значимые части данного Программного Обеспечения.

ДАННОЕ ПРОГРАММНОЕ ОБЕСПЕЧЕНИЕ ПРЕДОСТАВЛЯЕТСЯ «КАК ЕСТЬ», БЕЗ КАКИХ-ЛИБО ГАРАНТИЙ,
ЯВНО ВЫРАЖЕННЫХ ИЛИ ПОДРАЗУМЕВАЕМЫХ, ВКЛЮЧАЯ ГАРАНТИИ ТОВАРНОЙ ПРИГОДНОСТИ,
СООТВЕТСТВИЯ ПО ЕГО КОНКРЕТНОМУ НАЗНАЧЕНИЮ И ОТСУТСТВИЯ НАРУШЕНИЙ, НО НЕ ОГРАНИЧИВАЯСЬ
ИМИ.

НИ В КАКОМ СЛУЧАЕ АВТОРЫ ИЛИ ПРАВООБЛАДАТЕЛИ НЕ НЕСУТ ОТВЕТСТВЕННОСТИ ПО КАКИМ-ЛИБО ИСКАМ,
ЗА УЩЕРБ ИЛИ ПО ИНЫМ ТРЕБОВАНИЯМ, В ТОМ ЧИСЛЕ, ПРИ ДЕЙСТВИИ КОНТРАКТА, ДЕЛИКТЕ ИЛИ ИНОЙ
СИТУАЦИИ, ВОЗНИКШИМ ИЗ-ЗА ИСПОЛЬЗОВАНИЯ ПРОГРАММНОГО ОБЕСПЕЧЕНИЯ ИЛИ ИНЫХ ДЕЙСТВИЙ
С ПРОГРАММНЫМ ОБЕСПЕЧЕНИЕМ.

*/
# if !defined( __libmorph_serialize__ )
# define  __libmorph_serialize__
# include <stdlib.h>
# include <string.h>
# include <stdio.h>
# include <errno.h>
# include <vector>
# include <string>

namespace mtc
{

  class sourcebuf
  {
    const char* p;
    const char* e;

  public:     // construction
    sourcebuf( const void* t = nullptr, size_t l = 0 ) noexcept: p( (char*)t ), e( l + (char*)t ) {}
    sourcebuf( const sourcebuf& s ): p( s.p ), e( s.e ) {}
    sourcebuf&  operator = ( const sourcebuf& s )
      {
        p = s.p;
        e = s.e;
        return *this;
      }
    sourcebuf* ptr() const {  return (sourcebuf*)this;  }
    operator sourcebuf* () const    {  return ptr();  }
    const char* getptr() const      {  return p < e ? p : nullptr;  }
    sourcebuf*  skipto( size_t l )  {  return (p = l + p) <= e ? this : nullptr;  }

  public:     // fetch
    sourcebuf*  FetchFrom( void* o, size_t l )  {  return p + l <= e ? (memcpy( o, p, l ), p += l, this) : nullptr;  }
  };

  class serialbuf
  {
    const void* data;
    size_t      size;

  public:
    serialbuf( const void* p, size_t l ): data( p ), size( l ) {}
    serialbuf( const serialbuf& s ): data( s.data ), size( s.size ) {}

  public:
    template <class O>
    O*  Serialize( O* o ) const {  return ::Serialize( o, data, size );  }
  };

}

inline  mtc::sourcebuf*  FetchFrom( mtc::sourcebuf* s, void* p, size_t l )
  {  return s != nullptr ? s->FetchFrom( p, l ) : nullptr;  }

//[]=========================================================================[]

template <class T>
inline  size_t  GetBufLen( T dwdata )
  {
    T       bitest = 0x007f;
    size_t  ncount = 1;

    while ( (dwdata & ~bitest) != 0 )
    {
      bitest = (T)((bitest << 7) | 0x7f);
        ++ncount;
    }
    return ncount;
  }

inline  size_t  GetBufLen(  char  )
  {
    return 1;
  }

inline  size_t  GetBufLen( unsigned char )
  {
    return 1;
  }

inline  size_t  GetBufLen(  bool  )
  {
    return 1;
  }

inline  size_t  GetBufLen( float )
  {
    return sizeof(float);
  }

inline  size_t  GetBufLen( double )
  {
    return sizeof(double);
  }

inline  size_t  GetBufLen( const char*  string )
  {
    auto length = strlen( string );

    return sizeof(*string) * length + GetBufLen( length );
  }

inline  size_t  GetBufLen( char*        string )
  {
    return GetBufLen( (const char*)string );
  }

//[]=========================================================================[]

inline  char*       Serialize( char* o, const void* p, size_t l )
  {
    return o != nullptr ? l + (char*)memcpy( o, p, l ) : nullptr;
  }
inline  const char* FetchFrom( const char* s, void* p, size_t l )
  {
    return s != nullptr ? (memcpy( p, s, l ), l + s) : nullptr;
  }
inline  const unsigned char* FetchFrom( const unsigned char* s, void* p, size_t l )
  {
    return s != nullptr ? (memcpy( p, s, l ), l + s) : nullptr;
  }
inline  FILE*       Serialize( FILE* o, const void* p, size_t l )
  {
    return o != nullptr && fwrite( p, sizeof(char), l, o ) == l ? o : NULL;
  }
inline  FILE*       FetchFrom( FILE* s, void* p, size_t l )
  {
    return s != nullptr && fread( p, sizeof(char), l, s ) == l ? s : nullptr;
  }

template <class O>  O*  Serialize( O* o, char c )                         {  return Serialize( o, &c, sizeof(c) );  }
template <class O>  O*  Serialize( O* o, unsigned char c )                {  return Serialize( o, &c, sizeof(c) );  }
template <class O>  O*  Serialize( O* o, float  f )                       {  return Serialize( o, &f, sizeof(f) );  }
template <class O>  O*  Serialize( O* o, double d )                       {  return Serialize( o, &d, sizeof(d) );  }
template <class O>  O*  Serialize( O* o, bool b )                         {  return Serialize( o, (char)(b ? 1 : 0) );  }

template <class O, class T>
O*  Serialize( O*  o, T t )
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
      o = Serialize( o, &bstore, sizeof(bstore) );
    } while ( o != NULL && (bstore & 0x80) != 0 );

    return o;
  }

template <class O>
O*  Serialize( O* o, const char* s )
  {
    auto  length = strlen( s );

    return Serialize( Serialize( o, length ), (const void*)s, sizeof(*s) * length );
  }

template <class O>
O*  Serialize( O* o, char* s )      {  return Serialize( o, (const char*)s );  }

//[]=========================================================================[]

template <class S>  S*  FetchFrom( S* s, char& c )                        {  return FetchFrom( s, &c, sizeof(c) );  }
template <class S>  S*  FetchFrom( S* s, unsigned char& c )               {  return FetchFrom( s, &c, sizeof(c) );  }
template <class S>  S*  FetchFrom( S* s, float&  f )                      {  return FetchFrom( s, &f, sizeof(f) );  }
template <class S>  S*  FetchFrom( S* s, double& d )                      {  return FetchFrom( s, &d, sizeof(d) );  }
template <class S>  S*  FetchFrom( S* s, bool& b )
  {
    char  c;

    s = FetchFrom( s, c );
    b = s != nullptr && c != 0;
    return s;
  }

template <class S, class T>
S*  FetchFrom( S* s, T& t )
  {
    int   nshift = 0;
    char  bfetch;

    t = 0;
    do  {
      if ( (s = FetchFrom( s, &bfetch, sizeof(bfetch) )) == nullptr ) return nullptr;
        else  t |= (((T)bfetch & 0x7f)) << (nshift++ * 7);
    } while ( bfetch & 0x80 );

    return s;
  }

template <class S>
S*  FetchFrom( S* s, char*&  r )
  {
    unsigned  length;

    if ( (s = FetchFrom( s, length )) == nullptr )
      return nullptr;
    if ( (r = (char*)malloc( length + 1 )) == nullptr )
      return nullptr;
    if ( (s = FetchFrom( s, r, length )) == nullptr ) free( r );
      else  r[length] = '\0';
    return s;
  }

template <class S>
S*  FetchFrom( S* s, const char*& r )
  {
    return FetchFrom( s, (char*&)r );
  }

/*
  vectors serialization/deserialization
*/

template <class T>
size_t  GetBufLen( const std::vector<T>& a )
  {
    size_t  cc = ::GetBufLen( a.size() );

    for ( auto& t: a )
      cc += ::GetBufLen( t );

    return cc;
  }

template <class O, class T>
O*  Serialize( O* o, const std::vector<T>& a )
  {
    o = ::Serialize( o, a.size() );

    for ( auto p = a.begin(); p != a.end() && o != nullptr; ++p )
      o = ::Serialize( o, *p );

    return o;
  }

template <class S, class T>
inline  S*  FetchFrom( S* s, std::vector<T>& a )
  {
    int   length;

    a.clear();

    if ( (s = ::FetchFrom( s, length )) == nullptr )
      return s;

    a.reserve( (length + 0x0f) & ~0x0f );
    a.resize( length );

    for ( auto i = 0; i < length && s != nullptr; ++i )
      s = ::FetchFrom( s, a.at( i ) );

    return s;
  }

/*
  strings serialization/deserialization
*/

template <class O, class C>
O*  Serialize( O* o, const std::basic_string<C>& s )
  {
    return ::Serialize( ::Serialize( o, s.length() ), s.c_str(), sizeof(C) * s.length() );
  }

template <class S, class C>
S*  FetchFrom( S* s, std::basic_string<C>& o )
  {
    int   l;

    o.clear();

    if ( (s = ::FetchFrom( s, l )) == nullptr )
      return nullptr;

    o.reserve( (l + 0x10) & ~0x0f );
    o.resize( l );
    o[l] = (C)0;

    if ( (s = ::FetchFrom( s, (C*)o.c_str(), l * sizeof(C) )) == nullptr )
      o.clear();

    return s;
  }

# endif  // __libmorph_serialize__
