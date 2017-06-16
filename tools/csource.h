# if !defined( __csource_h__ )
# define  __csource_h__
# include "sweets.h"
# include <mtc/autoptr.h>
# include <mtc/wcsstr.h>
# include <mtc/file.h>
# include <assert.h>
# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <stdarg.h>

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
        char* getstr;

        if ( fgets( buffer, sizeof(buffer), infile ) != nullptr ) ++lineId;
          else return false;

        if ( (getstr = strstr( buffer, "//" )) != nullptr )
        {
          if ( getstr == buffer || mtc::isspace( getstr[-1] ) )
            *getstr = '\0';
        }

        if ( *(getstr = libmorph::TrimString( buffer )) == '\0' )
          continue;

        strcpy( output, getstr );
          break;
      }
      return true;
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
