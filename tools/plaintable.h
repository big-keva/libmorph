# if !defined( __plaintable_h__ )
#	define	__plaintable_h__
# include "plaintree.h"
# include <mtc/serialize.h>
# include <errno.h>

namespace libmorph
{

  struct graminfo
  {
    unsigned short  grinfo;
    unsigned char   bflags;

    bool  operator == ( const graminfo& r ) const
      {  return grinfo == r.grinfo && bflags == r.bflags;  }
  };

  struct gramlist: public array<graminfo>
  {
    int Insert( unsigned short g, unsigned char b )
      {
        graminfo  grinfo = { g, b };

        return Lookup( grinfo ) == end() ? Append( grinfo ) : 0;
      }
    size_t  GetBufLen() const
      {  return 1 + GetLen() * 3;  }
    template <class O>
    O*      Serialize( O* o )
      {
        graminfo* p;

        for ( o = ::Serialize( o, &ncount, 1 ), p = begin(); o != NULL && p < end(); ++p )
          o = ::Serialize( ::Serialize( o, &p->grinfo, sizeof(p->grinfo) ),
                                           &p->bflags, sizeof(p->bflags) );

        return o;
      }
  };

  inline  int FillFlexTree( wordtree<gramlist>& rplain,
                            const void*         tables, unsigned      tfoffs,
                            unsigned short      grInfo, unsigned char bFlags,
                            char*               prefix, unsigned      ccpref )
  {
    const unsigned char*  ftable = (tfoffs << 1) + (const unsigned char*)tables;
    int                   nitems = *ftable++;

    while ( nitems-- > 0 )
    {
      unsigned char         bflags = *ftable++;
      unsigned short        grinfo = *(unsigned short*)ftable;  ftable += sizeof(unsigned short);
      const unsigned char*  szflex = ftable;
      int                   ccflex = *szflex++;
      unsigned short        ofnext;
    
      ftable = szflex + ccflex;
    
      if ( (bflags & 0xc0) != 0 ) {  ofnext = *(unsigned short*)ftable;  ftable += sizeof(unsigned short);  }
        else ofnext = 0;

      memcpy( prefix + ccpref, szflex, ccflex );

      if ( (bflags & 0x80) == 0 )
      {
        gramlist* grlist;

        if ( (grlist = rplain.InsertStr( prefix, ccpref + ccflex )) == NULL )
          return ENOMEM;
        if ( grlist->Insert( grInfo | grinfo, bFlags & bflags ) != 0 )
          return ENOMEM;
      }
      if ( ofnext != 0 && FillFlexTree( rplain, tables, ofnext, grInfo | grinfo, bFlags & bflags, prefix, ccpref + ccflex ) != 0 )
        return ENOMEM;
    }
    return 0;
  }

}  // libmorph namespace

# endif // __plaintable_h__
