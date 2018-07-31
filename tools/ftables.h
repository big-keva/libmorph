# if !defined( __ftables_h__ )
# define __ftables_h__
# include <cstdint>
# include <cstring>
# include <string>
# include <tuple>

namespace libmorph {

  extern const uint16_t verbLevels[];
  extern const uint16_t nounLevels[];

  class FlexStripper
  {
    const uint16_t* levels;
    const void*     tables;

  public:     // construction
    FlexStripper( const uint16_t* lev, const void* tab ): levels( lev ), tables( tab )  {}

  public:     // helpers
    static  std::tuple<uint8_t, uint8_t>  GetMinMaxChar( const char* tables, uint16_t     tfoffs );
    static  bool                          StringHasTail( const char* szstem, const char*  sztail );
    static  bool                          StringHasTail( std::string& rstem, const char*  sztail );

  public:     // stripper
    bool  StripStr( std::string&, uint16_t grinfo, uint16_t tfoffs, int clevel = 0 ) const;

  };

  inline  bool  FlexStripper::StringHasTail( const char* stem, const char* tail )
  {
    size_t  ccStem = strlen( stem );
    size_t  ccTail = 0x0f & *tail++;

    return ccTail <= ccStem && memcmp( stem + ccStem - ccTail, tail, ccTail ) == 0;
  }

  inline  bool  FlexStripper::StringHasTail( std::string& rstr, const char* tail )
  {
    return StringHasTail( rstr.c_str(), tail );
  }

}

# endif // __ftables_h__
