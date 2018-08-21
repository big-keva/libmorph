# include "ftable.h"
# include "gramap.h"
# include "../tools/sourcefile.h"
# include "../tools/utf81251.h"
# include "../tools/sweets.h"
# include <libcodes/codes.h>
# include <errno.h>

using namespace codepages;

bool  source1251 = false;

template <class filter>
const char* substr( std::string& output, const filter& _allow, const char* source )
{
  const char* srcorg = source;

  while ( source != nullptr && *source != '\0' && _allow( *source ) )
    ++source;

  output = std::move( std::string( srcorg, source - srcorg ) );
    return source;
}

fxitem  MapLine( const char*  string, graminfo cginfo )
{
  auto    is_space = [ ]( char c ){  return c != '\0' && (unsigned char)c <= 0x20;  };
  auto    flexchar = [&]( char c ){  return c != ',' && !is_space( c );  };
  auto    no_space = [&]( const char* s ){  while ( is_space( *s ) ) ++s;  return s;  };
  auto    no_comma = [&]( const char* s ){  return *(s = no_space( s )) == ',' ? s + 1 : s;  };

  fxitem      inflex;
  std::string grtext;
  std::string sznext;
  std::string szopts;

  substr(        szopts, flexchar, no_space( no_comma(
  substr(        sznext, flexchar, no_space( no_comma(
  substr(        grtext, flexchar, no_space( no_comma(
  substr( inflex.sztail, flexchar, no_space( string ) ) ) ) ) ) ) ) ) ) );

  if ( inflex.sztail == "''" )
    inflex.sztail = "";

  if ( grtext != "" )
    cginfo = MapInfo( grtext.c_str(), cginfo );

  inflex.grinfo = cginfo.grinfo;
  inflex.bflags = cginfo.bflags;

  if ( sznext != "" ) {  inflex.bflags |= 0x80;  inflex.sznext = sznext;  }  else
  if ( szopts != "" ) {  inflex.bflags |= 0x40;  inflex.sznext = szopts;  }  else
    inflex.sznext = "";

  return inflex;
}

inline  bool  is_space( char ch ) {  return ch != '\0' && (unsigned char)ch <= 0x20;  }
inline  bool  is_space( const char* pch ) {  return is_space( *pch );  }

inline  bool  has_command( const std::string& s, const std::string& cmd )
{
  size_t  cmdlen = cmd.length();

  return s.length() > cmdlen && strncmp( s.c_str(), cmd.c_str(), cmdlen ) == 0
    && is_space( s[cmdlen] );
}

unsigned  source_encoding = codepages::codepage_866;
# if defined( WIN32 )
unsigned  console_encoding = codepages::codepage_866;
# else
unsigned  console_encoding = codepages::codepage_utf8;
# endif

auto  s_rus_table   = utf8to1251( ".таблица" );
auto  s_eng_table   = utf8to1251( ".table" );
auto  s_rus_type    = utf8to1251( ".тип" );
auto  s_eng_type    = utf8to1251( ".type" );
auto  s_rus_include = utf8to1251( ".включить" );
auto  s_eng_include = utf8to1251( ".include" );

void  MakeTab( Source& src, fxlist& tab )
{
  auto        header = src.Get();
  std::string stnext;
  ftable      newtab;
  graminfo    cginfo = { 0, afAnimated|afNotAlive };

// извлечь индексы таблицы
  if ( header.length() == 0 )
    throw std::runtime_error( "unexpected end of file, table header expected" );

  if ( has_command( header, s_rus_table ) ) header.erase( 0, s_rus_table.length() + 1 );  else
  if ( has_command( header, s_eng_table ) ) header.erase( 0, s_eng_table.length() + 1 );  else
  if ( has_command( header, s_rus_type ) )  header.erase( 0, s_rus_type.length() + 1 );   else
  if ( has_command( header, s_eng_type ) )  header.erase( 0, s_eng_type.length() + 1 );   else
    throw std::runtime_error( "'.table' declaration followed by table index expected" );

  if ( (header = libmorph::trim( header )).length() == 0 )
    throw std::runtime_error( "unexpected end of line, table index expected" );

  if ( strstr( header.c_str(), utf8to1251( "нсв 4a32'" ).c_str() ) != nullptr )
  {
    int i = 0;
  }

// извлечь первую строку
  if ( (stnext = src.Get()).length() == 0 )
    throw std::runtime_error( "unexpected end of file" );

  if ( stnext != "{" )
    throw std::runtime_error( "'{' exected" );

// последовательно считать таблицу
  for ( ; ; )
  {
    if ( (stnext = src.Get()).length() == 0 )
      throw std::runtime_error( "unexpected end of file" );

    if ( stnext == "}" )
      break;

  // Check if the command string
    if ( stnext.front() == '.' ) cginfo = MapInfo( stnext.c_str() + 1, cginfo );
      else newtab.Insert( std::move( MapLine( stnext.c_str(), cginfo ) ) );
  }

  if ( newtab.empty() )
    throw std::runtime_error( "invalid (empty) inflexion table" );

  tab.Insert( std::move( newtab ), header.c_str() );
}

void  Compile( Source& source, fxlist& tabset )
{
  try
  {
    std::string stnext;

    while ( (stnext = source.Get()).length() != 0 )
    {
      if ( has_command( stnext, s_rus_include ) ) stnext.erase( 0, s_rus_include.length() + 1 );  else
      if ( has_command( stnext, s_eng_include ) ) stnext.erase( 0, s_eng_include.length() + 1 );  else
      {
        MakeTab( source.Put( std::move( stnext ) ), tabset );
        continue;
      }

      if ( (stnext = libmorph::trim( stnext )).length() != 0 )
      {
        auto  subsrc = source.Open( stnext );

        fprintf( stderr, "\t%s\n", stnext.c_str() );
        Compile( subsrc, tabset );
      } else throw std::runtime_error( "file name expected" );
    }
  }
  catch ( const std::runtime_error& e )
  {
    throw std::runtime_error( std::string( e.what() ) + "\n\tfrom " + source.Name() + ", line " + std::to_string( source.Line() ) );
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
  FILE*       dumper;

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

  fprintf( stderr, "Compiling tables...\n" );

  try
  {
    auto  infile = OpenSource( inname, source_encoding );

    fprintf( stderr, "\t%s\n", inname );
    Compile( infile, tables );
    tables.Relocate();
  }
  catch ( const std::exception& x )
  {
    fprintf( stderr, "Error: %s\n", toCodepage( console_encoding, x.what() ).c_str() );
    return -1;
  }

// create the tables
  if ( (dumper = fopen( ptrbin, "wb" )) == nullptr )
    return (fprintf( stderr, "Could not create file \'%s\'!\n", ptrbin ), -1);

  if ( tables.StoreTab( dumper ) == nullptr )
    return (fprintf( stderr, "Error writing the file \'%s\'!\n", ptrbin ), EACCES);
  else fclose( dumper );

// Open output file
  if ( (dumper = fopen( ptrsym, "wb" )) == NULL )
    return (fprintf( stderr, "Could not create file \'%s\'!\n", ptrsym), -1);

  if ( tables.StoreRef( dumper ) == nullptr )
    return (fprintf( stderr, "Error writing the file \'%s\'!\n", ptrsym ), EACCES);
  else fclose( dumper );

  return 0;
}
