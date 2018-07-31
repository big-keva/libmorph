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
# if !defined( __mtc_serialize__ )
# define  __mtc_serialize__
# include "serialize.decl.h"

namespace mtc
{

  template <class O>
  O*  serialbuf::Serialize( O* o ) const {  return ::Serialize( o, data, size );  }

}

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

constexpr inline  size_t  GetBufLen( char )           {  return 1;  }
constexpr inline  size_t  GetBufLen( unsigned char )  {  return 1;  }
constexpr inline  size_t  GetBufLen( bool )           {  return 1;  }
constexpr inline  size_t  GetBufLen( float )          {  return sizeof(float);  }
constexpr inline  size_t  GetBufLen( double )         {  return sizeof(double);  }

inline  size_t  GetBufLen( const char*  string )
  {
    auto length = strlen( string );

    return sizeof(*string) * length + GetBufLen( length );
  }

inline  size_t  GetBufLen( char* string )
  {
    return GetBufLen( (const char*)string );
  }

template <class T>
inline  size_t  GetBufLen( const std::vector<T>& a )
  {
    size_t  cc = ::GetBufLen( a.size() );

    for ( auto& t: a )
      cc += ::GetBufLen( t );

    return cc;
  }

template <class C>
inline  size_t  GetBufLen( const std::basic_string<C>& s )
  {
    return ::GetBufLen( s.length() ) + sizeof(C) * s.length();
  }

//[]=========================================================================[]

inline  char*           Serialize( char* o, const void* p, size_t l )
  {
    return o != nullptr ? l + (char*)memcpy( o, p, l ) : nullptr;
  }

inline  unsigned char*  Serialize( unsigned char* o, const void* p, size_t l )
  {
    return o != nullptr ? l + (unsigned char*)memcpy( o, p, l ) : nullptr;
  }

inline  FILE*           Serialize( FILE* o, const void* p, size_t l )
  {
    return o != nullptr && fwrite( p, sizeof(char), l, o ) == l ? o : nullptr;
  }

//[]=========================================================================[]

inline  const char* FetchFrom( const char* s, void* p, size_t l )
  {
    return s != nullptr ? (memcpy( p, s, l ), l + s) : nullptr;
  }

inline  const unsigned char* FetchFrom( const unsigned char* s, void* p, size_t l )
  {
    return s != nullptr ? (memcpy( p, s, l ), l + s) : nullptr;
  }

inline  FILE*       FetchFrom( FILE* s, void* p, size_t l )
  {
    return s != nullptr && fread( p, sizeof(char), l, s ) == l ? s : nullptr;
  }

//[]=========================================================================[]

template <class O>  O*  Serialize( O* o, char c )                         {  return Serialize( o, &c, sizeof(c) );  }
template <class O>  O*  Serialize( O* o, unsigned char c )                {  return Serialize( o, &c, sizeof(c) );  }
template <class O>  O*  Serialize( O* o, float  f )                       {  return Serialize( o, &f, sizeof(f) );  }
template <class O>  O*  Serialize( O* o, double d )                       {  return Serialize( o, &d, sizeof(d) );  }
template <class O>  O*  Serialize( O* o, bool b )                         {  return Serialize( o, (char)(b ? 1 : 0) );  }

template <class O,
          class T>  O*  Serialize( O*  o, T t )
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

template <class O>  O*  Serialize( O* o, const char* s )
  {
    auto  length = strlen( s );

    return Serialize( Serialize( o, length ), (const void*)s, sizeof(*s) * length );
  }

template <class O>  O*  Serialize( O* o, char* s )
  {
    return Serialize( o, (const char*)s );
  }

template <class O,
          class T>  O*  Serialize( O* o, const std::vector<T>& a )
  {
    o = ::Serialize( o, a.size() );

    for ( auto p = a.begin(); p != a.end() && o != nullptr; ++p )
      o = ::Serialize( o, *p );

    return o;
  }

template <class O,
          class C>  O*  Serialize( O* o, const std::basic_string<C>& s )
  {
    return ::Serialize( ::Serialize( o, s.length() ), s.c_str(), sizeof(C) * s.length() );
  }

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

template <class S,
          class T>  S*  FetchFrom( S* s, T& t )
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

template <class S>  S*  FetchFrom( S* s, char*&  r )
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

template <class S>  S*  FetchFrom( S* s, const char*& r )
  {
    return FetchFrom( s, (char*&)r );
  }

template <class S,
          class T>  S*  FetchFrom( S* s, std::vector<T>& a )
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

template <class S,
          class C>  S*  FetchFrom( S* s, std::basic_string<C>& o )
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

# endif  // __mtc_serialize__
