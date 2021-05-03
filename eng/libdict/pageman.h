# if !defined( _pageman_h_ )
# define _pageman_h_

# include "mlmadefs.h"
# include <string.h>

namespace __libmorpheng__
{

  # if defined( WIN32 )
  #   include <windows.h>
  # endif // WIN32

  typedef struct
  {
    word32_t        minvalue;
    word32_t        maxvalue;
    unsigned char*  pagedata;
  } SLidsRef;

  // Класс CPageManager обеспечивает доступ к страницам словаря по
  // начальным символам поданного слова.

  class CPageMan
  {
    unsigned char** ppages;
    unsigned        npages;
  public:
                    CPageMan( unsigned char**, unsigned );
                   ~CPageMan();
    unsigned char*  operator [] ( unsigned );
    unsigned char** FindPage( unsigned char   chword,
                              unsigned char** lplast );
    unsigned        GetCount() const;
  };

  class CLIDsMan
  {
    SLidsRef* pageList;
    unsigned  numPages;
  public:
                    CLIDsMan( SLidsRef*, unsigned );
                   ~CLIDsMan();
    SLidsRef*       FindPage( lexeme_t );
    unsigned        GetCount() const;
    SLidsRef*       operator [] ( unsigned );
  };

  // CPageMan inline implementation

  inline  CPageMan::CPageMan( unsigned char** pages,
                              unsigned        count ): ppages( pages ),
                                                       npages( count )
  {
  }

  inline  CPageMan::~CPageMan()
  {
  }

  inline  unsigned  CPageMan::GetCount() const
  {
    return npages;
  }

  inline  unsigned char*  CPageMan::operator [] ( unsigned uindex )
  {
    return ( uindex < npages ? ppages[uindex] : NULL );
  }

  // CLidsMan inline implementation

  inline  CLIDsMan::CLIDsMan( SLidsRef* pages, unsigned count ):
    pageList( pages ), numPages( count )
  {
  }

  inline  CLIDsMan::~CLIDsMan()
  {
  }

  inline  unsigned  CLIDsMan::GetCount() const
  {
    return numPages;
  }

  inline  SLidsRef* CLIDsMan::operator [] ( unsigned uindex )
  {
    return ( uindex < numPages ? pageList + uindex : NULL );
  }

} // end __libmorpheng__ namespace

# endif // _pageman_h_
