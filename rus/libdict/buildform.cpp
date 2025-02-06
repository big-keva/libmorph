# include "buildform.hpp"
# include <cstring>

namespace libmorph {
namespace rus {

 /*
  * GetWordForms(...)
  * Синтезирует варианты флективной части слова, исходя из его части речи,
  * чередований, окончаний и грамматической информации о форме.
  * Возвращает количество построенных вариантов.
  */
  int   GetWordForms(
    Collector&      output,
    const steminfo& stinfo,
    const flexinfo& fxinfo,
    const fragment& prefix,
    const fragment& suffix )
  {
    auto  ptflex = (const uint8_t*){};

    if ( stinfo.mtoffs != 0 )
    {
      auto    mixtab = stinfo.GetSwapTable();
      int     mpmask = 0x08 << stinfo.GetSwapLevel( fxinfo.gramm, fxinfo.flags );
      auto    mcount = *mixtab++;
      auto    mixstr = (const uint8_t*)nullptr;
      uint8_t szform[0x40];
      size_t  ccform = 0;

    // найти ступень чередования
      while ( mixstr == nullptr && mcount-- > 0 )
      {
        if ( (*mixtab & mpmask) != 0 )  mixstr = mixtab;
          else {  auto mixlen = *mixtab++ & 0x0f;  mixtab += mixlen;  }
      }

    // добавить к префиксу
      if ( mixstr != nullptr )
      {
      // проверить аргументы и сформировать новый префикс, если этот не пустой
        if ( (ccform = prefix.len + (0x0f & *mixstr)) < sizeof(szform) )
          memcpy( prefix.size() + (char*)memcpy( szform, prefix.str, prefix.len ), mixstr + 1, ccform - prefix.len );

        return GetFlexForms( output, stinfo.GetFlexTable(), fxinfo, { szform, ccform }, suffix );
      }
      return 0;
    }
      else
    if ( (ptflex = stinfo.GetFlexTable()) == nullptr )
    {
      return output.append( (const char*)prefix.begin(), prefix.size() )
        && output.append( (const char*)suffix.begin(), suffix.size() )
        && output.append( '\0' ) ? 1 : -1;
    }
      else
    return GetFlexForms( output, ptflex, fxinfo, prefix, suffix );
  }

 /*
  * Meth: GetDictF orms
  * Строит варианты флективной части слова, исходя из его части речи, чередований,
  * окончаний и грамматической информации о форме.
  * Возвращает количество построенных вариантов.
  */
  int   GetDictForms(
    Collector&      output,
    unsigned        dwsets,
    const steminfo& stinfo,
    const flexinfo& fxinfo,
    const fragment& prefix,
    const fragment& suffix )
  {
    auto  nfinfo = stinfo.FindDictForm( fxinfo, dwsets );
    int   nforms;

  // try create the exact form
    if ( (nforms = GetWordForms( output, stinfo, nfinfo, prefix, suffix )) != 0 )
      return nforms;

  // check if is inreverce - nothing to do on reverce forms
    if ( (nfinfo.gramm & gfRetForms) != 0 )
      return 0;
    
  // check if verb or adjective
    if ( !IsVerb( stinfo.wdinfo ) && !IsAdjective( stinfo.wdinfo ) )
      return 0;

  // try build reverce
    return GetWordForms( output, stinfo, { uint16_t(nfinfo.gramm | gfRetForms), nfinfo.flags },
      prefix, suffix );
  }

  // FormBuild implementation

  FormBuild::FormBuild(
    const MbcsCoder&  fm,
    const CapScheme&  cs,
    lexeme_t          lx,
    formid_t          fi,
    const uint8_t*    st ):
      pforms( fm ),
      casing( cs ),
      lexeme( lx ),
      szstem( st ),
      idform( fi ) {}

  int  FormBuild::operator()(
    lexeme_t          nlexid,
    const steminfo&   lextem,
    const fragment&   inflex,
    const fragment&   suffix,
    const SGramInfo&/*grinfo*/ ) const
  {
    return (*this)(
      nlexid,
      lextem,
      inflex,
      suffix, nullptr, 0 );
  }

  int  FormBuild::operator()(
    lexeme_t        /*nlexid*/,
    const steminfo&   lextem,
    const fragment&   inflex,
    const fragment&   suffix,
    const SGramInfo*/*pgrams*/,
    size_t          /*ngrams*/ ) const
  {
    char  fmbuff[256];
    auto  grinfo = GetGramInfo( lextem.wdinfo, idform );
    auto  nforms = int{};
    auto  ccform = size_t{};

  // проверить идентификатор формы
    if ( grinfo == flexinfo{ uint16_t(-1), uint8_t(-1) } && idform != (uint8_t)-1 )
      return 0;

    if ( (lextem.tfoffs != 0) != (idform != (uint8_t)-1) )
      return 0;

    nforms = GetWordForms( MakeCollector( fmbuff ).get(), lextem, grinfo,
      { szstem, size_t(inflex.str - szstem) }, suffix );

    if ( nforms <= 0 )
      return nforms;

  // Привести формы к минимальной возможной капитализации и скопировать на выход
    for ( auto p = fmbuff; nforms-- > 0; p += ccform + 1, ++nbuilt )
    {
      ccform = strlen( p );

      casing.Set( (unsigned char*)p, ccform, pspMinCapValue[lextem.wdinfo & 0x3f] );

      if ( !pforms.append( p, ccform ) || !pforms.append( '\0' ) )
        return LEMMBUFF_FAILED;
    }

    return 0;
  }

}}  // namespace
