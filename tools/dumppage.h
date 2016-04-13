# if !defined( __dumppage_h__ )
# define __dumppage_h__
# include <stdio.h>

namespace libmorph
{

  class dumpsource
  {
    FILE*       lpfile;
    unsigned    stored;
    const char* nspace;

  public:     // construction
        dumpsource( FILE* p, const char* n, const char* v ): lpfile( p ), stored( 0 ), nspace( 0 )
          {
            if ( n != 0 && n != "" )
            {
              fprintf( lpfile, "namespace %s\n"
                               "{\n"
                                "\n", nspace = n );
            }
            fprintf( lpfile, "  unsigned char %s[] =\n"
                             "  {\n"
                             "    ", v );
          }
       ~dumpsource()
          {
            fprintf( lpfile, "\n"
                             "  };\n" );
            if ( nspace != 0 )
            {
              fprintf( lpfile, "\n"
                               "}  // end namespace\n\n" );
            }
          }
    int putchar( char c )
          {
            fprintf( lpfile, "%s\'\\x%02x\'", stored == 0 ? "" : (stored % 0x10) == 0 ? ",\n    " : ",", (unsigned char)c );
              ++stored;
            return 0;
          }
  };

} // end 'libmorph' namespace

inline  libmorph::dumpsource* Serialize( libmorph::dumpsource* o, char c )
{
  return o != NULL && o->putchar( c ) == 0 ? o : NULL;
}

inline  libmorph::dumpsource* Serialize( libmorph::dumpsource* o, const void* p, unsigned l )
{
  for ( ; o != NULL && l-- > 0; p = 1 + (char*)p )
    o = Serialize( o, *(char*)p );
  return o;
}

# endif // __dumppage_h__
