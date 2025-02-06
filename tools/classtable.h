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
# pragma once
# if !defined( __libmorph_classtable_h__ )
# define __libmorph_classtable_h__

template <class serializable>
class classtable
{
  using classrefer = std::pair<serializable, uint16_t>;

  std::vector<classrefer> clsset;
  size_t                  length;

public:     // construction
  classtable(): length( 0 ) {}
  classtable( const classtable& ) = delete;
  classtable& operator = ( const classtable& ) = delete;

public:     // API
  uint16_t  AddClass( const serializable& rclass )
  {
    auto  pfound = std::find_if( clsset.begin(), clsset.end(),
      [&]( const classrefer& r ){  return r.first == rclass;  } );

    if ( pfound != clsset.end() )
      return pfound->second;

    clsset.push_back( { rclass, (uint16_t)length } );
      length += rclass.GetBufLen();

    if ( length > (uint16_t)-1 )
      throw std::range_error( "class offset is too big" );

    return clsset.back().second;
  }

public:     // serialization
  size_t  GetBufLen() const {  return length;  }
  template <class O>
  O*      Serialize( O* ) const;
};

template <class serializable>
template <class O>
O*  classtable<serializable>::Serialize( O* o ) const
{
  for ( auto& nextclass: clsset )
    if ( (o = nextclass.first.Serialize( o )) == nullptr )
      break;
  return o;
}

# endif   // __libmorph_classtable_h__
