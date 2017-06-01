# if !defined( __ftables_h__ )
# define __ftables_h__
# include <string.h>

extern const unsigned short verbLevels[];
extern const unsigned short nounLevels[];

# if defined( _MSC_VER )
#   pragma warning( disable: 4237 )
# endif

bool          StripDefault( char*           szstem,
                            unsigned short  grinfo,
                            unsigned short  tfoffs,
                            unsigned short* levels,
                            int             clevel,
                            const char*     tables );
unsigned char GetMinLetter( const char*     tables, unsigned  tfoffs );
unsigned char GetMaxLetter( const char*     tables, unsigned  tfoffs );

inline  bool  StringHasEnd( const char* stem, const char* tail )
{
  size_t  ccStem = strlen( stem );
  size_t  ccTail = 0x0f & *tail++;

  return ccTail <= ccStem && memcmp( stem + ccStem - ccTail, tail, ccTail ) == 0;
}

# if defined( _MSC_VER )
#   pragma warning( default: 4237 )
# endif

# endif // __ftables_h__
