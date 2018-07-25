# pragma once
# if !defined( __plaintable_h__ )
# define __plaintable_h__
# include <vector>

namespace libmorph
{

  std::vector<char> CreatePlainTable( const char* tables, size_t offset );

}  // libmorph namespace

# endif // __plaintable_h__
