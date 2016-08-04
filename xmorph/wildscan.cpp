# include "wildscan.h"
# include <xmorph/scandict.h>
# include <xmorph/scanlist.h>
# include <string.h>

namespace LIBMORPH_NAMESPACE
{

  inline  bool      IsWildMask( const byte_t* s, size_t l )
    {
      while ( l-- > 0 ) 
        if ( IsWildcard( *s++ ) )  return true;
      return false;
    }
  inline  void      InsertChar( unsigned* p, byte_t c )
    {
      p[c / (sizeof(*p) * CHAR_BIT)] |= (1 << (c % (sizeof(*p) * CHAR_BIT)));
    }

  struct  doFastCheck
  {
    int   InsertStem( lexeme_t, const steminfo&, const byte_t*, const byte_t*, const SGramInfo*, unsigned ) const
      {  return 1;  }
    bool  VerifyCaps( const steminfo& ) const
      {  return true;  }
  };
  
  static  void  WildScanFlex(       unsigned* output, const byte_t* thedic,
                              const byte_t*   thestr, size_t          cchstr,
                              const steminfo& stinfo, unsigned        mpower,
                              const byte_t*   szpost )
  {
    SGramInfo grbuff[0x20];
    byte_t    bflags = *thedic++;
    int       ncount = bflags & 0x7f;
    byte_t    chfind = *thestr;

    assert( IsWildMask( thestr, cchstr ) );

    while ( ncount-- > 0 )
    {
      gramBuffer    grlist( stinfo, mpower, grbuff );
      byte_t        chnext = *thedic++;
      unsigned      sublen = getserial( thedic );
      const byte_t* subdic = thedic;
                      thedic = subdic + sublen;

    // check character find type: wildcard, pattern or regular
      switch ( chfind )
      {
        case '*': InsertChar( output, chnext );
                  continue;
        case '?': if ( LinearScanDict<byte_t, int>( const_action<gramBuffer>( grlist ), subdic, thestr + 1, cchstr - 1 ) )
                    InsertChar( output, chnext );
                  continue;
        default:  if ( chnext == chfind )
                    WildScanFlex( output, subdic, thestr + 1, cchstr - 1, stinfo, mpower, szpost );
                  continue;
      }
    }

    if ( (bflags & 0x80) != 0 && cchstr == 1 )
    {
      int   nforms = getserial( thedic );

      assert( IsWildcard( *thestr ) );

      while ( nforms-- > 0 && (*output & 0x01) == 0 )
      {
        word16_t  grInfo = getword16( thedic );
        byte_t    bflags = *thedic++;
        int       desire = stinfo.GetSwapLevel( grInfo, bflags );

        assert( desire >= 1 );
            
        if ( ( (stinfo.wdinfo & wfMultiple) == 0 || (grInfo & gfMultiple) != 0 ) && (mpower & (1 << (desire - 1))) != 0 )
          InsertChar( output, '\0' );
      }
    }
  }

  static  void  WildScanList( unsigned* output, const byte_t* pstems, const byte_t* thestr, size_t  cchstr )
  {
    SGramInfo fxlist[0x40];     // Массив отождествлений на окончаниях
    unsigned  ucount = getserial( pstems );

    assert( cchstr > 0 && IsWildMask( thestr, cchstr ) );

    while ( ucount-- > 0 )
    {
      byte_t        clower = *pstems++;
      byte_t        cupper = *pstems++;
      lexeme_t      nlexid = getserial( pstems );
      word16_t      oclass = getword16( pstems );
      size_t        ccflex = cchstr;
      const byte_t* szpost;
      steminfo      stinfo;

    // load stem info
      if ( (oclass & wfPostSt) != 0 ) pstems += 1 + *(szpost = pstems);
        else  szpost = NULL;

    // check if non-flective
      if ( (stinfo.Load( classmap + (oclass & 0x7fff) ).wdinfo & wfFlexes) == 0 || (stinfo.wdinfo & 0x3f) == 51 )
      {
        if ( *thestr == '*' || cchstr == 1 && *thestr == '?' )
          InsertChar( output, '\0' );
      }
        else
      if ( (stinfo.wdinfo & wfMixTab) == 0 )
      {
        WildScanFlex( output, flexTree + (stinfo.tfoffs << 4), thestr, cchstr,
          stinfo, (unsigned)-1, szpost );
      }
        else
      {
        const byte_t* mixtab = stinfo.mtoffs + mxTables;  // Собственно таблица
        int           mixcnt = *mixtab++;                 // Количество чередований
        int           mindex;
        int           nforms;

        for ( mindex = nforms = 0; mindex < mixcnt; ++mindex, mixtab += 1 + (0x0f & *mixtab) )
        {
          const byte_t* curmix = mixtab;
          unsigned      mixlen = 0x0f & *curmix;
          unsigned      powers = *curmix++ >> 4;
          const byte_t* flextr = thestr;
          size_t        flexcc = cchstr;
          int           rescmp;

        // scan top match
          while ( flexcc > 0 && mixlen > 0 && (rescmp = *flextr - *curmix) == 0 )
            {  --flexcc;  --mixlen;  ++flextr;  ++curmix;  }

        // either wildcard found, or wildcard in the rest flexion
          assert( flexcc > 0 && IsWildMask( flextr, flexcc ) );

        // check full or partial match of template to mixstr;
        //  * on full match, call WildFlex( ... );
          if ( mixlen == 0 )    WildScanFlex( output, flexTree + (stinfo.tfoffs << 4), flextr, flexcc, stinfo, powers, szpost );
            else
          if ( *flextr == '*' ) InsertChar( output, *curmix );
            else
          if ( *flextr == '?' )
          {
            gramBuffer  grbuff( stinfo, powers, fxlist );
            byte_t      chsave;

            if ( szpost != NULL )
              if ( flexcc <= *szpost || memcmp( flextr + flexcc - *szpost, szpost + 1, *szpost ) != 0 ) continue;
                else  flexcc -= *szpost;

            for ( chsave = *curmix++, ++flextr, --flexcc, --mixlen; flexcc > 0 && mixlen > 0 && *flextr == *curmix;
              --flexcc, --mixlen, ++flextr, ++curmix ) (void)NULL;

            if ( mixlen == 0 && LinearScanDict<byte_t, int>( const_action<gramBuffer>( grbuff ), flexTree + (stinfo.tfoffs << 4), flextr, flexcc ) > 0 )
              InsertChar( output, chsave );
          }
            else
          if ( rescmp < 0 )
            break;
        }
      }
    }
  }

  static  void  WildScanDict( unsigned* output, const byte_t* thedic, const byte_t* thestr, size_t cchstr )
  {
    byte_t  bflags = *thedic++;
    int     ncount = getlower( bflags );
    byte_t  chfind;

    assert( IsWildMask( thestr, cchstr ) );

    switch ( chfind = *thestr )
    {
      case '*':
        while ( ncount-- > 0 )
        {
          InsertChar( output, *thedic++ );
            thedic += getserial( thedic );
        }
        break;
      case '?':
        while ( ncount-- > 0 )
        {
          byte_t    chnext = *thedic++;
          unsigned  sublen = getserial( thedic );

          if ( LinearScanDict<byte_t, int>( listLookup<doFastCheck, steminfo>( doFastCheck() ), thedic, thestr + 1, cchstr - 1 ) > 0 )
            InsertChar( output, chnext );
          thedic += sublen;
        }
        break;
      default:
        while ( ncount-- > 0 )
        {
          byte_t    chnext = *thedic++;
          unsigned  sublen = getserial( thedic );

          if ( chfind == chnext )
            WildScanDict( output, thedic, thestr + 1, cchstr - 1 );
          thedic += sublen;
        }
    }
    if ( hasupper( bflags ) )
      WildScanList( output, thedic, thestr, cchstr );
  }

  size_t  WildScan( byte_t* output, size_t  cchout, const byte_t* ptempl, size_t cchstr )
  {
    byte_t*   outorg = output;
    byte_t*   outend = output + cchout;
    unsigned  chmask[256 / (sizeof(unsigned) * CHAR_BIT)];
    int       nindex;
    int       cindex;

    WildScanDict( (unsigned*)memset( chmask, 0, sizeof(chmask) ), stemtree, ptempl, cchstr );

    for ( nindex = 0; nindex < sizeof(chmask) / sizeof(chmask[0]); ++nindex )
      for ( cindex = 0; cindex < sizeof(unsigned) * CHAR_BIT; ++cindex )
        if ( (chmask[nindex] & (1 << cindex)) != 0 )
          if ( output < outend ) *output++ = (byte_t)(nindex * sizeof(unsigned) * CHAR_BIT + cindex);
            else  return WORDBUFF_FAILED;

    return output - outorg;
  }

} // end namespace
