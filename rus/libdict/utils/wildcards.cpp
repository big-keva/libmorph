#include <cstring>

# include "../../../rus.h"

IMlmaMbXX*  morpho;

int main()
{
  char  szline[0x100];

  mlmaruGetAPI( "utf-8", (void**)&morpho );

  while ( fgets( szline, sizeof(szline), stdin ) != NULL )
  {
    char* strptr;

    for ( strptr = szline; *strptr != '\0'; ++strptr )
      (void)NULL;

    while ( strptr > szline && (unsigned char)strptr[-1] < 0x20 )
      --strptr;

    if ( strptr == szline )
      break;

    morpho->FindMatch( { szline, size_t(strptr - szline) },
      []( lexeme_t /*nlexid*/, int  /*nforms*/, const SStrMatch* pforms )
      {
        fprintf( stdout, "\t%s\n", pforms->sz );
        return 1;
      } );
  }
  return 0;
}