/******************************************************************************

    libmorphrus - dictiorary-based morphological analyser for Russian.

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
# if !defined( __libmorph_rus_buildform_hpp__ )
# define __libmorph_rus_buildform_hpp__
# include "xmorph/buildforms.hpp"
# include "codepages.hpp"
# include "grammap.h"

namespace libmorph {
namespace rus {

  class FormBuild
  {
    mutable MbcsCoder         pforms;       // the buffer for the forms
            const CapScheme   casing;       // capitalization scheme tool
            const lexeme_t    lexeme;
            const uint8_t*    szstem;
            const formid_t    idform;
    mutable int               nbuilt = 0;

  public:
    FormBuild(
      const MbcsCoder&  fm,
      const CapScheme&  cs,
      lexeme_t          lx,
      formid_t          fi,
      const uint8_t*    st );

  public:
    operator int() const  {  return nbuilt;  }

    int  operator()(
      lexeme_t          nlexid,
      const steminfo&   lextem,
      const fragment&   inflex,
      const fragment&   suffix,
      const SGramInfo&/*grinfo*/ ) const;

    int  operator()(
      lexeme_t        /*nlexid*/,
      const steminfo&   lextem,
      const fragment&   inflex,
      const fragment&   suffix,
      const SGramInfo*/*pgrams*/,
      size_t          /*ngrams*/ ) const;

  };

 /*
  * Meth: GetDictF orms
  * Функция синтезирует варианты флективной части слова, исходя из его
  * части речи, чередований, окончаний и грамматической информации
  * о форме.
  * Возвращает количество построенных вариантов.
  */
  int   GetDictForms( Collector&, unsigned dwsets,
    const steminfo&,
    const flexinfo&,
    const fragment& prefix,
    const fragment& suffix = {} );

 /*
  * GetWordForms(...)
  * Синтезирует варианты флективной части слова, исходя из его части речи,
  * чередований, окончаний и грамматической информации о форме.
  * Возвращает количество построенных вариантов.
  */
  int   GetWordForms( Collector&,
    const steminfo&,
    const flexinfo&,
    const fragment&   prefix,
    const fragment&   suffix = {} );

}}

# endif // !__libmorph_rus_buildform_hpp__
