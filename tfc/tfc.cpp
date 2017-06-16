# include "ftable.h"
# include "grammap.h"
# include "../tools/csource.h"
# include "../tools/sweets.h"
# include <libcodes/codes.h>
# include <mtc/autoptr.h>
# include <mtc/wcsstr.h>
# include <errno.h>

using namespace codepages;

bool  source1251 = false;

template <size_t N, class filter>
const char* substr( char (&output)[N], const filter& _allow, const char* source )
{
  char* outptr = output;
  char* outend = output + N - 1;

  while ( source != nullptr && output < outend && *source != '\0' && _allow( *source ) )
    *outptr++ = *source++;
  *outptr = '\0';
    return source;
}

fxitem  MapLine( const char*  string, graminfo cginfo )
{
  auto    fxchar = []( char c ){  return c != ',' && !isspace( c );  };
  auto    commas = []( const char* s ){  return *(s = ltrim( s )) == ',' ? s + 1 : s;  };
  fxitem  inflex;
  char    grtext[32];
  char    sznext[sizeof(inflex.sznext)];
  char    szopts[sizeof(inflex.sznext)];

  substr(        szopts, fxchar, ltrim( commas(
  substr(        sznext, fxchar, ltrim( commas(
  substr(        grtext, fxchar, ltrim( commas(
  substr( inflex.sztail, fxchar, ltrim( string ) ) ) ) ) ) ) ) ) ) );

  if ( w_strcmp( inflex.sztail, "\'\'" ) == 0 )
    inflex.sztail[0] = '\0';

  if ( grtext[0] != '\0' )
    cginfo = MapInfo( grtext, cginfo );

  inflex.grinfo = cginfo.grinfo;
  inflex.bflags = cginfo.bflags;

  if ( sznext[0] != '\0' )  {  inflex.bflags |= 0x80;  strcpy( inflex.sznext, sznext );  }  else
  if ( szopts[0] != '\0' )  {  inflex.bflags |= 0x40;  strcpy( inflex.sznext, szopts );  }  else
    inflex.sznext[0] = '\0';

  return inflex;
}

int   MakeTab( CSource& source, fxlist& rflist )
{
  char            header[256];
  char            string[256];
  char*           lphead;
  graminfo        cginfo = { 0, afAnimated|afNotAlive };
  _auto_<ftable>  tablep;
  int             nerror;

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
  if ( (tablep = allocate<ftable>()) == nullptr )
    return source.Message( ENOMEM, "Could not allocate memory!" );

// Parse the table
  while ( source.GetLine( string ) && strcmp( string, "}" ) != 0 )
  {
    if ( !source1251 )
      mbcstombcs( codepage_1251,  string, sizeof(string), codepage_866, string );

  // Check if the command string
    try
    {
      if ( string[0] == '.' ) cginfo = MapInfo( string + 1, cginfo );
        else
      if ( !tablep->Insert( MapLine( string, cginfo ) ) )
        return source.Message( ENOMEM, "Could not insert the entry to the table!" );
    }
    catch ( const _auto_<char>& msg )
    {  throw _auto_<char>( strduprintf( "line %d, %s", source.FetchId(), msg.ptr() ) );  }
  }

  if ( tablep->GetLen() == 0 )
    return source.Message( EINVAL, "Invalid (empty) inflexion table!" );

  if ( (nerror = rflist.Insert( tablep.ptr(), lphead )) != 0 )
    return source.Message( ENOMEM, "Could not register the table!" );

  return (tablep.detach(), 0);
}

int   Compile( CSource& source, fxlist& rflist )
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

      try
      {
        if ( (nerror = Compile( infile, rflist )) != 0 )
          return nerror;
        continue;
      }
      catch ( const _auto_<char>& msg )
      {  throw _auto_<char>( strduprintf( "file: %s,""\n\t%s", szpath, msg.ptr() ) );  }
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
  try
  {
    CSource source;

    if ( !source.SetFile( lppath ) )
    {
      fprintf( stderr, "Error: could not open file %s!\n", lppath );
      return 1;
    }
    return Compile( source, rflist );
  }
  catch ( const _auto_<char>& msg )
  {
    fprintf( stderr, "%s\n", msg.ptr() );
    return EFAULT;
  }
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
