# if !defined( __references_h__ )
# define __references_h__
# include <mtc/stringmap.h>

using namespace mtc;

struct  CReferences: protected stringmap<unsigned>
{
  unsigned  GetOffset( const char* thekey )
    {
      unsigned* ptrofs;

      return (ptrofs = Search( thekey )) != NULL ? *ptrofs : 0;
    }
  template <class S>
  S*        FetchFrom( S* s )
    {
      int   toload;

      for ( s = ::FetchFrom( s, toload ); s != NULL && toload != 0; --toload )
      {
        unsigned  offset;
        unsigned  cchkey;
        char      thekey[0x100];

        if ( (s = ::FetchFrom( ::FetchFrom( s, offset ), cchkey )) == NULL )
          return NULL;
        if ( cchkey < sizeof(thekey) )  thekey[cchkey] = 0;
          else  return NULL;
        if ( (s = ::FetchFrom( s, thekey, cchkey )) == NULL )
          return NULL;
        if ( Insert( thekey, offset ) == nullptr )
          return NULL;
      }
      return s;
    }
};

# endif  // __references_h__
