/******************************************************************************

    libmorph - morphological analysers.

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
# pragma once
# if !defined( __references_h__ )
# define __references_h__
# include "mtc/serialize.h"
# include <cstdint>
# include <cstdio>
# include <string>
# include <map>

namespace libmorph
{

  struct TableIndex: protected std::map<std::string, uint32_t>
  {
    auto  operator []( const char* key ) const -> uint32_t
    {
      auto  it = find( key );

      return it != end() ? it->second : 0;
    }
    auto  operator []( const std::string& key ) const -> uint32_t
    {
      auto  it = find( key );

      return it != end() ? it->second : 0;
    }

    template <class S>
    S*        Load( S* s )
    {
      return ::FetchFrom( s, *(std::map<std::string, uint32_t>*)this );
    }

  };

}

# endif  // __references_h__
