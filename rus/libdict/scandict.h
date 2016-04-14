# if !defined( __scandict_h__ )
# define  __scandict_h__
# include <namespace.h>
# include "mlmadefs.h"
# include "capsheme.h"
# include "grammap.h"
# include <assert.h>

namespace LIBMORPH_NAMESPACE
{
  template <class flattype>
  inline  bool  hasupper( flattype a )
    {  return (a & (1 << (sizeof(a) * CHAR_BIT - 1))) != 0;  }

  template <class flagtype>
  inline  int   getlower( flagtype a )
    {  return a & ~(1 << (sizeof(a) * CHAR_BIT - 1));  }

  template <class flagtype>
  struct  scan_stack
  {
    const byte08_t*       thestr;
    size_t                cchstr;
    byte08_t              chfind;
    const byte08_t*       thedic;
    flagtype              aflags;
    int                   ccount;

  public:     // init
    scan_stack*     setlevel( const byte08_t* p, const byte08_t* s, size_t l )
      {
        ccount = getlower( aflags = *(flagtype*)(thedic = p) );
          thedic += sizeof(flagtype);
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
          if ( chfind >  chnext && !hasupper( aflags ) )
            return 0;
        }
        return 0;
      }
  };

  template <class aflags, class result, class action>
  result  ScanDict( const action&   reader, const byte08_t* thedic,
                    const byte08_t* thestr, size_t          cchstr )
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

      if ( hasupper( pstack->aflags ) )
        if ( (retval = reader( pstack->thedic, pstack->thestr, pstack->cchstr )) != (result)0 )
          return retval;

      --pstack;
    }
    return (result)0;
  }

  class gramBuffer
  {
    SGramInfo*  outorg;
    SGramInfo*  outptr;

  public:     // construction
    gramBuffer( SGramInfo* p ): outorg( p ), outptr( p )
      {
      }
    size_t  size() const
      {
        return outptr - outorg;
      }
    void  append( word16_t grinfo, byte08_t bflags )
      {
        outptr->gInfo = grinfo;
        outptr->other = bflags;
        ++outptr;
      }
  };

  class gramLoader
  {
    gramBuffer& output;
    unsigned    wdinfo;
    unsigned    mpower;

  public:     // construction
    gramLoader( gramBuffer& a, unsigned w, unsigned m ): output( a ), wdinfo( w ), mpower( m )
      {
      }

  protected:
    size_t  CopyGramAA( const byte08_t* thedic ) const // no forced multiple, any interchange
      {
        int         nforms = getserial( thedic );

        assert( (wdinfo & wfMultiple) == 0 );
        assert( mpower == (unsigned)-1 );

        while ( nforms-- > 0 )
          output.append( getword16( thedic ), *thedic++ );

        return output.size();
      }
    size_t  CopyGramMA( const byte08_t* thedic ) const  // no forced multiple, any interchange
      {
        int         nforms = getserial( thedic );

        assert( (wdinfo & wfMultiple) != 0 );
        assert( mpower == (unsigned)-1 );

        while ( nforms-- > 0 )
        {
          word16_t  grInfo = getword16( thedic );
          byte08_t  bflags = *thedic++;

          if ( (grInfo & gfMultiple) != 0 )
            output.append( grInfo, bflags );
        }

        return output.size();
      }
    size_t  CopyGramAP( const byte08_t* thedic ) const  // no forced multiple, any interchange
      {
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
            output.append( grInfo, bflags );
        }

        return output.size();
      }
    size_t  CopyGramMP( const byte08_t* thedic ) const  // no forced multiple, any interchange
      {
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
            output.append( grInfo, bflags );
        }

        return output.size();
      }

  public:     // gramLoader functor
    int   operator () ( const byte08_t* thedic, const byte08_t* thestr, size_t cchstr ) const
      {
        return (int)(cchstr == 0 ?
          (mpower == (unsigned)-1 ? ((wdinfo & wfMultiple) != 0 ? CopyGramMA( thedic ) : CopyGramAA( thedic )) :
                                    ((wdinfo & wfMultiple) != 0 ? CopyGramMP( thedic ) : CopyGramAP( thedic ))) : 0);
      }

  };

  template <class action>
  class listLookup
  {
    action& output;

  public:
    listLookup( action& a ): output( a )
      {
      }
    int   operator () ( const byte08_t* pstems, const byte08_t* thestr, size_t cchstr ) const
      {
        SGramInfo fxlist[0x40];     // Массив отождествлений на окончаниях
        unsigned  ucount = getserial( pstems );

        while ( ucount-- > 0 )
        {
          byte08_t        chrmin = *pstems++;
          byte08_t        chrmax = *pstems++;
          lexeme_t        nlexid = getserial( pstems );
          word16_t        oclass = getword16( pstems );
          const byte08_t* szpost;
          size_t          ccflex = cchstr;
          steminfo        stinfo;
          int             rescmp;
          int             nforms;
          int             nerror;

        // check lower && upper characters
          if ( (oclass & 0x8000) != 0 )
          {
            const byte08_t* flestr;
            const byte08_t* matstr;
            size_t          ccpost;

            ccpost = *(szpost = pstems);
              pstems += 1 + ccpost;

            if ( ccpost > ccflex )
              continue;

            for ( flestr = thestr + ccflex - ccpost, ccflex -= ccpost, matstr = szpost + 1; ccpost-- > 0 && (rescmp = *flestr++ - *matstr++) == 0; )
              (void)0;

            if ( rescmp != 0 )
              continue;
          }
            else
          szpost = NULL;

        // оценить, может ли хотя бы потенциально такое окончание быть у основ начиная с этой и далее
          if ( ccflex > 0 )
          {
            if ( *thestr > chrmax )
              break;
            if ( *thestr < chrmin )
              continue;
          }

        // check capitalization scheme
          if ( !output.VerifyCaps( stinfo.Load( classmap + (oclass & 0x7fff) ).wdinfo ) )
            continue;

        // check if non-flective
          if ( (stinfo.wdinfo & wfFlexes) == 0 || (stinfo.wdinfo & 0x3f) == 51 )
          {
            if ( ccflex == 0 )
            {
              SGramInfo grprep = { 0, 0, stinfo.tfoffs, 0 };
                stinfo.tfoffs = 0;

              if ( (nerror = output.InsertStem( nlexid, stinfo, szpost, thestr, &grprep, 1 )) != 0 )
                return nerror;
            }
            continue;
          }

        // Теперь - обработка флективных слов. Сначала обработаем более
        // частый случай - флективное слово без чередований в основе.
        // Для этого достаточно проверить, не выставлен ли флаг наличия
        // чередований, так как при попадании в эту точку слово заведомо
        // будет флективным.
          if ( (stinfo.wdinfo & wfMixTab) == 0 )
          {
            gramBuffer  grbuff( fxlist );

            if ( (nforms = ScanDict<byte08_t, int>( gramLoader( grbuff, stinfo.wdinfo, (unsigned)-1 ),
              flexTree + (stinfo.tfoffs << 4), thestr, ccflex )) == 0 )
                continue;

            if ( (nerror = output.InsertStem( nlexid, stinfo, szpost, thestr, fxlist, nforms )) != 0 )
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
              gramBuffer      grbuff( fxlist );
              const byte08_t* curmix = mixtab;
              size_t          mixlen = 0x0f & *curmix;
              unsigned        powers = *curmix++ >> 4;
              const byte08_t* flextr = thestr;
              size_t          flexcc = cchstr;
              size_t          cmplen;
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
              if ( (nforms = ScanDict<byte08_t, int>( gramLoader( grbuff, stinfo.wdinfo, powers ), flexTree + (stinfo.tfoffs << 4), flextr, flexcc )) == 0 )
                continue;

              if ( (nerror = output.InsertStem( nlexid, stinfo, szpost, thestr, fxlist, nforms )) != 0 )
                return nerror;
            }
          }
        }
        return 0;
      }

  };

  template <class flagtype, class result, class action>
  result  EnumDict( action& output, const byte08_t* thedic )
  {
    flagtype  bflags;
    result    retval;
    int       acount;

    for ( acount = getlower( bflags = *(flagtype*)thedic ), thedic += sizeof(flagtype); acount-- > 0 ; )
    {
      unsigned        sublen = getserial( ++thedic );

      if ( (retval = EnumDict<flagtype, result>( output, thedic )) != 0 ) return retval;
        else  thedic += sublen;
    }
    return hasupper( bflags ) ? output( thedic ) : (result)0;
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
        lexeme_t        nlexid = getserial( thedic += 2 * sizeof(byte08_t) );
        word16_t        oclass = getword16( thedic );
        const byte08_t* szpost;
        steminfo        stinfo;

        if ( (oclass & wfPostSt) != 0 ) thedic = *(szpost = thedic) + thedic;
          else  szpost = NULL;

        if ( stmpos == dicpos )
          return output.InsertStem( nlexid, stinfo.Load( (oclass & ~wfPostSt) + classmap ), szpost, thestr, NULL, 0 );
      }
      return 0;
    }
    return 0;
  }

}  // LIBMORPH_NAMESPACE

# endif  // __scandict_h__
