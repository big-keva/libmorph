/*! \file
//[]================================================================== <br>
//[] Project: English Morphological Library Builder                    <br>
//[] File name: StringBox.h                                            <br>
//[] Abstract:                                                         <br>
//[]                                                                   <br>
//[]                                                                   <br>
//[] Author: K.Ignatchenko aka Kostyan                                 <br>
//[] Created: 29.04.2002                                               <br>
//[] Modifier:                                                         <br>
//[] Last modified:                                                    <br>
//[] Modifier notes:                                                   <br>
//[]==================================================================*/ 
# include "StringBox.h"
# include "../../include/array.h"

////////////////////////////////////////////////////////////////
// Add string to the StringBox
unsigned short  CStringBox::AddString( const char* string, unsigned cchstr )
{
  array<char, char> substr;
  unsigned short    offset = (unsigned short)length;

  if ( cchstr != (unsigned)-1 )
  {
    if ( substr.SetLen( cchstr + 1 ) != 0 )
      return STRING_NOT_FOUND;
    memcpy( (char*)substr, string, cchstr );
    string = (char*)substr;
  }
    else
  cchstr = strlen( string );

// register
  if ( Insert( string, offset ) != 0 )
    return STRING_NOT_FOUND;
  length += cchstr + 1;
  return offset;
}

////////////////////////////////////////////////////////////////
// Find string in the StringBox (do not add it)
unsigned short  CStringBox::GetOffset( const char* string, unsigned cchstr ) const
{
  array<char, char> substr;
  unsigned short*   pwoffs;

  if ( cchstr != (unsigned)-1 )
  {
    if ( substr.SetLen( cchstr + 1 ) != 0 )
      return STRING_NOT_FOUND;
    memcpy( (char*)substr, string, cchstr );
    string = (char*)substr;
  }
  return (pwoffs = Search( string )) != NULL ? *pwoffs : STRING_NOT_FOUND;
}

const char*     CStringBox::GetString( unsigned short offset )
{
  void* lpenum = NULL;

  while ( (lpenum = Enum( lpenum )) != NULL )
    if ( GetVal( lpenum ) == offset )
      return GetKey( lpenum );
  return NULL;
}

unsigned  CStringBox::GetLength() const
{
  return length;
}

char*     CStringBox::Serialize( char* lpbuff )
{
  array<void*, void*> ptrmap;
  void*               lpenum;
  int                 nindex;

  if ( ptrmap.SetLen( GetLen() ) != 0 )
    return NULL;
  for ( lpenum = NULL, nindex = 0; (lpenum = Enum( lpenum )) != NULL; nindex++ )
    ptrmap[nindex] = lpenum;
  ptrmap.Resort( cmp );

  for ( nindex = 0; nindex < ptrmap.GetLen(); nindex++ )
  {
    const char* string = GetKey( ptrmap[nindex] );
    int         cchstr = strlen( string );

    *lpbuff++ = (char)cchstr;
    while ( cchstr-- > 0 )
      *lpbuff++ = *string++;
  }
  return lpbuff;
}

int       CStringBox::cmp( void*& p1, void*& p2 )
{
  unsigned short k1 = GetVal( p1 );
  unsigned short k2 = GetVal( p2 );

  return (k1 > k2) - (k1 < k2);
}
