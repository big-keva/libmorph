# if !defined( _flexmake_h_ )
# define _flexmake_h_

# include <namespace.h>
# include "mlmadefs.h"
# include <string.h>

namespace LIBMORPH_NAMESPACE
{

  int   CreateDestStringsOnTable( byte08_t*       pszout,
                                  byte08_t*       curstr,
                                  unsigned        curlen,
                                  const byte08_t* ptable,
                                  word16_t        grInfo,
                                  byte08_t        bflags );

  inline  bool  IsParticiple( word16_t grInfo )
  {
    return (grInfo & gfVerbForm) == vfVerbActive || (grInfo & gfVerbForm) == vfVerbPassiv;
  }

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
  inline word16_t GetNormalInfo( word16_t wbInfo,
                                 word16_t grInfo,
                                 unsigned nFlags )
  {
    word16_t  nfInfo = 0;

    if ( IsVerb( wbInfo ) )
    {
      nfInfo = vtInfinitiv;
      if ( ( nFlags & nfAdjVerbs ) && IsParticiple( grInfo ) )
        nfInfo = (word16_t)((grInfo & (gfVerbTime|gfVerbForm)) | (1 << 9));
    }
      else
    if ( IsAdjective( wbInfo ) )
      nfInfo = 1 << 9;
    if ( wbInfo & wfMultiple )
      nfInfo |= gfMultiple;
    return nfInfo;
  }

} // end namespace

# endif // _flexmake_h_
