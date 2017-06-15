# include "lresolve.h"
# include <libcodes/codes.h>
# include <tools/ftables.h>
# include <assert.h>

#if defined( _MSC_VER )
  #pragma warning( disable: 4237 )
#endif

//=====================================================================
// Method: GetRemark()
// Функция ищет пометы об особенностях в чередовании, соответствующие
// формату -nx-, где nx - некоторое количество символов, и возвращает
// строку - помету.
//=====================================================================
static  char* GetRemark( char* lpdest, const char* string )
{
  const char* strTop;
  const char* strEnd;
  
  *lpdest = '\0';

  if ( (strEnd = strTop = strchr( string, '-' )) == nullptr )
    return lpdest;

  do ++strEnd;
    while ( *strEnd != '\0' && *strEnd != '-' && (unsigned char)*strEnd > 0x20 );

  if ( *strEnd == '-' )
    strncpy( lpdest, strTop + 1, strEnd - strTop - 1 )[strEnd - strTop - 1] = '\0';

  return lpdest;
}

//=========================================================================
// Method: GetCaseScale( ... )
//
// Функция извлекает падежную шкалу предлога из строки комметариев к слову
//=========================================================================
static byte_t GetCaseScale( const char* string )
{
  static char caseString[] = "ИРДВТП";
  static char caseScales[] = "\x01\x02\x04\x08\x10\x20";
  byte_t      fetchValue = 0;
  const char* searchChar;

  if ( (string = strstr( string, "ШП:" )) != nullptr )
    for ( string += 3; *string != '\0' && (searchChar = strchr( caseString, *string )) != nullptr; ++string )
      fetchValue |= caseScales[searchChar - caseString];

  return fetchValue;
}

//=====================================================================
// Method: ExtractPostfix()
// Функция извлекает строку постфикса из комментария к слову и возвра-
// щает длину постфикса
//=====================================================================
inline  int   ExtractPostfix( char* szpost, const char* string )
{
  const char* strtop;
  const char* strend;

// Проверить, есть ли постфикс в строке, пропустить пробелы
// Извлечь собственно текст постфикса
  if ( (strtop = strstr( string, "post:" )) == NULL )
    return 0;
  for ( strtop += 5; *strtop != '\0' && (unsigned char)*strtop <= 0x20; strtop++ )
    (void)NULL;
  for ( strend = strtop; (unsigned char)*strend > 0x20; strend++ )
    (void)NULL;

// Скопировать текст
  strncpy( szpost, strtop, strend - strtop )[strend - strtop] = '\0';
    return (int)(strend - strtop);
}

bool  ResolveClassInfo( const zarray<>& ztypes,
                        char*           pstems,
                        rusclassinfo&   clinfo,
                        const char*     sznorm, const char*   szdies, const char*   sztype, const char* zapart, const char*   szcomm,
                        const char*     ftable, CReferences&  findex, const char*   mtable, mixfiles&   mindex )
{
  char    ansiType[32];
  char    utf8Type[32];
  char    origType[32];
  char    szremark[32];
  int     mixIndex = 0;

// Сразу определить тип слова
  strcat( strcat( strcpy( ansiType, szdies ), " " ), sztype );
  strcat( strcat( strcpy( origType, sztype ), " " ), zapart );

  codepages::mbcstombcs( codepages::codepage_utf8, utf8Type, sizeof(utf8Type),
                         codepages::codepage_1251, ansiType );

  if ( (clinfo.wdinfo = (word16_t)(int32_t)ztypes[utf8Type]) == 0 )
    return false;

// Далее делается проверка типа слова, чтобы включить в обработку
// неизменяемые части речи, типы которых перечислены ниже. В этих
// случаях несмотря на нулевую ссылку на таблицы окончаний основа
// считается вполне легальной и поступает в список словооснов.
  if ( (clinfo.wdinfo & 0x3F) < 48 )
  {
    if ( (clinfo.tfoffs = findex.GetOffset( origType )) == 0 && strcmp( zapart, "0" ) != 0 )
      return false;
  }
    else
  clinfo.tfoffs = 0;

// Скопировать основную форму слова и отщепить постфикс, если он есть
  for ( auto d = strcpy( pstems, sznorm ), s = pstems; (*d = *s++) != '\0'; )
    if ( *d != '=' )  ++d;

// Отщепить постфикс
  pstems[strlen( sznorm ) - ExtractPostfix( clinfo.szpost, szcomm )] = '\0';

// Извлечь флаги описания лексической базы
  if ( strstr( szcomm, "[превосх.]" ) != nullptr )
    clinfo.wdinfo |= wfExcellent;
  if ( strstr( szcomm, "{исчисл.}" ) != nullptr )
    clinfo.wdinfo |= wfCountable;
  if ( strstr( szcomm, "{разг.}" ) != nullptr )
    clinfo.wdinfo |= wfInformal;
  if ( strstr( szcomm, "{руг.}" ) != nullptr )
    clinfo.wdinfo |= wfObscene;

// Закончить обработку нефлективных слов
  if ( clinfo.tfoffs == 0 )
  {
  // Если слово - предлог, извлечь падежную шкалу
    if ( (clinfo.wdinfo & 0x3F) == 51 )
      clinfo.tfoffs = GetCaseScale( zapart );
    clinfo.chrmin = clinfo.chrmax = '\0';
      return true;
  }

// В случае, если ссылка на таблицу окончаний ненулевая, отбросить
// окончание нормальной формы или специально помеченное символом
// @ - для случаев, когда в современном языке нормальная форма
// слова не употребляется
// Сначала делается проверка, нет ли специально помеченного
// окончания, как это бывает, например, у существительных, упот-
// ребляемых только в определенных формах. Если так, то окон-
// чание отщепляется простым уменьшением длины строки.
  if ( strchr( pstems, '@' ) != NULL ) strchr( pstems, '@' )[0] = '\0';
    else
  {
  //   В противном случае строится исходя из типа слова граммати-
  // ческая информация о нормальной форме и вызывается процедура
  // его отщепления. Это бывает, вообще, практически всегда.
  //   В случае глаголов (типы 1..6) нормальной формой является
  // инфинитив (с возможной возвратной частицей). В случае прила-
  // гательных и других слов, склоняющихся по родам, выставляется
  // признак мужского рода в дополнение к именительному падежу.
    unsigned short* lpLevels;
    unsigned short  normInfo;

    switch ( clinfo.wdinfo & 0x3F )
    {
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
        normInfo = vtInfinitiv|gfRetForms;
        lpLevels = (unsigned short*)verbLevels;
        break;
      case 25:
      case 26:
      case 27:
      case 28:
      case 34:
      case 36:
      case 42:
      case 52:
        normInfo = 1 << 9;    // Мужской род для прилагательных
        if ( strcmp( pstems + strlen( pstems ) - 2, "ся" ) == 0 )
          normInfo |= gfRetForms;
        lpLevels = (unsigned short*)nounLevels;
        break;
      default:
        normInfo = 0;
        lpLevels = (unsigned short*)nounLevels;
        break;
    }
  // Обозначить множественное число, если оно есть
    if ( clinfo.wdinfo & wfMultiple )
      normInfo |= gfMultiple;

  // Отщепить окончание нормальной формы
    if ( !StripDefault( pstems, normInfo, clinfo.tfoffs, lpLevels, 0, ftable ) && ( strchr( zapart, ':' ) == NULL ) )
      return false;
  }

// инициализировать минимальный и максимальный символы по таблице окончаний
  clinfo.chrmin = GetMinLetter( ftable, clinfo.tfoffs );
  clinfo.chrmax = GetMaxLetter( ftable, clinfo.tfoffs );

// После отщепления окончания нормальной формы словооснова прове-
// ряется на наличие чередований.
  clinfo.mtoffs = mindex.GetMix( clinfo.wdinfo, pstems, origType, GetRemark( szremark, szcomm ) );

// Если чередования присутствуют, то отщепляется первая ступень
// чередования в основе, т. к. она соответствует нормальной форме.
  if ( clinfo.mtoffs != 0 )
  {
    clinfo.chrmin = GetMinLetter( mtable, clinfo.mtoffs, clinfo.chrmin );
    clinfo.chrmax = GetMaxLetter( mtable, clinfo.mtoffs, clinfo.chrmax );

  // Проверить, нет ли явного указания типа чередования
  // Отщепить чередование первой ступени
    if ( (clinfo.wdinfo & 0x3F) <= 6 )
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
    pstems[strlen( pstems ) - (0x0f & *GetDefText( mtable, clinfo.mtoffs ))] = '\0';
  }
  if ( mixIndex != 0 )
    clinfo.wdinfo |= (mixIndex << 8);
  return true;
}

#if defined( _MSC_VER )
  #pragma warning( default: 4237 )
#endif
