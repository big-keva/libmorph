/******************************************************************************

    libfuzzyrus - fuzzy morphological analyser for Russian.

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
# if !defined( __libfuzzy_rus_scandict_hpp__ )
# define __libfuzzy_rus_scandict_hpp__
# include "mtc/serialize.h"
# include "xmorph/typedefs.h"
# include "moonycode/codes.h"

namespace libfuzzy {
namespace rus {

  using fragment = libmorph::fragment;

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

  protected:
    template <class Collector>  static
    int   GetStems(
      Collector&      output,
      const char*     pstems,
            uint8_t*  getstr, size_t  getlen,
      const uint8_t*  plemma, size_t  clemma )
    {
      unsigned  ccount;
      int       nerror;

    // check trigraph filter for stem and inflex connection
      if ( getlen > 1 )
      {
        uint8_t tgramm[3] = { 0, getstr[getlen - 1], getstr[getlen - 2] };

        if ( clemma > 0 && !IsWildcard( tgramm[0] = plemma[clemma - 1] ) )
        {
          if ( !codepages::detect::trigraph( (const char*)tgramm ) )
            return 0;

          if ( clemma > 1 && !IsWildcard( plemma[clemma - 2] ) )
          {
            tgramm[2] = tgramm[1];
            tgramm[1] = tgramm[0];
            tgramm[0] = plemma[clemma - 2];

            if ( !codepages::detect::trigraph( (const char*)tgramm ) )
              return 0;
          }
        }
      }
      for ( pstems = ::FetchFrom( pstems, ccount ); pstems != nullptr && ccount-- > 0; )
      {
        unsigned  uclass;
        unsigned  uoccur;
        uint8_t   idform;

      // get class reference
        if ( (pstems = ::FetchFrom( ::FetchFrom( ::FetchFrom( pstems,
          uclass ),
          uoccur ), &idform, 1 )) == nullptr ) continue;

        if ( (nerror = output( { plemma, clemma }, { getstr, getlen }, uclass, uoccur, idform )) != 0 )
          return nerror;
      }
      return 0;
    }

    template <class C>  static
    inline bool IsWildcard( C c ) {  return c == '*' || c == '?';  }

    template <class Collector>  static
    int   MatchKey(
      Collector&      output,
      const char*     patree,                   // следующая за ключом позиция
      size_t          nnodes,
      uint8_t*        getstr, size_t getlen,
      const uint8_t*  patkey, size_t patlen,
      const uint8_t*  revkey, size_t revlen )
    {
      auto    patend = patkey + patlen;
      auto    revend = revkey - revlen;
      size_t  sublen;
      int     nerror;

    // check key match testing wildcards
      for ( ; patkey != patend && revkey != revend; ++patkey, --revkey )
      {
        auto  chnext = *revkey;

        if ( chnext == '*' )
        {
        // предположить, что * не соответствует ни одного символа
          if ( (nerror = MatchKey( output, patree, nnodes, getstr, getlen, patkey, patlen, revkey - 1, revlen - 1 )) != 0 )
            return nerror;

          getstr[getlen++] = *patkey;

        // предполжить, что * соответствует один символ
          if ( (nerror = MatchKey( output, patree, nnodes, getstr, getlen, patkey + 1, patlen - 1, revkey, revlen )) != 0 )
            return nerror;
          return 0;
        }
          else
        if ( chnext == '?' || chnext == *patkey )
        {
          getstr[getlen++] = *patkey;
        }
          else
        return 0;
      }

    // check if string matches, if not, stop this search branch
      if ( patkey != patend )
        return 0;

      // check if fragment has any values on this level
      if ( (nnodes & 0x01) != 0 )
      {
        if ( (nerror = GetStems( output, JumpOver( nnodes >>= 1, ::FetchFrom( patree, sublen ) ),
          getstr, getlen, revend + 1, revkey - revend )) != 0 )
        return nerror;
      } else nnodes >>= 1;

      if ( revkey == revend )
        return 0;

    // loop over the nested nodes, select the node to contain the key
      for ( patree = ::FetchFrom( patree, sublen ); nnodes != 0; --nnodes )
      {
        size_t  cchars;
        size_t  cnodes;
        size_t  sublen;
        auto    pchars = ::FetchFrom( ::FetchFrom( patree,
          cchars ),
          cnodes );

        patree = ::FetchFrom( pchars + cchars, sublen );
          patree += sublen;

        if ( *revkey != '*' && *revkey != '?' )
        {
          int   rescmp = *revkey - (uint8_t)*pchars;

          if ( rescmp > 0 )
            continue;
          if ( rescmp < 0 )
            return 0;
        }

        if ( (nerror = MatchKey( output, pchars + cchars, cnodes,
          getstr, getlen,
          (const uint8_t*)pchars, cchars,
          (const uint8_t*)revkey, revkey - revend )) != 0 )
        return nerror;
      }
      return 0;
    }
  public:
    template <class Collector>  static
    int   WildScan(
      Collector&  output,
      const char* patree,
      uint8_t*    getstr, size_t getlen,
      uint8_t*    revkey, size_t revlen )
    {
      size_t  nchars;
      size_t  nnodes;

      patree = ::FetchFrom( ::FetchFrom( patree,
        nchars ),
        nnodes );

      return MatchKey( output, patree + nchars, nnodes,
        getstr, getlen,
        (const uint8_t*)patree, nchars,
        (const uint8_t*)revkey, revlen );
    }
  };

}}

# endif   // __libfuzzy_rus_scandict_hpp__
