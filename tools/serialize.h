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
# include <cstdlib>
# include <cstring>
# include <string>
# include <vector>
# include <list>
# include <map>
# include <utility>
# include <tuple>

/*
 * values serialization/deserialization
 */

template <class T>
size_t  GetBufLen( const T& );
template <class O, class T>
O*      Serialize( O*, const T& );
template <class S, class T>
S*      FetchFrom( S*, T& );
template <class S, class T>
S*      SkipToEnd( S*, const T* );

/*
 * common serialization templates declaration for base get/put operations   []
 */

template <class O>  O*  Serialize( O*, const void*, size_t );
template <class S>  S*  FetchFrom( S*,       void*, size_t );
template <class S>  S*  SkipBytes( S*, size_t );

/*
 * std:: types serialization/deserialization declarations
 */

template <class C>  size_t  GetBufLen( const std::basic_string<C>& );
template <class T,
          class A>  size_t  GetBufLen( const std::vector<T, A>& );
template <class T>  size_t  GetBufLen( const std::list<T>& );
template <class K,
          class V>  size_t  GetBufLen( const std::map<K, V>& );
template <class T1,
          class T2> size_t  GetBufLen( const std::pair<T1, T2>& );
template <
       class ... T> size_t  GetBufLen( const std::tuple<T...>& );

template <class O,
          class C>  O*  Serialize( O*, const std::basic_string<C>& );
template <class O,
          class T,
          class A>  O*  Serialize( O* o, const std::vector<T, A>& );
template <class O,
          class T>  O*  Serialize( O* o, const std::list<T>& );
template <class O,
          class K,
          class V>  O*  Serialize( O* o, const std::map<K, V>& );
template <class O,
          class T1,
          class T2> O*  Serialize( O*, const std::pair<T1, T2>& );
template <class O,
       class ... T> O*  Serialize( O* o, const std::tuple<T...>& );

template <class S,
          class C>  S*  FetchFrom( S*, std::basic_string<C>& );
template <class S,
          class T>  S*  FetchFrom( S*, std::vector<T>& );
template <class S,
          class T>  S*  FetchFrom( S*, std::list<T>& );
template <class S,
          class K,
          class V>  S*  FetchFrom( S*, std::map<K, V>& );
template <class S,
          class T1,
          class T2> S*  FetchFrom( S*, std::pair<T1, T2>& );
template <class S,
      class ... T> S*  FetchFrom( S*, std::tuple<T...>& );

template <class S,
          class C>  S*  SkipToEnd( S*, const std::basic_string<C>* );
template <class S,
          class T>  S*  SkipToEnd( S*, const std::vector<T>* );
template <class S,
          class T>  S*  SkipToEnd( S*, const std::list<T>* );
template <class S,
          class K,
          class V>  S*  SkipToEnd( S*, const std::map<K, V>* );
template <class S,
          class T1,
          class T2> S*  SkipToEnd( S*, const std::pair<T1, T2>* );
template <class S,
      class ... T> S*  SkipToEnd( S*, const std::tuple<T...>* );

namespace mtc
{
  class sourcebuf
  {
    const char* p;
    const char* e;

  public:     // construction
    sourcebuf( const void* t = nullptr, size_t l = 0 ) noexcept:
      p( (const char*)t ), e( l + (const char*)t ) {}
    sourcebuf( const sourcebuf& s ):
      p( s.p ), e( s.e ) {}
    sourcebuf&  operator = ( const sourcebuf& s )
    {
      p = s.p;
      e = s.e;
      return *this;
    }
    sourcebuf* ptr() const {  return const_cast<sourcebuf*>( this );  }
    operator sourcebuf* () const    {  return ptr();  }
    const char* getptr() const      {  return p < e ? p : nullptr;  }
    sourcebuf*  skipto( size_t l )  {  return (p = l + p) <= e ? this : nullptr;  }

  public:     // fetch
    sourcebuf*  FetchFrom( void* o, size_t l )  {  return p + l <= e ? (memcpy( o, p, l ), p += l, this) : (p = e, nullptr);  }
  };

  class serialbuf
  {
    const void* data;
    size_t      size;

    auto  operator = ( const serialbuf& ) -> serialbuf& = delete;
  public:
    serialbuf( const void* p, size_t l ): data( p ), size( l ) {}
    serialbuf( const serialbuf& s ): data( s.data ), size( s.size ) {}

  public:
    template <class O> O*  Serialize( O* o ) const;
  };

  /*
   * integer values serialization
   */
  struct integers final
  {
    template <class T>
    static  inline  size_t  len( T dwdata )
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
    template <class O, class T>
    static  inline O*  put( O*  o, T t )
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
    template <class S, class T>
    static inline S*  get( S* s, T& t )
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
    template <class S, class T>
    static inline S*  end( S* s, const T* )
    {
      char  bfetch;
      while ( (s = FetchFrom( s, &bfetch, sizeof(bfetch) )) != nullptr && (bfetch & 0x80) != 0 )
        (void)NULL;
      return s;
    }
  };

}

/*
 * base i/o specializations declarations
 */

template <> inline  auto  Serialize( char* o, const void* p, size_t l ) -> char*
  {  return o != nullptr ? l + (char*)memcpy( o, p, l ) : nullptr;  }
template <> inline  auto  Serialize( unsigned char* o, const void* p, size_t l ) -> unsigned char*
  {  return o != nullptr ? l + (unsigned char*)memcpy( o, p, l ) : nullptr;  }
template <> inline  auto  Serialize( FILE* o, const void* p, size_t l ) -> FILE*
  {  return o != nullptr && fwrite( p, sizeof(char), l, o ) == l ? o : nullptr;  }

template <> inline  auto  FetchFrom( const char* s, void* p, size_t l ) -> const char*
  {  return s != nullptr ? (memcpy( p, s, l ), l + s) : nullptr;  }
template <> inline  auto  FetchFrom( const unsigned char* s, void* p, size_t l ) -> const unsigned char*
  {  return s != nullptr ? (memcpy( p, s, l ), l + s) : nullptr;  }
template <> inline  auto  FetchFrom( FILE* s, void* p, size_t l ) -> FILE*
  {  return s != nullptr && fread( p, sizeof(char), l, s ) == l ? s : nullptr;  }

template <> inline  auto  SkipBytes( const char* s, size_t l ) -> const char*
  {  return s != nullptr ? s + l : s;  }
template <> inline  auto  SkipBytes( const unsigned char* s, size_t l ) -> const unsigned char*
  {  return s != nullptr ? s + l : s;  }
template <> inline  auto  SkipBytes( FILE* s, size_t l ) -> FILE*
  {  return s != nullptr && fseek( s, l, SEEK_CUR ) == 0 ? s : nullptr;  }

/*
 * values serialization/deserialization for standard types
 */
template <>  constexpr inline  size_t GetBufLen( const char& )          {  return 1;  }
template <>  constexpr inline  size_t GetBufLen( const unsigned char& ) {  return 1;  }
template <>  constexpr inline  size_t GetBufLen( const float&  )        {  return sizeof(float);  }
template <>  constexpr inline  size_t GetBufLen( const double& )        {  return sizeof(double);  }
template <>  constexpr inline  size_t GetBufLen( const bool& )          {  return 1;  }

template <>  inline  size_t GetBufLen( const int16_t& i )   {  return mtc::integers::len( i );  }
template <>  inline  size_t GetBufLen( const int32_t& i )   {  return mtc::integers::len( i );  }
template <>  inline  size_t GetBufLen( const int64_t& i )   {  return mtc::integers::len( i );  }

template <>  inline  size_t GetBufLen( const uint16_t& i )  {  return mtc::integers::len( i );  }
template <>  inline  size_t GetBufLen( const uint32_t& i )  {  return mtc::integers::len( i );  }
template <>  inline  size_t GetBufLen( const uint64_t& i )  {  return mtc::integers::len( i );  }

template <class O>  inline  O*  Serialize( O* o, const char& c )          {  return Serialize( o, &c, sizeof(c) );  }
template <class O>  inline  O*  Serialize( O* o, const unsigned char& c ) {  return Serialize( o, &c, sizeof(c) );  }
template <class O>  inline  O*  Serialize( O* o, const float&  f )        {  return Serialize( o, &f, sizeof(f) );  }
template <class O>  inline  O*  Serialize( O* o, const double& d )        {  return Serialize( o, &d, sizeof(d) );  }
template <class O>  inline  O*  Serialize( O* o, const bool& b )          {  return Serialize( o, (char)(b ? 1 : 0) );  }

template <class O>  inline  O*  Serialize( O* o, const int16_t& i )   {  return mtc::integers::put( o, i );  }
template <class O>  inline  O*  Serialize( O* o, const int32_t& i )   {  return mtc::integers::put( o, i );  }
template <class O>  inline  O*  Serialize( O* o, const int64_t& i )   {  return mtc::integers::put( o, i );  }

template <class O>  inline  O*  Serialize( O* o, const uint16_t& i )  {  return mtc::integers::put( o, i );  }
template <class O>  inline  O*  Serialize( O* o, const uint32_t& i )  {  return mtc::integers::put( o, i );  }
template <class O>  inline  O*  Serialize( O* o, const uint64_t& i )  {  return mtc::integers::put( o, i );  }

template <class S>  inline  S*  FetchFrom( S* s, char& c )          {  return FetchFrom( s, &c, sizeof(c) );  }
template <class S>  inline  S*  FetchFrom( S* s, unsigned char& c ) {  return FetchFrom( s, &c, sizeof(c) );  }
template <class S>  inline  S*  FetchFrom( S* s, float&  f )        {  return FetchFrom( s, &f, sizeof(f) );  }
template <class S>  inline  S*  FetchFrom( S* s, double& d )        {  return FetchFrom( s, &d, sizeof(d) );  }
template <class S>  inline  S*  FetchFrom( S* s, bool& b )
{
  char  c;

  return b = (s = FetchFrom( s, c )) != nullptr && c != 0, s;
}

template <class S>  inline  S*  FetchFrom( S* s, int16_t& i )   {  return mtc::integers::get( s, i );  }
template <class S>  inline  S*  FetchFrom( S* s, int32_t& i )   {  return mtc::integers::get( s, i );  }
template <class S>  inline  S*  FetchFrom( S* s, int64_t& i )   {  return mtc::integers::get( s, i );  }

template <class S>  inline  S*  FetchFrom( S* s, uint16_t& i )  {  return mtc::integers::get( s, i );  }
template <class S>  inline  S*  FetchFrom( S* s, uint32_t& i )  {  return mtc::integers::get( s, i );  }
template <class S>  inline  S*  FetchFrom( S* s, uint64_t& i )  {  return mtc::integers::get( s, i );  }

template <class S>  inline  S*  SkipToEnd( S* o, const char* )          {  return SkipBytes( o, sizeof(char) );  }
template <class S>  inline  S*  SkipToEnd( S* o, const unsigned char* ) {  return SkipBytes( o, sizeof(char) );  }
template <class S>  inline  S*  SkipToEnd( S* o, const float* )         {  return SkipBytes( o, sizeof(float) );  }
template <class S>  inline  S*  SkipToEnd( S* o, const double* )        {  return SkipBytes( o, sizeof(double) );  }
template <class S>  inline  S*  SkipToEnd( S* o, const bool* )          {  return SkipBytes( o, sizeof(char) );  }

template <class S>  inline  S*  SkipToEnd( S* s, const int16_t* )   {  return mtc::integers::end( s, (const int16_t*)nullptr );  }
template <class S>  inline  S*  SkipToEnd( S* s, const int32_t* )   {  return mtc::integers::end( s, (const int32_t*)nullptr );  }
template <class S>  inline  S*  SkipToEnd( S* s, const int64_t* )   {  return mtc::integers::end( s, (const int64_t*)nullptr );  }

template <class S>  inline  S*  SkipToEnd( S* s, const uint16_t* )   {  return mtc::integers::end( s, (const uint16_t*)nullptr );  }
template <class S>  inline  S*  SkipToEnd( S* s, const uint32_t* )   {  return mtc::integers::end( s, (const uint32_t*)nullptr );  }
template <class S>  inline  S*  SkipToEnd( S* s, const uint64_t* )   {  return mtc::integers::end( s, (const uint64_t*)nullptr );  }

/*
 * C strings serialization/deserialization specializations
 */
template <>
inline  size_t  GetBufLen( const char* const& string )
{
  auto length = strlen( string );

  return sizeof(*string) * length + GetBufLen( length );
}

template <> inline
size_t  GetBufLen( char* const& string )
  {  return GetBufLen( (char * const&)string );  }

template <class O>  inline  O*  Serialize( O* o, const char* const& s )
{
  auto  length = strlen( s );

  return Serialize( Serialize( o, length ), (const void*)s, sizeof(*s) * length );
}

template <class S>  inline  S*  FetchFrom( S* s, char*&  r )
{
  unsigned  length;

  if ( (s = FetchFrom( s, length )) != nullptr )
  {
    if ( (r = (char*)malloc( length + 1 )) == nullptr )
      return nullptr;
    if ( (s = FetchFrom( s, r, length )) == nullptr ) free( r );
      else  r[length] = '\0';
  }
  return s;
}

template <class O>  inline  O*  Serialize( O* o, char* s )  {  return Serialize( o, (const char*)s );  }
template <class S>  inline  S*  FetchFrom( S* s, const char*& r ) {  return FetchFrom( s, (char*&)r );  }

/*
 * structures serialization/deserialization prototypes
 */
namespace mtc
{
  template <class T>
  struct class_is_string  {  static const bool value = false;  };
  template <class T>
  struct class_is_vector  {  static const bool value = false;  };
  template <class T>
  struct class_is_list    {  static const bool value = false;  };
  template <class T>
  struct class_is_map     {  static const bool value = false;  };
  template <class ... T>
  struct class_is_pair    {  static const bool value = false;  };
  template <class T>
  struct class_is_tuple   {  static const bool value = false;  };

  template <class T>
  struct class_is_string<std::basic_string<T>>  {  static const bool value = true;  };
  template <class T, class A>
  struct class_is_vector<std::vector<T, A>>     {  static const bool value = true;  };
  template <class T>
  struct class_is_list<std::list<T>>            {  static const bool value = true;  };
  template <class ... T>
  struct class_is_map<std::map<T...>>           {  static const bool value = true;  };
  template <class ... T>
  struct class_is_pair<std::pair<T...>>         {  static const bool value = true;  };
  template <class ... T>
  struct class_is_tuple<std::tuple<T...>>       {  static const bool value = true;  };

  struct value_as_scalar
  {
    template <class T> constexpr
    static  size_t  GetBufLen( const T& t )  {  return ::GetBufLen( t );  }
    template <class O, class T>
    static  O*      Serialize( O* o, const T& t )  {  return ::Serialize( o, t );  }
    template <class S, class T>
    static  S*      FetchFrom( S* s, T& t )  {  return ::FetchFrom( s, t );  }
    template <class S, class T>
    static  S*      SkipToEnd( S* s, const T* ) {  return ::SkipToEnd( s, (const T*)nullptr );  }
  };

  struct value_as_struct
  {
    template <class T> constexpr
    static  size_t  GetBufLen( const T& t )  {  return t.GetBufLen();  }
    template <class O, class T>
    static  O*      Serialize( O* o, const T& t )  {  return t.Serialize( o );  }
    template <class S, class T>
    static  S*      FetchFrom( S* s, T& t )  {  return t.FetchFrom( s );  }
    template <class S, class T>
    static  S*      SkipToEnd( S* s, const T* )  {  return T::SkipToEnd( s );  }
  };

  template <size_t I>
  struct tuple_iterator
  {
    template <class ... Types>
    static  size_t  GetBufLen( const std::tuple<Types...>& t )
    {
      return ::GetBufLen( std::get<sizeof...(Types) - I>( t ) )
        + tuple_iterator<I - 1>::GetBufLen( t );
    }
    template <class O, class ... Types>
    static  O*  Serialize( O* o, const std::tuple<Types...>& t )
    {
      return tuple_iterator<I - 1>::Serialize( ::Serialize( o,
        std::get<sizeof...(Types) - I>( t ) ), t );
    }
    template <class S, class ... Types>
    static  S*  FetchFrom( S* s, std::tuple<Types...>& t )
    {
      return tuple_iterator<I - 1>::FetchFrom( ::FetchFrom( s,
        std::get<sizeof...(Types) - I>( t ) ), t );
    }
    template <class S, class ... Types>
    static  S*  SkipToEnd( S* s, const std::tuple<Types...>* )
    {
      using nexttype = typename std::tuple_element<sizeof...(Types) - I, std::tuple<Types...>>::type;

      return tuple_iterator<I - 1>::SkipToEnd( ::SkipToEnd( s,
        (const nexttype*)nullptr ), (const std::tuple<Types...>*)nullptr );
    }
  };

  template <>
  struct tuple_iterator<0>
  {
    template <class ... Types>
    static  size_t  GetBufLen( const std::tuple<Types...>& ) {  return 0;  }
    template <class O, class ... Types>
    static  O*      Serialize( O* o, const std::tuple<Types...>& ) {  return o;  }
    template <class S, class ... Types>
    static  S*      FetchFrom( S* s, std::tuple<Types...>& ) {  return s;  }
    template <class S, class ... Types>
    static  S*      SkipToEnd( S* s, const std::tuple<Types...>* ) {  return s;  }
  };
}

template <class T>
size_t  GetBufLen( const T& t )
{
  using value_type = typename std::conditional<
    std::is_fundamental<T>::value ||
    mtc::class_is_string<T>::value ||
    mtc::class_is_vector<T>::value ||
    mtc::class_is_list<T>::value ||
    mtc::class_is_map<T>::value ||
    mtc::class_is_pair<T>::value ||
    mtc::class_is_tuple<T>::value, mtc::value_as_scalar, mtc::value_as_struct>::type;
  return value_type::GetBufLen( t );
}

template <class O, class T>
O*  Serialize( O* o, const T& t )
{
  using value_type = typename std::conditional<
    std::is_fundamental<T>::value ||
    mtc::class_is_string<T>::value ||
    mtc::class_is_vector<T>::value ||
    mtc::class_is_list<T>::value ||
    mtc::class_is_map<T>::value ||
    mtc::class_is_pair<T>::value ||
    mtc::class_is_tuple<T>::value, mtc::value_as_scalar, mtc::value_as_struct>::type;
  return value_type::Serialize( o, t );
}

template <class S, class T>
S*  FetchFrom( S* s, T& t )
{
  using value_type = typename std::conditional<
    std::is_fundamental<T>::value ||
    mtc::class_is_string<T>::value ||
    mtc::class_is_vector<T>::value ||
    mtc::class_is_list<T>::value ||
    mtc::class_is_map<T>::value ||
    mtc::class_is_pair<T>::value ||
    mtc::class_is_tuple<T>::value, mtc::value_as_scalar, mtc::value_as_struct>::type;
  return value_type::FetchFrom( s, t );
}

template <class S, class T>
S*  SkipToEnd( S* s, const T* )
{
  using value_type = typename std::conditional<
    std::is_fundamental<T>::value ||
    mtc::class_is_string<T>::value ||
    mtc::class_is_vector<T>::value ||
    mtc::class_is_map<T>::value ||
    mtc::class_is_pair<T>::value ||
    mtc::class_is_tuple<T>::value, mtc::value_as_scalar, mtc::value_as_struct>::type;
  return value_type::SkipToEnd( s, (const T*)nullptr );
}

/*
 * std:: types serialization/deserialization specializations
 */

/*
 * std::basic_string<>
 */
template <class C>
size_t  GetBufLen( const std::basic_string<C>& s )
{
  return ::GetBufLen( s.length() ) + sizeof(C) * s.length();
}

template <class O,
class C>  O*  Serialize( O* o, const std::basic_string<C>& s )
{
  return ::Serialize( ::Serialize( o, s.length() ), s.c_str(), sizeof(C) * s.length() );
}

template <class S, class C>
S*  FetchFrom( S* s, std::basic_string<C>& o )
{
  int   l;

  if ( (s = ::FetchFrom( s, l )) != nullptr )
    o.resize( l );

  if ( s == nullptr || (s = ::FetchFrom( s, (C*)o.c_str(), l * sizeof(C) )) == nullptr )
    o.clear();

  return s;
}

template <class S, class C>
S*  SkipToEnd( S* s, const std::basic_string<C>* )
{
  int   l;

  return (s = FetchFrom( s, l )) != nullptr ? SkipBytes( s, l * sizeof(C) ) : s;
}

/*
 * std::vector<>
 */
template <class T, class A>
size_t  GetBufLen( const std::vector<T, A>& v )
{
  auto  value_size = ::GetBufLen( v.size() );

  for ( auto& element: v )
    value_size += ::GetBufLen( element );

  return value_size;
}

template <class O, class T, class A>
O*  Serialize( O* o, const std::vector<T, A>& a )
{
  o = ::Serialize( o, a.size() );

  for ( auto& element: a )
    o = ::Serialize( o, element );

  return o;
}

template <class S, class T>
S*  FetchFrom( S* s, std::vector<T>& a )
{
  size_t  array_size;

  a.clear();

  if ( (s = ::FetchFrom( s, array_size )) == nullptr )
    return s;

  a.reserve( (array_size + 0x0f) & ~0x0f );
  a.resize( array_size );

  for ( size_t i = 0; i < array_size && s != nullptr; ++i )
    s = ::FetchFrom( s, a.at( i ) );

  return s;
}

template <class S, class T>
S*  SkipToEnd( S* s, const std::vector<T>* )
{
  int   l;

  for ( s = ::FetchFrom( s, l ); s != nullptr && l-- > 0; s = SkipToEnd( s, (const T*)nullptr ) )
    (void)NULL;
  return s;
}

/*
 * std::list<>
 */
template <class T>
size_t  GetBufLen( const std::list<T>& v )
{
  auto  value_size = ::GetBufLen( v.size() );

  for ( auto& element: v )
    value_size += ::GetBufLen( element );

  return value_size;
}

template <class O, class T>
O*  Serialize( O* o, const std::list<T>& a )
{
  o = ::Serialize( o, a.size() );

  for ( auto& element: a )
    o = ::Serialize( o, element );

  return o;
}

template <class S,
class T>
S*  FetchFrom( S* s, std::list<T>& a )
{
  size_t  list_len;

  a.clear();

  for ( s = ::FetchFrom( s, list_len ); list_len != 0 && s != nullptr; --list_len )
  {
    a.resize( a.size() + 1 );
    s = ::FetchFrom( s, a.back() );
  }

  return s;
}

template <class S, class T>
S*  SkipToEnd( S* s, const std::list<T>* )
{
  int   l;

  for ( s = ::FetchFrom( s, l ); s != nullptr && l-- > 0; s = SkipToEnd( s, (const T*)nullptr ) )
    (void)NULL;
  return s;
}

/*
 * std::map<>
 */
template <class K, class V>
size_t  GetBufLen( const std::map<K, V>& m )
{
  size_t  cch = ::GetBufLen( m.size() );

  for ( auto ptr = m.begin(); ptr != m.end(); ++ptr )
    cch += ::GetBufLen( ptr->first ) + ::GetBufLen( ptr->second );

  return cch;
}

template <class O, class K, class V>
O*  Serialize( O* o, const std::map<K, V>& m )
{
  o = ::Serialize( o, m.size() );

  for ( auto ptr = m.begin(); o != nullptr && ptr != m.end(); ++ptr )
    o = ::Serialize( ::Serialize( o, ptr->first ), ptr->second );

  return o;
}

template <class S, class K, class V>
S*  FetchFrom( S* s, std::map<K, V>& m )
{
  size_t  len;

  s = ::FetchFrom( s, len );

  for ( m.clear(); s != nullptr && len-- > 0; )
  {
    std::pair<K, V> pair;

    if ( (s = ::FetchFrom( ::FetchFrom( s, pair.first ), pair.second )) != nullptr )
      m.insert( m.end(), std::move( pair ) );
  }

  return s;
}

template <class S, class K, class V>
S*  SkipToEnd( S* s, const std::map<K, V>* )
{
  size_t  len;

  for ( s = ::FetchFrom( s, len ); s != nullptr && len-- > 0;
    s = SkipToEnd( SkipToEnd( s, (const K*)nullptr ), (const V*)nullptr ) ) (void)NULL;

  return s;
}

/*
 * std::pair<>
 */
template <class T1, class T2>
size_t  GetBufLen( const std::pair<T1, T2>& p )
{
  return ::GetBufLen( p.first ) + ::GetBufLen( p.second );
}

template <class O, class T1, class T2>
O*  Serialize( O* o, const std::pair<T1, T2>& p )
{
  return ::Serialize( ::Serialize( o, p.first ), p.second );
}

template <class S, class T1, class T2>
S*  FetchFrom( S* s, std::pair<T1, T2>& p )
{
  return ::FetchFrom( ::FetchFrom( s, p.first ), p.second );
}

template <class S, class T1, class T2>
S*  SkipToEnd( S* s, const std::pair<T1, T2>* )
{
  return ::SkipToEnd( ::SkipToEnd( s, (const T1*)nullptr ), (const T2*)nullptr );
}

/*
 * std::tuple
 */
template <class ... T>
size_t  GetBufLen( const std::tuple<T...>& t )
{
  return mtc::tuple_iterator<sizeof...(T)>::GetBufLen( t );
}

template <class O, class ... T>
O*  Serialize( O* o, const std::tuple<T...>& t )
{
  return mtc::tuple_iterator<sizeof...(T)>::Serialize( o, t );
}

template <class S, class ... T>
S*  FetchFrom( S* s, std::tuple<T...>& t )
{
  return mtc::tuple_iterator<sizeof...(T)>::FetchFrom( s, t );
}

template <class S, class ... T>
S*  SkipToEnd( S* s, const std::tuple<T...>* )
{
  return mtc::tuple_iterator<sizeof...(T)>::SkipToEnd( s, (const std::tuple<T...>*)nullptr );
}

/*
 * helpers for buffers
 */
template <> inline  auto  FetchFrom( mtc::sourcebuf* s, void* p, size_t l ) -> mtc::sourcebuf*
  {  return s != nullptr ? s->FetchFrom( p, l ) : s;  }
template <> inline  auto  SkipBytes( mtc::sourcebuf* s, size_t l ) -> mtc::sourcebuf*
  {  return s != nullptr ? s->skipto( l ) : s;  }

namespace mtc
{
  template <class O> O*  serialbuf::Serialize( O* o ) const  {  return ::Serialize( o, data, size );  }
}

# endif  // __mtc_serialize__
