/******************************************************************************

    libmorph - morphological analysers.

    Copyright (C) 1994-2025 Andrew Kovalenko aka Keva

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

    Commercial license is available upon request.

    Contacts:
      email: keva@meta.ua, keva@rambler.ru
      Phone: +7(495)648-4058, +7(926)513-2991, +7(707)129-1418

******************************************************************************/
# include "ftable.h"
# include "../tools/sweets.h"
# include <stdio.h>
# include <assert.h>
# include <errno.h>

// ftable implementation

void  ftable::RelocateReferences( fxlist& rflist )
{
  for ( auto flex = flexet.begin(); flex != flexet.end(); ++flex )
    if ( (flex->bflags & 0xc0) != 0 )
    {
      auto  pfound = rflist.tabmap.find( flex->sznext );

      if ( pfound == rflist.tabmap.end() )
        throw std::runtime_error( "Could not resolve internal reference '" + flex->sznext + "'" );

      flex->ofnext = rflist.tables[pfound->second].offset;
    }
}

size_t  ftable::RelocateOffsetSize( size_t dwoffs )
{
  if ( dwoffs >= 0x1ffff || (dwoffs & 0x01) != 0 )
    throw std::runtime_error( "invalid (not aligned) offset passed to relocation" );

  offset = (uint16_t)(dwoffs++ >> 1);

  for ( auto flex = flexet.begin(); flex != flexet.end(); ++flex )
    dwoffs += flex->GetBufLen();

  return (dwoffs + 1) & ~0x01;
}

// fxlist implementation

//
// проверить, существует ли такая таблица; если существует, использует индекс существующей,
// иначе добавляет новую и использует её индекс
void  fxlist::Insert( ftable&& ft, const char* ix )
{
  auto    l_trim = []( const char* s ){  while ( *s != '\0' && (unsigned char)*s <= 0x20 ) ++s;  return s;  };
  auto    ptable = std::find_if( tables.begin(), tables.end(), [&]( const ftable& tf ){  return tf == ft;  } );
  size_t  findex;

  if ( ptable == tables.end() )
    {  findex = tables.size();  tables.push_back( std::move( ft ) );  }
  else
    {  findex = ptable - tables.begin();  }

  while ( *(ix = l_trim( ix )) != '\0' )
  {
    const char* pszorg = ix;

    while ( *ix != '\0' && *ix != ',' )
      ++ix;
    while ( ix > pszorg && (unsigned char)ix[-1] <= 0x20 )
      --ix;

    if ( ix > pszorg )
      tabmap.emplace( std::string( pszorg, ix - pszorg ), findex );

    if ( *(ix = l_trim( ix )) == ',' )
      ++ix;
  }
}

void  fxlist::Relocate()
{
  size_t  offset = 6;

// Relocate the tables
  for ( auto tab = tables.begin(); tab != tables.end(); ++tab )
    offset = tab->RelocateOffsetSize( offset );

// Relocate the references
  for ( auto tab = tables.begin(); tab != tables.end(); ++tab )
    tab->RelocateReferences( *this );
}
