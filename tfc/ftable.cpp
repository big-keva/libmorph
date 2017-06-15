# include "ftable.h"
# include "../tools/sweets.h"
# include <stdio.h>
# include <assert.h>
# include <errno.h>

# if defined( _MSC_VER )
#   pragma warning( disable: 4291 )
# endif  // _MSC_VER

// ftable implementation

bool      ftable::Insert( const fxitem& f )
{
  int   search;

  if ( !Search( [&]( const fxitem& r ){  return fxitem::compare( f, r );  }, search ) )
    return array<fxitem>::Insert( search, f ) == 0;
  fprintf( stderr, "Element already exists!\n" );
    return true;
}

int       ftable::Compare( const ftable& reftab ) const
{
  int   rescmp;
  int   findex;
  
  if ( (rescmp = ncount - reftab.ncount) != 0 )
    return rescmp;
    
  for ( findex = 0; findex < ncount; findex++ )
    if ( (rescmp = fxitem::compare( pitems[findex], reftab.pitems[findex] )) != 0 )
      return rescmp;

  return 0;
}

int       ftable::RelocateReferences( fxlist& rflist )
{
  fxitem* lpflex;

  for ( lpflex = begin(); lpflex < end(); ++lpflex )
    if ( (lpflex->bflags & 0xc0) != 0 )
    {
      ftable**  ptable;

    // Relocate the element
      if ( (ptable = rflist.tabmap.Search( lpflex->sznext )) == NULL )
        return libmorph::LogMessage( EINVAL, "Could not resolve internal reference \"%s\"!", lpflex->sznext );

      lpflex->ofnext = (*ptable)->offset;
    }
  return 0;
}

unsigned  ftable::RelocateOffsetSize( unsigned dwoffs )
{
  const fxitem* lpflex;

  assert( dwoffs < 0x1ffff && (dwoffs & 0x01) == 0 );

  for ( offset = (dwoffs++ >> 1), lpflex = begin(); lpflex < end(); ++lpflex )
    dwoffs += lpflex->GetBufLen();

  return (dwoffs + 1) & ~0x01;
}

// fxlist implementation

fxlist::~fxlist()
{
  ftable**  ptrtop;

  for ( ptrtop = tables.begin(); ptrtop < tables.end(); ++ptrtop )
    delete *ptrtop;
}

int   fxlist::Insert( ftable*     ptable,
                      const char* pnames )
{
  int     search;
  int     nerror;

// First check if the table already exists
  if ( tables.Search( [&]( const ftable* f1 ){  return ptable->Compare( *f1 );  }, search ) )
  {
    delete ptable;
    ptable = tables[search];
  }
    else
  if ( (nerror = tables.Insert( search, ptable )) != 0 )
  {
    delete ptable;
    return nerror;
  }

  while ( *pnames != '\0' )
  {
    char  szname[0x100];
    char* pszout = szname;

    while ( *pnames != '\0' && (unsigned char)*pnames <= 0x20 )
      ++pnames;
    while ( *pnames != '\0' && *pnames != ',' )
      *pszout++ = *pnames++;
    if ( pszout > szname )
    {
      *pszout = '\0';

      if ( tabmap.Insert( szname, ptable ) == nullptr )
        return ENOMEM;
    }
    if ( *pnames == ',' )
      ++pnames;
  }
  return 0;
}

int   fxlist::Relocate()
{
  ftable**  ptable;
  unsigned  offset;
  int       nerror;

// Relocate the tables
  for ( offset = 6, ptable = tables.begin(); ptable < tables.end(); ++ptable )
    offset = (*ptable)->RelocateOffsetSize( offset );

// Relocate the references
  for ( ptable = tables.begin(); ptable < tables.end(); ++ptable )
    if ( (nerror = (*ptable)->RelocateReferences( *this )) != 0 )
      return nerror;
    
  return 0;
}
