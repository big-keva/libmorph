/******************************************************************************

    libmorphrus - dictiorary-based morphological analyser for Russian.

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
