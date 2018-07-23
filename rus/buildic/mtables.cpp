# include "mtables.h"
# include <tools/utf81251.h>
# include <cassert>

namespace libmorph {
namespace rus {

  // Поддерживаемые типы чередований в данной реализации
  //{ ле            - перед последней согласной стоит "ле"          }
  //{ ге            - перед последней согласной стоит гласная + е   }
  //{ ш             - шипящие - ш, ж, ч, щ, ц                       }
  //{ к             - символы к, г, х                               }
  //{ ь             - символ ь                                      }
  //{ й             - символ й                                      }
  //{ о             - так называемый "обычный" вариант              }

  static const struct
  {
    std::string typeSt;
    int         typeId;
  } typeList[] =
  {
    { utf8to1251( "ге" ), 1 },
    { utf8to1251( "й" ),  5 },
    { utf8to1251( "к" ),  3 },
    { utf8to1251( "ле" ), 0 },
    { utf8to1251( "о" ),  6 },
    { utf8to1251( "ш" ),  2 },
    { utf8to1251( "ь" ),  4 }
  };

  auto  string_le   = utf8to1251( "ле" );
  auto  string_o    = utf8to1251( "о" );
  auto  string_e    = utf8to1251( "е" );
  auto  string_vow  = utf8to1251( "аеиоуыэюя" );
  auto  string_his  = utf8to1251( "жцчшщ" );
  auto  string_kgh  = utf8to1251( "кгх" );
  auto  string_soft = utf8to1251( "ь" );
  auto  string_yot  = utf8to1251( "й" );

  //=======================================================================
  // Method:: MapMixType
  // Определяет тип чередования по переданному тексту
  //=======================================================================
  inline  int  MapMixType( const char* strmixtype )
  {
    for ( int i = 0; i < (int)(sizeof(typeList) / sizeof(typeList[0])); i++ )
    {
      int   cmpres = strcmp( strmixtype, typeList[i].typeSt.c_str() );

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

  inline  bool  IsVowel( char c )
  {
    return strchr( string_vow.c_str(), c ) != nullptr;
  }

  inline  bool  IsHissing( char c )
  {
    return strchr( string_his.c_str(), c ) != nullptr;
  }
         
  inline  bool  IsKgh( char c )
  {
    return strchr( string_kgh.c_str(), c ) != nullptr;
  }
         
  // Alternator

  const char* Alternator::GetDefaultStr( const char* tables, unsigned tbOffs )
  {
    const char* mixtab = tables + tbOffs;
    int         tablen = *mixtab++;

    while ( tablen-- > 0 )
    {
      if ( (*mixtab & 0x10) != 0 )  return mixtab;
        else  mixtab += 1 + (*mixtab & 0x0f);
    }
    assert( "Invalid interchange table: no default string!" == NULL );
    return nullptr;
  }

  std::tuple<uint8_t, uint8_t>  Alternator::GetMinMaxChar( const char* tables, uint16_t tboffs, uint8_t chrmin, uint8_t chrmax )
  {
    const uint8_t*  mixtab = (const uint8_t*)tables + tboffs;
    int             tablen = *mixtab++;
    int             mixmin = 0xff;
    int             mixmax = -1;
    uint8_t         mflags;

    for ( ; tablen-- > 0; mixtab += (mflags & 0x0f) )
    {
      mflags = *mixtab++;

      if ( (mflags & 0x0f) > 0 )
      {
        mixmin = std::min( mixmin, (int)*mixtab );
        mixmax = std::max( mixmax, (int)*mixtab );
      }
        else
      mixmin = mixmax = 0;
    }

    return std::make_tuple(
      (uint8_t)(mixmin != 0 ? mixmin : chrmin),
      (uint8_t)(mixmax <= 0 ? chrmax : mixmax)
    );
  }

  uint16_t  Alternator::tab::Find( const char* tabs, uint16_t type, const char* stem, const char* rems ) const
  {
    size_t  ccStem = strlen( stem );

    for ( auto& alt: *this )
    {
      // Проверить, есть ли совпадение конца слова с чередованием по умолчанию;
      if ( !StemHasTail( stem, GetDefaultStr( tabs, alt.offset ) ) )
        continue;

      // Если слово является глаголом, применяются специальные правила определения чередования:
      // ссылка присваивается, если совпадает комментарий или это - обычный случай
      if ( (type & 0x001F) >= 1 && (type & 0x001F) <= 6 )
      {
        if ( strcmp( alt.szcond, rems ) == 0 || alt.szcond == string_o )
          return alt.offset;
        continue;
      }

      // Иначе применить обычные правила выбора ступени чередования
      switch ( MapMixType( alt.szcond ) )
      {
      /* ле */
        case 0:
          if ( ccStem >= 3 && strncmp( string_le.c_str(), stem + ccStem - 3, string_le.length() ) == 0 )  return alt.offset;
            else break;

      /* ге */
        case 1:
          if ( ccStem >= 3 && IsVowel( stem[ccStem - 3] ) && stem[ccStem - 2] == string_e[0] )  return alt.offset;
            else break;

      /* ш  */
        case 2:
          if ( ccStem >= 2 && IsHissing( stem[ccStem - 2] ) ) return alt.offset;
            else break;

      /* к  */
        case 3:
          if ( ccStem >= 2 && IsKgh( stem[ccStem - 2] ) ) return alt.offset;
            else break;

      /* ь  */
        case 4:
          if ( ccStem >= 2 && stem[ccStem - 2] == string_soft[0] )  return alt.offset;
            else break;

      /* й  */
        case 5:
          if ( ccStem >= 2 && stem[ccStem - 2] == string_yot[0] ) return alt.offset;
            else break;

      /* о  */
        case 6:
          return alt.offset;

        default:
          break;
      }
    }
    return 0;
  }

  uint16_t  Alternator::Find( const char* tabs, const char* ztyp, uint16_t  type, const char* stem, const char* rems ) const
  {
    auto  it = mapper.find( std::string( ztyp ) );

    return it != mapper.end() ? it->second->Find( tabs, type, stem, rems ) : 0;
  }

}}
