# if !defined( __conffile_h__ )
# define __conffile_h__
# include "sweets.h"
# include "stringmap.h"
# include "array.h"

namespace libmorph
{

  class conf
  {
    typedef array<char, char>   strset;
    stringmap<strset, strset&>  keymap;

  public:     // API
    int         Initialize( const char* szfile );
    int         Initialize( FILE*       lpfile );
    const char* FindString( const char* pszkey ) const;

  };

  // simple config implementation

  inline  int conf::Initialize( const char* szfile )
  {
    libmorph::file  infile;

    if ( (FILE*)(infile = fopen( szfile, "rt" )) == NULL )
      return ENOENT;
    return Initialize( (FILE*)infile );
  }

  inline  int conf::Initialize( FILE* lpfile )
  {
    char  szline[0x400];
    char* strtop;
    char* strend;

    while ( fgets( szline, sizeof(szline), lpfile ) != NULL )
    {
      char*   argend;
      char*   valtop;
      strset* pfound;
      int     curlen;

    // prepare string
      for ( strtop = szline; *strtop != '\0' && (unsigned char)*strtop <= 0x20; ++strtop )
        (void)NULL;
      if ( *strtop == ';' || *strtop == '#' )
        continue;
      for ( strend = strtop; *strend != '\0'; ++strend )
        (void)NULL;
      for ( ; strend > strtop && (unsigned char)strend[-1] <= 0x20; *--strend = '\0' )
        (void)NULL;
      if ( strtop >= strend || *strtop == '\0' )
        continue;

    // parse to parts
      for ( argend = strtop + 1; argend < strend && (unsigned char)*argend > 0x20; ++argend )
        (void)NULL;
      for ( valtop = argend; valtop < strend && (unsigned char)*valtop <= 0x20; *valtop++ = '\0' )
        (void)NULL;

    // check the arguments
      if ( valtop <= argend || strend <= valtop )
        continue;

    // add argument to argument list
      if ( (pfound = keymap.Search( strtop )) == NULL )
      {
        if ( (pfound = keymap.Insert( strtop )) == NULL )
          return ENOMEM;
        if ( pfound->Append( '\0' ) != 0 )
          return ENOMEM;
      }
      if ( pfound->SetLen( (curlen = pfound->GetLen()) + strend - valtop + 1 ) != 0 )
        return ENOMEM;
      memcpy( curlen - 1 + *pfound, valtop, strend - valtop );
        (*pfound)[curlen - 1 + strend - valtop] = '\0';
        (*pfound)[curlen     + strend - valtop] = '\0';
    }

    return 0;
  }

  inline  const char* conf::FindString( const char* thekey ) const
  {
    const strset* pfound;
  
    return (pfound = keymap.Search( thekey )) != NULL ? (const char*)*pfound : NULL;
  }

}  // end libmorph namespace

# endif // __conffile_h__
