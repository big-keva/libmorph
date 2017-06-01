# include "ftables.h"
# include <string.h>

#if defined( _MSC_VER )
  #pragma warning( disable: 4237 )
#endif

inline  unsigned char   flex_flag( const unsigned char* flex )
  {  return *flex;  }
inline  unsigned short  flex_info( const unsigned char* flex )
  {  return *(unsigned short*)(flex + 1);  }
inline  unsigned char*  flex_text( const unsigned char* flex )
  {  return (unsigned char*)(flex + 3);  }
inline  unsigned short  flex_next( const unsigned char* flex )
  {  return *(unsigned short*)(flex + flex[3] + 4);  }

const unsigned short verbLevels[3] = { 0x007F, 0x7F80, 0x8000/*gfRetForms*/ };
const unsigned short nounLevels[3] = { 0xFFFF, 0, 0 };

inline unsigned short GetUpLevels( unsigned short* levels,
                                   int             clevel )
{
  unsigned short  destBits = 0x0000;

  for ( int i = clevel + 1; i < 3; i++ )
    destBits |= levels[i];
  return destBits;
}

template <class T>
inline  unsigned short  getword16( const T*& p )
{
  unsigned short  v = *(unsigned short*)p;
    p = (T*)(sizeof(unsigned short) + (char*)p);
  return v;
}

bool  StripDefault( char*           szstem,
                    unsigned short  grinfo,
                    unsigned short  tfoffs,
                    unsigned short* levels,
                    int             clevel,
                    const char*     tables )
{
  char* pTable = (char*)tables + (tfoffs << 1); // Таблица окончаний
  int   nItems = *pTable++ & 0x7F;              // Количество элементов

// Перебрать все элементы таблицы
  while ( nItems > 0 )
  {
    unsigned char*  flex = (unsigned char*)pTable;
    unsigned char*  pstr = flex_text( flex );  // Текст фрагмента

  // skip to next
    pTable = (char*)(pstr + *pstr + 1 + ((flex_flag( flex ) & 0xC0) != 0 ? sizeof(unsigned short) : 0));
    nItems--; 

  // Пропустить окончания с заведомо не соответствующей грамматической
  // информацией о фрагменте
    if ( (grinfo & levels[clevel]) != (flex_info( flex ) & levels[clevel]) )
      continue;

    if ( ( (grinfo & levels[clevel]) == 0 ) && ( levels[clevel] != 0xFFFF ) )
      continue;

    if ( ( clevel < 2 ) && ( (flex_flag( flex ) & 0x80) != 0 ) )
    {
      if ( StripDefault( szstem, grinfo, flex_next( flex ), levels, clevel, tables )
        && StringHasEnd( szstem, (const char*)pstr ) )
      {
        szstem[strlen( szstem ) - pstr[0]] = '\0';
        return true;
      }
    }
      else
    {
      if ( !StringHasEnd( szstem, (const char*)pstr ) )
        continue;
      szstem[strlen( szstem ) - pstr[0]] = '\0';
      return true;
    }
  }
  if ( ( clevel < 2 ) && ( GetUpLevels( levels, clevel ) != 0 ) )
    return StripDefault( szstem, grinfo, tfoffs, levels, clevel + 1, tables );
  return false;
}

unsigned char GetMinLetter( const char* tables, unsigned tfoffs )
{
  const unsigned char*  ptable = (const unsigned char*)tables + (tfoffs << 1);  // Таблица окончаний
  int                   nitems = *ptable++ & 0x7F;        // Количество элементов
  int                   clower;

// Перебрать все элементы таблицы
  for ( clower = 0x100; nitems-- > 0; )
  {
    const unsigned char*  flex = ptable;
    const unsigned char*  pstr = flex_text( flex );  // Текст фрагмента
    int                   strl = *pstr++;
    int                   subc;

    ptable = pstr + strl + ((flex_flag( flex ) & 0xC0) != 0 ? sizeof(unsigned short) : 0);

    if ( strl > 0 ) clower = *pstr <= clower ? *pstr : clower; 
      else
    if ( (flex_flag( flex ) & 0x80) == 0 )  clower = 0;
      else
    if ( (subc = GetMinLetter( tables, flex_next( flex ) )) < clower )
      clower = subc;
  }
  return (unsigned char)clower;
}

unsigned char GetMaxLetter( const char* tables, unsigned tfoffs )
{
  const unsigned char*  ptable = (const unsigned char*)tables + (tfoffs << 1);  // Таблица окончаний
  int                   nitems = *ptable++ & 0x7F;        // Количество элементов
  int                   cupper;

// Перебрать все элементы таблицы
  for ( cupper = -1; nitems-- > 0; )
  {
    const unsigned char*  flex = ptable;
    const unsigned char*  pstr = flex_text( flex );  // Текст фрагмента
    int                   strl = *pstr++;
    int                   subc;

    ptable = pstr + strl + ((flex_flag( flex ) & 0xC0) != 0 ? sizeof(unsigned short) : 0);

    if ( strl > 0 ) cupper = *pstr >= cupper ? *pstr : cupper; 
      else
    if ( (flex_flag( flex ) & 0x80) == 0 )  cupper = cupper >= 0 ? cupper : 0;
      else
    if ( (subc = GetMaxLetter( tables, flex_next( flex ) )) > cupper )
      cupper = subc;
  }
  return (unsigned char)cupper;
}

#if defined( _MSC_VER )
  #pragma warning( default: 4237 )
#endif
