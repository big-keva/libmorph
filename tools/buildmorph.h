# if !defined( __buildmorph_h__)
# define __buildmorph_h__
# include "plaintree.h"
# include "dumppage.h"
# include "sweets.h"
# include <libcodes/codes.h>
# include <mtc/jsconfig.h>
# include <algorithm>

template <class theclass>
class classset
{
  struct  classofs: public theclass
  {
    unsigned  offset;

    classofs( const theclass& r, unsigned o ): theclass( r ), offset( o ) {}
    classofs( const classofs& c ): theclass( c ), offset( c.offset ) {}
    bool  operator == ( const theclass& r ) const {  return theclass::operator == ( r );  }
  };

  array<classofs>   clsset;
  unsigned          length;

public:     // construction
  classset(): length( 0 ) {}
  classset( const classset& ) = delete;

public:     // API
  unsigned  AddClass( const theclass& rclass )
    {
      classofs* p;

      for ( p = clsset.begin(); p < clsset.end(); ++p )
        if ( *p == rclass ) return p->offset;

      if ( clsset.Append( classofs( rclass, length ) ) != 0 )
        return (unsigned)-1;

      length += rclass.GetBufLen();
        return clsset[clsset.GetLen() - 1].offset;
    }

public:     // serialization
  size_t  GetBufLen() const
    {
      return length;
    }
  template <class O>
  O*      Serialize( O* o )
    {
      classofs* p;

      for ( p = clsset.begin(); o != NULL && p < clsset.end(); ++p )
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
  struct  stemlist: public array<steminfo>
  {
    stemlist( int ndelta = 0x4 ): array<steminfo>( ndelta ) {}
    stemlist( const stemlist& ) = delete;
    stemlist& operator = ( const stemlist& ) = delete;

  public:     // serialization
    size_t  GetBufLen()
      {
        const steminfo* p;
        size_t          l;

        std::sort( begin(), end(),
          []( const steminfo& r1, const steminfo& r2 ) { return r1 < r2; } );

        for ( l = ::GetBufLen( this->GetLen() ), p = begin(); p < end(); ++p )
          l += p->GetBufLen();

        return l;
      }
    template <class O>
    O*      Serialize( O* o ) const
      {
        const steminfo* p;

        for ( o = ::Serialize( o, size() ), p = begin(); o != NULL && p < end(); ++p )
          o = p->Serialize( o );
        return o;
      }
    template <class A>
    size_t  Enumerate( A a, size_t o ) const
      {
        const steminfo* p;

        for ( o += ::GetBufLen( size() ), p = begin(); p < end(); ++p )
          if ( a( *p, o ) ) o += p->GetBufLen();
            else return (size_t)-1;
        return o;
      }
  };

  struct  dictoffs
  {
    unsigned  offset;

  public:     // serialization
                        size_t  GetBufLen() const         {  return ::GetBufLen( offset );  }
    template <class O>  O*      Serialize( O* o ) const   {  return ::Serialize( o, offset );  }
  };

  typedef wordtree<stemlist, unsigned char>  StemTree;   // the actual tree
  typedef wordtree<dictoffs, unsigned short> LidsTree;

protected:
  StemTree            stemtree;   // the actual tree
  LidsTree            lidstree;
  classset<theclass>  classset;

  resolver&           clparser;
  file                unknowns;
  configuration       settings;
  unsigned            sourceCP;
  const char*         sLicense;

public:     // construction
            buildmorph( resolver& r, const char* l, unsigned c ): clparser( r ),
                                                                  sLicense( l ),
                                                                  sourceCP( c )
              {
              }

public:     // initialization
  int       Initialize( const char*     lpfile );
  int       CreateDict(                        );

protected:  // helpers
  int       DictReader( FILE*       lpfile );
  int       PutUnknown( const char* thestr );

};

// buildmorph implementation

template <class theclass, class steminfo, class resolver>
int   buildmorph<theclass, steminfo, resolver>::Initialize( const char* pszcfg )
{
  return (settings = OpenConfig( pszcfg )).haskeys() ? 0 : EINVAL;
}

template <class theclass, class steminfo, class resolver>
int   buildmorph<theclass, steminfo, resolver>::CreateDict()
{
  file                  lpfile;
  array<_auto_<char>>*  dicset;
  int                   nerror;
  size_t                length;
  auto                  maplid = [&]( const steminfo& s, size_t o )
    {
      char      lidstr[0x20];
      dictoffs* ptrpos;

      if ( (ptrpos = lidstree.InsertStr( lidstr, lexkeybuf( lidstr, s.nlexid ) )) == nullptr )
        return false;
      ptrpos->offset = (unsigned)o;
        return true;
    };

// load all the dictionaries
  if ( (dicset = settings.get_array_charstr( "dictionaries" )) == nullptr )
    return libmorph::LogMessage( EINVAL, "No \'dictionaries' variable found in the configuration!\n" );

  for ( auto p = dicset->begin(); p < dicset->end(); ++p )
  {
    if ( (lpfile = fopen( *p, "rt" )) == NULL )
      return libmorph::LogMessage( ENOENT, "Could not open file \'%s\'!\n", (const char*)*p );

    libmorph::LogMessage( 0, "Loading %s...", (const char*)*p );
      if ( (nerror = DictReader( lpfile )) != 0 )
        return libmorph::LogMessage( nerror, " fault :(\n" );
    libmorph::LogMessage( 0, " OK\n" );
  }

// dump word tree
  if ( (length = stemtree.GetBufLen()) != (size_t)-1 ) libmorph::LogMessage( 0, "info: main dictionary size is %d bytes.\n", length );
    else return libmorph::LogMessage( ENOMEM, "fault to calculate the main dictionary size!\n" );

  // create stem mapping
  if ( stemtree.Enumerate( maplid ) == (size_t)-1 )
    return ENOMEM;

  if ( (length = lidstree.GetBufLen()) != (size_t)-1 ) libmorph::LogMessage( 0, "info: lids dictionary size is %d bytes.\n", length );
    else return libmorph::LogMessage( ENOMEM, "fault to calculate the main dictionary size!\n" );

  if ( (length = classset.GetBufLen()) != (size_t)-1 ) libmorph::LogMessage( 0, "info: type dictionary size is %d bytes.\n", length );
    else return libmorph::LogMessage( ENOMEM, "fault to calculate the type dictionary size!\n" );

// dump word tree
  BinaryDumper().Namespace( settings.get_charstr( "namespace", "__libmorphrus__" ) ).
                    OutDir( settings.get_charstr( "TargetDir", "./" ) ).
                    Header( sLicense ).Dump( "stemtree", stemtree );

  BinaryDumper().Namespace( settings.get_charstr( "namespace", "__libmorphrus__" ) ).
                    OutDir( settings.get_charstr( "TargetDir", "./" ) ).
                    Header( sLicense ).Dump( "lidstree", lidstree );

  BinaryDumper().Namespace( settings.get_charstr( "namespace", "__libmorphrus__" ) ).
                    OutDir( settings.get_charstr( "TargetDir", "./" ) ).
                    Header( sLicense ).Dump( "classmap", classset );

  return 0;
}

template <class theclass, class steminfo, class resolver>
int   buildmorph<theclass, steminfo, resolver>::DictReader( FILE* lpfile )
{
  char    szline[0x400];
  int     nwords;
  int     nerror;

  for ( nwords = 0; fgets( szline, sizeof(szline) - 1, lpfile ) != NULL; ++nwords )
  {
    char*         strtop;
    char*         strend;
    char*         pszlid;
    steminfo      lexeme;
    char          astems[0x100];
    theclass      aclass[0x10];
    int           nstems;
    int           nindex;

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
      if ( (nerror = PutUnknown( strtop )) != 0 ) return nerror;
        else  continue;
    }

  // resolve lexeme identifier
    if ( (pszlid = strstr( strtop, " LID:" )) == NULL ) continue;
      lexeme.nlexid = strtoul( pszlid + 5, &strend, 0 );
    if ( lexeme.nlexid == 0 )
      continue;

  // resolve word properties
    if ( (nstems = clparser.BuildStems( astems, aclass, strtop )) < 0 )
      return libmorph::LogMessage( nstems, "BuildStems fault!\n" );
    if ( nstems == 0 && (nerror = PutUnknown( strtop )) != 0 )
      return nerror;

    for ( nindex = 0, strtop = astems; nindex < nstems; ++nindex, strtop += strlen( strtop ) + 1 )
    {
      stemlist* pstems;

      strcpy( lexeme.szpost, aclass[nindex].szpost );
        lexeme.chrmin = aclass[nindex].chrmin;
        lexeme.chrmax = aclass[nindex].chrmax;

      if ( (lexeme.oclass = classset.AddClass( aclass[nindex] )) == (unsigned)-1 )
        return ENOMEM;
      if ( (pstems = stemtree.InsertStr( strtop, strlen( strtop ) )) == NULL )
        return ENOMEM;
      if ( pstems->Append( lexeme ) != 0 )
        return ENOMEM;
    }
  }
  return 0;
}

template <class theclass, class steminfo, class resolver>
int   buildmorph<theclass, steminfo, resolver>::PutUnknown( const char* pszstr )
{
  if ( unknowns == NULL )
  {
    if ( (unknowns = fopen( settings.get_charstr( "faultList", "unknown.txt" ), "wt" )) == nullptr )
      return libmorph::LogMessage( ENOENT, "Could not create the unknown words file \'%s\'!\n",
        settings.get_charstr( "faultList", "unknown.txt" ) );
  }
  if ( unknowns != NULL )
    fprintf( unknowns, "%s\n", pszstr );
  return 0;
}

# endif  // __buildmorph_h__
