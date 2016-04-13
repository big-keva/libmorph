# if !defined( __csource_h__ )
# define  __csource_h__
# include <assert.h>
# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <stdarg.h>

inline  bool  _isspace_( char ch )
{
  return ch != '\0' && (unsigned char)ch <= 0x20;
}

class CSource
{
  FILE*   lpfile;
  char*   lppath;
  char*   pcache;
  int     lineId;
public:
        CSource(): lpfile( NULL ),
                   lppath( NULL ),
                   pcache( NULL ),
                   lineId( 0 )
          {
          }
       ~CSource()
          {
            if ( lpfile != NULL )
              fclose( lpfile );
          }
  int   Message( int e, const char* string, ... )
          {
            va_list arglst;

            fprintf( stderr, "Error %d (%s:%d): ", e, GetName(), FetchId() );
            va_start( arglst, string );
            vfprintf( stderr, string, arglst );
            va_end( arglst );
            fprintf( stderr, "\n" );
            return e;
          }
  bool  SetFile( const char*  lpname )
          {
            if ( lpfile != NULL )
              fclose( lpfile );
            if ( lppath != NULL )
              free( lppath );
            lineId = 0;
            if ( (lppath = strdup( lpname )) == NULL )
              return false;
            if ( (lpfile = fopen( lpname, "rt" )) == NULL )
            {
              free( lppath );
              lppath = NULL;
              return false;
            }
            return true;
          }
  void  MapName( char*  lpdest, char*  lpname )
          {
            char*   strpos;
            char*   altpos;

          // Create the file name
            strcpy( lpdest, lppath );
            strpos = strrchr( lpdest, '/' );
            altpos = strrchr( lpdest, '\\' );

            if ( strpos == NULL ) strpos = altpos;
              else
            if ( altpos != NULL ) strpos = ( strpos > altpos ? strpos : altpos );

            if ( strpos == NULL )
              strpos = lpdest;

            if ( strpos > lpdest )
              *strpos++ = '/';
            strcpy( strpos, lpname );
          }
  const char* GetName() const
          {
            return lppath != NULL ? lppath : "";
          }
  bool  GetLine( char*  string )
          {
          // Check if there is line
            if ( pcache != NULL )
            {
              strcpy( string, pcache );
              free( pcache );
              pcache = NULL;
              ++lineId;
              return true;
            }
            for ( ; ; )
            {
              char  buffer[1024];
              char* ptrtop = buffer;
              char* ptrend;

              if ( !fgets( buffer, sizeof(buffer), lpfile ) )
                return false;
              while ( _isspace_( *ptrtop ) )
                ++ptrtop;
              if ( ptrtop[0] == '/' && ptrtop[1] == '/' )
                continue;
              if ( (ptrend = strstr( ptrtop, "//" )) != NULL &&
                _isspace_( ptrend[-1] ) && _isspace_( ptrend[2] ) )
                  *ptrend = '\0';
              for ( ptrend = ptrtop; *ptrend != '\0'; ptrend++ )
                (void)NULL;
              while ( ptrend > ptrtop && _isspace_( ptrend[-1] ) )
                --ptrend;
              *ptrend = '\0';
              ++lineId;
              if ( *ptrtop != '\0' )
              {
                strcpy( string, ptrtop );
                return true;
              }
            }
            return false;
          }
  int   FetchId() const
          {
            return lineId;
          }
  bool  PutLine( char*  lpline )
          {
            if ( pcache != NULL )
            {
              assert( false );
              return false;
            }
            if ( (pcache = strdup( lpline )) != NULL )
            {
              --lineId;
              return true;
            }
            return false;
          }
};

# endif  // __csource_h__
