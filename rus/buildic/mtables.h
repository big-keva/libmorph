#if !defined( __mtables_h__ )
#define __mtables_h__
# include <algorithm>
# include <vector>
# include <tuple>
# include <map>

namespace libmorph {
namespace rus{

  class Alternator
  {
    //
    // „ередование с условием
    //
    struct  alt
    {
      uint16_t  offset;
      char      szcond[0x10];

    public:     // loading
      template <class S>  S*  Load( S* s );

    };

    //
    // ѕредставление ссылок на некоторую таблицу чередований
    //
    struct tab: public std::vector<alt>
    {
      uint16_t                Find( const char* tabs,
                                    uint16_t    type, const char* stem, const char* rems ) const;

      template <class S>  S*  Load( S* s );

    };

    struct ref: public std::pair<std::string, size_t>
    {
      template <class S>  S*  Load( S* s );
    };

  public:     // static helpers
    static  const char*                   GetDefaultStr( const char* tables, unsigned tbOffs );
    static  std::tuple<uint8_t, uint8_t>  GetMinMaxChar( const char* tables, uint16_t tboffs, uint8_t chrmin, uint8_t chrmax );

  public:     // API
    uint16_t                  Find( const char* tabs, const char* ztyp,
                                    uint16_t    type, const char* stem, const char* rems ) const;
    template <class S>  S*    Load( S* s );

  protected:  // vars
    std::vector<tab>            tabset;
    std::map<std::string, tab*> mapper;

  };

  // Alternator inline implementation

  template <class S>
  S*  Alternator::alt::Load( S* s )
  {
    size_t  cchstr;

    if ( (s = ::FetchFrom( ::FetchFrom( s, offset ), cchstr )) == nullptr )
      return nullptr;
    if ( cchstr >= sizeof(szcond) )
      return nullptr;
    if ( (s = ::FetchFrom( s, szcond, cchstr )) == nullptr )
      return nullptr;
    szcond[cchstr] = '\0';
      return s;
  }

  template <class S>
  S*  Alternator::tab::Load( S* s )
  {
    alt     newalt;
    size_t  toload;

    if ( (s = ::FetchFrom( s, toload )) != nullptr ) reserve( toload );
      else return nullptr;

    while ( toload-- > 0 && (s = newalt.Load( s )) != nullptr )
      push_back( newalt );

    return s;
  }

  template <class S>
  S*  Alternator::ref::Load( S* s )
  {
    size_t  ccname;

    if ( (s = ::FetchFrom( ::FetchFrom( s, second ), ccname )) != nullptr )
    {
      first.assign( ccname, ' ' );

      s = ::FetchFrom( s, (char*)first.c_str(), ccname );
    }
    return s;
  }

  template <class S>
  S*  Alternator::Load( S* s )
  {
    tab               newtab;
    ref               newref;
    size_t            toload;

    if ( (s = ::FetchFrom( s, toload )) != nullptr )  {  tabset.clear();  tabset.reserve( toload );  }
      else return nullptr;

    while ( toload-- > 0 && (s = newtab.Load( s )) != nullptr )
      tabset.push_back( std::move( newtab ) );

    if ( (s = ::FetchFrom( s, toload )) == nullptr )
      return nullptr;

    while ( toload-- > 0 && (s = newref.Load( s )) != nullptr )
      mapper.insert( { std::move( newref.first ), tabset.data() + newref.second } );

    return s;
  }

}}

#endif // __mtables_h__
