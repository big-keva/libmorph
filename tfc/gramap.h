# if !defined( __gramap_h__ )
# define  __gramap_h__
# include "../rus/include/mlma1049.h"
# include "../ukr/include/mlma1058.h"

struct  graminfo
{
  uint16_t  grinfo;
  uint8_t   bflags;
};

void      InitRus();
void      InitUkr();
graminfo  MapInfo( const char*, graminfo );   // throws std::exception

# endif  // __gramap_h__
