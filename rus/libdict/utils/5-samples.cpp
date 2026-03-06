# include "../../../rus.h"
# include <algorithm>
# include <cstring>
# include <moonycode/codes.h>
# include <mtc/wcsstr.h>
# include <time.h>

unsigned  charRank[32] =
{
  8010,     // u'а'
  1590,     // u'б'
  4540,     // u'в'
  1700,     // u'г'
  2980,     // u'д'
  8450,     // u'е'
  940,      // u'ж'
  1650,     // u'з'
  7350,     // u'и'
  1210,     // u'й'
  3490,     // u'к'
  4400,     // u'л'
  3210,     // u'м'
  6700,     // u'н'
  1097,     // u'о'
  2810,     // u'п'
  4730,     // u'р'
  5470,     // u'с'
  6260,     // u'т'
  2620,     // u'у'
  0260,     // u'ф'
  970,      // u'х'
  480,      // u'ц'
  1440,     // u'ч'
  0730,     // u'ш'
  0360,     // u'щ'
  0040,     // u'ъ'
  1900,     // u'ы'
  1740,     // u'ь'
  0320,     // u'э'
  0640,     // u'ю'
  2010      // u'я'
};

auto  SelectChar( const widechar* src, size_t len ) -> size_t
{
  auto  ranked = (unsigned*)alloca( sizeof(unsigned) * len );

  for ( size_t i = 0; i < len; ++i )
    ranked[i] = (i == 0 ? 0 : ranked[i - 1]) + charRank[src[i] - u'а'];

  auto  rndval = (unsigned)(rand() * 1.0 * ranked[len - 1] / RAND_MAX);

  for ( size_t i = 0; i < len; ++i )
    if ( rndval <= ranked[i] )
      return i;

  throw std::logic_error( "must not get here" );
}

auto  GetNewTask( IMlmaWcXX* morpho, widechar* buffer, size_t length ) -> const widechar*
{
  widechar  szhelp[0x40];
  int       cchelp;

  // для первых четырёх букв слова на каждом уровне запрашиваем все варианты
// следующей буквы, выбираем псевдослучайный и рекурсивно вызываем функцию
// с большей длиной
  if ( length < 4 )
  {
    buffer[length] = '*';

  // получить возможные символы после '*'
    if ( (cchelp = morpho->CheckHelp( szhelp, sizeof(szhelp), buffer, 1 + length )) == 0 )
      return nullptr;

  // удалить 0 и иные (не)возможные символы
    cchelp = std::remove_if( szhelp, szhelp + cchelp, []( widechar c )
      {  return c < u'а';  } ) - szhelp;

  // для оставшихся построить возможные гипотезы
    while ( cchelp > 0 )
    {
      auto  cindex = SelectChar( szhelp, cchelp );

      buffer[length] = szhelp[cindex];

      if ( GetNewTask( morpho, buffer, length + 1 ) != nullptr )
        return buffer;

      memmove( szhelp + cindex, szhelp + cindex + 1, sizeof(widechar) * (--cchelp - cindex) );
    }
    return nullptr;
  }

// для последней бурвы запрашиваем все возможные варианты и выбираем случайный
  buffer[length] = '?';
    cchelp = 0;

  morpho->FindMatch( { buffer, length + 1 }, [&](
    lexeme_t          nlexid,
    int               nforms,
    const SStrMatch*  pforms )
    {
      uint8_t  wdinfo;

    // уфильтровываем части речи, оставив только существительные
      morpho->GetWdInfo( &wdinfo, nlexid );

      if ( wdinfo < 7 || wdinfo > 23 )
        return 0;

    // фильтруем нецензурные
      if ( nlexid >= 192512 && nlexid <= 192641 )
        return 0;

    // перебираем найденные формы и регистрируем буквы для формы 0 (И.ед.)
      for ( ; nforms-- > 0; ++pforms )
        if ( pforms->id == 0 || pforms->id == 0xff )
        {
          auto  inspos = std::find( szhelp, szhelp + cchelp, pforms->ws[4] );

          if ( inspos == szhelp + cchelp || *inspos != pforms->ws[4] )
          {
            memmove( inspos + 1, inspos, szhelp + cchelp++ - inspos );
              *inspos = pforms->ws[4];
          }
        }
      return 0;
    } );

  if ( cchelp == 0 )
    return nullptr;

// выбираем вероятную букву
  buffer[4] = szhelp[SelectChar( szhelp, cchelp )];
  buffer[5] = 0;

  return buffer;
}
