/******************************************************************************

    libmorphrus - dictiorary-based morphological analyser for Russian.
    Copyright (C) 1994-2016 Andrew Kovalenko aka Keva

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

    Contacts:
      email: keva@meta.ua, keva@rambler.ru
      Skype: big_keva
      Phone: +7(495)648-4058, +7(926)513-2991

******************************************************************************/
# include "../../include/mlma1049.h"
# include <stdarg.h>
# include <string.h>

const char* TestLemmatize( const char*  szform, unsigned  dwsets, int nitems, ... )
{
  SLemmInfoA  lemmas[0x20];
  char        normal[0x100];
  SGramInfo   agrams[0x40];
  IMlmaMb*    pmorph;
  int         nlemma;
  int         nindex;
  va_list     valist;

  mlmaruLoadMbAPI( &pmorph );

  if ( (nlemma = pmorph->vtbl->Lemmatize( pmorph, szform, (size_t)-1, lemmas, 0x20, normal, 0x100, agrams, 0x40, dwsets )) != nitems )
    return "Lemmatization: result lexeme count mismatch!";

  va_start( valist, nitems );

  for ( nindex = 0; nindex < nlemma; ++nindex )
  {
    lexeme_t  nlexid = va_arg( valist, lexeme_t );
    char*     normal = va_arg( valist, char* );

    if ( nlexid != lemmas[nindex].nlexid )
      return "Lemmatization: invalid lexemes order or lexeme mismatch!";
    if ( strcmp( normal, lemmas[nindex].plemma ) != 0 )
      return "Lemmatization: invalid normal form!";
  }

  va_end( valist );

  return NULL;
}
