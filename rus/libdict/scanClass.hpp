# if !defined( __libmorph_rus_scanClass_hpp__ )
# define __libmorph_rus_scanClass_hpp__
# include "xmorph/scandict.h"
# include "xmorph/capsheme.h"

namespace libmorph {
namespace rus {

  class Grammar final
  {
  public:
    template <class Filter>
    class list
    {
      Filter      filter;
      SGramInfo*  vector;
      mutable int ncount = 0;

    public:
      list( Filter f, SGramInfo* p ):
        filter( f ),
        vector( p )  {}

    public:
    // Сканирует таблицу окончаний и сохранает те грамматики, которые проходят фильтр
    // по заданному филбтру грамматических описаний
      int   operator()( const uint8_t* ftable, const fragment& inflex ) const
      {
        if ( inflex.size() == 0 )
        {
          auto  grinfo = SGramInfo{};
          auto  nforms = *ftable++;

          while ( nforms-- > 0 )
            if ( filter( grinfo = { 0, 0, getword16( ftable ), *ftable++ } ) )
              vector[ncount++] = grinfo;
        }
        return ncount;
      }
    };

  public:
    struct anyvalue
    {
      bool  operator ()( const SGramInfo& grinfo ) const
        {  (void)grinfo;  return true;  }
    };
    struct multiple
    {
      bool  operator () ( const SGramInfo& grinfo ) const
        {  return (grinfo.grInfo & gfMultiple) != 0;  }
    };
    template <class nextstep>
    class  altlevel
    {
      nextstep        filter;
      const steminfo& stinfo;
      unsigned        powset;

    public:     // construction
      altlevel( const steminfo& s, unsigned m, nextstep n ):
        filter( n ),
        stinfo( s ),
        powset( m ) {}

    public:     // functor
      bool  operator () ( const SGramInfo& grinfo ) const
        {
          int   desire = stinfo.GetSwapLevel( grinfo.grInfo, grinfo.bFlags );
            assert( desire >= 1 );

          return (powset & (1 << (desire - 1))) != 0 && filter( grinfo );
        }
    };

    template <class nextstep = anyvalue>  static
    auto  Alternator( const steminfo& s, unsigned m, nextstep n = nextstep() ) -> altlevel<nextstep>
      {  return altlevel<nextstep>( s, m, n );  }

    template <class Filter> static
    auto  List( Filter f, SGramInfo* p ) -> list<Filter>
      {  return list<Filter>( f, p );  }

  };

  struct ScanDict final
  {
    template <size_t N, class Filter>
    static  int   ScanTables(
      SGramInfo     (&buffer)[N],
      Filter          filter,
      const fragment& inflex,
      const uint8_t*  ptable )
    {
      return Flat::ScanTree<uint8_t>( Grammar::List( filter, buffer ), ptable, inflex );
    }

    template <size_t N>
    static  int   ScanTables(
      SGramInfo     (&buffer)[N],
      const steminfo& lextem,
      const fragment& inflex )
    {
      return lextem.wdinfo & wfMultiple ?
        ScanTables( buffer, Grammar::multiple(), inflex, lextem.GetFlexTable() ) :
        ScanTables( buffer, Grammar::anyvalue(), inflex, lextem.GetFlexTable() );
    }

    template <size_t N>
    static  int   ScanTables(
      SGramInfo     (&buffer)[N],
      const steminfo& lextem,
      const fragment& inflex,
      unsigned        mpower )
    {
      return lextem.wdinfo & wfMultiple ?
        ScanTables( buffer, Grammar::Alternator( lextem, mpower, Grammar::multiple() ),
          inflex, lextem.GetFlexTable() ) :
        ScanTables( buffer, Grammar::Alternator( lextem, mpower, Grammar::anyvalue() ),
          inflex, lextem.GetFlexTable() );
    }
  };

  template <class Collect>
  class BuildClass
  {
    const Collect&  target;

  public:
    BuildClass( const Collect& out ):
      target( out ) {}

  public:
    int   operator()(
      lexeme_t        nlexid,
      uint16_t        oclass,
      const fragment& inflex,
      const fragment& suffix ) const
    {
      return target( nlexid, steminfo( oclass ), { inflex.str + inflex.len, 0 },
        suffix, nullptr, 0 );
    }
  };

  template <class Collect>
  class MatchClass
  {
    const Collect&  target;
    uint16_t        scheme = 0x0100;
    unsigned        dwsets = 0;

  public:
    MatchClass( const Collect& out ):
      target( out ) {}

    auto  SetCapitalization( uint16_t scheme ) -> MatchClass&
      {  return this->scheme = scheme, *this;  }
    auto  SetSearchSettings( unsigned dwsets ) -> MatchClass&
      {  return this->dwsets = dwsets, *this;  }

  public:
    int   operator()(
      lexeme_t        nlexid,
      uint16_t        oclass,
      const fragment& inflex,
      const fragment& suffix ) const
    {
      auto      lextem = steminfo( oclass );
      SGramInfo grbuff[0x40];
      int       ngrams;

    // check output capitalization
      if ( (dwsets & sfIgnoreCapitals) == 0 && !IsGoodSheme( scheme, lextem.MinCapScheme() ) )
        return 0;

    // check if non-flective; only zero flexion match it
      if ( lextem.GetFlexTable() == nullptr )
      {
        if ( inflex.size() == 0 )
          return target( nlexid, steminfo{ lextem.wdinfo, 0, 0 }, inflex, suffix, SGramInfo{ 0, 0, lextem.tfoffs, 0 } );
        else return 0;
      }

    // Теперь - обработка флективных слов. Сначала обработаем более частый случай - флективное слово
    // без чередований в основе.
    // Для этого достаточно проверить, не выставлен ли флаг наличия чередований, так как при попадании
    // в эту точку слово заведомо будет флективным.
      if ( lextem.GetSwapTable() == nullptr )
      {
        ngrams = ScanDict::ScanTables( grbuff, lextem, inflex );

        return ngrams != 0 ? target( nlexid, lextem, inflex, suffix, grbuff, ngrams ) : 0;
      }
        else
    // Ну и, наконец, последний случай - слово с чередованиями в основе. Никаких специальных проверок здесь делать
    // уже не требуется, так как попадание в эту точку само по себе означает наличие чередований в основе
      {
        const byte_t* mixtab = lextem.GetSwapTable();     // Собственно таблица
        int           mixcnt = *mixtab++;                 // Количество чередований
        int           mindex;

        for ( mindex = 0; mindex < mixcnt; ++mindex, mixtab += 1 + (0x0f & *mixtab) )
        {
          const byte_t* curmix = mixtab;
          size_t        mixlen = 0x0f & *curmix;
          unsigned      powers = *curmix++ >> 4;
          const byte_t* flextr = inflex.str;
          size_t        flexcc = inflex.len;
          size_t        cmplen;
          int           rescmp;

        // сравнить чередование с фрагментом строки
          if ( (cmplen = flexcc) > mixlen )
            cmplen = mixlen;

          for ( rescmp = 0, mixlen -= cmplen, flexcc -= cmplen; cmplen-- > 0 && (rescmp = *curmix++ - *flextr++) == 0; )
            (void)0;

          if ( rescmp == 0 )
            rescmp = mixlen > 0;

          if ( rescmp > 0 ) break;
          if ( rescmp < 0 ) continue;

        // Построить массив грамматических отождествлений в предположении, что используется
        // правильная ступень чередования основы
          ngrams = ScanDict::ScanTables( grbuff, lextem, { flextr, flexcc }, powers );

          if ( ngrams != 0 && (ngrams = target( nlexid, lextem, inflex, suffix, grbuff, ngrams )) != 0 )
            return ngrams;
        }
      }
      return 0;
    }
  };

  template <class Collect>
  auto  MakeClassMatch( const Collect& match ) -> MatchClass<Collect>
    {  return MatchClass<Collect>( match );  }

  template <class Collect>
  auto  MakeBuildClass( const Collect& match ) -> BuildClass<Collect>
    {  return BuildClass<Collect>( match );  }

}} // end namespace

# endif // !__libmorph_rus_scanClass_hpp__
