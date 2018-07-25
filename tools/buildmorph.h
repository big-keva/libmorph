# if !defined( __buildmorph_h__)
# define __buildmorph_h__
# include "wordtree.h"
# include "dumppage.h"
# include "sweets.h"
# include <libcodes/codes.h>
# include <stdexcept>
# include <algorithm>
# include <vector>

template <class theclass>
class classtable
{
  struct  classofs: public theclass
  {
    unsigned  offset;

    classofs( const theclass& r, unsigned o ): theclass( r ), offset( o ) {}
    classofs( const classofs& c ): theclass( c ), offset( c.offset ) {}
    bool  operator == ( const theclass& r ) const {  return theclass::operator == ( r );  }
  };

  std::vector<classofs> clsset;
  unsigned              length;

public:     // construction
  classtable(): length( 0 ) {}
  classtable( const classtable& ) = delete;

public:     // API
  unsigned  AddClass( const theclass& rclass )
    {
      auto  pfound = std::find( clsset.begin(), clsset.end(), rclass );

      if ( pfound != clsset.end() )
        return pfound->offset;

      clsset.push_back( classofs( rclass, length ) );
        length += rclass.GetBufLen();

      return clsset.back().offset;
    }

public:     // serialization
  size_t  GetBufLen() const
    {
      return length;
    }
  template <class O>
  O*      Serialize( O* o ) const
    {
      for ( auto p = clsset.begin(); o != nullptr && p < clsset.end(); ++p )
        o = p->Serialize( o );
      return o;
    }
};

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

template <class theclass, class steminfo, class resolver>
class buildmorph
{
  typedef wordtree<std::vector<steminfo>, unsigned char>  StemTree;   // the actual tree
  typedef wordtree<unsigned, unsigned short>              LidsTree;

protected:
  std::string           nspace;
  std::string           unknwn;
  std::string           outdir;

protected:
  StemTree              stemtree;   // the actual tree
  LidsTree              lidstree;
  classtable<theclass>  classset;
  resolver&             clparser;
  const char*           sLicense;
  FILE*                 unknowns;

public:     // construction
  buildmorph( resolver& r, const char* l, unsigned c ): clparser( r ), sLicense( l ), unknowns( nullptr )
    {
    }
 ~buildmorph()
    {
      if ( unknowns != nullptr )
        fclose( unknowns );
    }

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

template <class theclass, class steminfo, class resolver>
void  buildmorph<theclass, steminfo, resolver>::CreateDict( const std::vector<const char*>& dicset )
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

      for ( auto next = astems.begin(); next != astems.end(); offset += ::GetBufLen( *next ), ++next )
        *lidstree.Insert( lidstr, lexkeybuf( lidstr, next->nlexid ) ) = (unsigned)offset;
    } );

  if ( (length = lidstree.GetBufLen()) != (size_t)-1 )
    libmorph::LogMessage( 0, "info: lids dictionary size is %d bytes.\n", length );
  else throw std::runtime_error( "fault to calculate the main dictionary size!" );

  if ( (length = classset.GetBufLen()) != (size_t)-1 )
    libmorph::LogMessage( 0, "info: type dictionary size is %d bytes.\n", length );
  else throw std::runtime_error( "fault to calculate the type dictionary size!" );

// dump word tree
  libmorph::BinaryDumper().
    Namespace( nspace ).OutDir( outdir ).Header( sLicense ).Dump( "stemtree", stemtree );

  libmorph::BinaryDumper().
    Namespace( nspace ).OutDir( outdir ).Header( sLicense ).Dump( "lidstree", lidstree );

  libmorph::BinaryDumper().
    Namespace( nspace ).OutDir( outdir ).Header( sLicense ).Dump( "classmap", classset );
}

template <class theclass, class steminfo, class resolver>
void  buildmorph<theclass, steminfo, resolver>::DictReader( FILE* source )
{
  char    szline[0x400];
  int     nwords;

  for ( nwords = 0; fgets( szline, sizeof(szline) - 1, source ) != nullptr; ++nwords )
  {
    char*                 strtop;
    char*                 strend;
    char*                 pszlid;
    steminfo              lexeme;
    std::vector<theclass> aclass;

  // change codepage
    codepages::mbcstombcs( codepages::codepage_1251, szline, sizeof(szline), codepages::codepage_866, szline );

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
    if ( (pszlid = strstr( strtop, " LID:" )) == NULL ) continue;
      lexeme.nlexid = strtoul( pszlid + 5, &strend, 0 );
    if ( lexeme.nlexid == 0 )
      continue;

  // resolve word properties
    if ( (aclass = clparser.BuildStems( strtop )).size() == 0 )
    {
      PutUnknown( strtop );
      continue;
    }

    for ( auto& lex: aclass )
    {
      strcpy( lexeme.szpost, lex.stpost.c_str() );
        lexeme.chrmin = lex.chrmin;
        lexeme.chrmax = lex.chrmax;

      lexeme.oclass = classset.AddClass( lex );

      auto  pstems = stemtree.Insert( lex.ststem.c_str(), lex.ststem.length() );
      auto  inspos = pstems->begin();

      while ( inspos != pstems->end() && *inspos < lexeme )
        ++inspos;
      pstems->insert( inspos, lexeme );
    }
  }
}

template <class theclass, class steminfo, class resolver>
void  buildmorph<theclass, steminfo, resolver>::PutUnknown( const char* unknown )
{
  if ( unknowns == nullptr )
  {
    if ( unknwn == "" )
      return;
    if ( (unknowns = fopen( unknwn.c_str(), "wt" )) == nullptr )
      throw std::runtime_error( "could not create file '" + unknwn + "'" );
  }
  fprintf( unknowns, "%s\n", unknown );
}

# endif  // __buildmorph_h__
