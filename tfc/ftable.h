# if !defined( __ftable_h__ )
# define  __ftable_h__
# include "../tools/serialize.h"
# include <algorithm>
# include <vector>
# include <string>
# include <map>

class fxlist;

class fxitem
{
  friend class ftable;

public:
  std::string sztail;
  std::string sznext;
  uint16_t    grinfo;
  uint8_t     bflags;
  uint16_t    ofnext;

public:     // construction
  fxitem(): grinfo( 0 ), bflags( 0 ), ofnext( 0 ) {}
  fxitem( const fxitem& ) = delete;
  fxitem& operator = ( const fxitem& ) = delete;
  fxitem( fxitem&& fx ):
    sztail( std::move( fx.sztail ) ),
    sznext( std::move( fx.sznext ) ),
    grinfo( fx.grinfo ),
    bflags( fx.bflags ),
    ofnext( fx.ofnext ) {}
  fxitem& operator = ( fxitem&& fx )
    {
      sztail = std::move( fx.sztail );
      sznext = std::move( fx.sznext );
      grinfo = fx.grinfo;
      bflags = fx.bflags;
      ofnext = fx.ofnext;
      return *this;
    }

public:     // operators
  bool  operator == ( const fxitem& fx ) const  {  return comp( fx ) == 0;  }
  bool  operator != ( const fxitem& fx ) const  {  return !(*this == fx);   }
  bool  operator <  ( const fxitem& fx ) const  {  return comp( fx ) <  0;  }

protected:  // helpers
  int   comp( const fxitem& fx ) const
    {
      int   rescmp = strcmp( sztail.c_str(), fx.sztail.c_str() );

      if ( rescmp == 0 )
        rescmp = grinfo - fx.grinfo;
      if ( rescmp == 0 )
        rescmp = bflags - fx.bflags;
      if ( rescmp == 0 )
        rescmp = strcmp( sznext.c_str(), fx.sznext.c_str() );
      return rescmp;
    }

public:     // serialization
  template <class O>
  O*      Serialize( O* ) const;
  size_t  GetBufLen(    ) const;
};

class ftable
{
  friend class fxlist;

  std::vector<fxitem> flexet;
  uint16_t            offset;

public:     // construction
  ftable(): offset( 0 ) {}
  ftable( const ftable& ) = delete;
  ftable& operator = ( const ftable& ) = delete;
  ftable( ftable&& f ): flexet( std::move( f.flexet ) ), offset( f.offset ) {}
  ftable& operator = ( ftable&& f )
    {
      flexet = std::move( f.flexet );
      offset = f.offset;  return *this;
    }

public:     // compare
  bool  operator == ( const ftable& fx ) const  {  return flexet == fx.flexet;  }
  bool  operator != ( const ftable& fx ) const  {  return !(*this == fx);   }
  bool  operator <  ( const ftable& fx ) const  {  return flexet <  fx.flexet;  }
  
public:     // adding
  void  Insert( fxitem&& fx )
    {
      auto  beg = flexet.begin();
      auto  end = flexet.end();

      while ( beg != end && *beg < fx )
        ++beg;

      if ( beg == end || *beg != fx )
        flexet.insert( beg, std::move( fx ) );
    }

  bool  empty() const {  return flexet.empty();  }

public:     // serialization
  template <class O>
  O*    Serialize( O* ) const;

protected:  // helpers
  void    RelocateReferences( fxlist&  );
  size_t  RelocateOffsetSize( size_t );

};

class   fxlist
{
  friend class ftable;

  std::vector<ftable>           tables;
  std::map<std::string, size_t> tabmap;

public:     // API
  void  Insert( ftable&&, const char* );
  void  Relocate();

public:     // serialization
  template <class O>  O*  StoreTab( O* );
  template <class O>  O*  StoreRef( O* );
};

// fxitem serialization

inline
size_t  fxitem::GetBufLen() const
{
  size_t  l = sizeof(bflags) + sizeof(grinfo) + ::GetBufLen( sztail.c_str() );

  return (bflags & 0xC0) != 0 ? l + sizeof(ofnext) : l;
}

template <class O>
O*      fxitem::Serialize( O* o ) const
{
  o = ::Serialize(
      ::Serialize(
      ::Serialize( o, &bflags, sizeof(bflags) ),
                      &grinfo, sizeof(grinfo) ), sztail );

  return (bflags & 0xC0) != 0 ? ::Serialize( o, &ofnext, sizeof(ofnext) ) : o;
}

// ftable inline implementation

template <class O>
O*      ftable::Serialize( O*  o ) const
{
  size_t  l = sizeof(char);

  o = ::Serialize( o, (uint8_t)flexet.size() );

  for ( auto flex = flexet.begin(); o != nullptr && flex != flexet.end(); l += flex->GetBufLen(), ++flex )
    o = flex->Serialize( o );

  return (l & 0x01) != 0 ? ::Serialize( o, (char)0 ) : o;
}

// fxlist inline implementation

template <class O>
O*  fxlist::StoreTab( O* o )
{
  o = ::Serialize( o, "inflex", 6 );

  for ( auto tab = tables.begin(); o != nullptr && tab != tables.end(); ++tab )
    o = tab->Serialize( o );

  return o;
}

template <class O>
O*  fxlist::StoreRef( O* o )
{
  o = ::Serialize( o, tabmap.size() );

  for ( auto next = tabmap.begin(); o != nullptr && next != tabmap.end(); ++next )
    o = ::Serialize( ::Serialize( o, next->first ), tables[next->second].offset );

  return o;
}
# endif  // __ftable_h__
