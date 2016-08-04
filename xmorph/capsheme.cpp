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

  // Массив задает минимальные схемы капитализации для всех зарегистрированных
  // типов слов: 0 - все строчные, 1 - первая прописная, 2 - все прописные
  unsigned      pspMinCapValue[] =
  {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0
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

  unsigned char*  SetLowerCase( unsigned char* pszstr, size_t cchstr/*= (size_t)-1*/ )
  {
    unsigned char*  pszorg;
    unsigned char*  pszend;

    if ( cchstr == (size_t)-1 )
      for ( cchstr = 0; pszstr[cchstr] != 0; ++cchstr ) (void)NULL;

    for ( pszend = (pszorg = pszstr) + cchstr; pszstr < pszend; ++pszstr )
      *pszstr = toLoCaseMatrix[*pszstr];

    return pszorg;
  }

} // end namespace
