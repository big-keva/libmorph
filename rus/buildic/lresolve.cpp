# include "lresolve.h"
# include <tools/utf81251.h>
# include <tools/ftables.h>
# include <cstdint>
# include <cassert>
# include <map>

class TypeMatrix
{
  std::map<std::string, uint16_t> typesMap;

public:     // construction and initialization
  TypeMatrix();


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

TypeMatrix  typesMap;

// TypeMatrix implementation

TypeMatrix::TypeMatrix()
{
# define  add_type( key, val )  typesMap.insert( { utf8to1251( key ), val } )

  add_type( "# нсв",        1 );  // Глаголы несов. вида                
  add_type( "# нсв_нп",     2 );  // Глаголы несов. вида, непереходные  
  add_type( "# св",         3 );  // Глаголы сов. вида                  
  add_type( "# св_нп",      4 );  // Глаголы сов. вида, непереходные    
  add_type( "# св-нсв",     5 );  // Двухвидовые глаголы                
  add_type( "# св-нсв_нп",  6 );  // Непереходные двухвидовые глаголы   
		
		// Неодуш. сущ. мужского рода
  add_type( "# м",          7 );
  add_type( "м мп",         7 );
  add_type( "м ммс",        7 );
  add_type( "мн. м",        7 + wfMultiple );
  add_type( "# мн._от_м",   7 + wfMultiple );
  add_type( "мн._неод. мп", 7 + wfMultiple );

		// Одуш. сущ. мужского рода           
  add_type( "# мо",         8 );
  add_type( "мо мпо",       8 );
  add_type( "мо момс",      8 );
  add_type( "мн. мо",       8 + wfMultiple );
  add_type( "мн._одуш. мпо",8 + wfMultiple );

		// Одуш.-неодуш. сущ. мужского рода   
  add_type( "# м//мо",      9 );
  add_type( "# мо//м",      9 );

  add_type( "м с",          10 );   // Сущ. муж. рода, скл. по схеме ср.   
  add_type( "мо жо",        11 );   // Сущ. муж. рода, скл., как ж. одуш.  
  add_type( "мо со",        12 );   // Сущ. муж. рода, скл., как с. одуш.  

		// Неодуш. сущ. женского рода         
  add_type( "# ж",          13 );
  add_type( "ж жп",         13 );
  add_type( "ж жмс",        13 );
  add_type( "мн. ж",        13 + wfMultiple );
  add_type( "# мн._от_ж",   13 + wfMultiple );

		// Одуш. сущ. женского рода           
  add_type( "# жо",         14 );
  add_type( "жо жпо",       14 );
  add_type( "мн. жо",       14 + wfMultiple );

		// Одуш.-неодуш. сущ. женского рода   
  add_type( "# ж//жо",      15 );
  add_type( "# жо//ж",      15 );

		// Неодуш. сущ. среднего рода         
  add_type( "# с",          16 );
  add_type( "с сп",         16 );
  add_type( "с смс",        16 );
  add_type( "мн. с",        16 + wfMultiple );
  add_type( "# мн._от_с",   16 + wfMultiple );
  add_type( "мн._от_с сп",  16 + wfMultiple );

		// Одуш. сущ. среднего рода           
  add_type( "# со",         17 );
  add_type( "со спо",       17 );
  add_type( "мн. со",       17 + wfMultiple );

		// Одуш.-неодуш. сущ. среднего рода   
  add_type( "# с//со",      18 );
  add_type( "# со//с",      18 );

		// Существительные общего рода, напр., непоседа 
  add_type( "м//ж ж",       19 ); 
  add_type( "# мо-жо",      20 );
  add_type( "# мо//жо",     20 );
  add_type( "мо-жо жо",     20 );
  add_type( "мо//жо жо",    20 );

		// Сущ. мужского/среднего рода         
  add_type( "# м//с",       21 );
  add_type( "# с//м",       21 );
  add_type( "мо//со со",    22 );

		// Сущ. женского/среднего рода         
  add_type( "# ж//с",       23 );
  add_type( "# с//ж",       23 );

		// Существительные множественного числа 
  add_type( "мн. ж//м",     24 + wfMultiple );
  add_type( "мн. м//ж",     24 + wfMultiple );

		// Прилагательные 
  add_type( "# п",          25 );   // Прилагательные                
  add_type( "# г-п",        26 );   // "Географич." прилагательные   
  add_type( "п мс",         27 );   // Притяжательные местоимения    
  add_type( "мс-п мсп",     28 );   // Местоименные прилагательные   
  add_type( "# мс-п",       28 );   // Местоименные прилагательные   

		// Местоимение 
  add_type( "# мс",         29 );
  add_type( "# мс_мн.",     93 );

  add_type( "# мсм",        30 );   // Местоимения трех родов        
  add_type( "# мсж",        31 );
  add_type( "# мсс",        32 );

		// Числительные 
  add_type( "# числ.",      33 ); 
  add_type( "# числ._2",    34 ); 
  add_type( "# числ._с",    35 ); 
  add_type( "числ.-п мс",   36 );   // Порядковые числительные            
  add_type( "числ.-п мсп",  36 );   // Порядковые числительные            
  add_type( "числ.-п мс-п", 36 );   // Порядковые числительные            

  add_type( "# и",          37 );   // Имена собственные
  add_type( "# им",         38 );   // Имена мужского рода
  add_type( "им иж",        38 );
  add_type( "# иж",         39 );   // Имена женского рода
  add_type( "# ом",         40 );   // Отчества муж. рода
  add_type( "# ож",         41 );   // Отчества жен. рода
  add_type( "# ф",          42 );   // Фамилии
  add_type( "ф фп",         42 );   

		// Географические названия 
  add_type( "# г",          43 );
  add_type( "# гп",         43 );
  add_type( "# гм",         44 );   // мужского рода
  add_type( "# гмп",        44 );
  add_type( "# гж",         45 );   // Женского рода
  add_type( "# гжп",        45 );
  add_type( "# гс",         46 );   // Среднего рода
  add_type( "# гсп",        46 );

		// Множ. числа
  add_type( "# мн._от_гж",  47 + wfMultiple );
  add_type( "# мн._от_гм",  47 + wfMultiple );
  add_type( "мн. гп",       47 + wfMultiple );

		// Неизменяемые части речи 
  add_type( "# вводн.",         48 );
  add_type( "# межд.",          49 );
  add_type( "# предик.",        50 );
  add_type( "# предл.",         51 );
  add_type( "# союз",           52 );
  add_type( "# союз_соч.",      52 + wfUnionS );
  add_type( "# част.",          53 );
  add_type( "# н",              54 );
  add_type( "# сокр._сущ.",     55 );
  add_type( "# сокр._прил.",    56 );
  add_type( "# сокр._вводн.",   57 );
  add_type( "# сравн.",         58 );

		// Аббревиатуры 
  add_type( "# АБ",             59 );   // Пишущиеся только большими буквами  
  add_type( "# аб",             60 );   // Пишущиеся двояко                   

  add_type( "# #1",             61 );
  add_type( "# #2",             62 );

# undef add_type
}

auto  st_excellent  = utf8to1251( "{превосх.}" );
auto  st_countable  = utf8to1251( "{исчисл.}" );
auto  st_colloquial = utf8to1251( "{разг.}" );
auto  st_obscene    = utf8to1251( "{руг.}" );

auto  st_casemark   = utf8to1251( "ШП:" );
auto  st_casescale  = utf8to1251( "ИРДВТП" );

auto  st_reflex     = utf8to1251( "ся" );

//=====================================================================
// Method: GetRemark()
// Функция ищет пометы об особенностях в чередовании, соответствующие
// формату -nx-, где nx - некоторое количество символов, и возвращает
// строку - помету.
//=====================================================================
std::string GetRemark( const char* string )
{
  const char* strTop;
  const char* strEnd;
  
  if ( (strEnd = strTop = strchr( string, '-' )) == nullptr )
    return "";

  do ++strEnd;
    while ( *strEnd != '\0' && *strEnd != '-' && (unsigned char)*strEnd > 0x20 );

  return *strEnd == '-' ? std::string( strTop + 1, strEnd - strTop - 1 ) : "";
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

  if ( (lexeme.wdinfo = (word16_t)(int32_t)typesMap[stType]) == 0 )
    return lexemeinfo();

/*
  if ( utf8to1251( "учащийся" ) == sznorm )
  {
    int i = 0;
  }
*/
// Далее делается проверка типа слова, чтобы включить в обработку
// неизменяемые части речи, типы которых перечислены ниже. В этих
// случаях несмотря на нулевую ссылку на таблицы окончаний основа
// считается вполне легальной и поступает в список словооснов.
  if ( (lexeme.wdinfo & 0x3F) < 48 )
  {
    if ( (lexeme.tfoffs = findex.Find( stOrig.c_str() )) == 0 && strcmp( zapart, "0" ) != 0 )
      return lexemeinfo();
  }
    else
  lexeme.tfoffs = 0;

// Скопировать основную форму слова и отщепить постфикс, если он есть
  lexeme.ststem = std::string( sznorm );

  for ( auto pos = lexeme.ststem.find( '=' ); pos != std::string::npos; pos = lexeme.ststem.find( '=', pos ) )
    lexeme.ststem.erase( pos, 1 );
    
// Отщепить постфикс
  lexeme.stpost = GetPostfix( szcomm );
  lexeme.ststem.resize( lexeme.ststem.length() - lexeme.stpost.length() );

// Извлечь флаги описания лексической базы
  if ( strstr( szcomm, st_excellent.c_str() ) != nullptr )
    lexeme.wdinfo |= wfExcellent;
  if ( strstr( szcomm, st_countable.c_str() ) != nullptr )
    lexeme.wdinfo |= wfCountable;
  if ( strstr( szcomm, st_colloquial.c_str() ) != nullptr )
    lexeme.wdinfo |= wfInformal;
  if ( strstr( szcomm, st_obscene.c_str() ) != nullptr )
    lexeme.wdinfo |= wfObscene;

// Закончить обработку нефлективных слов
  if ( lexeme.tfoffs == 0 )
  {
  // Если слово - предлог, извлечь падежную шкалу
    if ( (lexeme.wdinfo & 0x3F) == 51 )
      lexeme.tfoffs = CaseScale( zapart );
    lexeme.chrmin = lexeme.chrmax = '\0';
      return std::move( lexeme );
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
    const uint16_t* lpLevels;
    uint16_t        normInfo;

    switch ( lexeme.wdinfo & 0x3F )
    {
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
        normInfo = vtInfinitiv|gfRetForms;
        lpLevels = libmorph::verbLevels;
        break;
      case 25:
      case 26:
      case 27:
      case 28:
      case 34:
      case 36:
      case 42:
      case 52:
        normInfo = (1 << 9) + (Reflexive( lexeme.ststem ) ? gfRetForms : 0);    // Мужской род для прилагательных
        lpLevels = libmorph::nounLevels;
        break;
      default:
        normInfo = 0;
        lpLevels = libmorph::nounLevels;
        break;
    }
  // Обозначить множественное число, если оно есть
    if ( lexeme.wdinfo & wfMultiple )
      normInfo |= gfMultiple;

  // Отщепить окончание нормальной формы
    if ( !libmorph::FlexStripper( lpLevels, ftable ).StripStr( lexeme.ststem, normInfo, lexeme.tfoffs ) && strchr( zapart, ':' ) == nullptr )
      return lexemeinfo();
  }

// инициализировать минимальный и максимальный символы по таблице окончаний
  std::tie(lexeme.chrmin, lexeme.chrmax) = libmorph::FlexStripper::GetMinMaxChar( ftable, lexeme.tfoffs );

// После отщепления окончания нормальной формы словооснова проверяется на наличие чередований.
  lexeme.mtoffs = mindex.Find( mtable, stOrig.c_str(),
                               lexeme.wdinfo, lexeme.ststem.c_str(), GetRemark( szcomm ).c_str() );

// Если чередования присутствуют, то отщепляется первая ступень
// чередования в основе, т. к. она соответствует нормальной форме.
  if ( lexeme.mtoffs != 0 )
  {
    std::tie(lexeme.chrmin, lexeme.chrmax) = libmorph::rus::Alternator::GetMinMaxChar( mtable, lexeme.mtoffs, lexeme.chrmin, lexeme.chrmax );

  // Проверить, нет ли явного указания типа чередования
  // Отщепить чередование первой ступени
    if ( (lexeme.wdinfo & 0x3F) <= 6 )
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
    lexeme.ststem.resize( lexeme.ststem.length() - (0x0f & *libmorph::rus::Alternator::GetDefaultStr( mtable, lexeme.mtoffs )) );
  }

  if ( mixIndex != 0 )
    lexeme.wdinfo |= (mixIndex << 8);

  return std::move( lexeme );
}

#if defined( _MSC_VER )
  #pragma warning( default: 4237 )
#endif
