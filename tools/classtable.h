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
