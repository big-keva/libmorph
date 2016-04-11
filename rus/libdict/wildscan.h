# if !defined( _wildscan_h_ )
# define _wildscan_h_
# include <namespace.h>
# include "mlmadefs.h"

namespace LIBMORPH_NAMESPACE
{
  size_t  WildScan( byte08_t* output, size_t cchout, const byte08_t* ptempl, size_t  cchstr );
}

# endif // _wildscan_h_

