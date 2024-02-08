# include "lresolve.h"
# include "../../tools/utf81251.h"
# include "../../tools/ftables.h"
# include <cstdint>
# include <map>

class TypeMatrix
{
  std::map<std::string, uint16_t> typesMap;

public:     // construction and initialization
  TypeMatrix( const std::initializer_list<std::pair<const char*, uint16_t>>& );

public:     // mapping
  uint16_t  operator [] ( const std::string& s ) const  {  return find( s );  }
  uint16_t  operator [] ( const char* s ) const {  return find( s );  }

protected:
  uint16_t  find( const std::string& s ) const
    {
      auto  it = typesMap.find( s );

      return it != typesMap.end() ? it->second : 0;
    }
};

TypeMatrix  typesMap( {
    { "# нсв",        1 },  // Глаголы несов. вида
    { "# нсв_нп",     2 },  // Глаголы несов. вида, непереходные
    { "# св",         3 },  // Глаголы сов. вида
    { "# св_нп",      4 },  // Глаголы сов. вида, непереходные
    { "# св-нсв",     5 },  // Двухвидовые глаголы
    { "# св-нсв_нп",  6 },  // Непереходные двухвидовые глаголы

    { "# нсв",        1 },  // Глаголы несов. вида
    { "# нсв_нп",     2 },  // Глаголы несов. вида, непереходные
    { "# св",         3 },  // Глаголы сов. вида
    { "# св_нп",      4 },  // Глаголы сов. вида, непереходные
    { "# св-нсв",     5 },  // Двухвидовые глаголы
    { "# св-нсв_нп",  6 },  // Непереходные двухвидовые глаголы

  // Неодуш. сущ. мужского рода
    { "# м",          7 },
    { "м мп",         7 },
    { "м ммс",        7 },
    { "мн. м",        7 + wfMultiple },
    { "# мн._от_м",   7 + wfMultiple },
    { "мн._неод. мп", 7 + wfMultiple },

  // Одуш. сущ. мужского рода
    { "# мо"         , 8 },
    { "мо мпо"       , 8 },
    { "мо момс"      , 8 },
    { "мн. мо"       , 8 + wfMultiple },
    { "мн._одуш. мпо", 8 + wfMultiple },

  // Одуш.-неодуш. сущ. мужского рода
    { "# м//мо", 9 },
    { "# мо//м", 9 },

    { "м с"  , 10 },  // Сущ. муж. рода, скл. по схеме ср.
    { "мо жо", 11 },  // Сущ. муж. рода, скл., как ж. одуш.
    { "мо со", 12 },  // Сущ. муж. рода, скл., как с. одуш.

  // Неодуш. сущ. женского рода
    { "# ж",        13 },
    { "ж жп",       13 },
    { "ж жмс",      13 },
    { "мн. ж",      13 + wfMultiple },
    { "# мн._от_ж", 13 + wfMultiple },

  // Одуш. сущ. женского рода
    { "# жо",   14 },
    { "жо жпо", 14 },
    { "мн. жо", 14 + wfMultiple },

  // Одуш.-неодуш. сущ. женского рода
    { "# ж//жо", 15 },
    { "# жо//ж", 15 },

  // Неодуш. сущ. среднего рода
    { "# с",          16 },
    { "с сп",         16 },
    { "с смс",        16 },
    { "мн. с",        16 + wfMultiple },
    { "# мн._от_с",   16 + wfMultiple },
    { "мн._от_с сп",  16 + wfMultiple },

  // Одуш. сущ. среднего рода
    { "# со",   17 },
    { "со спо", 17 },
    { "мн. со", 17 + wfMultiple },

  // Одуш.-неодуш. сущ. среднего рода
    { "# с//со",  18 },
    { "# со//с",  18 },

  // Существительные общего рода, напр., непоседа
    { "м//ж ж",     19 },
    { "# мо-жо",    20 },
    { "# мо//жо",   20 },
    { "мо-жо жо",   20 },
    { "мо//жо жо",  20 },

  // Сущ. мужского/среднего рода
    { "# м//с",     21 },
    { "# с//м",     21 },
    { "мо//со со",  22 },

  // Сущ. женского/среднего рода
    { "# ж//с", 23 },
    { "# с//ж", 23 },

  // Существительные множественного числа
    { "мн. ж//м", 24 + wfMultiple },
    { "мн. м//ж", 24 + wfMultiple },

  // Прилагательные
    { "# п",      25 },   // Прилагательные
    { "# г-п",    26 },   // "Географич." прилагательные
    { "п мс",     27 },   // Притяжательные местоимения
    { "мс-п мсп", 28 },   // Местоименные прилагательные
    { "# мс-п",   28 },   // Местоименные прилагательные

  // Местоимение
    { "# мс",     29 },
    { "# мс_мн.", 93 },

    { "# мсм",  30 }, // Местоимения трех родов
    { "# мсж",  31 },
    { "# мсс",  32 },

  // Числительные
    { "# числ.",      33 },
    { "# числ._2",    34 },
    { "# числ._с",    35 },
    { "числ.-п мс",   36 }, // Порядковые числительные
    { "числ.-п мсп",  36 }, // Порядковые числительные
    { "числ.-п мс-п", 36 }, // Порядковые числительные

    { "# и",    37 },   // Имена собственные
    { "# им",   38 },   // Имена мужского рода
    { "им иж",  38 },
    { "# иж",   39 },   // Имена женского рода
    { "# ом",   40 },   // Отчества муж. рода
    { "# ож",   41 },   // Отчества жен. рода
    { "# ф",    42 },   // Фамилии
    { "ф фп",   42 },

  // Географические названия
    { "# г",    43 },
    { "# гп",   43 },
    { "# гм",   44 },   // мужского рода
    { "# гмп",  44 },
    { "# гж",   45 },   // Женского рода
    { "# гжп",  45 },
    { "# гс",   46 },   // Среднего рода
    { "# гсп",  46 },

  // Множ. числа
    { "# мн._от_гж",  47 + wfMultiple },
    { "# мн._от_гм",  47 + wfMultiple },
    { "мн. гп",       47 + wfMultiple },

  // Неизменяемые части речи
    { "# вводн.",       48 },
    { "# межд.",        49 },
    { "# предик.",      50 },
    { "# предл.",       51 },
    { "# союз",         52 },
    { "# союз_соч.",    52 + wfUnionS },
    { "# част.",        53 },
    { "# н",            54 },
    { "# сокр._сущ.",   55 },
    { "# сокр._прил.",  56 },
    { "# сокр._вводн.", 57 },
    { "# сравн.",       58 },

  // Аббревиатуры
    { "# АБ", 59 }, // Пишущиеся только большими буквами
    { "# аб", 60 }, // Пишущиеся двояко

    { "# #1", 61 },
    { "# #2", 62 }
} );

// TypeMatrix implementation

TypeMatrix::TypeMatrix( const std::initializer_list<std::pair<const char*, uint16_t>>& typeset )
{
  for ( auto& it: typeset )
    typesMap.emplace( utf8to1251( it.first ), it.second );
}

static struct
{
  std::string s_flag;
  uint16_t    w_flag;
} lexflags[4] =
{
  { utf8to1251( "{превосх.}" ), wfExcellent },
  { utf8to1251( "{исчисл.}" ),  wfCountable },
  { utf8to1251( "{разг.}" ),    wfInformal  },
  { utf8to1251( "{руг.}" ),     wfObscene   }
};

uint16_t  LexFlags( const char* comments )
{
  uint16_t  uflags = 0;

  for ( auto p = std::begin( lexflags ); p != std::end( lexflags ); ++p )
    if ( strstr( comments, p->s_flag.c_str() ) != nullptr )
      uflags |= p->w_flag;

  return uflags;
}

auto  st_casemark   = utf8to1251( "ШП:" );
auto  st_casescale  = utf8to1251( "ИРДВТП" );

auto  st_reflex     = utf8to1251( "ся" );
auto  st_yo         = utf8to1251( "ё" );
auto  st_ye         = utf8to1251( "е" );

//=====================================================================
// Method: GetRemark()
// Функция ищет пометы об особенностях в чередовании, соответствующие
// формату -nx-, где nx - некоторое количество символов, и возвращает
// строку - помету.
//=====================================================================
std::string GetRemark( const char* comment )
{
  for ( auto next = comment; (next = strchr( next, '-' )) != nullptr; ++next )
  {
    auto  end = next + 1;

    while ( *end != '\0' && *end != '-' && (unsigned char)*end > 0x20 )
      ++end;

    if ( *end == '-' )
      return std::string( next + 1, end - next - 1 );
  }

  return "";
}

bool        Reflexive( const std::string& s )
{
  auto  cchstr = s.length();

  return cchstr > 2 && strcmp( s.c_str() + cchstr - 2, st_reflex.c_str() ) == 0;
}

//=========================================================================
// Method: CaseVector( ... )
//
// Функция извлекает падежную шкалу предлога из строки комметариев к слову
//=========================================================================
byte_t      CaseScale( const char* string )
{
  static char caseScales[] = "\x01\x02\x04\x08\x10\x20";
  byte_t      fetchValue = 0;
  const char* searchChar;

  if ( (string = strstr( string, st_casemark.c_str() )) != nullptr )
    for ( string += 3; *string != '\0' && (searchChar = strchr( st_casescale.c_str(), *string )) != nullptr; ++string )
      fetchValue |= caseScales[searchChar - st_casescale.c_str()];

  return fetchValue;
}

//=====================================================================
// Method: GetPostfix()
// Функция извлекает строку постфикса из комментария к слову и возвра-
// щает длину постфикса
//=====================================================================
std::string GetPostfix( const char* string )
{
  const char* strtop;
  const char* strend;

  if ( (strtop = strstr( string, "post:" )) != nullptr )
  {
    for ( strtop += 5; *strtop != '\0' && (unsigned char)*strtop <= 0x20; strtop++ )
      (void)NULL;
    for ( strend = strtop; (unsigned char)*strend > 0x20; strend++ )
      (void)NULL;

    return std::string( strtop, strend - strtop );
  }
  return "";
}

lexemeinfo  ResolveClassInfo(
  const char* sznorm, const char*   szdies, const char*   sztype, const char* zapart, const char*   szcomm,
  const char* ftable, const libmorph::TableIndex& findex, const char*   mtable, const libmorph::rus::Alternator&   mindex )
{
  lexemeinfo  lexeme;
  std::string stType = std::string( szdies ) + ' ' + sztype;
  std::string stOrig = std::string( sztype ) + ' ' + zapart;
  int         mixIndex = 0;

  if ( (lexeme.mclass.wdinfo = (word16_t)(int32_t)typesMap[stType]) == 0 )
    return lexemeinfo();

// Далее делается проверка типа слова, чтобы включить в обработку
// неизменяемые части речи, типы которых перечислены ниже. В этих
// случаях несмотря на нулевую ссылку на таблицы окончаний основа
// считается вполне легальной и поступает в список словооснов.
  if ( (lexeme.mclass.wdinfo & 0x3F) < 48 )
  {
    if ( (lexeme.mclass.tfoffs = findex.Find( stOrig.c_str() )) == 0 && strcmp( zapart, "0" ) != 0 )
      return lexemeinfo();
  }
    else
  lexeme.mclass.tfoffs = 0;

// Скопировать основную форму слова и отщепить постфикс, если он есть
  lexeme.ststem = std::string( sznorm );

  for ( auto pos = lexeme.ststem.find( '=' ); pos != std::string::npos; pos = lexeme.ststem.find( '=', pos ) )
    lexeme.ststem.erase( pos, 1 );

  for ( auto pos = lexeme.ststem.find( st_yo ); pos != std::string::npos; pos = lexeme.ststem.find( st_yo ) )
    lexeme.ststem.replace( pos, st_yo.length(), st_ye );
    
// Отщепить постфикс и извлечь флаги описания лексической базы
  lexeme.stpost = GetPostfix( szcomm );
  lexeme.ststem.resize( lexeme.ststem.length() - lexeme.stpost.length() );
  lexeme.mclass.wdinfo |= LexFlags( szcomm );

// Закончить обработку нефлективных слов
  if ( lexeme.mclass.tfoffs == 0 )
  {
  // Если слово - предлог, извлечь падежную шкалу
    if ( (lexeme.mclass.wdinfo & 0x3F) == 51 )
      lexeme.mclass.tfoffs = CaseScale( zapart );
    lexeme.chrmin = lexeme.chrmax = '\0';
      return lexeme;
  }

// В случае, если ссылка на таблицу окончаний ненулевая, отбросить
// окончание нормальной формы или специально помеченное символом
// @ - для случаев, когда в современном языке нормальная форма
// слова не употребляется
// Сначала делается проверка, нет ли специально помеченного
// окончания, как это бывает, например, у существительных, упот-
// ребляемых только в определенных формах. Если так, то окон-
// чание отщепляется простым уменьшением длины строки.
  if ( lexeme.ststem.find( '@' ) != std::string::npos )
  {
    lexeme.ststem.resize( lexeme.ststem.find( '@' ) );
  }
    else
  {
  //   В противном случае строится исходя из типа слова граммати-
  // ческая информация о нормальной форме и вызывается процедура
  // его отщепления. Это бывает, вообще, практически всегда.
  //   В случае глаголов (типы 1..6) нормальной формой является
  // инфинитив (с возможной возвратной частицей). В случае прила-
  // гательных и других слов, склоняющихся по родам, выставляется
  // признак мужского рода в дополнение к именительному падежу.
    libmorph::NounLevels        nlevel;
    libmorph::VerbLevels        vlevel;
    const libmorph::GramLevels* levels = &nlevel;
    uint16_t                    nfinfo = 0;

    switch ( lexeme.mclass.wdinfo & 0x3F )
    {
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
        nfinfo = vtInfinitiv|gfRetForms;
        levels = &vlevel;
        break;
      case 25:
      case 26:
      case 27:
      case 28:
      case 34:
      case 36:
      case 42:
      case 52:
        nfinfo = (1 << 9) + (Reflexive( lexeme.ststem ) ? gfRetForms : 0);    // Мужской род для прилагательных
        levels = &nlevel;
        break;
      default:
        nfinfo = 0;
        levels = &nlevel;
        break;
    }
  // Обозначить множественное число, если оно есть
    if ( lexeme.mclass.wdinfo & wfMultiple )
      nfinfo |= gfMultiple;

  // Отщепить окончание нормальной формы
    if ( !libmorph::FlexStripper( *levels, ftable ).StripStr( lexeme.ststem, nfinfo, lexeme.mclass.tfoffs ) && strchr( zapart, ':' ) == nullptr )
      return lexemeinfo();
  }

// инициализировать минимальный и максимальный символы по таблице окончаний
  std::tie(lexeme.chrmin, lexeme.chrmax) = libmorph::FlexStripper::GetMinMaxChar( ftable, lexeme.mclass.tfoffs );

// После отщепления окончания нормальной формы словооснова проверяется на наличие чередований.
  lexeme.mclass.mtoffs = mindex.Find( mtable, stOrig.c_str(),
    lexeme.mclass.wdinfo, lexeme.ststem.c_str(), GetRemark( szcomm ).c_str() );

// Если чередования присутствуют, то отщепляется первая ступень
// чередования в основе, т. к. она соответствует нормальной форме.
  if ( lexeme.mclass.mtoffs != 0 )
  {
    std::tie(lexeme.chrmin, lexeme.chrmax) = libmorph::rus::Alternator::GetMinMaxChar( mtable, lexeme.mclass.mtoffs, lexeme.chrmin, lexeme.chrmax );

  // Проверить, нет ли явного указания типа чередования
  // Отщепить чередование первой ступени
    if ( (lexeme.mclass.wdinfo & 0x3F) <= 6 )
    {
      switch ( zapart[0] )
      {
        case '6':
        case '9':
          mixIndex = 1;
          break;
        case '5':
          if ( memcmp( zapart, "5c/c", 4 ) == 0 || memcmp( zapart, "5*c/c", 5 ) == 0 )
            mixIndex = 1;
          break;
        case '1':
          switch ( zapart[1] )
          {
            case '0': // 10
            case '4': // 14
              mixIndex = 1;
              break;
            case '1': // 11
              mixIndex = 2;
              break;
          }
          break;
      }
    }
    lexeme.ststem.resize( lexeme.ststem.length() - (0x0f & *libmorph::rus::Alternator::GetDefaultStr( mtable, lexeme.mclass.mtoffs )) );
  }

  if ( mixIndex != 0 )
    lexeme.mclass.wdinfo |= ((mixIndex & 0x03) << 11);

  return lexeme;
}

#if defined( _MSC_VER )
  #pragma warning( default: 4237 )
#endif
