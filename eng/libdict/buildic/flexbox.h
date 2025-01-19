# if !defined( __flexbox_h__ )
# define  __flexbox_h__
# include <cstdint>
# include <vector>
# include <string>
# include <map>

class CFlexBox
{
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
  struct  ftable: public std::vector<fxinfo>
  {
    uint16_t  offset;

  public:
    auto  GetBufLen() const -> size_t
    {
      auto  length = size_t(1);

      for ( auto& next: *this )
        length += 2 + next.szflex.length();

      return length;
    }
  };

public:
  uint16_t  AddInflex( const char* );

public:
  size_t    GetBufLen() const;
  template <class O>
  O*        Serialize( O* ) const;

protected:
  std::map<std::string, ftable> tabmap;
  uint16_t                      length = 4;

};

// CFlexBox implementation

inline  auto  CFlexBox::AddInflex( const char* lpflex ) -> uint16_t
{
  auto  ptable = tabmap.find( lpflex );
  int   idflex = 0;

// check if present, register new table
  if ( ptable != tabmap.end() ) return ptable->second.offset;
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

      if ( strend < ptrend && *strend == '/' )
        ++strend;
      ptrtop = strend;
    }
    ++idflex;
  }

// register the offset
  ptable->second.offset = length;
    length += ptable->second.GetBufLen();
  return ptable->second.offset;
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
