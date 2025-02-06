/******************************************************************************

    libmorphrus - dictiorary-based morphological analyser for Russian.

    Copyright (C) 1994-2025 Andrew Kovalenko aka Keva

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
# if !defined( __libmorph_rus_wildClass_hpp__ )
# define __libmorph_rus_wildClass_hpp__
# include "scanClass.hpp"
# include "grammap.h"

namespace libmorph {
namespace rus {

  struct Wildcard final: public Counters
  {
   /*
    * ScanSuffix( ... )
    *
    * Сравнивает остаточный фрагмент окончания с суффиксом, отрабатывает также и шаблоны
    * в последнем.
    */
    template <class Action> static
    int   ScanSuffix(
      const Action&     target,
            uint8_t*    smatch,
            size_t      lmatch,
            fragment    inflex,
            fragment    suffix,
      const SGramInfo&  grinfo )
    {
      int   nerror;

    // skip matching sequence
      for ( ; !inflex.empty() && !suffix.empty() && inflex.front() == suffix.front(); ++inflex, ++suffix )
        smatch[lmatch++] = suffix.front();

    // check if unmatch
      if ( inflex.empty() )
        return suffix.empty() ? target( smatch, lmatch, grinfo ) : 0;

    // here inflex is not empty
      if ( inflex.front() == '*' )
      {
        if ( (nerror = ScanSuffix( target, smatch, lmatch, inflex.next(), suffix, grinfo )) != 0 )
          return nerror;

        if ( !suffix.empty() )
        {
          smatch[lmatch] = suffix.front();
          return ScanSuffix( target, smatch, lmatch + 1, inflex, suffix.next(), grinfo );
        }
        return 0;
      }

    // check empty suffix
      if ( suffix.empty() )
        return 0;

    // check mismatch reason
      if ( inflex.front() == '?' )
      {
        if ( !suffix.empty() )  smatch[lmatch] = suffix.front();
          else return 0;
        return ScanSuffix( target, smatch, lmatch + 1, inflex.next(), suffix.next(), grinfo );
      }

      return 0;
    }

   /*
    * FlexSearch()
    *
    * Перебирает все формы, соответствующие переданному шаблону
    */
    template <class Action, class Filter> static
    int   FlexSearch(
      const Action&   target,
      const Filter&   filter,
            uint8_t*  smatch,
            size_t    lmatch,
      const fragment& inflex,
      const fragment& suffix,
      const uint8_t*  ptable )
    {
      auto  uflags = *ptable++;
      auto  ucount = counter<uint8_t>::getlower( uflags );
      auto  sublen = unsigned{};
      int   nerror;

    // check for zero length match on asterisk
      if ( !inflex.empty() )
      {
        auto  chfind = inflex.front();

      // гипотеза о том, что '*' не соответствует ни одного символа
        if ( chfind == '*' )
          if ( (nerror = FlexSearch( target, filter, smatch, lmatch, inflex.next(), suffix, ptable - 1 )) != 0 )
            return nerror;

        while ( ucount-- > 0 )
        {
          auto  chnext = smatch[lmatch] = *ptable++;
                sublen = getserial( ptable );
          auto  subdic = ptable;
                ptable += sublen;

        // check character find type: wildcard, pattern or regular
          if ( chfind == '?' || chnext == chfind )
          {
            if ( (nerror = FlexSearch( target, filter, smatch, lmatch + 1, inflex.next(), suffix, subdic )) != 0 )
              return nerror;
          }
            else
          if ( chfind == '*' )
          {
          // гипотеза о том, что '*' соответстствует хотя один символ...
            if ( (nerror = FlexSearch( target, filter, smatch, lmatch + 1, inflex.next(), suffix, subdic )) != 0 )
              return nerror;
          // ... и более одного
            if ( (nerror = FlexSearch( target, filter, smatch, lmatch + 1, inflex,        suffix, subdic )) != 0 )
              return nerror;
          }
        }
      }

      if ( inflex.empty() || !suffix.empty() )
      {
      // skip until stems
        for ( ; ucount-- > 0; ptable += sublen )
          sublen = getserial( ++ptable );

        if ( counter<uint8_t>::hasupper( uflags ) )
        {
          auto  nforms = getserial( ptable );

          while ( nforms-- > 0 )
          {
            auto  grinfo = SGramInfo{ 0, 0, getword16( ptable ), *ptable++ };

            if ( filter( grinfo ) )
              if ( (nerror = ScanSuffix( target, smatch, lmatch, inflex, suffix, grinfo )) != 0 )
                return nerror;
          }
        }
      }
      return 0;
    }

   /*
    * SwapSearch( ... )
    *
    * Сравнение строки чередования с шаблоном
    */
    template <class Action, class Filter> static
    int   SwapSearch(
      const Action&   target,
      const Filter&   filter,
            uint8_t*  smatch,
            size_t    nmatch,
            fragment  inflex,
      const fragment& suffix,
            fragment  szswap,
      const uint8_t*  ptable )
    {
      int   nerror;

    // skip matching parts of string
      while ( !szswap.empty() && !inflex.empty() && (smatch[nmatch] = szswap.front()) == inflex.front() )
        ++szswap, ++inflex, ++nmatch;

    // check the type of stop: matching interch string
      if ( szswap.empty() )
        return FlexSearch( target, filter, smatch, nmatch, inflex, suffix, ptable );

    // check if the string is finished or if mismatch
      if ( inflex.empty() || !IsWildcard( inflex.front() ) )
        return 0;

    // check wildcard type for string match, mixlen > 0
      if ( inflex.front() == '?' )
      {
        smatch[nmatch] = szswap.front();

        return SwapSearch(
          target,
          filter,
          smatch, nmatch + 1,
          inflex.next(), suffix, szswap.next(), ptable );
      }

    // for asterisk, check if 0 match
      if ( (nerror = SwapSearch( target, filter, smatch, nmatch, inflex.next(), suffix, szswap, ptable )) != 0 )
        return nerror;
      if ( (nerror = SwapSearch( target, filter, smatch, nmatch, inflex, suffix, szswap.next(), ptable )) != 0 )
        return nerror;

      return 0;
    }

  };

  template <class Collect>
  class MatchModel
  {
    const Collect&  target;

  public:
    MatchModel( const Collect& out ):
      target( out ) {}

  public:
    int   operator()(
      lexeme_t        nlexid,
      uint16_t        oclass,
      const fragment& inflex,
      const fragment& suffix,
      uint8_t*        smatch,
      size_t          lmatch ) const
    {
      auto  lextem = steminfo( oclass );
      auto  output = [&]( const uint8_t* pmatch, size_t nmatch, const SGramInfo& grinfo )
        {
          return target( nlexid, pmatch, nmatch,
            SGramInfo{
              lextem.wdinfo,
              grinfo.idForm != (uint8_t)-1 ? MapWordInfo( lextem.wdinfo, grinfo.grInfo, grinfo.bFlags ) : grinfo.idForm,
              grinfo.grInfo,
              grinfo.bFlags } );
        };
      int   nerror;

    // check if non-flective; only zero flexion match it
      if ( lextem.GetFlexTable() == nullptr )
      {
        return inflex.empty() || IsAsterisk( inflex ) ?
          output( smatch, lmatch, SGramInfo{ 0, 0xff, 0, 0 } ) : 0;
      }

    // Теперь - обработка флективных слов. Сначала обработаем более частый случай - флективное слово
    // без чередований в основе.
    // Для этого достаточно проверить, не выставлен ли флаг наличия чередований, так как при попадании
    // в эту точку слово заведомо будет флективным.
      if ( lextem.GetSwapTable() == nullptr )
      {
        if ( (lextem.wdinfo & wfMultiple) != 0 )
        {
          nerror = Wildcard::FlexSearch( output, Grammar::multiple(), smatch, lmatch,
            inflex,
            suffix, lextem.GetFlexTable() );
        }
          else
        {
          nerror = Wildcard::FlexSearch( output, Grammar::anyvalue(), smatch, lmatch,
            inflex,
            suffix, lextem.GetFlexTable() );
        }
        if ( nerror != 0 )
          return nerror;
      }
        else
    // Ну и, наконец, последний случай - слово с чередованиями в основе. Никаких специальных проверок здесь делать
    // уже не требуется, так как попадание в эту точку само по себе означает наличие чередований в основе
      {
        auto  mixtab = lextem.GetSwapTable();     // Собственно таблица
        auto  mixcnt = *mixtab++;                 // Количество чередований
        int   mindex;

        for ( mindex = 0; mindex < mixcnt; ++mindex, mixtab += 1 + (0x0f & *mixtab) )
        {
          auto    curmix = mixtab;
          auto    mixlen = size_t(0x0f & *curmix);
          auto    powers = *curmix++ >> 4;

        // вызвать сканер чередований
          if ( (lextem.wdinfo & wfMultiple) != 0 )
          {
            nerror = Wildcard::SwapSearch( output, Grammar::Alternator( lextem, powers, Grammar::multiple() ),
              smatch,
              lmatch,
              inflex,
              suffix, { curmix, mixlen }, lextem.GetFlexTable() );
          }
            else
          {
            nerror = Wildcard::SwapSearch( output, Grammar::Alternator( lextem, powers, Grammar::anyvalue() ),
              smatch,
              lmatch,
              inflex,
              suffix, { curmix, mixlen }, lextem.GetFlexTable() );
          }

          if ( nerror != 0 )
            return nerror;
        }
      }
      return 0;
    }
  };

  template <class Collect>
  auto  MakeModelMatch( const Collect& match ) -> MatchModel<Collect>
    {  return MatchModel<Collect>( match );  }

}} // end namespace

# endif // !__libmorph_rus_scanClass_hpp__
