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
# if !defined( __mtc_serialize_decl_h__ )
# define  __mtc_serialize_decl_h__
# include <stdlib.h>
# include <string.h>
# include <stdio.h>
# include <string>
# include <vector>
# include <map>

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
    template <class O> O*  Serialize( O* o ) const;
  };

}

inline  mtc::sourcebuf*  FetchFrom( mtc::sourcebuf* s, void* p, size_t l )
  {  return s != nullptr ? s->FetchFrom( p, l ) : nullptr;  }

//[]=========================================================================[]

inline  char*                 Serialize( char*, const void*, size_t );
inline  unsigned char*        Serialize( unsigned char*, const void*, size_t );
inline  FILE*                 Serialize( FILE*, const void*, size_t );

inline  const char*           FetchFrom( const char*, void*, size_t );
inline  const unsigned char*  FetchFrom( const unsigned char*, void*, size_t );
inline  FILE*                 FetchFrom( FILE*, void*, size_t );

//[]=========================================================================[]

template <class T>
inline  size_t  GetBufLen( T );

constexpr inline  size_t  GetBufLen( char );
constexpr inline  size_t  GetBufLen( unsigned char );
constexpr inline  size_t  GetBufLen( bool );
constexpr inline  size_t  GetBufLen( float );
constexpr inline  size_t  GetBufLen( double );
          inline  size_t  GetBufLen( const char* );
          inline  size_t  GetBufLen( char* );

template <class T>
inline  size_t  GetBufLen( const std::vector<T>& );

template <class C>
inline  size_t  GetBufLen( const std::basic_string<C>& );

template <class K, class V>
inline  size_t  GetBufLen( const std::map<K, V>& );

//[]=========================================================================[]

template <class O>  O*  Serialize( O*, char );
template <class O>  O*  Serialize( O*, unsigned char );
template <class O>  O*  Serialize( O*, float );
template <class O>  O*  Serialize( O*, double );
template <class O>  O*  Serialize( O*, bool );

template <class O,
          class T>  O*  Serialize( O*, T );

template <class O>  O*  Serialize( O*, const char* );
template <class O>  O*  Serialize( O*, char* );

template <class O,
          class T>  O*  Serialize( O*, const std::vector<T>& );

template <class O,
          class C>  O*  Serialize( O*, const std::basic_string<C>& );

template <class O,
          class K,
          class V>  O*  Serialize( O*, const std::map<K, V>& );

//[]=========================================================================[]

template <class S>  S*  FetchFrom( S*, char& );
template <class S>  S*  FetchFrom( S*, unsigned char& );
template <class S>  S*  FetchFrom( S*, float& );
template <class S>  S*  FetchFrom( S*, double& );
template <class S>  S*  FetchFrom( S*, bool& );

template <class S,
          class T>  S*  FetchFrom( S*, T& );

template <class S>  S*  FetchFrom( S*, char*& );
template <class S>  S*  FetchFrom( S*, const char*& );

template <class S,
          class T>  S*  FetchFrom( S*, std::vector<T>& );

template <class S,
          class C>  S*  FetchFrom( S*, std::basic_string<C>& );

template <class S,
          class K,
          class V>  S*  FetchFrom( S*, std::map<K, V>& );

# endif  // __mtc_serialize_decl_h__
