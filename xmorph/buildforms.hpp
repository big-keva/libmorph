# if !defined( __libmorph_buildforms_hpp__ )
# define __libmorph_buildforms_hpp__
# include "xmorph/scandict.h"
# include <cstdint>

namespace libmorph {

  class Collector
  {
    char*   outbeg = nullptr;
    char*   outptr = nullptr;
    char*   outend = nullptr;

  public:
    Collector( char* beg, char* end ):
      outbeg( beg ),
      outptr( beg ),
      outend( end ) {}
    Collector( char* beg, size_t len ):
      Collector( beg, beg + len ) {}

    auto  get() -> Collector& {  return *this;  }

  public:
    auto  getptr() const -> const char*
      {  return outptr;  }
    bool  append( char c )
      {
        return outptr != outend ? (*outptr++ = c), true : false;
      }
    bool  append( const char* p, size_t l )
      {
        if ( l + outptr > outend )
          return false;
        while ( l-- > 0 )
          *outptr++ = *p++;
        return true;
      }

    bool  operator == ( nullptr_t ) const {  return outbeg == nullptr;  }
    bool  operator != ( nullptr_t ) const {  return !(*this == nullptr);  }
  };

  inline  auto  MakeCollector( char* beg, char* end ) -> Collector
    {  return Collector( beg, end );  }

  inline  auto  MakeCollector( char* beg, size_t len ) -> Collector
    {  return Collector( beg, beg + len );  }

  template <size_t N>
  auto  MakeCollector( char (&vec)[N] ) -> Collector
    {  return Collector( vec, N );  }

 /*
  * GetFlexForms( ... )
  * Формирует массив форм слова, добавляя окончания к переданному префиксу.
  * Тиражирует префикс на нужное количество форм по мере их построения.
  * Возвращает количество построенных форм слова или -1 при переполнении
  * аккумулятора.
  */
  int   GetFlexForms(
    Collector&        output,
    const uint8_t*    ptable,
    const flexinfo&   fxinfo,
    const fragment&   prefix,
    const fragment&   suffix = {} );

}

# endif // !__libmorph_buildforms_hpp__
