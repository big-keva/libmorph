# if !defined( __flexbox_h__ )
# define  __flexbox_h__
# include "../../include/array.h"
# include "../../include/stringmap.h"
# include "StringBox.h"

class CFlexBox
{
  struct  fxinfo
  {
    unsigned char   idform;
    unsigned short  stOffs;
  };
  struct  ftable: public array<fxinfo, fxinfo>
  {
    unsigned short  offset;
  };
  stringmap<ftable, ftable&>  tabmap;
  CStringBox                  strmap;
  unsigned short              length;
public:
                  CFlexBox(): length( 4 )
                    {
                    }
  unsigned short  AddTable( const char* lpflex );
  unsigned        GetStTabLen() const;
  char*           SerializeSt( char* buffer );
  unsigned        GetFxTabLen() const;
  char*           SerializeFx( char* buffer );
private:
  static  int     comparetabs( ftable*&, ftable*& );
};

// CFlexBox implementation

unsigned short  CFlexBox::AddTable( const char* lpflex )
{
  ftable*   ptable;
  int       idflex = 0;
  int       loitem;
  int       hiitem;

// check if present
  if ( (ptable = tabmap.Search( lpflex )) != NULL )
    return ptable->offset;

// register new table
  if ( (ptable = tabmap.Insert( lpflex )) == NULL )
    return (unsigned short)-1;

// skip first delimiter
  if ( *lpflex == '|' )
    ++lpflex;

// fill the table
  while ( *lpflex != '\0' )
  {
    const char* ptrtop = lpflex;
    const char* ptrend = lpflex;

  // select the flexion
    while ( *ptrend != '\0' && *ptrend != '|' )
      ++ptrend;

  // check if next
    lpflex = *ptrend == '|' ? ptrend + 1 : ptrend;

  // check if absent
    if ( ptrend - ptrtop == 1 && *ptrtop == '-' )
    {
      ++idflex;
      continue;
    }

  // register the flexions
    while ( ptrtop < ptrend )
    {
      fxinfo      ftinfo;
      const char* strend = ptrtop;
      int         reglen;

      while ( strend < ptrend && *strend != '/' )
        ++strend;
      if ( *ptrtop == '0' )
      {
        ptrtop = "";
        reglen = 0;
      }
        else
      reglen = strend - ptrtop;
      if ( (ftinfo.stOffs = strmap.AddString( ptrtop, reglen )) == STRING_NOT_FOUND )
        return (unsigned short)-1;

      if ( strend < ptrend && *strend == '/' )
        ++strend;
      ptrtop = strend;

      ftinfo.idform = idflex;
      if ( ptable->Insert( ptable->GetLen(), ftinfo ) != 0 )
        return (unsigned short)-1;
    }
    ++idflex;
  }

// resort the flexions
  loitem = 0;
  hiitem = ptable->GetLen() - 1;

  while ( hiitem > loitem )
  {
    int         maxone = loitem;
    const char* maxstr = strmap.GetString( (*ptable)[maxone].stOffs );
    int         idtest;

/* A[i] <= A[j] for i <= j, j > hi */
    for ( idtest = loitem + 1; idtest <= hiitem; idtest++ )
    {
      const char* cmpstr = strmap.GetString( (*ptable)[idtest].stOffs );
      if ( strcmp( cmpstr, maxstr ) > 0 )
      {
        maxone = idtest;
        maxstr = cmpstr;
      }
    }

  /* A[i] <= A[max] for lo <= i <= hi */
    fxinfo  toswap = (*ptable)[hiitem];
    (*ptable)[hiitem] = (*ptable)[maxone];
    (*ptable)[maxone] = toswap;

    --hiitem;
  }

// register the offset
  ptable->offset = length;
  length += ptable->GetLen() * 3 + 1;
  return ptable->offset;
}

unsigned        CFlexBox::GetStTabLen() const
{
  return strmap.GetLength();
}

char*           CFlexBox::SerializeSt( char* buffer )
{
  return strmap.Serialize( buffer );
}

unsigned        CFlexBox::GetFxTabLen() const
{
  return length;
}

char*           CFlexBox::SerializeFx( char* buffer )
{
  array<ftable*, ftable*> fxtabs( tabmap.GetLen() );
  void*                   lpenum = NULL;
  int                     nindex;

  while ( (lpenum = tabmap.Enum( lpenum )) != NULL )
    if ( fxtabs.Insert( fxtabs.GetLen(), &tabmap.GetVal( lpenum ) ) != 0 )
      return NULL;
  fxtabs.Resort( comparetabs );

  *buffer++ = 'F';
  *buffer++ = 'L';
  *buffer++ = 'E';
  *buffer++ = 'X';

  for ( nindex = 0; nindex < fxtabs.GetLen(); nindex++ )
  {
    ftable* ptable = fxtabs[nindex];
    int     findex;

    *buffer++ = (char)ptable->GetLen();
    for ( findex = 0; findex < ptable->GetLen(); findex++ )
    {
      fxinfo& rrinfo = (*ptable)[findex];

      *buffer++ = (char)rrinfo.idform;
      *buffer++ = (char)(rrinfo.stOffs);
      *buffer++ = (char)(rrinfo.stOffs >> 8);
    }
  }
  return buffer;
}

int             CFlexBox::comparetabs( ftable*& p1, ftable*& p2 )
{
  return p1->offset - p2->offset;
}

# endif  // __flexbox_h__
