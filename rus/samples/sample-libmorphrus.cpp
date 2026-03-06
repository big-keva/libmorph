# include "../../rus.h"

int  main()
{
  IMlmaMbXX*  pmorph;
  SLemmInfoA  lemmas[32];
  char        fmbuff[256];
  SGramInfo   grbuff[32];
  int         nlemma;

  mlmaruGetAPI( "utf-8", (void**)&pmorph );

  nlemma = pmorph->Lemmatize( "стали", lemmas, fmbuff, grbuff, sfIgnoreCapitals );

  for ( int i = 0; i < nlemma; i++ )
    printf( "* %s\tIDL:%u\n", lemmas[i].plemma, lemmas[i].nlexid );

  return 0;
}