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
#if !defined( __mtables_h__ )
#define __mtables_h__
# include <algorithm>
# include <vector>
# include <tuple>
# include <map>
# include <string>

namespace libmorph {
namespace rus{

  class Alternator
  {
    //
    // Чередование с условием
    //
    struct  alt
    {
      uint16_t  offset;
      char      szcond[0x10];
    };

    //
    // Представление ссылок на некоторую таблицу чередований
    //
    struct tab: public std::vector<alt>
    {
      uint16_t  Find( const char* tabs,
                      uint16_t    type,
                      const char* stem,
                      const char* rems ) const;
    };

    template <class S>  S*  Load( alt& a, S* s );
    template <class S>  S*  Load( tab& t, S* s );

  public:     // static helpers
    static  const char*                   GetDefaultStr( const char* tables, unsigned tbOffs );
    static  std::tuple<uint8_t, uint8_t>  GetMinMaxChar( const char* tables, uint16_t tboffs, uint8_t chrmin, uint8_t chrmax );

  public:     // API
    uint16_t  Find( const char* tabs,
                    const char* ztyp,
                    uint16_t    type,
                    const char* stem,
                    const char* rems ) const;
    FILE*     Load( FILE* );

  protected:  // vars
    std::vector<tab>            tabset;
    std::map<std::string, tab*> mapper;

  };

}}

#endif // __mtables_h__
