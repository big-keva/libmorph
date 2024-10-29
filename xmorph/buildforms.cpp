# include "buildforms.hpp"
# include <cstdint>

namespace libmorph {

 /*
  * GetFlexForms( ... )
  * Формирует массив форм слова, добавляя окончания к переданному префиксу.
  * Тиражирует префикс на нужное количество форм по мере их построения.
  * Возвращает количество построенных форм слова или -1 при переполнении
  * аккумулятора.
  */
  int   GetFlexForms(
    Collector&        output,
    const uint8_t*    ptable,
    const flexinfo&   fxinfo,
    const fragment&   prefix,
    const fragment&   suffix )
 {
    uint8_t atrack[0x40];
    int     ntails = 0;
    auto    rtrack = Flat::GetTrack<uint8_t>( [&]( const uint8_t* tabptr, const fragment& szflex )
    {
      for ( auto ngrams = *tabptr++; ngrams-- > 0; )
      {
        auto  fxnext = flexinfo{ getword16( tabptr ), *tabptr++ };

        if ( fxnext.gramm == fxinfo.gramm && (fxnext.flags & fxinfo.flags) != 0 )
        {
          return output.append( (const char*)prefix.str, prefix.len )
              && output.append( (const char*)szflex.str, szflex.len )
              && output.append( (const char*)suffix.str, suffix.len )
              && output.append( '\0' ) ? ++ntails, 0 : -1;
        }
      }
      return 0;
    }, ptable, atrack, 0, nullptr );

    return rtrack == 0 ? ntails : rtrack;
 }

}
