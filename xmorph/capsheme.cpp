# include "capsheme.h"

namespace LIBMORPH_NAMESPACE
{
  # define SCHEME_UNDEFINED 0     // ����� ������������� �� ����������
  # define SCHEME_ALL_SMALL 1     // ��� �����- ��������
  # define SCHEME_FIRST_CAP 2     // ������ ����� - ���������
  # define SCHEME_WORD_CAPS 3     // ��� ���������
  # define SCHEME_FIRST_WAS 4     // ������ ����� ���� ���������
  # define SCHEME_ERROR_CAP 5     // ��������� �������������
  # define SCHEME_OVERFLOW  6

  # define CT_CAPITAL  0
  # define CT_REGULAR  1
  # define CT_DLMCHAR  2         // ����������� - �����
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

      if ( srctop != srcend )
      {
        if ( output >= outend )
          return (unsigned)-1;

        if ( CharType( *output++ = *srctop++ ) == CT_DLMCHAR )  ++cdefis;
          else return (unsigned)-1;
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

  unsigned char capStateMatrix[6][4] =
  {
    { SCHEME_FIRST_WAS, SCHEME_ALL_SMALL, SCHEME_ERROR_CAP, SCHEME_ERROR_CAP },
    { SCHEME_ERROR_CAP, SCHEME_ALL_SMALL, SCHEME_ALL_SMALL, SCHEME_ERROR_CAP },
    { SCHEME_ERROR_CAP, SCHEME_FIRST_CAP, SCHEME_FIRST_CAP, SCHEME_ERROR_CAP },
    { SCHEME_WORD_CAPS, SCHEME_ERROR_CAP, SCHEME_WORD_CAPS, SCHEME_ERROR_CAP },
    { SCHEME_WORD_CAPS, SCHEME_FIRST_CAP, SCHEME_WORD_CAPS, SCHEME_ERROR_CAP },
    { SCHEME_ERROR_CAP, SCHEME_ERROR_CAP, SCHEME_ERROR_CAP, SCHEME_ERROR_CAP }
  };

  // ������ ������ ����������� ����� ������������� ��� ���� ������������������
  // ����� ����: 0 - ��� ��������, 1 - ������ ���������, 2 - ��� ���������
  unsigned      pspMinCapValue[] =
  {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0
  };

  //=====================================================================
  // ������� �������� ����� � ������ ����� �������������, �. �. � ���,
  // ������� �������� � �������� cSheme. ��������������, ��� ����� ������
  // � ������ ��������, ��� ��� ������ ����� ������� ����������� �������.
  //=====================================================================
  char*   SetCapScheme( char* pszstr, unsigned  scheme )
  {
    char* strorg = pszstr;
    int   nparts;
    
    for ( nparts = scheme >> 8; nparts-- > 0 && *pszstr != '\0'; scheme >>= 2 )
    {
      unsigned  strcap = scheme & 0x03;   // ������� ����� ������������� ��������� - 0 (a), 1 (Aa) ��� 2 (AA)

      for ( ; *pszstr != '\0' && CharType( *pszstr ) != CT_DLMCHAR; strcap &= ~0x01, ++pszstr )
        *pszstr = strcap ? (char)toUpCaseMatrix[(unsigned char)*pszstr] : *pszstr;

      pszstr += (CharType( *pszstr ) == CT_DLMCHAR ? 1 : 0);
    }
    return strorg;
  }

  unsigned  GetMinScheme( unsigned minCap, const char* lpword, unsigned nparts )
  {
  // ���� �� ������ ���������� ������ �����, ��������� ��� ����������
    if ( nparts == 0 && lpword != 0 )
      for ( nparts = 1; *lpword != '\0'; ++lpword ) nparts += (CharType( *lpword ) == CT_DLMCHAR);
    if ( nparts == 0 )
      nparts = 1;

  // ���������� ���������� ����� �������������
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
