# if !defined( __libmorph_capsheme_h__)
# define __libmorph_capsheme_h__
# include <cstddef>
# include <cstdint>

// For better compatibility and neighbour usage of different morphological
// modules in one UNIX application all the objects are packed to the
// single namespace named LIBMORPH_NAMESPACE

//=====================================================================
// Схема капитализации есть word16, где старший байт представляет
// собой количество разделенных дефисами частей в слове, а младший -
// кодировку дефисных частей слова по два бита на одну часть. При этом
// 00 означает, что слово написано строчными буквами, 01 - что
// слово написано с заглавной буквы, а 11 - всеми заглавными буквами
// При этом значение схемы 0 означает нелегальную схему
//=====================================================================

namespace libmorph {

  class CapScheme
  {
    enum: unsigned char
    {
      undefined = 0,    // Схема капитализации не определена
      all_small = 1,    // Все буквы- строчные
      first_cap = 2,    // Первая буква - заглавная
      word_caps = 3,    // Все заглавные
      first_was = 4,    // Первая буква была заглавной
      error_cap = 5     // Ошибочная капитализация
    };

    const unsigned char*  charTypeMatrix;
    const unsigned char*  toLoCaseMatrix;
    const unsigned char*  toUpCaseMatrix;
    const unsigned char*  pspMinCapValue;

    static
    const unsigned char (&capStateMatrix)[6][4];

  public:
    CapScheme(
      const unsigned char* charType,
      const unsigned char* toLoCase,
      const unsigned char* toUpCase,
      const unsigned char* minValue ):
        charTypeMatrix( charType ),
        toLoCaseMatrix( toLoCase ),
        toUpCaseMatrix( toUpCase ),
        pspMinCapValue( minValue )  {}
    CapScheme( const CapScheme& cs ):
        charTypeMatrix( cs.charTypeMatrix ),
        toLoCaseMatrix( cs.toLoCaseMatrix ),
        toUpCaseMatrix( cs.toUpCaseMatrix ),
        pspMinCapValue( cs.pspMinCapValue )  {}
    CapScheme& operator = ( const CapScheme& cs )
      {
        charTypeMatrix = cs.charTypeMatrix;
        toLoCaseMatrix = cs.toLoCaseMatrix;
        toUpCaseMatrix = cs.toUpCaseMatrix;
        pspMinCapValue = cs.pspMinCapValue;
        return *this;
      }
  public:
    enum: unsigned char
    {
      CT_CAPITAL = 0,
      CT_REGULAR = 1,
      CT_DLMCHAR = 2,        // Разделитель - дефис
      CT_INVALID = 3,
    };

  public:
    template <size_t N>
    auto  Get( unsigned char (&out)[N],
       const char* src, size_t len = (size_t)-1 ) const -> unsigned;
    auto  Get( unsigned char* out, size_t cch,
      const char*  src, size_t len = (size_t)-1 ) const -> unsigned;
    auto  Set( unsigned char* str, size_t len,
       uint8_t  psp ) const -> unsigned char*;
  };

  // CapScheme inline implementation

  template <size_t N>
  auto  CapScheme::Get( unsigned char (&out)[N], const char* src, size_t len ) const -> unsigned
    {  return Get( out, N, src, len );  }

  inline  bool  IsGoodShemeMin2( unsigned scheme )
  {
    return scheme == 0x0102 || scheme == 0x020A || scheme == 0x032A;
  }

  inline  bool  IsGoodShemeMin1( unsigned scheme )
  {
    return scheme == 0x0101 || scheme == 0x0205 || scheme == 0x0311
        || IsGoodShemeMin2( scheme );
  }

  inline  bool  IsGoodShemeMin0( unsigned scheme )
  {
    return scheme == 0x0100 || scheme == 0x0200 || scheme == 0x0300
        || scheme == 0x0201 || scheme == 0x0301 || IsGoodShemeMin1( scheme );
  }

  inline  bool  IsGoodSheme( unsigned scheme, unsigned minCap )
  {
    return ( minCap == 2 ? IsGoodShemeMin2( scheme ) :
           ( minCap == 1 ? IsGoodShemeMin1( scheme ) :
           ( minCap == 0 ? IsGoodShemeMin0( scheme ) : false ) ) );
  }

#if defined( unit_test )
  int   capscheme_unit_test();
#endif  // unit_test

} // namespace

# endif // !__libmorph_capsheme_h__
