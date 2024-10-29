# include "rus/include/mlfa1049.h"
# include "moonycode/codes.h"
# include "mtc/test-it-easy.hpp"
# include <algorithm>

class CheckPsp
{
  uint8_t psp;

public:
  CheckPsp( uint8_t to ): psp( to ) {}

  template <class MlfaStem>
  bool  operator()( const MlfaStem& stem ) const
  {
    return stem.pgrams != nullptr && stem.ngrams != 0 && stem.pgrams->wdInfo == psp;
  }
};

TestItEasy::RegisterFunc  testmorphrus( []()
{
  TEST_CASE( "fuzzy/rus" )
  {
    IMlfaMb*  mlfaMb;
    IMlfaWc*  mlfaWc;

    SECTION( "fuzzy morphology may be created for native codepage (1251), unicode-16 and a set of codepages supported" )
    {
      SECTION( "∙ native (1251) is created anyway" )
      {
        REQUIRE( mlfaruLoadMbAPI( &(mlfaMb = nullptr) ) == 0 );
        if ( REQUIRE( mlfaMb != nullptr ) )
          mlfaMb->Detach();
      }
      SECTION( "∙ named codepages are created also" )
      {
        REQUIRE( mlfaruLoadCpAPI( &(mlfaMb = nullptr), "koi8" ) == 0 );
        if ( REQUIRE( mlfaMb != nullptr ) )
          mlfaMb->Detach();
        REQUIRE( mlfaruLoadCpAPI( &(mlfaMb = nullptr), "utf8" ) == 0 );
        if ( REQUIRE( mlfaMb != nullptr ) )
          mlfaMb->Detach();
      }
      SECTION( "∙ utf16 is created" )
      {
        REQUIRE( mlfaruLoadWcAPI( &(mlfaWc = nullptr) ) == 0 );
        if ( REQUIRE( mlfaMb != nullptr ) )
          mlfaMb->Detach();
      }
      SECTION( "∙ called with invalid codepage names it return EINVAL" )
      {
        REQUIRE( mlfaruLoadCpAPI( &(mlfaMb = nullptr), "unknown codepage" ) == EINVAL );
      }
    }

    if ( !REQUIRE( mlfaruLoadCpAPI( &mlfaMb, "utf8" ) == 0 ) )
      abort();

    SECTION( "lemmatization of correct words creates lemmas" )
    {
      auto  lemmas = decltype(mlfaMb->Lemmatize( "" )){};

      SECTION( "lemmatization results of noun:m contain at least one noun:m" )
      {
        REQUIRE_NOTHROW( lemmas = mlfaMb->Lemmatize( "кринж" ) );
        if ( REQUIRE( !lemmas.empty() ) )
        {
          REQUIRE( std::find_if( lemmas.begin(), lemmas.end(), CheckPsp( 7 ) ) != lemmas.end() );
        }
      }
      SECTION( "lemmatization results of noun:f contain at least one noun:f" )
      {
        REQUIRE_NOTHROW( lemmas = mlfaMb->Lemmatize( "жиза" ) );
        if ( REQUIRE( !lemmas.empty() ) )
        {
          REQUIRE( std::find_if( lemmas.begin(), lemmas.end(), CheckPsp( 13 ) ) != lemmas.end() );
        }
      }
      SECTION( "lemmatization results of noun:n contain at least one noun:n" )
      {
        REQUIRE_NOTHROW( lemmas = mlfaMb->Lemmatize( "палево" ) );
        if ( REQUIRE( !lemmas.empty() ) )
        {
          REQUIRE( std::find_if( lemmas.begin(), lemmas.end(), CheckPsp( 16 ) ) != lemmas.end() );
        }
      }
      SECTION( "lemmatization results of adjective contain at least one adjective" )
      {
        REQUIRE_NOTHROW( lemmas = mlfaMb->Lemmatize( "карюзный" ) );
        if ( REQUIRE( !lemmas.empty() ) )
        {
          REQUIRE( std::find_if( lemmas.begin(), lemmas.end(), []( const IMlfaMb::SStemInfo& ws )
            {  return ws.pgrams != nullptr && ws.pgrams->wdInfo == 25;  } ) != lemmas.end() );
        }
      }
    }
  }
} );

int   main()
{
  return TestItEasy::Conclusion();
}

/*
int   main()
{
  IMlfaMb*  mb;

  mlfaruLoadCpAPI( &mb, "utf-8" );

  auto  s = std::string( "закоржевский" );

  SStemInfoA  astems[32];
  char        lemmas[0x200];
  SGramInfo   grinfo[64];
  int         nbuilt = mb->Lemmatize( s, astems, lemmas, grinfo, sfIgnoreCapitals );

  for ( int i = 0; i != nbuilt; ++i )
  {
    auto  prefix = "\t";

    auto  slemma = codepages::mbcstowide( codepages::codepage_utf8, astems[i].plemma );
      slemma.insert( slemma.begin() + astems[i].ccstem, '|' );

    fprintf( stdout, "%6.4f  %-2u  %-4u  %-30s", astems[i].weight, astems[i].pgrams->wdInfo,
      astems[i].nclass,
      codepages::widetombcs( codepages::codepage_utf8, slemma ).c_str() );

    for ( auto g = astems[i].pgrams, e = astems[i].ngrams + g; g != e; ++g )
    {
      fprintf( stdout, "%s%u", prefix, g->idForm );
      prefix = ", ";
    }
    fputc( '\n', stdout );
  }

  for ( auto i = 0; i != 256; ++i )
  {
    auto  aforms = mb->BuildForm( astems[1], (formid_t)i );

    if ( !aforms.empty() )
    {
      fprintf( stdout, "\t%u", i );

      for ( auto& s: aforms )
        fprintf( stdout, "\t%s", s.c_str() );

      fprintf( stdout, "\n" );
    }
  }

  return 0;
}
*/
