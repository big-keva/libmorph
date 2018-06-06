# if !defined( __sweets_h__ )
# define __sweets_h__
# include <mtc/file.h>
# include <mtc/serialize.h>
# include <cstdarg>
# include <cstdint>
# include <cstdio>

namespace libmorph
{

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

  std::vector<char> LoadSource( const char* szfile )
  {
    std::vector<char> output;
    mtc::file         lpfile;
    long              cbfile;

    if ( (lpfile = fopen( szfile, "rb" )) == nullptr )
      throw std::runtime_error( "file '" + std::string( szfile ) + "' not found" );

    fseek( lpfile, 0, SEEK_END );
      output.resize( (size_t)(cbfile = ftell( lpfile )) );
    fseek( lpfile, 0, SEEK_SET );

    if ( fread( output.data(), 1, cbfile, lpfile ) != cbfile )
      throw std::runtime_error( "could not read " + std::to_string( cbfile ) + " bytes from '" + szfile + "'" );

    return std::move( output );
  }

  inline  char* TrimString( char* thestr )
  {
    char* endstr;

    while ( *thestr != '\0' && (uint8_t)*thestr <= 0x20 )
      ++thestr;
    for ( endstr = thestr; *endstr != '\0'; ++endstr )
      (void)NULL;
    while ( endstr > thestr && (uint8_t)endstr[-1] <= 0x20 )
      *--endstr = '\0';
    return thestr;
  }

}  // end libmorph namespace

# endif // __sweets_h__
