# if !defined( __libmorph_eng_wildClass_hpp__ )
# define __libmorph_eng_wildClass_hpp__
# include "scanClass.hpp"
# include "grammap.h"

namespace libmorph {
namespace eng {

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

    template <class Action> static
    int   ScanString(
      const Action&   target,
            uint8_t*  smatch,
            size_t    lmatch,
            fragment  inflex,
      const fragment& suffix,
            fragment  stflex,
            uint8_t   idform )
    {
      int   nerror = 0;

    // skip matching characters
      while ( !inflex.empty() && !stflex.empty() && (smatch[lmatch] = stflex.front()) == inflex.front() )
      {
        ++lmatch;
        ++inflex;
        ++stflex;
      }

    // check if table entry matched
      if ( stflex.empty() )
        return ScanSuffix( target, smatch, lmatch, inflex, suffix, { 0, idform, idform, 0 } );

    // if mismatches, check the reason
      if ( inflex.empty() )
        return 0;

      switch ( inflex.front() )
      {
        case '?':
          return ScanString( target, smatch, lmatch + 1, inflex.next(), suffix, stflex.next(), idform );
        case '*':
        // suppose zero and one match
          if ( (nerror = ScanString( target, smatch, lmatch, inflex.next(), suffix, stflex, idform )) == 0 )
            nerror = ScanString( target, smatch, lmatch + 1, inflex, suffix, stflex.next(), idform );
        default:
          return nerror;
      }
    }

   /*
    * FlexSearch()
    *
    * Перебирает все формы, соответствующие переданному шаблону
    */
    template <class Action> static
    int   FlexSearch(
      const Action&   target,
            uint8_t*  smatch,
            size_t    lmatch,
      const fragment& inflex,
      const fragment& suffix,
      const uint8_t*  ptable )
    {
      int   nerror = 0;

    // check for zero length match on asterisk
      for ( auto ncount = *ptable++; ncount-- > 0; )
      {
        auto  idform = *ptable++;
        auto  szflex = ptable++;
        auto  ccflex = *szflex++;
              ptable += ccflex;

        if ( ccflex == 0 )
          nerror = ScanSuffix( target, smatch, lmatch, inflex, suffix, { 0, idform, idform, 0 } );
        else
          nerror = ScanString( target, smatch, lmatch, inflex, suffix, { szflex, ccflex }, idform );

        if ( nerror != 0 )
          break;
      }
      return nerror;
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
              grinfo.idForm,
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

    // Теперь - обработка флективных слов.
      if ( (nerror = Wildcard::FlexSearch(
        output,
        smatch,
        lmatch,
        inflex,
        suffix, lextem.GetFlexTable() )) != 0 )
      return nerror;

      return 0;
    }
  };

  template <class Collect>
  auto  MakeModelMatch( const Collect& match ) -> MatchModel<Collect>
    {  return MatchModel<Collect>( match );  }

}} // end namespace

# endif // !__libmorph_eng_scanClass_hpp__
