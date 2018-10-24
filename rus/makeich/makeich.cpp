# include <moonycode/codes.h>
# include <tools/utf81251.h>
# include <tools/sourcefile.h>
# include "mtable.h"

std::string trim( std::string s )
{
  while ( s.length() != 0 && (unsigned char)s.back() <= 0x20 )
    s.pop_back();
  while ( s.length() != 0 && (unsigned char)s.front() <= 0x20 )
    s.erase( 0, 1 );
  return s;
}

std::string load( std::string& s )
{
  const char* p = s.c_str();
  std::string r;

  while ( (unsigned char)*p > 0x20 && *p != ',' ) ++p;

  r = s.substr( 0, p - s.c_str() );

  while ( *p != '\0' && (unsigned char)*p <= 0x20 ) ++p;
  if ( *p == ',' )  ++p;
  while ( *p != '\0' && (unsigned char)*p <= 0x20 ) ++p;

  s.erase( 0, p - s.c_str() );

  return r;
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
auto  s_rus_include = utf8to1251( ".включить" );
auto  s_eng_include = utf8to1251( ".include" );

void  MakeTab( Source&    source,
               collector& tabset )
{
  auto        header = source.Get();
  std::string stnext;

// извлечь индексы таблицы
  if ( header.length() == 0 )
    throw std::runtime_error( "unexpected end of file" );
      
  if ( has_command( header, s_rus_table ) ) header.erase( 0, s_rus_table.length() + 1 );  else
  if ( has_command( header, s_eng_table ) ) header.erase( 0, s_eng_table.length() + 1 );  else
    throw std::runtime_error( "'.table' declaration followed by table index expected" );

  if ( (header = trim( header )).length() == 0 )
    throw std::runtime_error( "unexpected end of line, table index expected" );

// извлечь первую строку
  if ( (stnext = source.Get()).length() == 0 )
    throw std::runtime_error( "unexpected end of file" );

  if ( stnext != "{" )
    throw std::runtime_error( "'{' exected" );

// последовательно считать таблицу
  for ( ; ; )
  {
    if ( (stnext = source.Get()).length() == 0 )
      throw std::runtime_error( "unexpected end of file" );

    if ( stnext == "}" )
      break;

  // загрузить чередование
    interchange ichone;
    std::string scondi;
    std::string st_one;
    std::string st_two;

    if ( (scondi = load( stnext )).length() == 0
      || (st_one = load( stnext )).length() == 0
      || (st_two = load( stnext )).length() == 0 )
        throw std::runtime_error( "interchange table has less than 2 fragments" );

    if ( st_one == "''" ) st_one = "";
    if ( st_two == "''" ) st_two = "";

    ichone.addstep( st_one, 0 );
    ichone.addstep( st_two, 1 );

    for ( int step = 2; stnext.length() != 0; ++step )
    {
      auto  st_add = load( stnext );

      if ( st_add == "''" ) st_add = "";
      ichone.addstep( st_add, step );
    }

  // зарегистрировать его
    tabset.add_interchange( header.c_str(), scondi.c_str(), std::move( ichone ) );
  }
}

void  Compile( Source& source, collector& tabset )
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

      if ( (stnext = trim( stnext )).length() != 0 )
      {
        auto  intext = source.Open( stnext );

        fprintf( stderr, "\t%s\n", stnext.c_str() );
        Compile( intext, tabset );
      } else throw std::runtime_error( "file name expected" );
    }
  }
  catch ( const std::runtime_error& e )
  {
    throw std::runtime_error( std::string( e.what() ) + "\n\tfrom " + source.Name() + ", line " + std::to_string( source.Line() ) );
  }
}

char  about[] = "libmorphrus stem-interchange tables compiler, version 1.0 (portable)\n"
                "Usage: makeich [options] inputname binaryname symbolsname\n"
                "Options are:\n"
                  "\t-w\tassume source tables use 1251 Windows Cyrillic instead of 866.\n";

int   main( int argc, char* argv[] )
{
  collector   tables;
  const char* inname = NULL;
  const char* ptrbin = NULL;
  const char* ptrsym = NULL;
  FILE*       output;
  int         argpos;

// Check the arguments
  for ( argpos = 1; argpos < argc; ++argpos )
  {
    if ( argv[argpos][0] == '-' )
    {
      if ( strcmp( argv[argpos], "-w" ) == 0 )
      {
        source_encoding = codepages::codepage_1251;
        continue;
      }
      fprintf( stderr, "Invalid switch \'%s\'!\n", argv[argpos] );
      return -1;
    }
    if ( inname == nullptr ) inname = argv[argpos];
      else
    if ( ptrbin == nullptr ) ptrbin = argv[argpos];
      else
    if ( ptrsym == nullptr ) ptrsym = argv[argpos];
      else
    break;
  }

// Check if completed
  if ( ptrsym == nullptr )
  {
    fprintf( stderr, about );
    return -1;
  }

  fprintf( stderr, "Compiling tables...\n" );

  try
  {
    auto  intext = OpenSource( inname, source_encoding );

    fprintf( stderr, "\t%s\n", inname );
    Compile( intext, tables );
  }
  catch ( const std::exception& x )
  {
    fprintf( stderr, "Error: %s\n", toCodepage( console_encoding, x.what() ).c_str() );
    return -1;
  }

  tables.relocate_tables();

// Open output file
  if ( (output = fopen( ptrbin, "wb" )) == NULL )
  {
    fprintf( stderr, "Could not create file \'%s\'!\n", ptrbin );
    return -1;
  }
  if ( tables.StoreTab( output ) == NULL )
  {
    fprintf( stderr, "Error writing the file \'%s\'!\n", ptrbin );
    fclose( output );
    return EACCES;
  }
  fclose( output );
  if ( (output = fopen( ptrsym, "wb" )) == NULL )
  {
    fprintf( stderr, "Could not create file \'%s\'!\n", ptrbin );
    return -1;
  }
  if ( tables.StoreRef( output ) == NULL )
  {
    fprintf( stderr, "Error writing the file \'%s\'!\n", ptrsym );
    fclose( output );
    return EACCES;
  }
  fclose( output );
  return 0;
}
