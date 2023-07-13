# if !defined( __buildmorph_h__)
# define __buildmorph_h__
# include "wordtree.h"
# include "dumppage.h"
# include "sweets.h"
# include <moonycode/codes.h>
# include <stdexcept>
# include <algorithm>
# include <vector>

inline  size_t  lexkeybuf( char* lexbuf, unsigned nlexid )
{
  char*   lexorg = lexbuf;

  if ( (nlexid & ~0x000000ff) == 0 )  { *lexbuf++ = (char)nlexid; }
    else
  if ( (nlexid & ~0x0000ffFF) == 0 )  { *lexbuf++ = (char)(nlexid >> 8);  *lexbuf++ = (char)nlexid;  }
    else
  if ( (nlexid & ~0x00ffffff) == 0 )  { *lexbuf++ = (char)(nlexid >> 16);  *lexbuf++ = (char)(nlexid >> 8);  *lexbuf++ = (char)nlexid;  }
    else
  { *lexbuf++ = (char)(nlexid >> 24);  *lexbuf++ = (char)(nlexid >> 8);  *lexbuf++ = (char)nlexid;  }

  return lexbuf - lexorg;
}

template <class resolver>
class buildmorph
{
  using   steminfo = typename resolver::entry_type;
  using   stemdata = std::pair<std::string, steminfo>;

  typedef wordtree<std::vector<steminfo>, unsigned char>  StemTree;   // the actual tree
  typedef wordtree<unsigned, unsigned short>              LidsTree;


protected:
  std::string           nspace;
  std::string           unknwn;
  std::string           outdir;

protected:
  const unsigned        sourceCP;
  StemTree              stemtree;   // the actual tree
  LidsTree              lidstree;
  resolver&             clparser;
  const char*           sLicense;
  FILE*                 unknowns;

public:     // construction
  buildmorph( resolver& cparser, const char* license, unsigned srcpage ):
    sourceCP( srcpage ),
    clparser( cparser ),
    sLicense( license ),
    unknowns( nullptr ) {}
 ~buildmorph();

public:
  buildmorph& SetTargetDir( const std::string& s )  {  outdir = s;  return *this;  }
  buildmorph& SetNamespace( const std::string& s )  {  nspace = s;  return *this;  }
  buildmorph& SetUnknowns ( const std::string& s )  {  unknwn = s;  return *this;  }

public:     // initialization
  void  CreateDict( const std::vector<const char*>& );

protected:  // helpers
  void  DictReader( FILE* );
  void  PutUnknown( const char* );

};

// buildmorph implementation

template <class resolver>
buildmorph<resolver>::~buildmorph()
{
  if ( unknowns != nullptr )
    fclose( unknowns );
}

template <class resolver>
void  buildmorph<resolver>::CreateDict( const std::vector<const char*>& dicset )
{
  size_t    length;

  for ( auto s: dicset )
  {
    FILE* lpfile;

    if ( (lpfile = fopen( s, "rt" )) == nullptr )
      throw std::runtime_error( std::string( "Could not open file \'" ) + s + "\'!" );

    try
    {
      libmorph::LogMessage( 0, "Loading %s...", s );
        DictReader( lpfile );
        fclose( lpfile );
      libmorph::LogMessage( 0, " OK\n" );
    }
    catch ( ... )
    {
      fclose( lpfile );
      throw;
    }
  }

// dump word tree
  if ( (length = stemtree.GetBufLen()) != (size_t)-1 )
    libmorph::LogMessage( 0, "info: main dictionary size is %d bytes.\n", length );
  else throw std::runtime_error( "fault to calculate the main dictionary size!" );

  // create stem mapping
  stemtree.Enumerate( [&]( std::vector<steminfo>& astems, size_t offset )
    {
      char  lidstr[0x20];

      offset += ::GetBufLen( astems.size() );

      for ( auto& next: astems )
      {
        *lidstree.Insert( lidstr, lexkeybuf( lidstr, next.nlexid ) ) = (unsigned)offset;
          offset += ::GetBufLen( next );
      }
    } );

  if ( (length = lidstree.GetBufLen()) != (size_t)-1 )
    libmorph::LogMessage( 0, "info: lids dictionary size is %d bytes.\n", length );
  else throw std::runtime_error( "fault to calculate the main dictionary size!" );

// dump word tree
  libmorph::BinaryDumper().
    Namespace( nspace ).OutDir( outdir ).Header( sLicense ).Dump( "stemtree", stemtree );

  libmorph::BinaryDumper().
    Namespace( nspace ).OutDir( outdir ).Header( sLicense ).Dump( "lidstree", lidstree );
}

template <class resolver>
void  buildmorph<resolver>::DictReader( FILE* source )
{
  char    szline[0x400];
  int     nwords;

  for ( nwords = 0; fgets( szline, sizeof(szline) - 1, source ) != nullptr; ++nwords )
  {
    char*                 strtop;
    char*                 strend;
    char*                 pszlid;
    lexeme_t              nlexid;
    std::vector<stemdata> astems;

  // change codepage
    codepages::mbcstombcs( codepages::codepage_1251, szline, sizeof(szline), sourceCP, szline );

  // string prepare block
    for ( strtop = szline; *strtop != '\0' && (unsigned char)*strtop <= 0x20; ++strtop )  (void)NULL;
    for ( strend = strtop; *strend != '\0'; ++strend )                                    (void)NULL;
    while ( strend > strtop && (unsigned char)strend[-1] <= 0x20 )  *--strend = '\0';

    if ( *strtop == '\0' )
      continue;
    if ( strncmp( strtop, "//", 2 ) == 0 )
    {
      PutUnknown( strtop );
      continue;
    }

  // resolve lexeme identifier
    if ( (pszlid = strstr( strtop, " LID:" )) == nullptr )
      continue;
      
    if ( (nlexid = strtoul( pszlid + 5, &strend, 0 )) == 0 )
      continue;

  // resolve word properties
    if ( (astems = clparser( strtop )).size() == 0 )
    {
      PutUnknown( strtop );
      continue;
    }

    for ( auto& stdata: astems )
    {
      auto  pstems = stemtree.Insert( stdata.first.c_str(), stdata.first.length() );
      auto  inspos = pstems->begin();

      stdata.second.nlexid = nlexid;

      while ( inspos != pstems->end() && *inspos < stdata.second )
        ++inspos;
      pstems->insert( inspos, stdata.second );
    }
  }
}

template <class resolver>
void  buildmorph<resolver>::PutUnknown( const char* unknown )
{
  if ( unknowns == nullptr )
  {
    if ( unknwn.empty() )
      return;
    if ( (unknowns = fopen( unknwn.c_str(), "wt" )) == nullptr )
      throw std::runtime_error( "could not create file '" + unknwn + "'" );
  }
  fprintf( unknowns, "%s\n", unknown );
}

# endif  // __buildmorph_h__
