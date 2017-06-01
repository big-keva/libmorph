# if !defined( __dumppage_h__ )
# define __dumppage_h__
# include "sweets.h"
# include <stdio.h>

namespace libmorph
{

  class dumpsource
  {
    FILE*       lpfile;
    unsigned    stored;
    int         nchars;

  public:     // construction
    dumpsource( FILE* p, int maxchars = 12 ): lpfile( p ), stored( 0 ), nchars( maxchars )
      {
      }
    operator FILE*()
      {
        return lpfile;
      }
    int putchar( char c )
      {
        fprintf( lpfile, "%s0x%02x", stored == 0 ? "" : (stored % nchars) == 0 ? ",\n    " : ",", (unsigned char)c );
          ++stored;
        return 0;
      }
  };

  class BinaryDumper
  {
    const char* pszdir;
    const char* nspace;
    const char* szname;
    const char* header;
    file        lpfile;
    int         nerror;

  public:     // construction
    BinaryDumper(): pszdir( NULL ), nspace( NULL ), szname( NULL ), header( NULL ), nerror( 0 )
      {
      }
   ~BinaryDumper()
      {
        if ( (FILE*)lpfile != NULL && nspace != NULL && *nspace != '\0' )
          fprintf( lpfile, "\n"  "}  // end namespace\n\n" );
      }

    BinaryDumper& OutDir( const char* d )     {  pszdir = d;  return *this;  }
    BinaryDumper& Header( const char* h )     {  header = h;  return *this;  }
    BinaryDumper& Namespace( const char* n )  {  nspace = n;  return *this;  }
    BinaryDumper& Output( const char* f )     {  szname = f;  return *this;  }

    operator int()  {  return nerror;  }

  public:
    BinaryDumper& Type( const char* format, ... )
      {
        FILE*   pf;

        if ( (pf = OpenFile( NULL )) != NULL )
        {
          va_list valist;
          va_start( valist, format );

          vfprintf( pf, format, valist );

          va_end( valist );
        }
        return *this;
      }
    template <class serializable>
    BinaryDumper& Dump( const char* vaname, const serializable& serial )
      {
        FILE* pf;

        if ( (pf = OpenFile( vaname )) != NULL )
        {
          fprintf( pf, "  unsigned char %s[] =\n"  "  {\n"  "    ", vaname );
            {
              dumpsource  dumpsc( lpfile );
                serial.Serialize( &dumpsc );
            }
          fprintf( pf, "\n"  "  };\n" );
        }
        return *this;
      }
  protected:  // helper
    FILE* OpenFile( const char* vaname )
      {
        if ( (FILE*)lpfile == NULL )
        {
          const char* thedir = pszdir != NULL ? pszdir : "";
          char        szpath[0x400];
          const char* sslash = "";

          if ( *thedir != '\0' && strchr( "/\\", thedir[strlen( thedir ) - 1] ) == NULL )
            sslash = "/";
          sprintf( szpath, "%s%s%s.cpp", thedir, sslash, szname != NULL ? szname : vaname );

          if ( (lpfile = fopen( szpath, "wt" )) == NULL )
          {
            nerror = LogMessage( ENOENT, "Could not create file \'%s\'!\n", szpath );
            return NULL;
          }

        // print header if available
          if ( header != NULL && *header != '\0' )
            fprintf( lpfile, "%s\n", header );

        // print namespace if available
          if ( nspace != NULL && *nspace != '\0' )
            fprintf( lpfile, "namespace %s\n"  "{\n"  "\n", nspace );
        }
        return lpfile;
      }
  };

  template <class serializable>
  int       DumpBinary( const char* pszdir, const char*   nspace,
                        const char* vaname, serializable& serial )
  {
    return BinaryDumper().OutDir( pszdir ).Namespace( nspace ).Dump( vaname, serial );
  }

} // end 'libmorph' namespace

inline  libmorph::dumpsource* Serialize( libmorph::dumpsource* o, char c )
{
  return o != NULL && o->putchar( c ) == 0 ? o : NULL;
}

inline  libmorph::dumpsource* Serialize( libmorph::dumpsource* o, const void* p, size_t l )
{
  for ( ; o != NULL && l-- > 0; p = 1 + (char*)p )
    o = Serialize( o, *(char*)p );
  return o;
}

# endif // __dumppage_h__
