/******************************************************************************

    libfuzzyrus - fuzzy morphological analyser for Russian.

    Copyright (C) 1994-2016 Andrew Kovalenko aka Keva

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

    Commercial license is available upon request.

    Contacts:
      email: keva@meta.ua, keva@rambler.ru
      Phone: +7(495)648-4058, +7(926)513-2991, +7(707)129-1418

******************************************************************************/
# if !defined( __libfuzzy_rus_lemmatizer_hpp__ )
# define __libfuzzy_rus_lemmatizer_hpp__
# include "rus/include/mlfa1049.h"
# include "rus/chartype.h"
# include "classtable.hpp"
# include "mtc/serialize.h"
# include <algorithm>
# include <cmath>

namespace libfuzzy {
namespace rus {

  using namespace libmorph;
  using namespace libmorph::rus;

  class Lemmatizer
  {
    template <class T>
    struct  target
    {
      T*  beg = nullptr;
      T*  cur = nullptr;
      T*  lim = nullptr;

    public:
      target() = default;
      target( T* p, size_t l ): beg( p ), cur( p ), lim( p + l )  {}

    };

    const CapScheme     casing;
    const uint8_t*      pszstr;
    const size_t        cchstr;
    const uint16_t      scheme;
    const unsigned      dwsets;

    mutable target<SStemInfoA>  pstems;
    mutable MbcsCoder           pforms;
    mutable target<SGramInfo>   pgrams;

    SStemInfoA                  astems[64];
    int                         nbuilt = 0;

  public:
    Lemmatizer(
      const CapScheme&          cs,
      const target<SStemInfoA>& sm,
      const MbcsCoder&          fm,
      const target<SGramInfo>&  gr,
      const uint8_t*            st,
      size_t                    cc,
      uint16_t                  sh,
      unsigned                  us ):
        casing( cs ),
        pszstr( st ),
        cchstr( cc ),
        scheme( sh ),
        dwsets( us ),

        pstems( sm ),
        pforms( fm ),
        pgrams( gr ) {}

  public:
    int   operator ()(
      const char* plemma,
      size_t      clemma,
      unsigned    uclass,
      unsigned    uoccur,
      uint8_t     idform )
    {
      uint8_t   partsp;
      int       fcount;
      auto      pclass = ::FetchFrom( ::FetchFrom( GetClass( uclass ), partsp ), fcount );
      int       nerror;

    // check capitalization scheme
      if ( (dwsets & sfIgnoreCapitals) == 0 && !IsGoodSheme( scheme, pspMinCapValue[partsp] ) )
        return 0;

    // убедиться, что заполняется текущий грмматический класс
      if ( nbuilt == 0 || IsAnotherClass( astems[nbuilt - 1], uclass, clemma + 2 ) )
      {
      // проверить внутреннюю размерность массива
        if ( nbuilt == sizeof(astems) / sizeof(astems[0] ) )
          return LEMMBUFF_FAILED;

      // если есть старый грамматический класс, ограничить количество грамматик в нём
        if ( nbuilt != 0 )
          astems[nbuilt - 1].ngrams = pgrams.cur - astems[nbuilt - 1].pgrams;

      // добавить новую лемму
        astems[nbuilt++] = { (unsigned)clemma + 2, uclass,
          pforms.getptr(),
          pgrams.cur, 0, float( log( uoccur ) / log( 10000 ) * sin( atan( 0.4 * (cchstr - clemma) ) ) ) };   // глубина сканирования окончания*/

      // если требуется восстановить нормальные формы слов, построить её
        if ( pforms.getptr() != nullptr )
          if ( (nerror = BuildNormalStr( plemma, clemma + 2, pclass, partsp )) != 0 )
          return nerror;
      }

    // если требуется восстановить грамматические описания, проверить, умещаются ли они
    // в существующий массив
      if ( pgrams.beg != nullptr )
      {
        if ( pgrams.cur != pgrams.lim ) *pgrams.cur++ = { partsp, idform, 0, 0 };
          else return GRAMBUFF_FAILED;
      }

      return 0;
    }
    operator int()
    {
      if ( nbuilt != 0 )
        astems[nbuilt - 1].ngrams = pgrams.cur - astems[nbuilt - 1].pgrams;

      if ( pstems.beg != nullptr )
      {
        if ( nbuilt > pstems.lim - pstems.cur )
          return LEMMBUFF_FAILED;

        std::sort( astems, astems + nbuilt, []( const SStemInfoA& s1, const SStemInfoA& s2 )
          {  return s1.weight > s2.weight;  } );

        std::copy( astems, astems + nbuilt, pstems.beg );
      }
      return nbuilt;
    }

  protected:
    static
    bool  IsAnotherClass(
      const SStemInfoA& rclass,
      unsigned          uclass,
      unsigned          ccstem )
    {
      return rclass.nclass != uclass || rclass.ccstem != ccstem;
    }
    int   BuildNormalStr(
      const char* lpstem,
      size_t      ccstem,
      const char* pclass,
      uint8_t     partSp )
    {
      char    szform[0x40];
      uint8_t ccflex = (uint8_t)pclass[1];  pclass += 2;
      auto    ccform = ccstem + ccflex;

      if ( ccform >= sizeof(szform) )
        return WORDBUFF_FAILED;

    // create normal form
      strncpy( ccstem + (char*)memcpy( szform,
        lpstem, ccstem ),
        pclass, ccflex );

    // set minimal capitalization
      casing.Set( (uint8_t*)szform, scheme >> 8, partSp );

    // encode to output
      if ( !pforms.append( szform, ccform ) || !pforms.append( '\0' ) )
        return LEMMBUFF_FAILED;

      return 0;
    }
  };

}}

# endif // !__libfuzzy_rus_lemmatizer_hpp__
