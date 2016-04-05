# if !defined( _wildscan_h_ )
# define _wildscan_h_
# include <namespace.h>
# include "mlmadefs.h"

namespace LIBMORPH_NAMESPACE
{
  int  WildScan( byte08_t* output, const byte08_t* ptempl, unsigned cchstr );
}

# endif // _wildscan_h_

