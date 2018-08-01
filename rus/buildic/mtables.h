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
    };

    //
    // ѕредставление ссылок на некоторую таблицу чередований
    //
    struct tab: public std::vector<alt>
    {
      uint16_t              Find( const char* tabs,
                                  uint16_t    type,
                                  const char* stem,
                                  const char* rems ) const;
    };

    template <class S>  S*  Load( alt& a, S* s );
    template <class S>  S*  Load( tab& t, S* s );

  public:     // static helpers
    static  const char*                   GetDefaultStr( const char* tables, unsigned tbOffs );
    static  std::tuple<uint8_t, uint8_t>  GetMinMaxChar( const char* tables, uint16_t tboffs, uint8_t chrmin, uint8_t chrmax );

  public:     // API
    uint16_t                Find( const char* tabs,
                                  const char* ztyp,
                                  uint16_t    type,
                                  const char* stem,
                                  const char* rems ) const;
    FILE*                   Load( FILE* );

  protected:  // vars
    std::vector<tab>            tabset;
    std::map<std::string, tab*> mapper;

  };

}}

#endif // __mtables_h__
