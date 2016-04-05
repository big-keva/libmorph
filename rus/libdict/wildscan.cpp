# include "wildscan.h"
# include "scandict.h"
# include "lemmatiz.h"
# include <string.h>

namespace LIBMORPH_NAMESPACE
{

  inline  bool      IsWildChar( byte08_t  c )
    {
      return c == '*' || c == '?';
    }
  inline  bool      IsWildMask( const byte08_t* s, int l )
    {
      while ( l-- > 0 ) 
        if ( IsWildChar( *s++ ) )  return true;
      return false;
    }
  inline  void      InsertChar( word32_t* p, byte08_t c )
    {
      p[c / (sizeof(*p) * CHAR_BIT)] |= (1 << (c % (sizeof(*p) * CHAR_BIT)));
    }

  struct  doFastCheck: public doCheckWord
  {
    doFastCheck( const byte08_t* szbase, unsigned uflags ): doCheckWord( szbase, uflags | sfIgnoreCapitals )
      {
      }
    int   InsertStem( lexeme_t, const byte08_t*, const steminfo&, const SGramInfo*, unsigned )
      {
        return 1;
      }
  };
  
  static
  void  WildFlex( word32_t*       output, const byte08_t* thedic,
                  const byte08_t* thestr, unsigned        cchstr,
                  unsigned        wdinfo, unsigned        mpower )
  {
    SGramInfo grbuff[0x20];
    byte08_t  bflags = *thedic++;
    int       ncount = bflags & 0x7f;
    byte08_t  chfind = *thestr;

    assert( IsWildMask( thestr, cchstr ) );

    while ( ncount-- > 0 )
    {
      byte08_t        chnext = *thedic++;
      unsigned        sublen = getserial( thedic );
      const byte08_t* subdic = thedic;
                      thedic = subdic + sublen;

    // check character find type: wildcard, pattern or regular
      switch ( chfind )
      {
        case '*': InsertChar( output, chnext );
                  continue;
        case '?': if ( ScanDict<byte08_t, int>( gramLoader( grbuff, wdinfo, mpower ), subdic, thestr + 1, cchstr - 1 ) ) InsertChar( output, chnext );
                  continue;
        default:  if ( chnext == chfind ) WildFlex( output, subdic, thestr + 1, cchstr - 1, wdinfo, mpower );
                  continue;
      }
    }

    if ( (bflags & 0x80) != 0 && cchstr == 1 )
    {
      int   nforms = getserial( thedic );

      assert( IsWildChar( *thestr ) );

      while ( nforms-- > 0 && (*output & 0x01) == 0 )
      {
        word16_t  grInfo = getword16( thedic );
        byte08_t  bflags = *thedic++;
        int       desire = GetMixPower( wdinfo, grInfo, bflags );

        assert( desire >= 1 );
            
        if ( ( (wdinfo & wfMultiple) == 0 || (grInfo & gfMultiple) != 0 ) && (mpower & (1 << (desire - 1))) != 0 )
          InsertChar( output, '\0' );
      }
    }
  }

  static
  int   WildList( word32_t*       output, const byte08_t* pstems,
                  const byte08_t* thestr, unsigned        cchstr )
  {
    SGramInfo fxlist[0x40];     // Массив отождествлений на окончаниях
    unsigned  ucount = getserial( pstems );

    assert( cchstr > 0 && IsWildMask( thestr, cchstr ) );

    while ( ucount-- > 0 )
    {
      steminfo        stinfo ( getserial( pstems ) + classmap );
      lexeme_t        nlexid = getserial( pstems );
      unsigned        ccflex = cchstr;

    // check if non-flective
      if ( (stinfo.wdinfo & wfFlexes) == 0 || (stinfo.wdinfo & 0x3f) == 51 )
      {
        if ( cchstr == 1 && IsWildChar( *thestr ) )
          InsertChar( output, '\0' );
      }
        else
      if ( (stinfo.wdinfo & wfMixTab) == 0 )
      {
        WildFlex( output, flexTree + (stinfo.tfoffs << 4), thestr, cchstr,
          stinfo.wdinfo, (unsigned)-1 );
      }
        else
      {
        const byte08_t* mixtab = stinfo.mtoffs + mxTables;  // Собственно таблица
        int             mixcnt = *mixtab++;                 // Количество чередований
        int             mindex;
        int             nforms;

        for ( mindex = nforms = 0; mindex < mixcnt; ++mindex, mixtab += 1 + (0x0f & *mixtab) )
        {
          const byte08_t* curmix = mixtab;
          unsigned        mixlen = 0x0f & *curmix;
          unsigned        powers = *curmix++ >> 4;
          const byte08_t* flextr = thestr;
          unsigned        flexcc = cchstr;
          int             rescmp;

        // scan top match
          while ( flexcc > 0 && mixlen > 0 && (rescmp = *flextr - *curmix) == 0 )
            {  --flexcc;  --mixlen;  ++flextr;  ++curmix;  }

        // either wildcard found, or wildcard in the rest flexion
          assert( flexcc > 0 && IsWildMask( flextr, flexcc ) );

        // check full or partial match of template to mixstr;
        //  * on full match, call WildFlex( ... );
          if ( mixlen == 0 )    WildFlex( output, flexTree + (stinfo.tfoffs << 4), flextr, flexcc, stinfo.wdinfo, powers );
            else
          if ( *flextr == '*' ) InsertChar( output, *curmix );
            else
          if ( *flextr == '?' )
          {
            byte08_t  chsave;

            for ( chsave = *curmix++, ++flextr, --flexcc, --mixlen; flexcc > 0 && mixlen > 0 && *flextr == *curmix;
              --flexcc, --mixlen, ++flextr, ++curmix ) (void)NULL;

            if ( mixlen == 0 && ScanDict<byte08_t, int>( gramLoader( fxlist, stinfo.wdinfo, powers ), flexTree + (stinfo.tfoffs << 4), flextr, flexcc ) > 0 )
              InsertChar( output, chsave );
          }
            else
          if ( rescmp < 0 )
            break;
        }
      }
    }
    return 0;
  }

  static
  int   WildScan( word32_t*       output, const byte08_t* thedic,
                  const byte08_t* thestr, unsigned        cchstr )
  {
    countflags<byte08_t>  bflags;
    int                   ncount = bflags.load( thedic ).getcount();
    byte08_t              chfind;

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
          byte08_t  chnext = *thedic++;
          unsigned  sublen = getserial( thedic );

          if ( ScanDict<byte08_t, int>( listLookup<doFastCheck>(), thedic, thestr + 1, cchstr - 1 ) > 0 )
            InsertChar( output, chnext );
          thedic += sublen;
        }
        break;
      default:
        while ( ncount-- > 0 )
        {
          byte08_t  chnext = *thedic++;
          unsigned  sublen = getserial( thedic );

          if ( chfind == chnext )
            WildScan( output, thedic, thestr + 1, cchstr - 1 );
          thedic += sublen;
        }
    }
    return bflags.extended() ? WildList( output, thedic, thestr, cchstr ) : 0;
  }

  int  WildScan( byte08_t* output, const byte08_t* ptempl, unsigned cchstr )
  {
    byte08_t* outorg = output;
    word32_t  chmask[256 / (sizeof(word32_t) * CHAR_BIT)];
    int       nindex;
    int       cindex;

    for ( memset( chmask, nindex = 0, sizeof(chmask) ), WildScan( chmask, stemtree, ptempl, cchstr );
      nindex < sizeof(chmask) / sizeof(chmask[0]); ++nindex )
        for ( cindex = 0; cindex < sizeof(word32_t) * CHAR_BIT; ++cindex )
          if ( (chmask[nindex] & (1 << cindex)) != 0 )
            *output++ = nindex * sizeof(word32_t) * CHAR_BIT + cindex;

    return output - outorg;
  }

} // end namespace
