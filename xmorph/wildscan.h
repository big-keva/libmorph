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
# if !defined( __libmorph_wildscan_h__ )
# define __libmorph_wildscan_h__
# include "mlmadefs.h"
# include "scandict.h"
# include <cstring>

namespace libmorph {

  inline  bool  IsWildcard( uint8_t  c )
  {
    return c == '*' || c == '?';
  }

  inline  bool  IsWildcard( const fragment& s )
  {
    for ( auto p = s.str, e = s.str + s.len; p != e; )
      if ( IsWildcard( *p++ ) ) return true;
    return false;
  }

  inline  bool  IsAsterisk( const fragment& str )
  {
    if ( str.len > 0 )
    {
      auto  ptr = str.str;
      auto  end = str.len + ptr;

      while ( ptr != end && *ptr == '*' )
        ++ptr;
      return ptr == end;
    }
    return false;
  }

  struct Wild final: public Counters
  {
   /*
    * Wild::ScanTree( action, dict, string, match, len )
    *
    * Высокоуровневый сканер словаря, вызывает action для каждой найденной лексемы
    * с ведущей к ней строкой трассы
    */
    template <class aflags, class action> static
    int   ScanTree(
      const action&   target,
      const uint8_t*  thedic,
      const fragment& thestr,
      uint8_t*        smatch,
      size_t          nmatch )
    {
      auto    dicorg = thedic;
      auto    uflags = counter<aflags>::getvalue( thedic );
      auto    ncount = counter<aflags>::getlower( uflags );
      auto    sublen = unsigned{};
      int     nerror;

      if ( thestr.size() > 0 )
      {
        auto  chfind = *thestr.str;

        switch ( chfind )
        {
          // '?' - любой непустой символ; подставить по очереди все символы в строку на позицию шаблона
          //       и продолжить сканирование на вложенных поддеревьях; пустая строка точно в пролёте
          case '?':
            for ( smatch[nmatch] = *thedic; ncount-- > 0; smatch[nmatch] = *thedic )
              if ( (nerror = ScanTree<aflags>( target, getsubdic( thedic ), thestr.next(), smatch, nmatch + 1 )) != 0 )
                return nerror;
            break;

        // '*' - любая последовательность символов длиной от 0 до ...; для каждого поддерева вызвать
        //       два варианта к каждому возможному поддереву:
          case '*':
            if ( thestr.size() > 1 )
              if ( (nerror = ScanTree<aflags>( target, dicorg, thestr.next(), smatch, nmatch )) != 0 ) // zero chars match
                return nerror;
            for ( smatch[nmatch] = *thedic; ncount-- > 0; smatch[nmatch] = *thedic )
              if ( (nerror = ScanTree<aflags>( target, getsubdic( thedic ), thestr, smatch, nmatch + 1 )) != 0 ) // more than one char match
                return nerror;
            break;

        // любой другой символ - должно быть точное соответствие
          default:
            while ( ncount-- > 0 )
              if ( (smatch[nmatch] = *thedic) != chfind )  getsubdic( thedic );
                else
              if ( (nerror = ScanTree<aflags>( target, getsubdic( thedic ), thestr.next(), smatch, nmatch + 1 )) != 0 )
                return nerror;
            break;
        }
      }
        else
      for ( ; ncount-- > 0; thedic += sublen )
        sublen = getserial( ++thedic );

    // если есть список лексем, провести там отождествление
      return counter<aflags>::hasupper( uflags ) ? ScanList( target, thedic, thestr, smatch, nmatch ) : 0;
    }

  protected:
   /*
    * Wild::ScanList( target, dict, string, match, len )
    *
    * Проводит отождествление шаблона с массивом лексем
    */
    template <class action> static
    int   ScanList(
      const action&   target,
      const uint8_t*  pstems,
      const fragment& thestr,
      uint8_t*        smatch,
      size_t          nmatch )
    {
      unsigned  ucount = getserial( pstems );

      while ( ucount-- > 0 )
      {
        auto      chrmin = *pstems++;
        auto      chrmax = *pstems++;
        lexeme_t  nlexid = getserial( pstems );
        auto      oclass = getword16( pstems );
        fragment  suffix = { nullptr, 0 };
        uint8_t   chnext;
        int       nerror;

      // check postfix
        if ( (oclass & wfPostSt) != 0 )
          pstems += (suffix = { pstems + 1, *pstems }).len + 1;

      // оценить, может ли хотя бы потенциально такое окончание быть у основ начиная с этой и далее
        if ( !thestr.empty() && !IsWildcard( chnext = *thestr.str ) )
        {
          if ( chnext > chrmax )  break;
          if ( chnext < chrmin )  continue;
        }

      // check capitalization scheme
        if ( (nerror = target( nlexid, oclass & ~0x8000, thestr, suffix, smatch, nmatch )) != 0 )
          return nerror;
      }
      return 0;
    }

    static
    auto  getsubdic( const uint8_t*& thedic ) -> const uint8_t*
    {
      auto  sublen = getserial( ++thedic );
      auto  subdic = thedic;  thedic += sublen;

      return subdic;
    }

  };

}  // end namespace

# endif // !__libmorph_wildscan_h__
