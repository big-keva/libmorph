# if !defined( _wildscan_h_ )
# define _wildscan_h_
# include <namespace.h>
# include "mlmadefs.h"
# include "scandict.h"
# include "grammap.h"
# include <cassert>

namespace LIBMORPH_NAMESPACE
{

  inline  bool  IsWildcard( byte_t  c )
  {
    return c == '*' || c == '?';
  }

  inline  bool  IsAsterisk( const fragment& str )
  {
    if ( str.len > 0 )
    {
      auto  ptr = str.str;
      auto  end = str.len + ptr;

      while ( ptr != end && *ptr == '*' )
        ++ptr;
      return ptr == end;
    }
    return false;
  }

  /*
    templates pre-declaration
  */
  struct  matchArg;

  template <class action>
  int   GetDictMatch( const fragment&, const byte_t*,
    size_t nmatch, action& );
  template <class action>
  int   GetListMatch( const fragment&, const byte_t*,
    size_t nmatch, action& );
  template <class action>
  bool  GetIntrMatch( fragment, fragment, const matchArg&, unsigned,
    size_t nmatch, action& );
  template <class action>
  bool  GetFlexMatch( const fragment&, const byte_t*, const matchArg&, unsigned,
    size_t nmatch, action& );

  /*
    GetWordMatch( template, action )

    сканер словаря, вызывает action для каждой найденной лексемы
  */
  template <class action>
  int   GetWordMatch( const byte_t* thestr, size_t  cchstr, action& _do_it )
  {
    return GetDictMatch( { thestr, cchstr }, stemtree, 0, _do_it );
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
  int   GetDictMatch(
    const fragment& thestr,
    const byte_t*   thedic,
    size_t          nmatch, action& _do_it )
  {
    const byte_t* dicorg = thedic;
    byte_t        bflags = counter<unsigned char>::getvalue( thedic );
    int           ncount = counter<unsigned char>::getlower( bflags );
    byte_t        chfind;
    int           nerror;

    if ( thestr.size() > 0 )
    {
      switch ( chfind = *thestr.str )
      {
        // '?' - любой непустой символ; подставить по очереди все символы в строку на позицию шаблона
        //       и продолжить сканирование на вложенных поддеревьях; пустая строка точно в пролёте
        case '?':
          while ( ncount-- > 0 )
            if ( (nerror = GetDictMatch( thestr.next(), getsubdic( thedic ), nmatch + 1, _do_it )) != 0 )
              return nerror;
          break;

      // '*' - любая последовательность символов длиной от 0 до ...; для каждого поддерева вызвать
      //       два варианта к каждому возможному поддереву:
        case '*':
          if ( thestr.size() > 1 )
            if ( (nerror = GetDictMatch( thestr.next(), dicorg, nmatch, _do_it )) != 0 ) // zero chars match
              return nerror;
          while ( ncount-- > 0 )
            if ( (nerror = GetDictMatch( thestr, getsubdic( thedic ), nmatch + 1, _do_it )) != 0 ) // more than one char match
              return nerror;
          break;

      // любой другой символ - должно быть точное соответствие
        default:
          while ( ncount-- > 0 )
            if ( *thedic != chfind )  getsubdic( thedic );
              else
            if ( (nerror = GetDictMatch( thestr.next(), getsubdic( thedic ), nmatch + 1, _do_it )) != 0 )
              return nerror;
          break;
      }
    }
      else
    while ( ncount-- > 0 )
    {
      auto  sublen = getserial( ++thedic );
      thedic += sublen;
    }

  /*
    если есть список лексем, провести там отождествление
  */
    return counter<unsigned char>::hasupper( bflags ) ? GetListMatch( thestr, thedic, nmatch, _do_it ) : 0;
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
    unsigned        aforms[0x100];

  public:     // construction
    fmLister( const steminfo& st ): stinfo( st )
    {
      for ( auto ptr = std::begin( aforms ); ptr != std::end( aforms ); ++ptr )
        *ptr = (unsigned)-1;
    }

  public:     // operators
    void  operator () ( word16_t grinfo, byte_t bflags, unsigned lmatch )
      {
        auto  pmatch = aforms + MapWordInfo( (byte_t)stinfo.wdinfo, grinfo, bflags );
          *pmatch = std::min( *pmatch, lmatch );
      }

  public:     // forms buffer lister
    bool  HasForms() const
      {
        for ( auto ptr = std::begin( aforms ); ptr != std::end( aforms ); ++ptr )
          if ( *ptr != (unsigned)-1 )  return true;
        return false;
      }
    template <size_t N>
    int   GetForms( SStrMatch (&output)[N] ) const
      {
        auto  outptr = output;
        auto  outend = output + N;

        for ( auto ptr = std::begin( aforms ); ptr != std::end( aforms ) && outptr != outend; ++ptr )
          if ( *ptr != (unsigned)-1 )
            *outptr++ = { (byte_t)(ptr - std::begin( aforms )), *ptr };

        return (int)(outptr - output);
      }
  };

  /*
    GetListMatch( template, list, action )

    Проводит отождествление шаблона с массивом лексем
  */
  template <class action>
  int   GetListMatch( const fragment& thestr, const byte_t* pstems, size_t nmatch, action& _do_it )
  {
    unsigned  ucount = getserial( pstems );

    while ( ucount-- > 0 )
    {
      byte_t    clower = *pstems++;
      byte_t    cupper = *pstems++;
      lexeme_t  nlexid = getserial( pstems );
      matchArg  maargs;
      int       nerror;
      byte_t    chnext;

    // load stem info
      pstems = maargs.Load( pstems );

    // cut by character
      if ( !thestr.empty() && !IsWildcard( chnext = *thestr.str ) )
      {
        if ( chnext > cupper )  break;
        if ( chnext < clower )  continue;
      }

    // нефлективным основам может соответствовать либо пустая строка, либо символ '*'
      if ( (maargs.stinfo.wdinfo & wfFlexes) == 0 || (maargs.stinfo.wdinfo & 0x3f) == 51 )
      {
        auto  formff = SStrMatch{ 0xff, (unsigned)nmatch };

        if ( thestr.empty() || IsAsterisk( thestr ) )
          if ( (nerror = _do_it( nlexid, 1, &formff )) != 0 )
            return nerror;
      }
        else
    // при отсутствии чередований приложить к таблицам окончаний; постфикс также учитывать
      if ( (maargs.stinfo.wdinfo & wfMixTab) == 0 )
      {
        fmLister  fmlist( maargs.stinfo );
        SStrMatch aforms[0x100];

        if ( GetFlexMatch( thestr, maargs.ftable(), maargs, (unsigned)-1, nmatch, fmlist ) )
          if ( (nerror = _do_it( nlexid, fmlist.GetForms( aforms ), aforms )) != 0 )
            return nerror;
      }
        else
    // приложить последовательно к возможным таблицам чередования
      {
        fmLister      fmlist( maargs.stinfo );
        const byte_t* mixtab = maargs.mtable();   // Собственно таблица
        int           mixcnt = *mixtab++;         // Количество чередований
        int           mindex;
        SStrMatch     aforms[0x100];
        int           nforms;

        for ( mindex = 0; mindex < mixcnt; ++mindex, mixtab += 1 + (0x0f & *mixtab) )
        {
          const byte_t* curmix = mixtab;
          unsigned      mixlen = 0x0f & *curmix;
          unsigned      powers = *curmix++ >> 4;

          GetIntrMatch( { curmix, mixlen }, thestr, maargs, powers, nmatch, fmlist );
        }

        if ( (nforms = fmlist.GetForms( aforms )) > 0 )
          if ( (nerror = _do_it( nlexid, nforms, aforms )) != 0 )
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
  bool  GetIntrMatch( fragment mix, fragment str, const matchArg& maargs, unsigned  powers,
    size_t nmatch, action& _do_it )
  {
  // skip matching parts of string
    while ( !mix.empty() && !str.empty() && str.front() == mix.front() )
      ++str, ++mix, ++nmatch;

  // check the type of stop: matching interch string
    if ( mix.empty() )
      return GetFlexMatch( str, maargs.ftable(), maargs, powers, nmatch, _do_it );

  // check if the string is finished or if mismatch
    if ( str.empty() || !IsWildcard( str.front() ) )
      return false;

  // check wildcard type for string match, mixlen > 0
    if ( str.front() == '?' )
      return GetIntrMatch( mix.next(), str.next(), maargs, powers, nmatch + 1, _do_it );

  // for asterisk, check if 0 match
    GetIntrMatch( mix, str.next(), maargs, powers, nmatch, _do_it );
    GetIntrMatch( mix.next(), str, maargs, powers, nmatch + 1, _do_it );

    return _do_it.HasForms();
  }

  inline
  bool  GetPostMatch( fragment str, fragment match )
  {
    if ( match.empty() )
      return str.empty();

    while ( str.len != 0 && match.len != 0 && str.front() == match.front() )
      ++str, ++match;

    if ( str.empty() )
      return match.empty();

    if ( match.empty() )
      return IsAsterisk( str );

    if ( str.front() == '?' )
      return GetPostMatch( str.next(), match.next() );

    if ( str.front() != '*' )
      return false;

    return GetPostMatch( str, match.next() ) || GetPostMatch( str.next(), match.next() );
  }

  /*
    ListAllForms() - все формы для всех окончаний, соответствующие mpower
  */
  template <class action>
  void  ListAllForms(
    const byte_t*   fttree,
    const steminfo& stinfo,
    unsigned        mpower,
    size_t          nmatch,
    action&         _do_it )
  {
    byte_t    bflags = *fttree++;
    int       ncount = bflags & 0x7f;

    while ( ncount-- > 0 )
      ListAllForms( getsubdic( fttree ), stinfo, mpower, nmatch + 1, _do_it );

    if ( (bflags & 0x80) != 0 )
    {
      int   nforms = getserial( fttree );

      while ( nforms-- > 0 )
      {
        word16_t  grInfo = getword16( fttree );
        byte_t    bflags = *fttree++;
        int       desire = stinfo.GetSwapLevel( grInfo, bflags );

        if ( ((stinfo.wdinfo & wfMultiple) == 0 || (grInfo & gfMultiple) != 0) && (mpower & (1 << (desire - 1))) != 0 )
          _do_it( grInfo, bflags, nmatch );
      }
    }
  }

  /*
    GetFlexMatch() - все формы, соответствующие переданному шаблону
  */
  template <class action>
  bool  GetFlexMatch( const fragment& thestr, const byte_t* fttree,
    const matchArg& maargs, unsigned mpower, size_t nmatch, action& _do_it )
  {
    byte_t        bflags = *fttree++;
    int           ncount = bflags & 0x7f;

  // check for zero length match on asterisk
    if ( !thestr.empty() )
    {
      byte_t  chfind = thestr.front();

      if ( chfind == '*' )
        GetFlexMatch( thestr.next(), fttree - 1, maargs, mpower, nmatch, _do_it );

      while ( ncount-- > 0 )
      {
        byte_t        chnext = *fttree++;
        unsigned      sublen = getserial( fttree );
        const byte_t* subdic = fttree;
                      fttree = subdic + sublen;

      // check character find type: wildcard, pattern or regular
        if ( chfind == '?' || chnext == chfind )
        {
          GetFlexMatch( thestr.next(), subdic, maargs, mpower, nmatch + 1, _do_it );
        }
          else
        if ( chfind == '*' )
        {
          if ( thestr.size() == 1 )
          {
            ListAllForms( subdic, maargs.stinfo, mpower, nmatch + 1, _do_it );
          }
            else
          {
            GetFlexMatch( thestr.next(), subdic, maargs, mpower, nmatch, _do_it );
            GetFlexMatch( thestr,        subdic, maargs, mpower, nmatch, _do_it );
          }
        }
      }
    }
      else
    while ( ncount-- > 0 )
    {
      auto  sublen = getserial( ++fttree );
      fttree += sublen;
    }

    if ( (bflags & 0x80) != 0 && GetPostMatch( thestr, { maargs.szpost, maargs.ccpost } ) )
    {
      int   nforms = getserial( fttree );

      while ( nforms-- > 0 )
      {
        word16_t  grInfo = getword16( fttree );
        byte_t    bflags = *fttree++;
        int       desire = maargs.stinfo.GetSwapLevel( grInfo, bflags );

        if ( ((maargs.stinfo.wdinfo & wfMultiple) == 0 || (grInfo & gfMultiple) != 0) && (mpower & (1 << (desire - 1))) != 0 )
          _do_it( grInfo, bflags, nmatch );
      }
    }
    return _do_it.HasForms();
  }

  size_t  WildScan( byte_t* output, size_t cchout, const byte_t* ptempl, size_t  cchstr );

}

# endif // _wildscan_h_
