# if !defined( __sweets_h__ )
# define __sweets_h__
# include <stdarg.h>
# include <stdio.h>

namespace libmorph
{

  class file
  {
    FILE* lpfile;
  public:     // construction
        file( FILE* f = NULL ): lpfile( f )
          {
          }
        file( const file& f ): lpfile( f.lpfile )
          {
            ((file&)f).lpfile = NULL;
          }
  file& operator = ( FILE* f )
          {
            if ( lpfile != NULL )
              fclose( lpfile );
            lpfile = f;
              return *this;
          }
  file& operator = ( const file& f )
          {
            if ( lpfile != NULL )
              fclose( lpfile );
            if ( (lpfile = f.lpfile) != NULL )
              ((file&)f).lpfile = NULL;
            return *this;
          }
        ~file()
          {
            if ( lpfile != NULL )
              fclose( lpfile );
          }
  operator FILE* ()
          {
            return lpfile;
          }
  };
		
  inline  int   LogMessage( int nerror, const char* format, ... )
  {
    va_list   valist;
    va_start( valist, format );
      vfprintf( stderr, format, valist );
    va_end( valist );
    return nerror;
  }

  inline  int   LoadSource( array<char, char>&  output, const char* szfile )
  {
    libmorph::file  lpfile;
    long            cbfile;

    if ( (lpfile = fopen( szfile, "rb" )) != NULL ) fseek( lpfile, 0, SEEK_END );
      else  return ENOENT;
    if ( output.SetLen( cbfile = ftell( lpfile ) ) == 0 ) fseek( lpfile, 0, SEEK_SET );
      else  return ENOMEM;
    return fread( (char*)output, 1, cbfile, lpfile ) == cbfile ? 0 : EACCES;
  }

}  // end libmorph namespace

# endif // __sweets_h__
