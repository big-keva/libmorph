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
# if !defined( __flexbox_h__ )
# define  __flexbox_h__
# include <cstdint>
# include <vector>
# include <string>
# include <map>

class CFlexBox
{
public:
  struct tableref
  {
    uint16_t  offset;
    uint8_t   clower = 0xff;
    uint8_t   cupper = 0;
  };

protected:
  struct  fxinfo
  {
    std::string szflex;
    uint8_t     idform;

    bool  operator < ( const fxinfo& fx ) const
    {
      int  rcmp = szflex.compare( fx.szflex );

      return rcmp != 0 ? rcmp < 0 : idform < fx.idform;
    }
  };
  struct  ftable: public tableref, public std::vector<fxinfo>
  {
    auto  GetBufLen() const -> size_t
    {
      auto  length = size_t(1);

      for ( auto& next: *this )
        length += 2 + next.szflex.length();

      return length;
    }
  };
  
public:
  auto  AddInflex( const char* ) -> tableref;

public:     // serialization
  template <class O>
  O*    Serialize( O* ) const;

protected:
  std::map<std::string, ftable> tabmap;
  uint16_t                      length = 4;

};

// CFlexBox implementation

inline  auto  CFlexBox::AddInflex( const char* lpflex ) -> tableref
{
  auto  ptable = tabmap.find( lpflex );
  int   idflex = 0;

// check if present, register new table
  if ( ptable != tabmap.end() ) return ptable->second;
    else  ptable = tabmap.insert( { lpflex, {} } ).first;

// skip first delimiter
  if ( *lpflex == '|' )
    ++lpflex;

// fill the table
  while ( *lpflex != '\0' )
  {
    auto  ptrtop = lpflex;
    auto  ptrend = lpflex;

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
      auto    strend = ptrtop;
      size_t  reglen;
      auto    inflex = fxinfo{};

      while ( strend < ptrend && *strend != '/' )
        ++strend;

      if ( *ptrtop == '0' )
      {
        ptrtop = "";
        reglen = 0;
      }
        else
      reglen = strend - ptrtop;

      inflex = fxinfo{ { ptrtop, reglen }, (uint8_t)idflex };

      ptable->second.insert( std::lower_bound( ptable->second.begin(), ptable->second.end(), inflex ),
        inflex );

      ptable->second.clower = std::min( ptable->second.clower, uint8_t(reglen > 0 ? *ptrtop : '\0') );
      ptable->second.cupper = std::max( ptable->second.cupper, uint8_t(reglen > 0 ? *ptrtop : '\0') );

      if ( strend < ptrend && *strend == '/' )
        ++strend;
      
      ptrtop = strend;
    }
    ++idflex;
  }

// register the offset
  ptable->second.offset = length;
    length += ptable->second.GetBufLen();
  return ptable->second;
}

template <class O>
O* CFlexBox::Serialize( O* o ) const
{
  using tab_iterator = decltype(tabmap.begin());
  
  std::vector<tab_iterator> sorted;

  for ( auto it = tabmap.begin(); it != tabmap.end(); ++it )
    sorted.push_back( it );

  std::sort( sorted.begin(), sorted.end(), []( const tab_iterator& i1, const tab_iterator& i2 )
    {  return i1->second.offset < i2->second.offset;  } );

  o = ::Serialize( o, "FLEX", 4 );

  for ( auto& next: sorted )
  {
    auto& table = next->second;

    o = ::Serialize( o, (char)table.size() );

    for ( auto& flex: table )
    {
      o = ::Serialize( ::Serialize( ::Serialize( o,
        (char)flex.idform ),
        (char)flex.szflex.length() ), flex.szflex.c_str(), flex.szflex.length() );
    }
  }
  return o;
}

# endif  // __flexbox_h__
