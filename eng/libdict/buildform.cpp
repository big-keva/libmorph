/******************************************************************************

    libmorpheng - dictionary-based morphological analyser for English.

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
# include "buildform.hpp"
# include <cstring>

namespace libmorph {
namespace eng {

  int   GetFlexForms(
    Collector&      output,
    const fragment& prefix,
    const fragment& suffix,
    const uint8_t*  ptflex,
    formid_t        idform )
  {
    int     ntails = 0;

    for ( auto fcount = *ptflex++; fcount-- > 0; )
    {
      auto  formid = *ptflex++;
      auto  inflex = fragment{ ptflex + 1, *ptflex };
            ptflex += inflex.len + 1;

      if ( formid != idform )
        continue;

      if ( output.append( (const char*)prefix.str, prefix.len )
        && output.append( (const char*)inflex.str, inflex.len )
        && output.append( (const char*)suffix.str, suffix.len )
        && output.append( '\0' ) )  ++ntails;
      else return -1;
    }

    return ntails;
  }

 /*
  * GetWordForms(...)
  * Синтезирует варианты флективной части слова, исходя из его части речи,
  * чередований, окончаний и грамматической информации о форме.
  * Возвращает количество построенных вариантов.
  */
  int   GetWordForms(
    Collector&      output,
    const fragment& prefix,
    const fragment& suffix,
    const steminfo& stinfo,
    uint8_t         idform )
  {
    auto  ptflex = stinfo.GetFlexTable();

    if ( ptflex == nullptr )
    {
      return output.append( (const char*)prefix.begin(), prefix.size() )
        && output.append( '\0' ) ? 1 : -1;
    }
      else
    return GetFlexForms( output, prefix, suffix, ptflex, idform );
  }

 /*
  * Meth: GetDictF orms
  * Строит варианты флективной части слова, исходя из его части речи, чередований,
  * окончаний и грамматической информации о форме.
  * Возвращает количество построенных вариантов.
  */
  int   GetDictForms(
    Collector&      output,
    const fragment& prefix,
    const fragment& suffix,
    const steminfo& stinfo )
  {
    int   nforms;

    for ( formid_t idform = 0; idform != 0x10; ++idform )
      if ( (nforms = GetWordForms( output, prefix, suffix, stinfo, idform )) != 0 )
        return nforms;

    return 0;
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
    auto  nforms = int{};
    auto  ccform = size_t{};

    if ( (lextem.tfoffs != 0) != (idform != (uint8_t)-1) )
      return 0;

    nforms = GetWordForms( MakeCollector( fmbuff ).get(),
      { szstem, size_t(inflex.str - szstem) }, suffix, lextem, idform );

    if ( nforms <= 0 )
      return nforms;

  // Привести формы к минимальной возможной капитализации и скопировать на выход
    for ( auto p = fmbuff; nforms-- > 0; p += ccform + 1, ++nbuilt )
    {
      ccform = strlen( p );

      casing.Set( (unsigned char*)p, ccform, (lextem.wdinfo >> 7) & 0x3 );

      if ( !pforms.append( p, ccform ) || !pforms.append( '\0' ) )
        return LEMMBUFF_FAILED;
    }

    return 0;
  }

}}  // namespace
