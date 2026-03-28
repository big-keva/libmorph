/******************************************************************************

    libfuzzyrus - fuzzy morphological analyser for Russian.

    Copyright (c) 1994-2026 Andrew Kovalenko aka Keva

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
      email: keva@rambler.ru
      Phone: +7(495)648-4058, +7(926)513-2991

******************************************************************************/
# if !defined( __libfuzzy_rus_lemmatizer_hpp__ )
# define __libfuzzy_rus_lemmatizer_hpp__
# include <rus.h>
# include "../chartype.h"
# include "classtable.hpp"
# include <mtc/serialize.h>
# include <algorithm>
# include <cmath>

namespace NAMESPACE
{

  extern const float inflexProbTable[];

  extern struct tagPspProbTable
  {
    const float     weight;
    const float*    ranges;
    const uint16_t  maxLen;
  } pspfidProbTable[];

  using namespace libmorph;

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
      const CapScheme&          capset,
      const target<SStemInfoA>& astems,
      const MbcsCoder&          decode,
      const target<SGramInfo>&  agrams,
      const uint8_t*            strorg,
      size_t                    strlen,
      uint16_t                  scheme,
      unsigned                  dwSets ):
        casing( capset ),
        pszstr( strorg ),
        cchstr( strlen ),
        scheme( scheme ),
        dwsets( dwSets ),

        pstems( astems ),
        pforms( decode ),
        pgrams( agrams ) {}

  public:
    int   operator ()(
      const char* plemma,
      size_t      clemma,
      unsigned    uclass,
      unsigned    upower,   /* мощность модели          */
      uint8_t     idform )
    {
      uint8_t   partsp;     /* часть речи               */
      uint8_t   clSets;
      int       fcount;     /* размер таблицы окончаний */
      auto      pclass = ::FetchFrom( ::FetchFrom( ::FetchFrom( GetClass( uclass ),
        partsp ),
        clSets ),
        fcount );
      int       nerror;

    // убедиться, что заполняется текущий грмматический класс
    // для классво с ударным окончанием оштрафовать на 60%
      if ( nbuilt == 0 || IsAnotherClass( astems[nbuilt - 1], uclass, clemma + 2 ) )
      {
        auto  fxRank = inflexProbTable[cchstr - clemma - 1];
        auto  ocRank = log( upower ) / log( 9000 );
        auto& ranker = pspfidProbTable[partsp];
        auto  psRank = ranker.ranges != nullptr ? ranker.weight : 0.00;
        auto  fmRank = ranker.ranges != nullptr ? ranker.ranges[idform] : 0.00;
        auto  weight = (0.4 * fxRank + 0.3 * ocRank + 0.2 * psRank + 0.1 * fmRank)
          * ((clSets & 0x01) != 0 ? 0.4 : 1.0);

      // проверить внутреннюю размерность массива
        if ( nbuilt == sizeof(astems) / sizeof(astems[0] ) )
          return LEMMBUFF_FAILED;

      // если есть старый грамматический класс, ограничить количество грамматик в нём
        if ( nbuilt != 0 )
          astems[nbuilt - 1].ngrams = pgrams.cur - astems[nbuilt - 1].pgrams;

      // добавить новую лемму
        astems[nbuilt++] = { pforms.getptr(), (unsigned)clemma + 2, uclass,
          pgrams.cur, 0, float(weight) };   // глубина сканирования окончания

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

      // for utf-8, correct stem lengths
        if ( pforms.codepage() == codepages::codepage_utf8 && pforms.getptr() != nullptr )
          for ( auto p = astems; p != astems + nbuilt; ++p )
          {
            auto  srcstr = pszstr;
            auto  stmlen = p->ccstem;
            auto  newlen = 0U;

            while ( stmlen-- > 0 )
              newlen += codepages::utf8::cbchar( codepages::xlatWinToUtf16[*srcstr++] );

            p->ccstem = newlen;
          }

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
      uint8_t   /*partSp*/ )
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

    // encode to output
      if ( !pforms.append( szform, ccform ) || !pforms.append( '\0' ) )
        return LEMMBUFF_FAILED;

      return 0;
    }
  };

}

# endif // !__libfuzzy_rus_lemmatizer_hpp__
