# include "lemmatize.hpp"
# include "buildform.hpp"
# include "grammap.h"
# include "codepages.hpp"
# include "xmorph/buildforms.hpp"
# include "xmorph/capsheme.h"
# include "moonycode/codes.h"
# include <algorithm>

namespace libmorph {
namespace rus {

  inline  bool  has_graminfo( const SLemmInfoA& l, const SGramInfo& g )
  {
    auto  beg = l.pgrams;
    auto  end = l.pgrams + l.ngrams;

    return std::find_if( beg, end, [idForm = g.idForm]( const SGramInfo& gr )
      {  return gr.idForm == idForm;  } ) != end;
  }

  inline  void  set_graminfo( SGramInfo& o, const steminfo& s, const SGramInfo& g )
  {
    if ( s.tfoffs != 0 )
    {
      o.idForm = MapWordInfo( o.wdInfo = s.wdinfo & ~(wfPostSt + wfFlexes + wfMixTab + 0x1800),
                              o.grInfo = g.grInfo,                                  /* ^^^ mix type  */
                              o.bFlags = g.bFlags );
    }
      else
    {
      o = { (word16_t)(s.wdinfo & ~0xe000), 0xff, g.grInfo, g.bFlags };
    }
  }

  // Lemmatizer implementation

  Lemmatizer::Lemmatizer(
    const CapScheme&          cs,
    const target<SLemmInfoA>& lm,
    const MbcsCoder&          fm,
    const target<SGramInfo>&  gr, const uint8_t* st, unsigned us ):
      casing( cs ),
      plemms( lm ),
      pforms( fm ),
      pgrams( gr ),
      szstem( st ),
      dwsets( us ) {}


  int  Lemmatizer::operator()(
    lexeme_t          nlexid,
    const steminfo&   lextem,
    const fragment&   inflex,
    const fragment&   suffix,
    const SGramInfo&  grinfo ) const
  {
    return (*this)(
      nlexid, lextem,
      inflex, suffix, &grinfo, 1 );
  }

  int  Lemmatizer::operator()(
    lexeme_t          nlexid,
    const steminfo&   lextem,
    const fragment&   inflex,
    const fragment&   suffix,
    const SGramInfo*  grbuff,
    size_t            ngrams ) const
  {
    bool  merge_lexeme = false;

    if ( nbuilt < 0 )
      return nbuilt;

  // check if overflow will occur in this call; check if last lemma exists
  // and may be merged to current
    if ( plemms != nullptr )
    {
      if ( nbuilt == 0 || plemms.cur[-1].nlexid != nlexid )
      {
        if ( plemms.cur == plemms.lim )
          return nbuilt = LIDSBUFF_FAILED;

        *plemms.cur = { nlexid, pforms.getptr(), pgrams.cur, 0 };
      } else merge_lexeme = true;
    }

  // check if there is a buffer for normal forms; create normal forms
  // and store single form to the buffer selected for forms
    if ( pforms != nullptr && !merge_lexeme )
    {
      char  fmbuff[256];
      int   nforms = GetDictForms( MakeCollector( fmbuff ).get(),
        dwsets,
        lextem, { grbuff->grInfo, grbuff->bFlags }, { szstem, size_t(inflex.str - szstem) }, suffix );
      auto  ccform = size_t{};

      if ( nforms <= 0 )
        return nforms;

    // Привести слово к минимальной возможной капитализации
      casing.Set( (uint8_t*)fmbuff, ccform = strlen( fmbuff ), lextem.wdinfo & 0x3f );

    // отправить на выход
      if ( !pforms.append( fmbuff, ccform ) || !pforms.append( '\0' ) )
        return LEMMBUFF_FAILED;
    }

  // Проверить, надо ли восстанавливать грамматические описания
    if ( plemms != nullptr && pgrams != NULL )
    {
      if ( merge_lexeme )
      {
        for ( ; ngrams > 0 && pgrams.cur != pgrams.lim; --ngrams )
          if ( !has_graminfo( plemms.cur[-1], *grbuff ) )
            set_graminfo( *pgrams.cur++, lextem, *grbuff++ );
          else ++grbuff;
      }
        else
      {
        for ( ; ngrams > 0 && pgrams.cur != pgrams.lim; --ngrams )
          set_graminfo( *pgrams.cur++, lextem, *grbuff++ );
      }

      if ( ngrams == 0 )  plemms.cur->ngrams = pgrams.cur - plemms.cur->pgrams;
        else return GRAMBUFF_FAILED;
    }

    if ( !merge_lexeme )
    {
      if ( plemms != nullptr )
        ++plemms.cur;
      ++nbuilt;
    }

    return 0;
  }

}} // end namespace
