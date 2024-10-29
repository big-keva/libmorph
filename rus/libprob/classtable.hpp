# if !defined( __libfuzzy_rus_classtable_hpp__ )
# define __libfuzzy_rus_classtable_hpp__
# include <stdexcept>

namespace libfuzzyrus {

// reverted flextion dictionary and class map
  extern  unsigned char ReverseDict[];
  extern  unsigned      ClassNumber;
  extern  unsigned      ClassOffset[];
  extern  unsigned char ClassTables[];

}

namespace libfuzzy {
namespace rus {

  auto  GetClass( unsigned uclass ) -> const char*
  {
    if ( uclass < libfuzzyrus::ClassNumber )
      return (const char*)libfuzzyrus::ClassTables + libfuzzyrus::ClassOffset[uclass];
    else throw std::invalid_argument( "invalid class offset" );
  }

}}

# endif // !__libfuzzy_rus_classtable_hpp__
