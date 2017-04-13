# if !defined( _wildscan_h_ )
# define _wildscan_h_
# include <namespace.h>
# include "mlmadefs.h"
# include "scandict.h"
# include <xmorph/grammap.h>
# include <cassert>

namespace LIBMORPH_NAMESPACE
{

  inline  bool  IsWildcard( byte_t  c )
    {
      return c == '*' || c == '?';
    }

  inline  bool  IsAsterisk( const byte_t* p, size_t l )
    {
      if ( l > 0 && *p++ == '*' )
      {
        do --l;
          while ( l > 0 && *p++ == '*' );
        return l == 0;
      }
      return false;
    }

  /*
    templates pre-declaration
  */
  struct  matchArg;

  template <class action>
  int   GetDictMatch( const byte_t*, size_t, const byte_t*, const action& );
  template <class action>
  int   GetListMatch( const byte_t*, size_t, const byte_t*, const action& );
  template <class action>
  bool  GetIntrMatch( const byte_t*, size_t, const byte_t*, size_t, const matchArg&, unsigned, action& );
  template <class action>
  bool  GetFlexMatch( const byte_t*, size_t, const byte_t*,         const matchArg&, unsigned, action& );

  /*
    GetWordMatch( template, action )

    сканер словаря, вызывает action для каждой найденной лексемы
  */
  template <class action>
  int   GetWordMatch( const byte_t* thestr, size_t  cchstr, const action& _do_it )
  {
    return GetDictMatch( thestr, cchstr, stemtree, _do_it );
  }

  inline  const byte_t* getsubdic( const byte_t*& thedic )
  {
    unsigned      sublen = getserial( ++thedic );
    const byte_t* subdic = thedic;  thedic += sublen;

    return subdic;
  }

  /*
    GetDictMatch( template, dict, action )

    Высокоуровневый сканер словаря, вызывает action для каждой найденной лексемы
    со строкой трассы, ведущей к ней
  */
  template <class action>
  int   GetDictMatch( const byte_t* thestr, size_t        cchstr,
                      const byte_t* thedic, const action& _do_it )
  {
    const byte_t* dicorg = thedic;
    byte_t        bflags = *thedic++;
    int           ncount = getlower( bflags );
    byte_t        chfind;
    int           nerror;

    if ( cchstr > 0 )
    {
      switch ( chfind = *thestr )
      {
        // '?' - любой непустой символ; подставить по очереди все символы в строку на позицию шаблона
        //       и продолжить сканирование на вложенных поддеревьях; пустая строка точно в пролёте
        case '?':
          while ( ncount-- > 0 )
            if ( (nerror = GetDictMatch( thestr + 1, cchstr - 1, getsubdic( thedic ), _do_it )) != 0 )
              return nerror;
          break;

      // '*' - любая последовательность символов длиной от 0 до ...; для каждого поддерева вызвать
      //       два варианта к каждому возможному поддереву:
        case '*':
          if ( (nerror = GetDictMatch( thestr + 1, cchstr - 1, dicorg, _do_it )) != 0 ) // zero chars match
            return nerror;
          while ( ncount-- > 0 )
            if ( (nerror = GetDictMatch( thestr, cchstr, getsubdic( thedic ), _do_it )) != 0 ) // more than one char match
              return nerror;
          break;

      // любой другой символ - должно быть точное соответствие
        default:
          while ( ncount-- > 0 )
            if ( *thedic != chfind )  getsubdic( thedic );
              else
            if ( (nerror = GetDictMatch( thestr + 1, cchstr - 1, getsubdic( thedic ), _do_it )) != 0 )
              return nerror;
          break;
      }
    }
      else
    while ( ncount-- > 0 )
      thedic += getserial( ++thedic );

  /*
    если есть список лексем, провести там отождествление
  */
    return hasupper( bflags ) ? GetListMatch( thestr, cchstr, thedic, _do_it ) : 0;
  }

  struct  matchArg
  {
    steminfo        stinfo;
    const byte_t*   szpost;
    size_t          ccpost;

  public:     // construction
    matchArg(): szpost( nullptr ), ccpost( 0 )
      {
      }

  public:     // initialization
    const byte_t* Load( const byte_t* thedic )
      {
        word16_t  oclass;

        oclass = getword16( thedic );

        if ( (oclass & wfPostSt) != 0 )
        {
          szpost = thedic;
          ccpost = *szpost++;
          thedic = szpost + ccpost;
        }

        stinfo.Load( classmap + (oclass & 0x7fff) );

        return thedic;
      }

  public:     // access
    const byte_t* ftable() const
      {
        return flexTree + (stinfo.tfoffs << 4);
      }
    const byte_t* mtable() const
      {
        return stinfo.mtoffs + mxTables;
      }
  };

  class fmLister
  {
    const steminfo& stinfo;
    byte_t          aforms[0x100];
    byte_t*         pforms;

  public:     // construction
    fmLister( const steminfo& st ): stinfo( st ), pforms( aforms )  {}

  public:     // operators
    void  operator () ( word16_t grinfo, byte_t bflags )
      {
        assert( pforms - aforms <= sizeof(aforms) );

        *pforms++ = MapWordInfo( (byte_t)stinfo.wdinfo, grinfo, bflags );
      }
    int   size() const
      {
        return (int)(pforms - aforms);
      }
    const byte_t* begin() const
      {
        return aforms;
      }
  };

  /*
    GetListMatch( template, list, action )

    Проводит отождествление шаблона с массивом лексем
  */
  template <class action>
  int   GetListMatch( const byte_t* thestr, size_t        cchstr,
                      const byte_t* pstems, const action& _do_it )
  {
    unsigned  ucount = getserial( pstems );

    while ( ucount-- > 0 )
    {
      byte_t    clower = *pstems++;
      byte_t    cupper = *pstems++;
      lexeme_t  nlexid = getserial( pstems );
      matchArg  maargs;
      size_t    ccflex = cchstr;
      int       nerror;
      byte_t    chnext;

    // load stem info
      pstems = maargs.Load( pstems );

    // cut by character
      if ( cchstr > 0 && !IsWildcard( chnext = *thestr ) )
      {
        if ( chnext > cupper )  break;
        if ( chnext < clower )  continue;
      }

    // нефлективным основам может соответствовать либо пустая строка, либо символ '*'
      if ( (maargs.stinfo.wdinfo & wfFlexes) == 0 || (maargs.stinfo.wdinfo & 0x3f) == 51 )
      {
        byte_t  formff = 0xff;

        if ( cchstr == 0 || IsAsterisk( thestr, cchstr ) )
          if ( (nerror = _do_it( nlexid, 1, &formff )) != 0 )
            return nerror;
      }
        else
    // при отсутствии чередований приложить к таблицам окончаний; постфикс также учитывать
      if ( (maargs.stinfo.wdinfo & wfMixTab) == 0 )
      {
        fmLister  fmlist( maargs.stinfo );

        if ( GetFlexMatch( thestr, cchstr, maargs.ftable(), maargs, (unsigned)-1, fmlist ) )
          if ( (nerror = _do_it( nlexid, fmlist.size(), fmlist.begin() )) != 0 )
            return nerror;
      }
        else
    // приложить последовательно к возможным таблицам чередования
      {
        fmLister      fmlist( maargs.stinfo );
        const byte_t* mixtab = maargs.mtable();   // Собственно таблица
        int           mixcnt = *mixtab++;         // Количество чередований
        int           mindex;

        for ( mindex = 0; mindex < mixcnt; ++mindex, mixtab += 1 + (0x0f & *mixtab) )
        {
          const byte_t* curmix = mixtab;
          unsigned      mixlen = 0x0f & *curmix;
          unsigned      powers = *curmix++ >> 4;

          GetIntrMatch( curmix, mixlen, thestr, cchstr, maargs, powers, fmlist );
        }

        if ( fmlist.size() > 0 )
          if ( (nerror = _do_it( nlexid, fmlist.size(), fmlist.begin() )) != 0 )
            return nerror;
      }
    }
    return 0;
  }

  /*
    GetIntrMatch()

    сравнение строки чередования с шаблоном
  */
  template <class action>
  bool  GetIntrMatch( const byte_t*   mixstr, size_t    mixlen,
                      const byte_t*   thestr, size_t    cchstr,
                      const matchArg& maargs, unsigned  powers, action& _do_it )
  {
  // skip matching parts of string
    while ( mixlen > 0 && cchstr > 0 && *thestr - *mixstr == 0 )
      {  --mixlen;  --cchstr;  ++thestr;  ++mixstr;  }

  // check the type of stop: matching interch string
    if ( mixlen == 0 )
      return GetFlexMatch( thestr, cchstr, maargs.ftable(), maargs, powers, _do_it );

  // check if the string is finished or if mismatch
    if ( cchstr == 0 || !IsWildcard( *thestr ) )
      return false;

  // check wildcard type for string match, mixlen > 0
    if ( *thestr == '?' )
      return GetIntrMatch( mixstr + 1, mixlen - 1, thestr + 1, cchstr - 1, maargs, powers, _do_it );

  // for asterisk, check if 0 match
    GetIntrMatch( mixstr, mixlen, thestr + 1, cchstr - 1, maargs, powers, _do_it );
    GetIntrMatch( mixstr + 1, mixlen - 1, thestr, cchstr, maargs, powers, _do_it );

    return _do_it.size() > 0;
  }

  inline
  bool  GetPostMatch( const byte_t* thestr, size_t cchstr, const byte_t* szpost, size_t ccpost )
  {
    if ( szpost == nullptr || ccpost == 0 )
      return cchstr == 0;
    while ( cchstr > 0 && ccpost > 0 && *thestr == *szpost )
      {  --cchstr;  --ccpost;  ++thestr;  ++szpost;  }
    if ( cchstr == 0 )
      return ccpost == 0;
    if ( ccpost == 0 )
      return IsAsterisk( thestr, cchstr );
    if ( *thestr == '?' )
      return GetPostMatch( thestr + 1, cchstr - 1, szpost + 1, ccpost - 1 );
    if ( *thestr != '*' )
      return false;
    return GetPostMatch( thestr,     cchstr,     szpost + 1, ccpost - 1 )
        || GetPostMatch( thestr + 1, cchstr - 1, szpost + 1, ccpost - 1 );
  }

  /*
    ListAllForms() - все формы для всех окончаний, соответствующие mpower
  */
  template <class action>
  void  ListAllForms( const byte_t*   fttree, const steminfo& stinfo,
                      unsigned        mpower, action&         _do_it )
  {
    byte_t    bflags = *fttree++;
    int       ncount = bflags & 0x7f;

    while ( ncount-- > 0 )
      ListAllForms( getsubdic( fttree ), stinfo, mpower, _do_it );

    if ( (bflags & 0x80) != 0 )
    {
      int   nforms = getserial( fttree );

      while ( nforms-- > 0 )
      {
        word16_t  grInfo = getword16( fttree );
        byte_t    bflags = *fttree++;
        int       desire = stinfo.GetSwapLevel( grInfo, bflags );

        if ( ((stinfo.wdinfo & wfMultiple) == 0 || (grInfo & gfMultiple) != 0) && (mpower & (1 << (desire - 1))) != 0 )
          _do_it( grInfo, bflags );
      }
    }
  }

  /*
    GetFlexMatch() - все формы, соответствующие переданному шаблону
  */
  template <class action>
  bool  GetFlexMatch( const byte_t*   thestr, size_t          cchstr,
                      const byte_t*   fttree, const matchArg& maargs,
                      unsigned        mpower, action&         _do_it )
  {
    byte_t        bflags = *fttree++;
    int           ncount = bflags & 0x7f;

  // check for zero length match on asterisk
    if ( cchstr > 0 )
    {
      byte_t  chfind = *thestr;

      if ( chfind == '*' )
        GetFlexMatch( thestr + 1, cchstr - 1, fttree - 1, maargs, mpower, _do_it );

      while ( ncount-- > 0 )
      {
        byte_t        chnext = *fttree++;
        unsigned      sublen = getserial( fttree );
        const byte_t* subdic = fttree;
                      fttree = subdic + sublen;

      // check character find type: wildcard, pattern or regular
        if ( chfind == '?' || chnext == chfind )
        {
          GetFlexMatch( thestr + 1, cchstr - 1, subdic, maargs, mpower, _do_it );
        }
          else
        if ( chfind == '*' )
        {
          if ( cchstr == 1 )  ListAllForms( subdic, maargs.stinfo, mpower, _do_it );
            else
          {
            GetFlexMatch( thestr + 1, cchstr - 1, subdic, maargs, mpower, _do_it );
            GetFlexMatch( thestr + 0, cchstr - 0, subdic, maargs, mpower, _do_it );
          }
        }
      }
    }
      else
    while ( ncount-- > 0 )
      fttree += getserial( ++fttree );

    if ( (bflags & 0x80) != 0 && GetPostMatch( thestr, cchstr, maargs.szpost, maargs.ccpost ) )
    {
      int   nforms = getserial( fttree );

      while ( nforms-- > 0 )
      {
        word16_t  grInfo = getword16( fttree );
        byte_t    bflags = *fttree++;
        int       desire = maargs.stinfo.GetSwapLevel( grInfo, bflags );

        if ( ((maargs.stinfo.wdinfo & wfMultiple) == 0 || (grInfo & gfMultiple) != 0) && (mpower & (1 << (desire - 1))) != 0 )
          _do_it( grInfo, bflags );
      }
    }
    return _do_it.size() > 0;
  }

  size_t  WildScan( byte_t* output, size_t cchout, const byte_t* ptempl, size_t  cchstr );

}

# endif // _wildscan_h_
