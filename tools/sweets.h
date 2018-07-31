# if !defined( __sweets_h__ )
# define __sweets_h__
# include <stdexcept>
# include <cstdarg>
# include <cstdint>
# include <cstdio>

namespace libmorph
{

  inline  int   LogMessage( int nerror, const char* format, ... )
  {
    va_list   valist;
    va_start( valist, format );
      vfprintf( stderr, format, valist );
    va_end( valist );
    return nerror;
  }

  inline  std::vector<char> LoadSource( const char* szfile )
  {
    std::vector<char> output;
    FILE*             lpfile;

    if ( (lpfile = fopen( szfile, "rb" )) == nullptr )
      throw std::runtime_error( "file '" + std::string( szfile ) + "' not found" );

    while ( !feof( lpfile ) )
    {
      char  chbuff[0x1000];
      auto  cbread = fread( chbuff, 1, sizeof(chbuff), lpfile );

      if ( cbread != 0 )
        output.insert( output.end(), chbuff, cbread + chbuff );
    }

    fclose( lpfile );

    return std::move( output );
  }

  inline  char* trim( char* thestr )
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

  inline  std::string trim( std::string s )
  {
    while ( s.length() != 0 && (unsigned char)s.back() <= 0x20 )
      s.pop_back();
    while ( s.length() != 0 && (unsigned char)s.front() <= 0x20 )
      s.erase( 0, 1 );
    return s;
  }

}  // end libmorph namespace

# endif // __sweets_h__
