/******************************************************************************

    libmorpheng - dictionary-based morphological analyser for English.

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
# if !defined( __libmorph_rus_scanClass_hpp__ )
# define __libmorph_rus_scanClass_hpp__
# include "xmorph/capsheme.h"

namespace libmorph {
namespace eng {

  struct ScanDict final
  {
    template <size_t N>
    static  int   ScanTables(
      SGramInfo     (&buffer)[N],
      const steminfo& lextem,
      const fragment& inflex )
    {
      auto  ptable = lextem.GetFlexTable();
      int   nitems = 0;               //

      for ( int ncount = *ptable++; ncount-- > 0; )
      {
        auto  idform = *ptable++;
        auto  szflex = ptable++;
        auto  ccflex = size_t(*szflex++);
        auto  cmplen = std::min( ccflex, inflex.len );
        int   rescmp = cmplen > 0 ? memcmp( szflex, inflex.str, cmplen ) : 0;

        // compare the string with element; if nothing more, return immidiately
        if ( cmplen > 0 && rescmp > 0 ) break;
          else ptable += ccflex;

        // check if equal
        if ( ccflex != inflex.len || rescmp < 0 )
          continue;

        // create grammatical description
        buffer[nitems++] = { uint16_t(lextem.wdinfo & 0x7f), idform, 0, 0 };
      }
      return nitems;
    }
  };

  template <class Collect>
  class BuildClass
  {
    const Collect&  target;

  public:
    BuildClass( const Collect& out ):
      target( out ) {}

  public:
    int   operator()(
      lexeme_t        nlexid,
      uint16_t        oclass,
      const fragment& inflex,
      const fragment& suffix ) const
    {
      return target( nlexid, steminfo( oclass ), { inflex.str + inflex.len, 0 },
        suffix, nullptr, 0 );
    }
  };

  template <class Collect>
  class MatchClass
  {
    const Collect&  target;
    uint16_t        scheme = 0x0100;
    unsigned        dwsets = 0;

  public:
    MatchClass( const Collect& out ):
      target( out ) {}

    auto  SetCapitalization( uint16_t scheme ) -> MatchClass&
      {  return this->scheme = scheme, *this;  }
    auto  SetSearchSettings( unsigned dwsets ) -> MatchClass&
      {  return this->dwsets = dwsets, *this;  }

  public:
    int   operator()(
      lexeme_t        nlexid,
      uint16_t        oclass,
      const fragment& inflex,
      const fragment& suffix ) const
    {
      auto      lextem = steminfo( oclass );
      SGramInfo grbuff[0x40];
      int       ngrams;

    // check output capitalization
      if ( (dwsets & sfIgnoreCapitals) == 0 && !IsGoodSheme( scheme, lextem.MinCapScheme() ) )
        return 0;

    // check if non-flective; only zero flexion match it
      if ( lextem.GetFlexTable() == nullptr )
      {
        if ( inflex.size() == 0 )
          return target( nlexid, steminfo{ lextem.wdinfo, 0 }, inflex, suffix, SGramInfo{ 0, 0, lextem.tfoffs, 0 } );
        else return 0;
      }

    // Теперь - обработка флективных слов. Сначала обработаем более частый случай - флективное слово
    // без чередований в основе.
    // Для этого достаточно проверить, не выставлен ли флаг наличия чередований, так как при попадании
    // в эту точку слово заведомо будет флективным.
      ngrams = ScanDict::ScanTables( grbuff, lextem, inflex );

      return ngrams != 0 ? target( nlexid, lextem, inflex, suffix, grbuff, ngrams ) : 0;
    }
  };

  template <class Collect>
  auto  MakeClassMatch( const Collect& match ) -> MatchClass<Collect>
    {  return MatchClass<Collect>( match );  }

  template <class Collect>
  auto  MakeBuildClass( const Collect& match ) -> BuildClass<Collect>
    {  return BuildClass<Collect>( match );  }

}} // end namespace

# endif // !__libmorph_rus_scanClass_hpp__
