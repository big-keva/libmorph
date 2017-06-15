# if !defined( __grammap_h__ )
# define  __grammap_h__
# include "../rus/include/mlma1049.h"
# include "../ukr/include/mlma1058.h"

int     InitRus();
int     InitUkr();
bool    MapInfo( const char*, unsigned&, unsigned& );   // throws std::exception

# endif  // __grammap_h__
