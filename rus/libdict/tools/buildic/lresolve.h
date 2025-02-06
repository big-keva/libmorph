/******************************************************************************

    libmorphrus - dictiorary-based morphological analyser for Russian.

    Copyright (C) 1994-2016 Andrew Kovalenko aka Keva

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
#if !defined( __lresolve_h__ )
#define __lresolve_h__
# include "../libdict/mlmadefs.h"
# include "../../tools/references.h"
# include "mtables.h"
# include <cassert>

struct  morphclass
{
  uint16_t  wdinfo = 0;
  uint16_t  tfoffs = 0;
  uint16_t  mtoffs = 0;

public:     // compare
  bool  operator == ( const morphclass& r ) const
    {
      return wdinfo == r.wdinfo
          && tfoffs == r.tfoffs
          && mtoffs == r.mtoffs;
    }
  bool  operator != ( const morphclass& r ) const
    {
      return !(*this == r);
    }

public:     // serialization
  unsigned  GetBufLen() const
    {
      return sizeof(wdinfo)
        + (tfoffs != 0 ? sizeof(tfoffs) : 0)
        + (mtoffs != 0 ? sizeof(mtoffs) : 0);
    }
  template <class O>
  O*        Serialize( O* o ) const
    {
      unsigned short  wdInfo = wdinfo | (tfoffs    != 0 ? wfFlexes : 0)
                                      | (mtoffs    != 0 ? wfMixTab : 0);

                     o = ::Serialize( o, &wdInfo, sizeof(wdInfo) );
      o = (tfoffs != 0 ? ::Serialize( o, &tfoffs, sizeof(tfoffs) ) : o);
      o = (mtoffs != 0 ? ::Serialize( o, &mtoffs, sizeof(mtoffs) ) : o);

      return o;
    }
};

constexpr morphclass nullclass;

struct  lexemeinfo
{
  std::string ststem;

  morphclass  mclass;
  uint8_t     chrmin = 0;
  uint8_t     chrmax = 0;

  std::string stpost;

  lexemeinfo( const lexemeinfo& ) = delete;
  lexemeinfo& operator = ( const lexemeinfo& ) = delete;
public:
  lexemeinfo() = default;
  lexemeinfo( lexemeinfo&& li ):
      ststem( std::move( li.ststem ) ),
      mclass( std::move( li.mclass ) ),
      chrmin( li.chrmin ),
      chrmax( li.chrmax ),
      stpost( std::move( li.stpost ) ) {}
  lexemeinfo& operator = ( lexemeinfo&& li )
    {
      ststem = std::move( li.ststem );
      mclass = std::move( li.mclass );
      chrmin = li.chrmin;
      chrmax = li.chrmax;
      stpost = std::move( li.stpost );
      return *this;
    }
};

lexemeinfo  ResolveClassInfo( 
  const char* sznorm, const char*   szdies, const char*   sztype, const char* zapart, const char*   szcomm,
  const char* ftable, const libmorph::TableIndex& findex,
  const char* mtable, const libmorph::rus::Alternator& mindex );

#endif // __lresolve_h__
