# include "capsheme.h"
# include <cstdint>

namespace libmorph
{
  const unsigned char (&CapScheme::capStateMatrix)[6][4] =
  {
    { first_was, all_small, error_cap, error_cap },
    { error_cap, all_small, all_small, error_cap },
    { error_cap, first_cap, first_cap, error_cap },
    { word_caps, error_cap, word_caps, error_cap },
    { word_caps, first_cap, word_caps, error_cap },
    { error_cap, error_cap, error_cap, error_cap }
  };

  //=====================================================================
  // Функция вычисляет схему капитализации всего слова, его длину и
  // создает образ слова, переведенный в нижний регистр
  //=====================================================================
  auto  CapScheme::Get(
    unsigned char* output, size_t outlen,
    const char*    srctop, size_t srclen ) const -> unsigned
  {
    unsigned char*  outend = output + outlen;
    const char*     srcend;
    unsigned        scheme;
    int             cdefis;
    bool            bvalid;

    if ( srclen == (size_t)-1 ) for ( srcend = srctop; *srcend != '\0'; ++srcend ) (void)0;
      else srcend = srctop + srclen;

    for ( cdefis = 0, scheme = 0, bvalid = true; srctop < srcend && output < outend; )
    {
      unsigned  subcap = undefined;
      unsigned  chtype;

      while ( srctop < srcend && output < outend && (chtype = charTypeMatrix[(uint8_t)*srctop]) != CT_DLMCHAR )
      {
        subcap = capStateMatrix[subcap][chtype];
          *output++ = toLoCaseMatrix[(unsigned char)*srctop++];
      }

      if ( output >= outend )
        return (unsigned)-1;

      bvalid &= ((subcap = capStateMatrix[subcap][CT_DLMCHAR]) != error_cap);
        scheme |= (subcap - 1) << (cdefis << 1);

      if ( srctop != srcend )
      {
        if ( output >= outend )
          return (unsigned)-1;

        if ( charTypeMatrix[uint8_t( *output++ = *srctop++ )] == CT_DLMCHAR )  ++cdefis;
          else return (unsigned)-1;
      }
    }

    if ( output < outend ) *output = '\0';
      else return (unsigned)-1;

    return (((unsigned)(output + outlen - outend)) << 16) | ((cdefis + 1) << 8) | (bvalid ? scheme : 0xff);
  }

  //=====================================================================
  // Функция приводит слово к нужной схеме капитализации, т. е. к той,
  // которая передана в качестве cSheme. Предполагается, что слово подано
  // в нижнем регистре, так как именно таким образом организован словарь.
  //=====================================================================
  auto  CapScheme::Set( unsigned char* str, size_t len, uint8_t psp ) const -> unsigned char*
  {
    auto  strorg = str;
    auto  minCap = pspMinCapValue[psp];
    auto  scheme = uint16_t{};
    int   nparts = 1;

  // first check length and get part count
    if ( len == (size_t)-1 )
    {
      for ( len = 0; str[len] != 0; ++len )
        nparts += charTypeMatrix[str[len]] == CT_DLMCHAR;
    }
      else
    {
      for ( auto beg = str, end = beg + len; beg != end; ++beg )
        nparts += charTypeMatrix[*beg] == CT_DLMCHAR;
    }

  // Определить собственно схему капитализации
    switch ( nparts )
    {
      case 1:
        scheme = minCap;
        break;
      case 2:
        scheme = minCap | (minCap << 2);
        break;
      case 3:
        scheme = minCap | (minCap << 4) | ((minCap & 0x02) << 2);
        break;
      default:
        scheme = 0;
    }

  // приложить схему капитализации к строке
    for ( ; nparts-- > 0 && *str != '\0'; scheme >>= 2 )
    {
      unsigned  strcap = scheme & 0x03;   // Возможные значения схемы капитализации - 0 (a), 1 (Aa) или 2 (AA)

      for ( ; *str != '\0' && charTypeMatrix[*str] != CT_DLMCHAR; strcap &= ~0x01, ++str )
        *str = strcap ? toUpCaseMatrix[*str] : *str;

      str += (charTypeMatrix[*str] == CT_DLMCHAR ? 1 : 0);
    }
    return strorg;
  }

# if 0
  # define CT_CAPITAL  0
  # define CT_REGULAR  1
  # define CT_DLMCHAR  2         // Разделитель - дефис
  # define CT_INVALID  3

# if defined( unit_test )
# include <string.h>
# include <stdio.h>

  struct
  {
    const char* thestr;
    unsigned    scheme;
  } capschemeTestData[] =
  {
    { "�����",                0x00050000 },
    { "�����",                0x00050001 },
    { "�����",                0x00050002 },
    { "�����",                0x0005ffff },
    { "�����",                0x0005ffff },
    { "�����",                0x0005ffff },
    { "���-����",             0x00080100 },
    { "���-����",             0x00080101 },
    { "���-����",             0x00080102 },
    { "���-����",             0x0008ffff },
    { "���-����",             0x0008ffff },
    { "���-����",             0x0008ffff },
    { "���-����",             0x00080104 },
    { "���-����",             0x00080108 },
    { "���-����",             0x0008ffff },
    { "���-����",             0x00080105 },
    { "���-����",             0x00080106 },
    { "���-����",             0x0008010A },
    { "�����������-��-�����", 0x00140211 },
    { "�����������-��-�����-���-�����-��-���-�����-�-�����-�������-������", (unsigned)-1 }
  };

  int   capscheme_unit_test()
  {
    unsigned      scheme;
    unsigned char slower[0x20];     

  // ��������� ����������� ����� ������������� �����
    for ( int i = 0; i < sizeof(capschemeTestData) / sizeof(capschemeTestData[0]); ++i )
      if ( (scheme = GetCapScheme( slower, sizeof(slower), capschemeTestData[i].thestr )) != capschemeTestData[i].scheme )
      {
        printf( "Invalid capitalization scheme for \'%s\': %08x instead of %08x!\n",
          capschemeTestData[i].thestr, scheme, capschemeTestData[i].scheme );
        return -1;
      }

  // ��������� ��������� ����� ���� �������������
    if ( strcmp( SetCapScheme( strcpy( (char*)slower, "�����������-��-�����" ), GetMinScheme( 1, "�����������-��-�����" ) ), "�����������-��-�����" ) != 0 )
    {
      printf( "Invalid SetCapScheme() result for '�����������-��-�����'");
      return -1;
    }
    return 0;
  }

# endif  // unit_test

  // Массив задает минимальные схемы капитализации для всех зарегистрированных
  // типов слов: 0 - все строчные, 1 - первая прописная, 2 - все прописные
  unsigned      pspMinCapValue[] =
  {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0
  };

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
# endif
} // end lubmorph namespace
