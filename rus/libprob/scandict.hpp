# if !defined( __libfuzzy_rus_scandict_hpp__ )
# define __libfuzzy_rus_scandict_hpp__
# include "mtc/serialize.h"

namespace libfuzzy {
namespace rus {

  class patricia final
  {
    inline  static
    auto  JumpOver( int nnodes, const char* thedic ) -> const char*
    {
      while ( nnodes-- > 0 )
      {
        int   cchars;
        int   cnodes;
        int   curlen;

        thedic = ::FetchFrom( ::FetchFrom( thedic, cchars ), cnodes );
        thedic = ::FetchFrom( thedic + cchars, curlen );
        thedic = thedic + curlen;
      }
      return thedic;
    }

    template <class Collector>  static
    int   ScanList(
      Collector&  output,
      const char* pstems,
      const char* plemma,
      size_t      clemma )
    {
      unsigned  ccount;
      int       nerror;

      for ( pstems = ::FetchFrom( pstems, ccount ); pstems != nullptr && ccount-- > 0; )
      {
        unsigned  uclass;
        unsigned  uoccur;
        uint8_t   idform;

      // get class reference
        if ( (pstems = ::FetchFrom( ::FetchFrom( ::FetchFrom( pstems,
          uclass ),
          uoccur ), &idform, 1 )) == nullptr ) continue;

        if ( (nerror = output( plemma, clemma, uclass, uoccur, idform )) != 0 )
          return nerror;
      }
      return 0;
    }

  public:
    template <class Collector>  static
    int   ScanTree(
      Collector&  output,
      const char* patree,
      const char* revkey, size_t cchkey )
    {
      size_t  nchars;
      size_t  nnodes;
      int     nerror;

      for ( patree = ::FetchFrom( ::FetchFrom( patree, nchars ), nnodes ); nnodes != 0; )
      {
        size_t  sublen;
        auto    hasval = (nnodes & 0x01) != 0;  nnodes >>= 1;

        // check key match
        if ( cchkey < nchars )
          return 0;

        for ( auto keyend = patree + nchars; patree != keyend; --cchkey )
          if ( *patree++ != *revkey-- )
            return 0;

        // check if value
        if ( hasval )
        {
          if ( (nerror = ScanList( output, JumpOver( nnodes, ::FetchFrom( patree, sublen ) ), revkey - cchkey + 1, cchkey )) != 0 )
            return nerror;
        }

        // loop over the nested nodes, select the node to contain the key
        for ( patree = ::FetchFrom( patree, sublen ); nnodes != 0; --nnodes )
        {
          size_t  cchars;
          size_t  cnodes;
          int     rescmp;

          patree = ::FetchFrom( ::FetchFrom( patree,
            cchars ),
            cnodes );
          rescmp = (uint8_t)*revkey - (uint8_t)*patree;

          if ( rescmp > 0 )
          {
            patree = ::FetchFrom( patree + cchars, sublen );
            patree += sublen;
          }
            else
          if ( rescmp == 0 )
          {
            nchars = cchars;
            nnodes = cnodes;
            break;
          }
            else
          return 0;
        }
      }
      return 0;
    }
  };

}}

# endif   // __libfuzzy_rus_scandict_hpp__
