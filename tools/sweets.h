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

  inline  auto  LoadSource( const std::string& szpath ) -> std::vector<char>
  {
    std::vector<char> output;
    FILE*             lpfile;

    if ( (lpfile = fopen( szpath.c_str(), "rb" )) == nullptr )
      throw std::runtime_error( "could not open file '" + szpath + "'" );

    while ( !feof( lpfile ) )
    {
      char  chbuff[0x1000];
      auto  cbread = fread( chbuff, 1, sizeof(chbuff), lpfile );

      if ( cbread != 0 )
        output.insert( output.end(), chbuff, cbread + chbuff );
    }

    fclose( lpfile );

    return output;
  }

  template <class O>
  auto    LoadObject( O& object, const std::string& szpath ) -> O&
  {
    FILE* lpfile;

    if ( (lpfile = fopen( szpath.c_str(), "rb" )) == nullptr )
      throw std::runtime_error( "could not open file '" + szpath + "'" );

    if ( object.Load( (FILE*)lpfile ) == nullptr )
    {
      fclose( lpfile );
      throw std::runtime_error( "could not load object from file '" + std::string( szpath ) + "'" );
    }

    fclose( lpfile );
    return object;
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
