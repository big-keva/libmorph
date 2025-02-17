/******************************************************************************

    libfuzzyrus - fuzzy morphological analyser for Russian.

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
# if !defined( __libfuzzy_rus_buildforms_hpp__ )
# define __libfuzzy_rus_buildforms_hpp__
# include "rus/include/mlfa1049.h"
# include "codepages.hpp"
# include "xmorph/capsheme.h"
# include "classtable.hpp"
# include "mtc/serialize.h"
# include <algorithm>

namespace libfuzzy {
namespace rus {

  using namespace libmorph;

  class Buildform
  {
    const CapScheme&  casing;
    mutable MbcsCoder output;
    mutable int       nbuilt = 0;   // built form count

  public:
    Buildform(
      const CapScheme&  cs,
      const MbcsCoder&  fm ):
        casing( cs ),
        output( fm )  {}

  public:
  // build form with lemma and class id
    int   operator()( const uint8_t* plemma, size_t clemma, unsigned uclass, formid_t idform ) const
    {
      char      szform[0x40];
      uint8_t   partsp;
      int       fcount;
      int       nbuilt = 0;
      auto      pclass = ::FetchFrom( ::FetchFrom( GetClass( uclass ),
        partsp ),
        fcount );

    // skip until form
      for ( ; fcount > 0 && (uint8_t)*pclass < idform; --fcount )
        pclass += 2 + (uint8_t)pclass[1];

    // build all variants
      for ( ; fcount > 0 && (uint8_t)*pclass == idform; --fcount, ++nbuilt )
      {
        auto  ccflex = (uint8_t)*++pclass;  ++pclass;
        auto  ccform = clemma + ccflex;

      // check enough space
        if ( ccform > sizeof(szform) )
          return WORDBUFF_FAILED;

      // create form string
        strncpy( clemma + (char*)memcpy( szform,
          plemma, clemma ),
          pclass, ccflex );

        pclass += ccflex;

      // set minimal capitalization
        casing.Set( (uint8_t*)szform, ccform, partsp );

        if ( !output.append( szform, ccflex ) || !output.append( '\0' ) )
          return LEMMBUFF_FAILED;
      }
      return nbuilt;
    }
  };

}}

# endif // !__libfuzzy_rus_buildforms_hpp__
