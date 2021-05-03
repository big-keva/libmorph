# if !defined( _capsheme_h_ )
# define _capsheme_h_

// For better compatibility and neighbour usage of different morphological
// modules in one UNIX application all the objects are packed to the
// single namespace named __libmorphrus__

namespace __libmorpheng__
{
  //=====================================================================
  // —хема капитализации есть word16, где старший байт представл€ет
  // собой количество разделенных дефисами частей в слове, а младший -
  // кодировку дефисных частей слова по два бита на одну часть. ѕри этом
  // 00 означает, что слово написано строчными буквами, 01 - что
  // слово написано с заглавной буквы, а 11 - всеми заглавными буквами
  // ѕри этом значение схемы 0 означает нелегальную схему
  //=====================================================================

  unsigned  GetCapScheme( const char*     lpWord,
                          char*           lpDest,
                          unsigned        cbBuff,
                          unsigned&       scheme );
  void      SetCapScheme( char*           lpWord,
                          unsigned        scheme );
  unsigned  GetMinScheme( unsigned        nParts,
                          unsigned        minCap,
                          const char*     lpWord = 0 );
  void      SetLowerCase( unsigned char*  lpWord );

  inline  unsigned  GetWordCaps( unsigned short wdInfo )
  {
    return (wdInfo & 0x0180) >> 7;
  }

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

} // __libmorpheng__ namespace

# endif // _capsheme_h_
