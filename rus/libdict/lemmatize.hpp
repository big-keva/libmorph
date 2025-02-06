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
# if !defined( __libmorph_rus_lemmatize_hpp__ )
# define __libmorph_rus_lemmatize_hpp__
# include "xmorph/buildforms.hpp"
# include "codepages.hpp"
# include "grammap.h"

namespace libmorph {
namespace rus {

 /*
  * Lemmatizer - create lemmatization result, build normal (dictionary) form for
  * a stem passed;
  * the output data - the references to the arrays to be filled by the lemmatizer
  */
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

      bool  operator == ( nullptr_t ) const {  return beg == nullptr;  }
      bool  operator != ( nullptr_t ) const {  return !(*this == nullptr);  }
    };

  public:
    Lemmatizer(
      const CapScheme&          cs,
      const target<SLemmInfoA>& lm,
      const MbcsCoder&          fm,
      const target<SGramInfo>&  gr, const uint8_t* st, unsigned us );

  public:
    operator int() const  {  return nbuilt;  }

    int  operator()(
      lexeme_t          nlexid,
      const steminfo&   lextem,
      const fragment&   inflex,
      const fragment&   suffix,
      const SGramInfo&  grinfo ) const;

    int  operator()(
      lexeme_t          nlexid,
      const steminfo&   lextem,
      const fragment&   inflex,
      const fragment&   suffix,
      const SGramInfo*  grbuff,
      size_t            ngrams ) const;

  protected:
            const CapScheme     casing;       // capitalization scheme tool
    mutable target<SLemmInfoA>  plemms;       // the output buffer for descriptions
    mutable MbcsCoder           pforms;       // the buffer for the forms
    mutable target<SGramInfo>   pgrams;       // the buffer for grammar descriptions
    mutable int                 nbuilt = 0;
            const uint8_t*      szstem;
            unsigned            dwsets;

  };

}} // end namespace

# endif // !__libmorph_rus_lemmatize_hpp__
