# if !defined( __libmorph_flexmake_h__ )
# define __libmorph_flexmake_h__
# include <namespace.h>
# include "typedefs.h"

namespace LIBMORPH_NAMESPACE
{

/*
 * BuildFlexSet(
 *   [out] output buffer,
 *   [in] flexion table,
 *   [in] grammatical description,
 *   [in] default flexion prefix
 *
 * Заполняет массив output вариантами окончаний для переданной грамматики, дополнив их
 * переданным префиксом и разделив нулевым символом.
 *
 * Возвращает количество построенных строк.
 *
 * Проверка размера массива output не делается - считается, что в статически выверенном словаре
 * с предопределённой системой окончаний всегда можно предсказать достаточный его размер.
 */
  int   BuildFlexSet(
    byte_t*         output,
    const byte_t*   ptable,
    const flexinfo& inflex,
    const fragment& prefix = { nullptr, 0 } );

} // end namespace

# endif // !__libmorph_flexmake_h__
