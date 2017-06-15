# include "mtables.h"

// CMixTable

// Поддерживаемые типы чередований в данной реализации
//{ ле            - перед последней согласной стоит "ле"          }
//{ ге            - перед последней согласной стоит гласная + е   }
//{ ш             - шипящие - ш, ж, ч, щ, ц                       }
//{ к             - символы к, г, х                               }
//{ ь             - символ ь                                      }
//{ й             - символ й                                      }
//{ о             - так называемый "обычный" вариант              }

//=======================================================================
// Method:: MapMixType
// Определяет тип чередования по переданному тексту
//=======================================================================
static int  MapMixType( const char* strmixtype )
{
  const struct
  {
    const char* typeSt;
    int         typeId;
  } typeList[] =
  {
    { "ге", 1 },
    { "й",  5 },
    { "к",  3 },
    { "ле", 0 },
    { "о",  6 },
    { "ш",  2 },
    { "ь",  4 }
  };

  for ( int i = 0; i < (int)(sizeof(typeList) / sizeof(typeList[0])); i++ )
  {
    int   cmpres = strcmp( strmixtype, typeList[i].typeSt );

    if ( cmpres > 0 )
      continue;
    if ( cmpres < 0 )
      break;
    return typeList[i].typeId;
  }
  return -1;
}

inline  bool  StemHasTail( const char* stem, const char* tail )
{
  size_t  ccStem = strlen( stem );
  size_t  ccTail = 0x0f & *tail++;

  return ccTail <= ccStem && memcmp( stem + ccStem - ccTail, tail, ccTail ) == 0;
}
         
unsigned  mixtable::GetMix( unsigned short type, const char* stem, const char* rems )
{
  const mixclass* p;

  for ( p = begin(); p < end(); ++p )
  {
  // Проверить, есть ли совпадение конца слова с чередованием по умолчанию
    if ( !StemHasTail( stem, GetDefText( tables, p->offset ) ) )
      continue;

  // Если слово является глаголом, применяются специальные правила
  // определения соответствия чередования
    if ( (type & 0x001F) >= 1 && (type & 0x001F) <= 6 )
    {
    // Ссылка присваивается, если совпадает комментарий или это -
    // обычный случай, и если есть совпадение первой ступени че-
    // редования с концом переданного слова
      if ( strcmp( p->szcond, rems ) != 0 && strcmp( p->szcond, "о" ) != 0 )
        continue;

      return p->offset;
    }
      else
    {
      size_t  ccStem = strlen( stem );

      switch ( MapMixType( p->szcond ) )
      {
      /* ле */
        case 0:
          if ( ccStem < 3 )
            break;
          if ( memcmp( stem + ccStem - 3, "ле", 2 ) != 0 )
            break;
          return p->offset;
      /* ге */
        case 1:
          if ( ccStem < 3 )
            break;
          if ( stem[ccStem - 2] != 'е' )
            break;
          if ( strchr( "аеиоуыэюя", stem[ccStem - 3] ) == NULL )
            break;
          return p->offset;
      /* ш  */
        case 2:
          if ( strchr( "жцчшщ", stem[ccStem - 2] ) == NULL )
            break;
          return p->offset;
      /* к  */
        case 3:
          if ( strchr( "кгх", stem[ccStem - 2] ) == NULL )
            break;
          return p->offset;
      /* ь  */
        case 4:
          if ( stem[ccStem - 2] != 'ь' )
            break;
          return p->offset;
      /* й  */
        case 5:
          if ( stem[ccStem - 2] != 'й' )
            break;
      /* о  */
        case 6:
          return p->offset;
        default:
          continue;
      }
    }
  }
  return 0;
}

// mixfiles

unsigned  mixfiles::GetMix( unsigned short type, const char* stem, const char* ztyp, const char* rems )
{
  int*  tabpos;

  if ( (tabpos = tableref.Search( ztyp )) != NULL )
    return (*this)[(size_t)*tabpos].GetMix( type, stem, rems );

  return 0;
}
