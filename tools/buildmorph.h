# if !defined( __buildmorph_h__)
# define __buildmorph_h__
# include "plaintree.h"
# include "conffile.h"
# include "sweets.h"

template <class theclass>
class classset
{
  struct  classofs: public theclass
  {
    unsigned  offset;

    classofs( const theclass& r, unsigned o ): theclass( r ), offset( o )
      {                                         }
    bool  operator == ( const theclass& r ) const
      {   return theclass::operator == ( r );   }
  };
  array<classofs, classofs&>  clsset;
  unsigned                    length;

public:     // construction
  classset(): length( 0 )
    {
    }
  unsigned  AddClass( const theclass& rclass )
    {
      classofs* p;

      for ( p = clsset.begin(); p < clsset.end(); ++p )
        if ( *p == rclass ) return p->offset;

      if ( clsset.Append( (classofs&)classofs( rclass, length ) ) != 0 )
        return (unsigned)-1;

      length += rclass.GetBufLen();
        return clsset[clsset.GetLen() - 1].offset;
    }
  unsigned  GetBufLen() const
    {
      return length;
    }
  template <class O>
  O*  Serialize( O* o )
    {
      classofs* p;

      for ( p = clsset.begin(); o != NULL && p < clsset.end(); ++p )
        o = p->Serialize( o );
      return o;
    }
};

inline  int   lexkeybuf( char* lexbuf, unsigned nlexid )
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
  struct  stemlist: public array<steminfo, steminfo&>
  {
    stemlist( int ndelta = 0x4 ): array<steminfo, steminfo&>( ndelta )
      {
      }
    unsigned  GetBufLen()
      {
        const steminfo* p;
        unsigned        l;

        std::sort( begin(), end(),
          []( const steminfo& r1, const steminfo& r2 ) { return r1 < r2; } );

        for ( l = ::GetBufLen( this->GetLen() ), p = begin(); p < end(); ++p )
          l += p->GetBufLen();
        return l;
      }
    template <class O>
    O*        Serialize( O* o ) const
      {
        const steminfo* p;

        for ( o = ::Serialize( o, size() ), p = begin(); o != NULL && p < end(); ++p )
          o = p->Serialize( o );
        return o;
      }
    template <class A>
    unsigned  Enumerate( A a, unsigned o ) const
      {
        const steminfo* p;

        for ( o += ::GetBufLen( size() ), p = begin(); p < end(); ++p )
          if ( a( *p, o ) ) o += p->GetBufLen();
            else return (unsigned)-1;
        return o;
      }
  };

  struct  dictoffs
  {
    unsigned  offset;

  public:     // serialization
    unsigned  GetBufLen() const
      {  return ::GetBufLen( offset );  }
    template <class O>
    O*        Serialize( O* o ) const
      {  return ::Serialize( o, offset );  }
  };

  typedef wordtree<stemlist, unsigned char>  StemTree;   // the actual tree
  typedef wordtree<dictoffs, unsigned short> LidsTree;

  struct  enumdict
  {
    LidsTree&  reflid;

  public:     // construction
    enumdict( LidsTree& refdic ): reflid( refdic )
      {
      }
    bool  operator () ( const steminfo& s, unsigned o )
      {
        char      lidstr[0x20];
        dictoffs* ptrpos;

        if ( (ptrpos = reflid.InsertStr( lidstr, lexkeybuf( lidstr, s.nlexid ) )) == NULL )
          return false;
        ptrpos->offset = o;
          return true;
      }
  };

protected:
  StemTree            stemtree;   // the actual tree
  LidsTree            lidstree;
  classset<theclass>  classset;

  resolver&           clparser;
  libmorph::file      unknowns;
  libmorph::conf      settings;

public:     // construction
            buildmorph( resolver& r ): clparser( r )
              {
              }

public:     // initialization
  int       Initialize( const char*     lpfile );
  int       CreateDict(                        );

protected:  // helpers
  int       DictReader( FILE*       lpfile );
  int       PutUnknown( const char* thestr );

};

class serialbuff
{
  const void* buffer;
  unsigned    length;

public:
  serialbuff( const void* p, unsigned l ): buffer( p ), length( l )
    {}
  template <class O> O*  Serialize( O* o ) const
    {  return ::Serialize( o, buffer, length );  }
};

template <class serializable>
int       DumpBinary( const char* pszdir, const char*   nspace,
                      const char* vaname, serializable& serial )
{
  char            szname[0x400];
  const char*     sslash = "";
  libmorph::file  lpfile;

  if ( pszdir == NULL )
    pszdir = "";
  if ( pszdir != "" && strchr( "/\\", pszdir[strlen( pszdir ) - 1] ) == NULL )
    sslash = "/";
  sprintf( szname, "%s%s%s.cpp", pszdir, sslash, vaname );

  if ( (lpfile = fopen( szname, "wt" )) != NULL )
  {
    libmorph::dumpsource  dumpsc( lpfile, nspace, vaname );
      serial.Serialize( &dumpsc );
    return 0;
  }
  return libmorph::LogMessage( ENOENT, "Could not create file \'%s\'!\n", szname );
}

// buildmorph implementation

template <class theclass, class steminfo, class resolver>
int   buildmorph<theclass, steminfo, resolver>::Initialize( const char* pszcfg )
{
  libmorph::file  lpfile;

  if ( (lpfile = fopen( pszcfg, "rt" )) == NULL )
    return libmorph::LogMessage( ENOENT, "Could not open file \'%s\'!\n", pszcfg );

  return settings.Initialize( (FILE*)lpfile );
}

template <class theclass, class steminfo, class resolver>
int   buildmorph<theclass, steminfo, resolver>::CreateDict()
{
  enumdict                        dienum( lidstree );
  libmorph::file                  lpfile;
  const char*                     dicset;
  int                             nerror;
  unsigned                        length;

// load all the dictionaries
  if ( (dicset = settings.FindString( "dictionary" )) == NULL )
    return libmorph::LogMessage( EINVAL, "No \'dictionary' variable found in the configuration!\n" );

  for ( ; *dicset != '\0'; dicset += strlen( dicset ) + 1 )
  {
    if ( (lpfile = fopen( dicset, "rt" )) == NULL )
      return libmorph::LogMessage( ENOENT, "Could not open file \'%s\'!\n", dicset );

    libmorph::LogMessage( 0, "Loading %s...", dicset );
      if ( (nerror = DictReader( lpfile )) != 0 )
        return libmorph::LogMessage( nerror, " fault :(\n" );
    libmorph::LogMessage( 0, " OK\n" );
  }

// dump word tree
  if ( (length = stemtree.GetBufLen()) != (unsigned)-1 ) libmorph::LogMessage( 0, "info: main dictionary size is %d bytes.\n", length );
    else return libmorph::LogMessage( ENOMEM, "fault to calculate the main dictionary size!\n" );

  // create stem mapping
    if ( stemtree.Enumerate( dienum ) == (unsigned)-1 )
      return ENOMEM;

  if ( (length = lidstree.GetBufLen()) != (unsigned)-1 ) libmorph::LogMessage( 0, "info: lids dictionary size is %d bytes.\n", length );
    else return libmorph::LogMessage( ENOMEM, "fault to calculate the main dictionary size!\n" );

  if ( (length = classset.GetBufLen()) != (unsigned)-1 ) libmorph::LogMessage( 0, "info: type dictionary size is %d bytes.\n", length );
    else return libmorph::LogMessage( ENOMEM, "fault to calculate the type dictionary size!\n" );

// dump word tree
  if ( (nerror = DumpBinary( settings.FindString( "TargetDir" ), settings.FindString( "namespace" ), "stemtree", stemtree )) != 0 )
    return nerror;

  if ( (nerror = DumpBinary( settings.FindString( "TargetDir" ), settings.FindString( "namespace" ), "lidstree", lidstree )) != 0 )
    return nerror;

  if ( (nerror = DumpBinary( settings.FindString( "TargetDir" ), settings.FindString( "namespace" ), "classmap", classset )) != 0 )
    return nerror;

  return 0;
}

template <class theclass, class steminfo, class resolver>
int   buildmorph<theclass, steminfo, resolver>::DictReader( FILE* lpfile )
{
  char  szline[0x400];
  int   nwords;
  int   nerror;

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
    mbcstombcs( codepage_1251, szline, sizeof(szline), codepage_866, szline );

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
    const char* szname;

    if ( (szname = settings.FindString( "faultList" )) != NULL )
      if ( (unknowns = fopen( szname, "wt" )) == NULL )
        return libmorph::LogMessage( ENOENT, "Could not create the unknown words file \'%s\'!\n", szname );
  }
  if ( unknowns != NULL )
    fprintf( unknowns, "%s\n", pszstr );
  return 0;
}

# endif  // __buildmorph_h__
