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
# if !defined( __conditions_h__ )
# define  __conditions_h__
# include "mtc/serialize.h"
# include <cstdint>
# include <cassert>
# include <string>
# include <vector>
# include <map>

//
// the binary record of fragments
//
class interchange
{
  friend class conditions;
  friend class collector;

  struct fragment
  {
    std::string string;
    uint8_t     nflags;

  public:
    fragment( const char* s, uint8_t f ): string( s ), nflags( f )  {}
    fragment( const std::string& s, uint8_t f ): string( s ), nflags( f )  {}
    fragment( const fragment& ) = delete;
    fragment& operator = ( const fragment& ) = delete;
    fragment( fragment&& f ): string( std::move( f.string ) ), nflags( f.nflags ) {}
    fragment& operator = ( fragment&& f )
      {
        string = std::move( f.string );
        nflags = f.nflags;
        return *this;
      }

    bool  operator == ( const fragment& f ) const {  return nflags == f.nflags && string == f.string;  }
    bool  operator != ( const fragment& f ) const {  return !(*this == f);  }
  };

  std::vector<fragment> ichset;
  uint16_t              offset;

public:
  interchange(): offset( 0 ) {}
  interchange( interchange&& i ): ichset( std::move( i.ichset ) ), offset( i.offset ) {}
  interchange( const interchange& ) = delete;
  interchange& operator = ( const interchange& ) = delete;
  interchange& operator = ( interchange&& ) = delete;

public:
  bool  operator == ( const interchange& i ) const  {  return ichset == i.ichset;  }
  bool  operator != ( const interchange& i ) const  {  return !(*this == i);  }
  void  addstep( const char*        mix, int step );
  void  addstep( const std::string& mix, int step )  {  return addstep( mix.c_str(), step );  }

public:     // serialization
  auto  GetBufLen() const -> uint16_t;
  template <class O>
  O*    Serialize( O* ) const;
};

//
// the condition-based references to binary tables holder
//
class conditions
{
  struct reference
  {
    std::string cond;
    int         ipos;

  public:
    reference( const std::string& s, int i ): cond( s ), ipos( i )  {}
    reference( const reference& ) = delete;
    reference& operator = ( const reference& ) = delete;
    reference( reference&& r ): cond( std::move( r.cond ) ), ipos( r.ipos ) {}
    reference& operator = ( reference&& ) = delete;

  public:
    bool  operator == ( const reference& r ) const  {  return ipos == r.ipos && cond == r.cond;  }
    bool  operator != ( const reference& r ) const  {  return !(*this == r);  }
  };

  std::vector<reference>  refset;

public:     // compare
  bool  operator == ( const conditions& c ) const {  return refset == c.refset;  }
  bool  operator != ( const conditions& c ) const {  return !(*this == c);  }

public:     //
  void  add_condition( const char* cond, int ipos ) {  refset.emplace_back( reference( cond, ipos ) );  }
  
public:     // serialization
  template <class O>  O*  Serialize( O* o, const std::vector<interchange>& ) const;

};

class collector
{
  std::vector<interchange>      interSet;
  std::vector<conditions>       condiSet;
  std::map<std::string, size_t> tabIndex;

public:     // building
  void  add_interchange( const char* names, const char* condi, interchange&& inter )
    {
      set_conditions( names ).
        add_condition( condi, 
          set_interchange( std::move( inter ) ) );
    }
  void  relocate_tables();

protected:  // helpers
  conditions&               set_conditions( const char* names );
  int                       set_interchange( interchange&& );
  std::vector<std::string>  parse_tabindex( const char* names ) const;
  
public:     // serialization
  template <class O>  O*  StoreTab( O* ) const;
  template <class O>  O*  StoreRef( O* ) const;

};

// interchange inline implementation

inline  void  interchange::addstep( const char* mix, int step )
{
  auto  beg = ichset.begin();
  auto  end = ichset.end();
  int   res;

  while ( beg != end && (res = strcmp( beg->string.c_str(), mix )) < 0 )
    ++beg;

  if ( beg != end && res == 0 )
    return (void)(beg->nflags |= (1 << step));

  return (void)ichset.emplace( beg, fragment( mix, 1 << step ) );
}

inline  uint16_t  interchange::GetBufLen() const
{
  uint16_t  length = 1;   /*count of mixes*/

  for ( const auto& ich: ichset )
    length += 1/*length and flags*/ + (uint16_t)ich.string.length();

  return length;
}

template <class O>
O*  interchange::Serialize( O* o ) const
{
  auto  upower = 0x00;

  if ( (o = ::Serialize( o, (uint8_t)ichset.size() )) == nullptr )
    return nullptr;

  for ( const auto& ich: ichset )
    upower |= (ich.nflags << 4);

  upower = (upower ^ 0x70) & 0x70;

  for ( const auto& ich: ichset )
  {
    uint8_t bflags = (uint8_t)(ich.string.length() | (ich.nflags << 4));

  // set the upper powers for base power
    if ( upower != 0 && (bflags & 0x10) != 0 )
      bflags |= upower;
      
    o = ::Serialize( ::Serialize( o, bflags ), ich.string.c_str(), ich.string.length() );
  }
  return o;
}

// conditions inline implementation

template <class O>
O*  conditions::Serialize( O* o, const std::vector<interchange>&  ichset ) const
{
  auto  beg = refset.begin();
  auto  end = refset.end();

  for ( o = ::Serialize( o, refset.size() ); o != nullptr && beg != end; ++beg )
    o = ::Serialize( ::Serialize( o, ichset[beg->ipos].offset ), beg->cond.c_str() );

  return o;
}

// collector inline implementation

inline  void  collector::relocate_tables()
{
  uint16_t  offset = 6;

// relocate binary representations
  for ( auto beg = interSet.begin(), end = interSet.end(); beg != end; ++beg )
    offset = (uint16_t)((beg->offset = offset) + beg->GetBufLen());
}

inline  conditions& collector::set_conditions( const char* pnames )
{
  auto    idxset = parse_tabindex( pnames );
  size_t  cindex = (size_t)-1;

  // если ключ не найден, создать новый набор условий, если таковой ещё не создан,
  // и назначить индекс набора условий на это значение
  for ( const auto& tabkey: idxset )
  {
    auto  pfound = tabIndex.find( tabkey );

    if ( pfound == tabIndex.end() )
    {
      if ( cindex == (size_t)-1 )
        condiSet.resize( (cindex = condiSet.size()) + 1 );
      tabIndex.emplace( tabkey, cindex );
    }
      else
  // если найден, проверить его соответствие уже существующему
    {
      if ( cindex == (size_t)-1 )
        cindex = pfound->second;

      if ( cindex != pfound->second )
        throw std::runtime_error( "table name '" + tabkey + "' conflicts with '" + pfound->first + "'" );
    }
  }

  if ( condiSet.size() <= cindex )
    throw std::runtime_error( "empty table index" );

  return condiSet.at( cindex );
}

inline  int   collector::set_interchange( interchange&& inter )
{
  auto  beg = interSet.begin();
  auto  end = interSet.end();

  while ( beg != end && *beg != inter )
    ++beg;

  if ( beg != end )
    return beg - interSet.begin();

  interSet.push_back( std::move( inter ) );
    return interSet.size() - 1;
}

inline  std::vector<std::string>  collector::parse_tabindex( const char* names ) const
{
  auto                      isspace = [ ]( const char ch ){  return ch != '\0' && (unsigned char)ch <= 0x20;  };
  auto                      nospace = [&]( const char* s ){  while ( isspace( *s ) ) ++s;  return s;  };
  std::vector<std::string>  idxlist;

  while ( *(names = nospace( names )) != '\0' )
  {
    const char* pszorg = names;

    while ( *names != '\0' && *names != ',' )
      ++names;
    while ( names != pszorg && isspace( names[-1] ) )
      --names;

    if ( names != pszorg )
      idxlist.emplace_back( std::string( pszorg, names - pszorg ) );

    if ( *(names = nospace( names )) == ',' )
      ++names;
  }

  return idxlist;
}

template <class O>
O*  collector::StoreTab( O* o ) const
{
  if ( (o = ::Serialize( o, "interc", 6 )) == nullptr )
    return o;

  for ( const auto& ich: interSet )
    o = ich.Serialize( o );

  return o;
}

template <class O>
O*  collector::StoreRef( O* o ) const
{
  if ( (o = ::Serialize( o, condiSet.size() )) == nullptr )
    return nullptr;

  for ( auto beg = condiSet.begin(), end = condiSet.end(); beg != end && o != nullptr; ++beg )
    o = beg->Serialize( o, interSet );

  return ::Serialize( o, tabIndex );
}

# endif  // __conditions_h__
