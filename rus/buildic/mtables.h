#if !defined( __mtables_h__ )
#define __mtables_h__
# include <mtc/stringmap.h>
# include <mtc/array.h>

using namespace mtc;

struct  mixclass
{
  word16_t  offset;
  char      szcond[0x10];
};

// ѕредставление ссылок на некоторую таблицу чередований, то есть элементов CMixItem
class mixtable: public array<mixclass>
{
  const char* tables;

public:     // construction
            mixtable(): array<mixclass>( 0x4 ), tables( nullptr ) {}

public:     // API
  unsigned  GetMix( unsigned short  type,
                    const char*     stem,
                    const char*     rems );
  template <class S>
  S*        DoLoad( const char* ptable, S* s );

};

class mixfiles: public array<mixtable>
{
  stringmap<int>  tableref;

public:     // cnostruction
            mixfiles(): array<mixtable>( 0x80 ) {}

public:     // API
  unsigned  GetMix( unsigned short type,
                    const char*    stem,
                    const char*    ztyp,
                    const char*    rems );
  template <class S>
  S*        DoLoad( const char* mtable, S* s );
};

// helpers

inline  const char* GetDefText( const char* tables, unsigned tbOffs )
{
  const char* mixtab = tables + tbOffs;
  int         tablen = *mixtab++;

  while ( tablen-- > 0 )
  {
    if ( (*mixtab & 0x10) != 0 )  return mixtab;
      else  mixtab += 1 + (*mixtab & 0x0f);
  }
  assert( "Invalid interchange table: no default string!" == NULL );
  return nullptr;
}

inline  unsigned char GetMinLetter( const char* tables, unsigned tbOffs, unsigned char chrmin )
{
  const char*   mixtab = tables + tbOffs;
  int           tablen = *mixtab++;
  unsigned char mixmin = 0xff;

  while ( tablen-- > 0 )
  {
    unsigned char mflags = (unsigned char)*mixtab++;

    if ( (mflags & 0x0f) > 0 )  mixmin = mixmin <= (unsigned char)*mixtab ? mixmin : (unsigned char)*mixtab;
      else mixmin = '\0';
    mixtab += (mflags & 0x0f);
  }
  return mixmin != '\0' ? mixmin : chrmin;
}

inline  unsigned char GetMaxLetter( const char* tables, unsigned tbOffs, unsigned char chrmax )
{
  const char*   mixtab = tables + tbOffs;
  int           tablen = *mixtab++;
  int           mixmax = -1;

  while ( tablen-- > 0 )
  {
    unsigned char mflags = (unsigned char)*mixtab++;

    if ( (mflags & 0x0f) > 0 )  mixmax = mixmax >= (unsigned char)*mixtab ? mixmax : (unsigned char)*mixtab;
      else  mixmax = '\0';
    mixtab += (mflags & 0x0f);
  }
  return mixmax <= 0 ? chrmax : mixmax;
}

// mixtable inline implementation

template <class S>
S*  mixtable::DoLoad( const char*  mxtabs, S* s )
{
  mixclass* getone;
  int       toload;

  if ( (s = ::FetchFrom( s, toload )) == nullptr || SetLen( toload ) != 0 )
    return nullptr;

  for ( tables = mxtabs, getone = begin(); s != nullptr && getone < end(); ++getone )
  {
    unsigned  cchstr;

    if ( (s = ::FetchFrom( ::FetchFrom( s, getone->offset ), cchstr )) == nullptr )
      return nullptr;
    if ( cchstr >= sizeof(getone->szcond) )
      return nullptr;
    if ( (s = ::FetchFrom( s, getone->szcond, cchstr )) == nullptr )
      return nullptr;
    getone->szcond[cchstr] = '\0';
  }
  return s;
}

// mixfiles inline implementation

template <class S>
S*  mixfiles::DoLoad( const char* mtable, S* s )
{
  mixtable* mixtab;
  int       tcount;

  if ( (s = ::FetchFrom( s, tcount )) == NULL || SetLen( tcount ) != 0 )
    return NULL;

  for ( mixtab = begin(); s != NULL && mixtab < end(); ++mixtab )
    s = mixtab->DoLoad( mtable, s );

  if ( (s = ::FetchFrom( s, tcount )) == NULL )
    return NULL;

  while ( s != NULL && tcount-- > 0 )
  {
    char  szname[0x100];
    int   tabpos;
    int   ccname;

    if ( (s = ::FetchFrom( ::FetchFrom( s, tabpos ), ccname )) == NULL )
      return NULL;
    if ( ccname < sizeof(szname) )  szname[ccname] = '\0';
      else  return NULL;
    if ( (s = ::FetchFrom( s, szname, ccname )) == NULL )
      return NULL;
    if ( tableref.Insert( szname, tabpos ) == nullptr )
      return NULL;
  }
  return s;
}

#endif // __mtables_h__
