# include "ftable.h"
# include "grammap.h"
# include "../tools/csource.h"
# include "../tools/sweets.h"
# include <libcodes/codes.h>
# include <mtc/wcsstr.h>
# include <errno.h>

using namespace codepages;

bool  source1251 = false;

inline  const char*   getnext( char* lpdest, const char* string )
{
  for ( string = ltrim( string ); *string != '\0' && *string != ',' && !isspace( *string ); )
    *lpdest++ = *string++;
  if ( *(string = ltrim( string )) == ',' )
    ++string;
  *lpdest = '\0';
    return string;
}

bool  MapLine( const char*  string,
               char*        lptail,
               char*        lpnext,
               char*        lpopts,
               unsigned&    grinfo,
               unsigned&    rflags )
{
  char  grtext[32];

  getnext( lpopts, getnext( lpnext, getnext( grtext, getnext(
    lptail, string ) ) ) );
  if ( strcmp( lptail, "\'\'" ) == 0 )
    *lptail = '\0';
  return grtext[0] == '\0' ? true : MapInfo( grtext, grinfo, rflags );
}

int   MakeTab( CSource& source,
               fxlist&  rflist )
{
  char      header[256];
  char      string[256];
  char*     lphead;
  unsigned  grinfo = 0;
  unsigned  bflags = afAnimated | afNotAlive;
  ftable*   ptable;
  int       nerror;

// Get top line
  if ( !source.GetLine( header ) || !source.GetLine( string ) )
    return source.Message( EINVAL, "Invalid file format!" );

// Create header info
  if ( !source1251 )
  {
    mbcstombcs( codepage_1251, header, sizeof(header), codepage_866, header );
    mbcstombcs( codepage_1251, string, sizeof(string), codepage_866, string );
  }

  if ( strncmp( header, ".таблица", 8 ) == 0 )  lphead = header + 8;  else
  if ( strncmp( header, ".тип",     4 ) == 0 )  lphead = header + 4;  else
  if ( strncmp( header, ".table",   6 ) == 0 )  lphead = header + 6;  else
  if ( strncmp( header, ".type",    5 ) == 0 )  lphead = header + 5;  else
    return source.Message( EINVAL, "Table header '.table' or '.type' expected!" );

// Check if the empty header
  if ( *(lphead = libmorph::TrimString( lphead )) == '\0' )
    return source.Message( EINVAL, "Unnamed table!" );
  
// Check table start
  if ( strcmp( string, "{" ) != 0 )
    return source.Message( EINVAL, "\'{\' expected!" );

// Create the table
  if ( (ptable = allocate<ftable>()) == nullptr )
    return source.Message( ENOMEM, "Could not allocate memory!" );

// Parse the table
  while ( source.GetLine( string ) && strcmp( string, "}" ) != 0 )
  {
    if ( !source1251 )
      mbcstombcs( codepage_1251,  string, sizeof(string), codepage_866, string );

  // Check if the command string
    if ( string[0] == '.' )
    {
      if ( !MapInfo( string + 1, grinfo, bflags ) )
      {
        delete ptable;
        return source.Message( EINVAL, "invalid grammatical instruction!" );
      }
      continue;
    }
      else
    {
      fxitem    inflex;
      char      sznext[64];
      char      szopts[64];
      unsigned  flinfo = grinfo;
      unsigned  fflags = bflags;

    // Map the string to info
      if ( !MapLine( string, inflex.sztail, sznext, szopts, flinfo, fflags ) )
      {
        delete ptable;
        return source.Message( EINVAL, "Invalid flexion description!" );
      }

    // Register the flexion
      inflex.grinfo = (unsigned short)flinfo;
      inflex.bflags = (unsigned char)fflags;

      if ( sznext[0] != 0 )
      {
        inflex.bflags |= 0x80;
        strcpy( inflex.sznext, sznext );
      }
        else
      if ( szopts[0] != 0 )
      {
        inflex.bflags |= 0x40;
        strcpy( inflex.sznext, szopts );
      }
        else
      inflex.sznext[0] = '\0';

      if ( !ptable->Insert( inflex ) )
      {
        delete ptable;
        return source.Message( ENOMEM, "Could not insert the entry to the table!" );
      }
    }
  }
  if ( ptable->GetLen() == 0 )
  {
    delete ptable;
    return source.Message( EINVAL, "Invalid (empty) inflexion table!" );
  }
  if ( (nerror = rflist.Insert( ptable, lphead )) != 0 )
    return source.Message( ENOMEM, "Could not register the table!" );

  return 0;
}

int   Compile( CSource& source,
               fxlist&  rflist )
{
  char  string[1024];
  int   nerror;

// Load all the file lines
  while ( source.GetLine( string ) )
  {
  // Check the codepage switch and recode the string
    if ( !source1251 )
      mbcstombcs( codepage_1251,  string, sizeof(string), codepage_866, string );

  // Check the command
    if ( string[0] != '.' )
      return source.Message( EINVAL, "invalid source line %s!\n", string );

    if ( strncmp( string + 1, "включить", 8 ) == 0 && isspace( string[9] )
      || strncmp( string + 1, "include",  7 ) == 0 && isspace( string[8] ) )
    {
      CSource infile;
      char*   ptrtop;
      char    szpath[256];

      for ( ptrtop = string + 8; *ptrtop != '\0' && !isspace( *ptrtop ); ++ptrtop )
        (void)NULL;

    // Create the file name
      source.MapName( szpath, ltrim( ptrtop ) );

    // Try open the nested file
      if ( !infile.SetFile( szpath ) )
        return source.Message( ENOENT, "could not open file %s!\n", szpath );
      else
        fprintf( stderr, "\t%s\n", szpath );

    // Compile the file
      if ( (nerror = Compile( infile, rflist )) != 0 )
        return nerror;
      continue;
    }

  // Unget the line
    if ( !source1251 )
      mbcstombcs( codepage_866,  string, sizeof(string), codepage_1251, string );
    source.PutLine( string );

  // Compile inflexion table
    if ( (nerror = MakeTab( source, rflist )) != 0 )
      return nerror;
  }
  return 0;
}

int   Compile( const char*  lppath,
               fxlist&      rflist )
{
  CSource source;

  if ( !source.SetFile( lppath ) )
  {
    fprintf( stderr, "Error: could not open file %s!\n", lppath );
    return 1;
  }
  return Compile( source, rflist );
}

char  about[] = "libmorphrus inflexion tables compiler, version 1.0 (portable)\n"
                "Usage: tfc [options] inputname binaryname symbolsname\n"
                "Options are:\n"
                  "\t-w\tassume source tables use 1251 Windows Cyrillic instead of 866;\n"
                  "\t-lang:rus/ukr - set the tables description syntax, russian or ukrainian; the detault is russian.\n";

int   main( int argc, char* argv[] )
{
  fxlist      tables;
  int         nerror;
  const char* inname = NULL;
  const char* ptrbin = NULL;
  const char* ptrsym = NULL;
  mtc::file   output;

  InitRus();

// Check the arguments
  for ( nerror = 1; nerror < argc; nerror++ )
  {
    if ( argv[nerror][0] == '-' )
    {
      if ( strcmp( argv[nerror], "-w" ) == 0 )
      {
        source1251 = true;
        continue;
      }
      if ( strncmp( argv[nerror], "-lang:", 6 ) == 0 )
      {
        if ( strcmp( argv[nerror] + 6, "rus" ) == 0 )
        {
          InitRus();
          continue;
        }
        if ( strcmp( argv[nerror] + 6, "ukr" ) == 0 )
        {
          InitUkr();
          continue;
        }
      }
      fprintf( stderr, "Invalid switch \'%s\'!\n", argv[nerror] );
      return -1;
    }
    if ( inname == NULL ) inname = argv[nerror];
      else
    if ( ptrbin == NULL ) ptrbin = argv[nerror];
      else
    if ( ptrsym == NULL ) ptrsym = argv[nerror];
      else
    break;
  }

// Check if completed
  if ( ptrsym == nullptr )
    return (fprintf( stderr, about ), -1);

  fprintf( stderr, "Compiling tables...\n""\t%s\n", inname );

  if ( (nerror = Compile( inname, tables )) != 0 )
    return nerror;

  tables.Relocate();

// create the tables
  if ( (output = fopen( ptrbin, "wb" )) == nullptr )
    return (fprintf( stderr, "Could not create file \'%s\'!\n", ptrbin ), -1);

  if ( tables.StoreTab( (FILE*)output ) == NULL )
    return (fprintf( stderr, "Error writing the file \'%s\'!\n", ptrbin ), EACCES);

// Open output file
  if ( (output = fopen( ptrsym, "wb" )) == NULL )
    return (fprintf( stderr, "Could not create file \'%s\'!\n", ptrsym), -1);

  if ( tables.StoreRef( (FILE*)output ) == 0 )
    return (fprintf( stderr, "Error writing the file \'%s\'!\n", ptrsym ), EACCES);

  return 0;
}
