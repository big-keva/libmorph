# if !defined( __conffile_h__ )
# define __conffile_h__
# include <mtc/jsonTools.h>
# include <mtc/autoptr.h>
# include <mtc/zarray.h>
# include <mtc/file.h>
# include "sweets.h"

namespace libmorph
{
  using namespace mtc;

  ZArray  LoadConfig( FILE* lpfile )
  {
    jsonRevive* revive =  add_zarray( "flex",
                            add_charstr( "table",
                            add_charstr( "index", nullptr ) ),
                          add_zarray( "intr",
                            add_charstr( "table",
                            add_charstr( "index", nullptr ) ),
                          add_array_charstr( "dictionaries",
                          add_charstr( "namespace",
                          add_charstr( "faultList",
                          add_charstr( "TargetDir",
                          add_charstr( "codepage", nullptr )))))));
    zarray<>    zvalue;

    if ( ParseJson( lpfile, zvalue, revive ) == nullptr )
      return SetError( EINVAL, "Could not parse json configuration @" __FILE__ ":%u", __LINE__ );

    if ( revive )
      revive->Delete();

    return zvalue;
  }

  ZArray  LoadConfig( const char* szfile )
  {
    file  infile;

    if ( (FILE*)(infile = fopen( szfile, "rt" )) == NULL )
      return SetError( ENOENT, "Could not open file '%s' @" __FILE__ ":%u", __LINE__ );
    return LoadConfig( (FILE*)infile );
  }

}  // end libmorph namespace

# endif // __conffile_h__
