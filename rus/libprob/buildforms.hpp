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
