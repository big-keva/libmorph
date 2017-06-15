# if !defined( __ftable_h__ )
# define  __ftable_h__
# include <mtc/stringmap.h>
# include <mtc/serialize.h>
# include <mtc/array.h>
# include <string.h>
# include <stdlib.h>
# include <stdio.h>

# if defined( _MSC_VER )
#   pragma  warning( disable: 4291 )
# endif  // _MSC_VER

using namespace mtc;

struct  fxitem;
class   ftable;
class   fxlist;

struct  fxitem
{
  char      sztail[32];
  char      sznext[32];
  word16_t  grinfo;
  byte_t    bflags;
  word16_t  ofnext;

public:
  static int  compare( const fxitem& r1, const fxitem& r2 )
              {
                int   rescmp = strcmp( r1.sztail, r2.sztail );

                if ( rescmp == 0 )
                  rescmp = (int)r1.grinfo - (int)r2.grinfo;
                if ( rescmp == 0 )
                  rescmp = (int)r1.bflags - (int)r2.bflags;
                if ( rescmp == 0 )
                  rescmp = strcmp( r1.sznext, r2.sznext );
                return rescmp;
              }
public:     // serialization
  size_t  GetBufLen(      ) const;
  template <class O>
  O*      Serialize( O* o ) const;
};

class   ftable: public array<fxitem>
{
  friend class fxlist;

  word16_t  offset = 0;

public:
  bool  Insert( const fxitem& );

public:     // serialization
  template <class O>
  O*    Serialize( O* ) const;

protected:
  int         Compare( const ftable& ) const;
  int         RelocateReferences( fxlist&   rflist );
  unsigned    RelocateOffsetSize( unsigned  offset );

};

class   fxlist
{
  friend class ftable;

  array<ftable*>      tables;
  stringmap<ftable*>  tabmap;

public:     // construction
           ~fxlist();

public:     // API
                      int Insert( ftable*, const char* );
                      int Relocate();
  template <class O>  O*  StoreTab( O* );
  template <class O>  O*  StoreRef( O* );
};

// fxitem serialization

inline
size_t  fxitem::GetBufLen() const
{
  size_t  l = sizeof(bflags) + sizeof(grinfo) + ::GetBufLen( sztail );

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
  size_t  l = sizeof(char);   assert( size() != 0 );

  o = ::Serialize( o, (byte_t)size() );

  for ( auto flex = begin(); flex < end() && o != nullptr; l += (flex++)->GetBufLen() )
    o = flex->Serialize( o );

  return (l & 0x01) != 0 ? ::Serialize( o, (char)0 ) : o;
}

// fxlist inline implementation

template <class O>
O*  fxlist::StoreTab( O* o )
{
  ftable**  ptable;

  for ( o = ::Serialize( o, "inflex", 6 ), ptable = tables.begin(); o != NULL && ptable < tables.end(); ++ptable )
    o = (*ptable)->Serialize( o );

  return o;
}

template <class O>
O*  fxlist::StoreRef( O* o )
{
  void*   lpenum;

  for ( o = ::Serialize( o, tabmap.GetLen() ), lpenum = NULL; o != NULL && (lpenum = tabmap.Enum( lpenum )) != NULL; )
    o = ::Serialize( ::Serialize( o, tabmap.GetVal( lpenum )->offset ), tabmap.GetKey( lpenum ) );

  return o;
}

# endif  // __ftable_h__
