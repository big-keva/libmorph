# if !defined( _capsheme_h_ )
# define _capsheme_h_
# include <namespace.h>
# include <stddef.h>

// For better compatibility and neighbour usage of different morphological
// modules in one UNIX application all the objects are packed to the
// single namespace named LIBMORPH_NAMESPACE

//=====================================================================
// —хема капитализации есть word16, где старший байт представл€ет
// собой количество разделенных дефисами частей в слове, а младший -
// кодировку дефисных частей слова по два бита на одну часть. ѕри этом
// 00 означает, что слово написано строчными буквами, 01 - что
// слово написано с заглавной буквы, а 11 - всеми заглавными буквами
// ѕри этом значение схемы 0 означает нелегальную схему
//=====================================================================

namespace LIBMORPH_NAMESPACE
{
  unsigned        GetCapScheme( unsigned char*  output, size_t      outlen,
                          const char*     srctop, size_t      srclen = (size_t)-1 );
  unsigned        GetMinScheme( unsigned        minCap, const char* lpword = 0, unsigned nparts = 0 );
  char*           SetCapScheme( char*           pszstr, unsigned    scheme );
  unsigned char*  SetLowerCase( unsigned char*  pszstr, size_t      cchstr = (size_t)-1 );

  extern  unsigned char charTypeMatrix[256];
  extern  unsigned char toLoCaseMatrix[256];
  extern  unsigned char toUpCaseMatrix[256];

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

# endif // _capsheme_h_
