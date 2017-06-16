# if !defined( __grammap_h__ )
# define  __grammap_h__
# include "../rus/include/mlma1049.h"
# include "../ukr/include/mlma1058.h"

struct  graminfo
{
  uint16_t  grinfo;
  uint8_t   bflags;
};

int       InitRus();
int       InitUkr();
graminfo  MapInfo( const char*, graminfo );   // throws std::exception

# endif  // __grammap_h__
