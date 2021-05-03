#if !defined( _flexmake_h_ )
#define _flexmake_h_

#include <string.h>
#include "mlmadefs.h"

namespace __libmorpheng__
{
  //=====================================================================
  // Meth: GetNormalInfo
  // Функция строит грамматическую информацию о нормальной форме слова,
  // используя тип этого слова, грамматическую информацию об отождествлении
  // и настройки поиска и нормализации.
  // Нормальной формой считается:
  // Для существительных - именительный падеж единственного числа;
  // Для прилагательных - именительный падеж мужского рода;
  // Для глаголов - инфинитив (или причастная форма - по настройкам).
  //=====================================================================
  inline byte08_t GetNormalInfo( word16_t tfoffs )
  {
    byte08_t        minval = 0xff;

    if ( tfoffs != 0 )
    {
      const byte08_t* ptable = fxTables + tfoffs;
      int             ntails = *ptable++;

      while ( ntails-- > 0 )
      {
        if ( *ptable < minval )
          minval = *ptable;
        ptable += sizeof(byte08_t) + sizeof(word16_t);
      }
    }
    return minval;
  }

}

#endif // _flexmake_h_
