# include "../include/mlma1049.h"
# include <stdint.h>
# include <stdio.h>

template <class C>
inline  C* next( C* s )
{
  while ( *s++ != 0 )
    ++s;
  return s;
}

void  BuildDicDump( FILE* dump, IMlmaMb* mlma )
{
  unsigned char wdinfo;
  char          aforms[0x400];
  bool          dumped = false;

  for ( lexeme_t nlexid = 0; nlexid != 0x3ffff; ++nlexid, dumped = false )
    if ( mlma->GetWdInfo( &wdinfo, nlexid ) != 0 )
      for ( int formid = 0; formid != 0x100; ++formid )
      {
        int   nforms = mlma->BuildForm( aforms, sizeof(aforms), nlexid, (formid_t)formid );
        char* pforms;

        if ( nforms == 0 )
          continue;

        if ( !dumped )
          fprintf( dump, "%u: %u\n", nlexid, wdinfo );

        fprintf( dump, "\t%02x", formid );

        for ( dumped = true, pforms = aforms; nforms-- > 0; pforms = next( pforms ) )
          fprintf( dump, "\t%s", pforms );

        fprintf( dump, "\n" );
      }
}

int   main( int argc, char* argv[] )
{
  IMlmaMb*  mb_api;

  mlmaruLoadCpAPI( &mb_api, "utf-8" );

  BuildDicDump( stdout, mb_api );

  mb_api->Detach();
  return 0;
}

