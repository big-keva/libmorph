# if !defined( __csource_h__ )
# define  __csource_h__
# include <mtc/autoptr.h>
# include <mtc/wcsstr.h>
# include <mtc/file.h>
# include <assert.h>
# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <stdarg.h>

inline  bool  _isspace_( char ch )  {  return ch != '\0' && (unsigned char)ch <= 0x20;  }

class CSource
{
  mtc::file         infile;
  mtc::_auto_<char> szpath;
  mtc::_auto_<char> scache;
  int               lineId;

public:     // construction
  CSource(): lineId( 0 )
    {
    }

public:     // accessors
  const char* GetName() const {  return szpath != nullptr ? szpath : "";  }
  int         FetchId() const {  return lineId;  }

public:     // message dumpers
  int         Message( int e, const char* string, ... )
    {
      va_list arglst;

      fprintf( stderr, "Error %d (%s:%d): ", e, GetName(), FetchId() );

      va_start( arglst, string );
        vfprintf( stderr, string, arglst );
      va_end( arglst );

      fprintf( stderr, "\n" );
      return e;
    }
  bool        SetFile( const char*  lpname )
    {
      lineId = 0;

      if ( (szpath = mtc::w_strdup( lpname )) == nullptr )
        return false;
      if ( (infile = fopen( szpath, "rt" )) == nullptr )
        return (szpath = nullptr, false);

      return true;
    }
  /*
    compose a new name from the current file name
  */
  char*       MapName( char*  lpdest, const char* lpname ) const
    {
      char*   pslash;

      for ( pslash = strcpy( lpdest, szpath ); *pslash != '\0'; ++pslash )
        (void)NULL;
      while ( pslash > lpdest && *pslash != '/' && *pslash != '\\' )
        --pslash;
      if ( pslash > lpdest )
        *pslash++ = '/';
      strcpy( pslash, lpname );
        return lpdest;
    }
  bool        GetLine( char*  output )
    {
      if ( scache != nullptr )
      {
        strcpy( output, scache );
        scache = nullptr;
        ++lineId;
      }
        else
      for ( ; ; )
      {
        char  buffer[1024];
        char* ptrtop = buffer;
        char* ptrend;

        if ( !fgets( buffer, sizeof(buffer), infile ) )
          return false;
        while ( _isspace_( *ptrtop ) )
          ++ptrtop;

        if ( mtc::w_strncmp( ptrtop, "//", 2 ) == 0 ) *(ptrend = ptrtop) = '\0';
          else
        if ( (ptrend = strstr( ptrtop, "//" )) != nullptr && _isspace_( ptrend[-1] ) && _isspace_( ptrend[2] ) )  *ptrend = '\0';
          else
        for ( ptrend = ptrtop; *ptrend != '\0'; ptrend++ )  (void)NULL;

        while ( ptrend > ptrtop && _isspace_( ptrend[-1] ) )
          *ptrend-- = '\0';

        ++lineId;

        if ( *ptrtop == '\0' )
          continue;

        strcpy( output, ptrtop );
          return true;
      }
      return false;
    }
  bool  PutLine( const char* sunget )
    {
      assert( scache == nullptr );

      if ( (scache = mtc::w_strdup( sunget )) == nullptr )
        return false;
      --lineId;
        return true;
    }
};

# endif  // __csource_h__
