# if !defined( __scandict_h__ )
# define  __scandict_h__
# include <namespace.h>
# include "mlmadefs.h"
# include "capsheme.h"
# include "grammap.h"
# include <assert.h>

namespace LIBMORPH_NAMESPACE
{
  template <class flagtype>
  struct  countflags
  {
    flagtype  uflags;

    countflags& load( const byte08_t*& p )
      {
        uflags = *(const flagtype*)p;
          p += sizeof(flagtype); 
        return *this;
      }
    bool  extended() const
      {  return (uflags & (1 << (sizeof(flagtype) * CHAR_BIT - 1))) != 0;  }
    int   getcount() const
      {  return uflags & ((1 << (sizeof(flagtype) * CHAR_BIT - 1)) - 1);   }
  };

  template <class flagtype>
  struct  scan_stack
  {
    const byte08_t*       thestr;
    unsigned              cchstr;
    byte08_t              chfind;
    const byte08_t*       thedic;
    countflags<flagtype>  aflags;
    int                   ccount;

  public:     // init
    scan_stack*     setlevel( const byte08_t* p, const byte08_t* s, unsigned l )
      {
        ccount = aflags.load( thedic = p ).getcount();
          thestr = s;
        chfind = (cchstr = l) > 0 ? *thestr : 0;
          return this;
      }
    const byte08_t* findchar()
      {
        while ( ccount-- > 0 )
        {
          byte08_t        chnext = *thedic++;
          unsigned        sublen = getserial( thedic );
          const byte08_t* subdic = thedic;  thedic += sublen;

          if ( chfind == chnext )
            return subdic;
          if ( chfind >  chnext && !aflags.extended() )
            return 0;
        }
        return 0;
      }
  };

  class gramLoader
  {
    SGramInfo*  output;
    unsigned    wdinfo;
    unsigned    mpower;

  public:     // construction
    gramLoader( SGramInfo* p, unsigned w, unsigned m ): output( p ), wdinfo( w ), mpower( m )
      {
      }

  protected:
    int   CopyGramAA( const byte08_t* thedic )    // no forced multiple, any interchange
      {
        SGramInfo*  outorg = output;
        int         nforms = getserial( thedic );

        assert( (wdinfo & wfMultiple) == 0 );
        assert( mpower == (unsigned)-1 );

        while ( nforms-- > 0 )
          SetGramInfo( *output++, getword16( thedic ), *thedic++ );

        return output - outorg;
      }
    int   CopyGramMA( const byte08_t* thedic )    // no forced multiple, any interchange
      {
        SGramInfo*  outorg = output;
        int         nforms = getserial( thedic );

        assert( (wdinfo & wfMultiple) != 0 );
        assert( mpower == (unsigned)-1 );

        while ( nforms-- > 0 )
        {
          word16_t  grInfo = getword16( thedic );
          byte08_t  bflags = *thedic++;

          if ( (grInfo & gfMultiple) != 0 )
            SetGramInfo( *output++, grInfo, bflags );
        }

        return output - outorg;
      }
    int   CopyGramAP( const byte08_t* thedic )    // no forced multiple, any interchange
      {
        SGramInfo*  outorg = output;
        int         nforms = getserial( thedic );

        assert( (wdinfo & wfMultiple) == 0 );
        assert( mpower != (unsigned)-1 );

        while ( nforms-- > 0 )
        {
          word16_t  grInfo = getword16( thedic );
          byte08_t  bflags = *thedic++;
          int       desire = GetMixPower( wdinfo, grInfo, bflags );

          assert( desire >= 1 );
            
          if ( (mpower & (1 << (desire - 1))) != 0 )
            SetGramInfo( *output++, grInfo, bflags );
        }

        return output - outorg;
      }
    int   CopyGramMP( const byte08_t* thedic )    // no forced multiple, any interchange
      {
        SGramInfo*  outorg = output;
        int         nforms = getserial( thedic );

        assert( (wdinfo & wfMultiple) != 0 );
        assert( mpower != (unsigned)-1 );

        while ( nforms-- > 0 )
        {
          word16_t  grInfo = getword16( thedic );
          byte08_t  bflags = *thedic++;
          int       desire = GetMixPower( wdinfo, grInfo, bflags );

          assert( desire >= 1 );
            
          if ( (grInfo & gfMultiple) != 0 && (mpower & (1 << (desire - 1))) != 0 )
            SetGramInfo( *output++, grInfo, bflags );
        }

        return output - outorg;
      }

  public:     // gramLoader functor
    int   operator () ( const byte08_t* thedic, const byte08_t* thestr, unsigned cchstr )
      {
        return cchstr == 0 ?
          (mpower == (unsigned)-1 ? ((wdinfo & wfMultiple) != 0 ? CopyGramMA( thedic ) : CopyGramAA( thedic )) :
                                    ((wdinfo & wfMultiple) != 0 ? CopyGramMP( thedic ) : CopyGramAP( thedic ))) : 0;
      }

  };

  template <class stemLister>
  struct  listLookup: public stemLister
  {
    listLookup( const byte08_t* szbase = NULL, unsigned uflags = 0 ): stemLister( szbase, uflags )
      {
      }
    int   operator () ( const byte08_t* pstems, const byte08_t* thestr, unsigned cchstr )
      {
        SGramInfo fxlist[0x40];     // Массив отождествлений на окончаниях
        unsigned  ucount = getserial( pstems );

        while ( ucount-- > 0 )
        {
          steminfo        stinfo ( getserial( pstems ) + classmap );
          lexeme_t        nlexid = getserial( pstems );
          unsigned        ccflex = cchstr;
          const byte08_t* flestr;
          const byte08_t* matstr;
          int             rescmp;
          int             nforms;
          int             nerror;

        // check for postfix
          if ( stinfo.ccpost != 0 )
          {
            unsigned  ccpost;

            if ( stinfo.ccpost > ccflex )
              continue;

            for ( ccpost = stinfo.ccpost,
                  matstr = stinfo.szpost,
                  flestr = thestr + ccflex - ccpost,
                  ccflex -= ccpost; ccpost-- > 0 && (rescmp = *flestr++ - *matstr++) == 0; )
              (void)0;

            if ( rescmp != 0 )
              continue;
          }

        // check capitalization scheme
          if ( !VerifyCaps( stinfo.wdinfo ) )
            continue;

        // check if non-flective
          if ( (stinfo.wdinfo & wfFlexes) == 0 || (stinfo.wdinfo & 0x3f) == 51 )
          {
            if ( ccflex == 0 )
            {
              SGramInfo grprep = { 0, 0, stinfo.tfoffs, 0 };
                stinfo.tfoffs = 0;

              if ( (nerror = InsertStem( nlexid, thestr, stinfo, &grprep, 1 )) != 0 )
                return nerror;
            }
            continue;
          }

        // оценить, может ли хотя бы потенциально такое окончание быть у основ начиная с этой и далее
          if ( ccflex > 0 )
          {
            if ( *thestr > stinfo.chrmax )
              break;
            if ( *thestr < stinfo.chrmin )
              continue;
          }

        // Теперь - обработка флективных слов. Сначала обработаем более
        // частый случай - флективное слово без чередований в основе.
        // Для этого достаточно проверить, не выставлен ли флаг наличия
        // чередований, так как при попадании в эту точку слово заведомо
        // будет флективным.
          if ( (stinfo.wdinfo & wfMixTab) == 0 )
          {
            if ( (nforms = ScanDict<byte08_t, int>( gramLoader( fxlist, stinfo.wdinfo, (unsigned)-1 ),
              flexTree + (stinfo.tfoffs << 4), thestr, ccflex )) == 0 )
                continue;

            if ( (nerror = InsertStem( nlexid, thestr, stinfo, fxlist, nforms )) != 0 )
              return nerror;
          }
            else
        // Ну и, наконец, последний случай - слово с чередованиями в основе. Никаких специальных проверок здесь делать
        // уже не требуется, так как попадание в эту точку само по себе означает наличие чередований в основе
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
              unsigned        cmplen;
              int             rescmp;

            // перейти к следующей строке чередования
            // сравнить чередования
              if ( (cmplen = flexcc) > mixlen )
                cmplen = mixlen;

              for ( rescmp = 0, mixlen -= cmplen, flexcc -= cmplen; cmplen-- > 0 && (rescmp = *curmix++ - *flextr++) == 0; )
                (void)0;
              if ( rescmp == 0 )
                rescmp = mixlen > 0;

              if ( rescmp > 0 ) break;
              if ( rescmp < 0 ) continue;

            // Построить массив грамматических отождествлений в предположении, что используется правильная ступень чередования основы
              if ( (nforms = ScanDict<byte08_t, int>( gramLoader( fxlist, stinfo.wdinfo, powers ), flexTree + (stinfo.tfoffs << 4), flextr, flexcc )) == 0 )
                continue;

              if ( (nerror = InsertStem( nlexid, thestr, stinfo, fxlist, nforms )) != 0 )
                return nerror;
            }
          }
        }
        return 0;
      }

  };

  template <class aflags, class result, class action>
  result  ScanDict( action&         output, const byte08_t* thedic,
                    const byte08_t* thestr, unsigned        cchstr )
  {
    scan_stack<aflags>  astack[0x40];     // never longer words
    scan_stack<aflags>* pstack;
    result              retval;

    for ( (pstack = astack)->setlevel( thedic, thestr, cchstr ); pstack >= astack; )
    {
      const byte08_t* subdic;

    // check if not found or no more data
      if ( (subdic = pstack->findchar()) != 0 )
      {
        pstack = (pstack + 1)->setlevel( subdic, pstack->thestr + 1, pstack->cchstr - 1 );
        continue;
      }

      if ( pstack->aflags.extended() )
        if ( (retval = output( pstack->thedic, pstack->thestr, pstack->cchstr )) != (result)0 )
          return retval;

      --pstack;
    }
    return (result)0;
  }

  template <class aflags, class result, class action>
  result  EnumDict( action& output, const byte08_t* thedic )
  {
    countflags<aflags>  bflags;
    result              retval;
    int                 acount;

    for ( acount = bflags.load( thedic ).getcount(); acount-- > 0 ; )
    {
      unsigned        sublen = getserial( ++thedic );

      if ( (retval = EnumDict<aflags, result>( output, thedic )) != 0 ) return retval;
        else  thedic += sublen;
    }
    return bflags.extended() ? output( thedic ) : (result)0;
  }

  template <class action>
  int   FindStem( action&         output, const byte08_t* thedic,
                  byte08_t*       thestr, const byte08_t* dicpos )
  {
    byte08_t  bflags = *thedic++;
    int       ncount = bflags & ~0x80;

    while ( ncount-- > 0 && thedic < dicpos )
    {
      unsigned        sublen;
      const byte08_t* subdic;

      *thestr = *thedic++;
        sublen = getserial( thedic );
      if ( (thedic = (subdic = thedic) + sublen) > dicpos )
        return FindStem( output, subdic, thestr + 1, dicpos );
    }
    if ( thedic < dicpos && (bflags & 0x80) != 0 )
    {
      ncount = getserial( thedic );

      while ( ncount-- > 0 )
      {
        const byte08_t* stmpos = thedic;
        steminfo        stinfo ( getserial( thedic ) + classmap );
        lexeme_t        nlexid = getserial( thedic );

        if ( stmpos == dicpos )
          return output.InsertStem( nlexid, thestr, stinfo, NULL, 0 );
      }
      return 0;
    }
    return 0;
  }

}  // LIBMORPH_NAMESPACE

# endif  // __scandict_h__
