# if !defined( __sweets_h__ )
# define __sweets_h__
# include <mtc/file.h>
# include <mtc/array.h>
# include <mtc/zarray.h>
# include <mtc/serialize.h>
# include <stdarg.h>
# include <stdio.h>

namespace libmorph
{
  using namespace mtc;

  class serialbuff
  {
    const void* buffer;
    size_t      length;

  public:
    serialbuff( const void* p, size_t l ): buffer( p ), length( l )
      {}
    template <class O> O*  Serialize( O* o ) const
      {  return ::Serialize( o, buffer, length );  }
  };

  inline  int   LogMessage( int nerror, const char* format, ... )
  {
    va_list   valist;
    va_start( valist, format );
      vfprintf( stderr, format, valist );
    va_end( valist );
    return nerror;
  }

  inline  int   LoadSource( array<char>&  output, const char* szfile )
  {
    file  lpfile;
    long  cbfile;

    if ( (lpfile = fopen( szfile, "rb" )) != NULL ) fseek( lpfile, 0, SEEK_END );
      else  return ENOENT;
    if ( output.SetLen( cbfile = ftell( lpfile ) ) == 0 ) fseek( lpfile, 0, SEEK_SET );
      else  return ENOMEM;
    return fread( (char*)output, 1, cbfile, lpfile ) == cbfile ? 0 : EACCES;
  }

  inline  char* TrimString( char* thestr )
  {
    char* endstr;

    while ( *thestr != '\0' && (byte_t)*thestr <= 0x20 )
      ++thestr;
    for ( endstr = thestr; *endstr != '\0'; ++endstr )
      (void)NULL;
    while ( endstr > thestr && (byte_t)endstr[-1] <= 0x20 )
      *--endstr = '\0';
    return thestr;
  }

}  // end libmorph namespace

# endif // __sweets_h__
