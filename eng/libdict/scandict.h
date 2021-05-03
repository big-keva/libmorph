# if !defined( _scandict_h_ )
# define _scandict_h_

# include "pageman.h"

namespace __libmorpheng__
{
  bool  ScanDictionary( CPageMan&, const char*, int, SScanPage&, FAction );
  bool  DirectPageJump( CLIDsMan& lids, CPageMan& main, word32_t wlid,
    SScanPage& scan, FAction action );
}

# endif // _scandict_h_
