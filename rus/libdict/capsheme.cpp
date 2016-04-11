# include "capsheme.h"

namespace LIBMORPH_NAMESPACE
{
  # define SCHEME_UNDEFINED 0     // Схема капитализации не определена
  # define SCHEME_ALL_SMALL 1     // Все буквы- строчные
  # define SCHEME_FIRST_CAP 2     // Первая буква - заглавная
  # define SCHEME_WORD_CAPS 3     // Все заглавные
  # define SCHEME_FIRST_WAS 4     // Первая буква была заглавной
  # define SCHEME_ERROR_CAP 5     // Ошибочная капитализация
  # define SCHEME_OVERFLOW  6

  # define CT_CAPITAL  0
  # define CT_REGULAR  1
  # define CT_DLMCHAR  2         // Разделитель - дефис
  # define CT_INVALID  3

  extern unsigned char capStateMatrix[6][4];
  extern unsigned char charTypeMatrix[256];

  template <class chartype>
  inline  unsigned  CharType( chartype c )
  {
    return charTypeMatrix[(unsigned char)c];
  }

  unsigned GetCapScheme( unsigned char* output, size_t  outlen, const char*  srctop, size_t srclen )
  {
    unsigned char*  outend = output + outlen;
    const char*     srcend;
    unsigned        scheme;
    int             cdefis;
    bool            bvalid;

    if ( srclen == (unsigned)-1 ) for ( srcend = srctop; *srcend != '\0'; ++srcend ) (void)0;
      else srcend = srctop + srclen;

    for ( cdefis = 0, scheme = 0, bvalid = true; srctop < srcend && output < outend; )
    {
      unsigned  subcap = SCHEME_UNDEFINED;
      unsigned  chtype;

      while ( srctop < srcend && output < outend && (chtype = CharType( *srctop )) != CT_DLMCHAR )
      {
        subcap = capStateMatrix[subcap][chtype];
          *output++ = toLoCaseMatrix[(unsigned char)*srctop++];
      }

      if ( output >= outend )
        return (unsigned)-1;

      bvalid &= ((subcap = capStateMatrix[subcap][CT_DLMCHAR]) != SCHEME_ERROR_CAP);
        scheme |= (subcap - 1) << (cdefis << 1);

      if ( CharType( *output = *srctop ) == CT_DLMCHAR && *output != '\0' )
      {
        ++output;
        ++srctop;
        ++cdefis;
      }
    }

    if ( output < outend ) *output = '\0';
      else return (unsigned)-1;

    return (((unsigned)(output + outlen - outend)) << 16) | ((cdefis + 1) << 8) | (bvalid ? scheme : 0xff);
  }

# if defined( unit_test )
# include <string.h>
# include <stdio.h>

  struct
  {
    const char* thestr;
    unsigned    scheme;
  } capschemeTestData[] =
  {
    { "слово",                0x00050000 },
    { "Слово",                0x00050001 },
    { "СЛОВО",                0x00050002 },
    { "СЛово",                0x0005ffff },
    { "СлоВо",                0x0005ffff },
    { "словО",                0x0005ffff },
    { "это-дело",             0x00080100 },
    { "Это-дело",             0x00080101 },
    { "ЭТО-дело",             0x00080102 },
    { "ЭтО-дело",             0x0008ffff },
    { "ЭТо-дело",             0x0008ffff },
    { "этО-дело",             0x0008ffff },
    { "это-Дело",             0x00080104 },
    { "это-ДЕЛО",             0x00080108 },
    { "это-ДелО",             0x0008ffff },
    { "Это-Дело",             0x00080105 },
    { "ЭТО-Дело",             0x00080106 },
    { "ЭТО-ДЕЛО",             0x0008010A },
    { "Комсомольск-на-Амуре", 0x00140211 },
    { "Комсомольск-на-Амуре-или-какой-то-ещё-город-с-очень-длинным-именем", (unsigned)-1 }
  };

  int   capscheme_unit_test()
  {
    unsigned      scheme;
    unsigned char slower[0x20];     

  // проверить определение схемы капитализации слова
    for ( int i = 0; i < sizeof(capschemeTestData) / sizeof(capschemeTestData[0]); ++i )
      if ( (scheme = GetCapScheme( slower, sizeof(slower), capschemeTestData[i].thestr )) != capschemeTestData[i].scheme )
      {
        printf( "Invalid capitalization scheme for \'%s\': %08x instead of %08x!\n",
          capschemeTestData[i].thestr, scheme, capschemeTestData[i].scheme );
        return -1;
      }

  // проверить установку новой семы капитализации
    if ( strcmp( SetCapScheme( strcpy( (char*)slower, "Комсомольск-на-амуре" ), GetMinScheme( 1, "Комсомольск-на-амуре" ) ), "Комсомольск-на-Амуре" ) != 0 )
    {
      printf( "Invalid SetCapScheme() result for 'Комсомольск-на-амуре'");
      return -1;
    }
    return 0;
  }

# endif  // unit_test

  unsigned char capStateMatrix[6][4] =
  {
    { SCHEME_FIRST_WAS, SCHEME_ALL_SMALL, SCHEME_ERROR_CAP, SCHEME_ERROR_CAP },
    { SCHEME_ERROR_CAP, SCHEME_ALL_SMALL, SCHEME_ALL_SMALL, SCHEME_ERROR_CAP },
    { SCHEME_ERROR_CAP, SCHEME_FIRST_CAP, SCHEME_FIRST_CAP, SCHEME_ERROR_CAP },
    { SCHEME_WORD_CAPS, SCHEME_ERROR_CAP, SCHEME_WORD_CAPS, SCHEME_ERROR_CAP },
    { SCHEME_WORD_CAPS, SCHEME_FIRST_CAP, SCHEME_WORD_CAPS, SCHEME_ERROR_CAP },
    { SCHEME_ERROR_CAP, SCHEME_ERROR_CAP, SCHEME_ERROR_CAP, SCHEME_ERROR_CAP }
  };

  unsigned char charTypeMatrix[256] =
  {
  // Characters in range 0 - 31
    CT_DLMCHAR, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
  // Space till plus, characters in range 32 - 47
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_DLMCHAR,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_REGULAR,         // Comma is a regular char
    CT_DLMCHAR,         // Minus - a defis - is a delimiter
                            CT_INVALID, CT_INVALID,
  // Characters in range 48 - 127
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
  // Characters in range 0x80 - 0x9F
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
  // Characters in range 0xA0 - 0xA7
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_CAPITAL,             // Capital character - Ё, 0xA8
  // Characters in range 0xA9 - 0xB7
                CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
    CT_REGULAR,             // Regular character - ё, 0xB8
  // Characters in range 0xB9 - 0xBF
                CT_INVALID, CT_INVALID, CT_INVALID,
    CT_INVALID, CT_INVALID, CT_INVALID, CT_INVALID,
  // Characters in range 0xC0 - 0xDF, cyrillic capitals
    CT_CAPITAL, CT_CAPITAL, CT_CAPITAL, CT_CAPITAL,
    CT_CAPITAL, CT_CAPITAL, CT_CAPITAL, CT_CAPITAL,
    CT_CAPITAL, CT_CAPITAL, CT_CAPITAL, CT_CAPITAL,
    CT_CAPITAL, CT_CAPITAL, CT_CAPITAL, CT_CAPITAL,
    CT_CAPITAL, CT_CAPITAL, CT_CAPITAL, CT_CAPITAL,
    CT_CAPITAL, CT_CAPITAL, CT_CAPITAL, CT_CAPITAL,
    CT_CAPITAL, CT_CAPITAL, CT_CAPITAL, CT_CAPITAL,
    CT_CAPITAL, CT_CAPITAL, CT_CAPITAL, CT_CAPITAL,
  // Regular characters in range 0xE0 - 0xFF
    CT_REGULAR, CT_REGULAR, CT_REGULAR, CT_REGULAR,
    CT_REGULAR, CT_REGULAR, CT_REGULAR, CT_REGULAR,
    CT_REGULAR, CT_REGULAR, CT_REGULAR, CT_REGULAR,
    CT_REGULAR, CT_REGULAR, CT_REGULAR, CT_REGULAR,
    CT_REGULAR, CT_REGULAR, CT_REGULAR, CT_REGULAR,
    CT_REGULAR, CT_REGULAR, CT_REGULAR, CT_REGULAR,
    CT_REGULAR, CT_REGULAR, CT_REGULAR, CT_REGULAR,
    CT_REGULAR, CT_REGULAR, CT_REGULAR, CT_REGULAR
  };

  // Массив задает минимальные схемы капитализации для всех зарегистрированных
  // типов слов: 0 - все строчные, 1 - первая прописная, 2 - все прописные
  unsigned      pspMinCapValue[] =
  {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0
  };

  unsigned char toLoCaseMatrix[256] =
  {
  // Characters in range 0 - 31
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
  // Space till plus, characters in range 32 - 63
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
  // Characters in range 64 - 127
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
  // Characters in range 0x80 - 0x9F
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
    0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
  // Characters in range 0xA0 - 0xA7
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
    0xB8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7,
    0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
  // Characters in range 0xC0 - 0xDF, cyrillic capitals
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
    0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
    0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF,
  // Regular characters in range 0xE0 - 0xFF
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
    0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
    0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
  };

  unsigned char toUpCaseMatrix[256] =
  {
  /* Characters in range 0 - 31       */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
  /* Space till plus, characters in range 32 - 63 */
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
  /* Characters in range 64 - 127     */
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
    0x60, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x5A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
  /* Characters in range 0x80 - 0x9F  */
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
    0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
  /* Characters in range 0xA0 - 0xA7  */
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
    0xB8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
    0xB0, 0xB1, 0xB2, 0xB2, 0xA5, 0xB5, 0xB6, 0xB7,
    0xA8, 0xB9, 0xAA, 0xBB, 0xBC, 0xBD, 0xBE, 0xAF,
  /* Characters in range 0xC0 - 0xDF, cyrillic capitals */
    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
    0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
    0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
  /* Regular characters in range 0xC0 - 0xDF  */
    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
    0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
    0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF
  };

  //=====================================================================
  // Функция приводит слово к нужной схеме капитализации, т. е. к той,
  // которая передана в качестве cSheme. Предполагается, что слово подано
  // в нижнем регистре, так как именно таким образом организован словарь.
  //=====================================================================
  char*   SetCapScheme( char* pszstr, unsigned  scheme )
  {
    char* strorg = pszstr;
    int   nparts;
    
    for ( nparts = scheme >> 8; nparts-- > 0 && *pszstr != '\0'; scheme >>= 2 )
    {
      unsigned  strcap = scheme & 0x03;   // Извлечь схему капитализации фрагмента - 0 (a), 1 (Aa) или 2 (AA)

      for ( ; *pszstr != '\0' && CharType( *pszstr ) != CT_DLMCHAR; strcap &= ~0x01, ++pszstr )
        *pszstr = strcap ? (char)toUpCaseMatrix[(unsigned char)*pszstr] : *pszstr;

      pszstr += (CharType( *pszstr ) == CT_DLMCHAR ? 1 : 0);
    }
    return strorg;
  }

  unsigned  GetMinScheme( unsigned minCap, const char* lpword, unsigned nparts )
  {
  // Если не задано количество частей слова, вычислить это количество
    if ( nparts == 0 && lpword != 0 )
      for ( nparts = 1; *lpword != '\0'; ++lpword ) nparts += (CharType( *lpword ) == CT_DLMCHAR);
    if ( nparts == 0 )
      nparts = 1;

  // Определить собственно схему капитализации
    switch ( nparts )
    {
      case 1:
        return 0x0100 | minCap;
      case 2:
        return 0x0200 | minCap | (minCap << 2);
      case 3:
        return 0x0300 | minCap | (minCap << 4) | ((minCap & 0x02) << 2);
      default:
        return 0x0100;
    }
  }

  void      SetLowerCase( unsigned char* pszstr )
  {
    while ( (*pszstr = toLoCaseMatrix[*pszstr]) != '\0' )
      ++pszstr;
  }

} // end namespace
