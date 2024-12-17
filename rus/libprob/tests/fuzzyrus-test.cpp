# include "rus/include/mlfa1049.h"
# include "moonycode/codes.h"
# include "mtc/test-it-easy.hpp"
# include <algorithm>
# include <vector>

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

class CMatch: public IMatchStemMb
{
  int   Attach() override {  return 1;  }
  int   Detach() override {  return 1;  }
public:
  int   Add(
    const char* plemma,
    size_t      clemma,
    size_t      cchstr,
    unsigned    uclass,
    size_t      nforms, const SStrMatch* pforms ) override
  {
    fprintf( stdout, "%s\t%u\t", plemma, uclass );

    for ( int i = 0; i != nforms; ++i )
      fprintf( stdout, "\t%s\n", pforms[i].sz );
  }
};

class CMatchFormsList: public IMatchStemMb
{
  std::vector<std::string>  aforms;

public:
  int   Attach() override {  return 1;  }
  int   Detach() override {  return 1;  }
public:
  int   Add(
    const char* plemma,
    size_t      clemma,
    size_t      cchstr,
    unsigned    uclass,
    size_t      nforms, const SStrMatch* pforms ) override
  {
    for ( ; nforms-- > 0; ++pforms )
    {
      auto  pfound = std::lower_bound( aforms.begin(), aforms.end(), pforms->sz );

      if ( pfound == aforms.end() || *pfound != pforms->sz )
        aforms.insert( pfound, pforms->sz );
    }
  }
  auto  Get() -> std::string
  {
    auto  output = std::string();
    auto  prefix = "";

    for ( auto& next: aforms )
    {
      (output += prefix) += next;
      prefix = ", ";
    }
    return output;
  }
};

class CMatchClassList: public IMatchStemMb
{
  std::vector<std::string>  aclass;

public:
  int   Attach() override {  return 1;  }
  int   Detach() override {  return 1;  }
public:
  int   Add(
    const char* plemma,
    size_t      clemma,
    size_t      cchstr,
    unsigned    uclass,
    size_t      nforms, const SStrMatch* pforms ) override
  {
    auto  clsstr = std::string( plemma, cchstr ) + mtc::strprintf( "[%u]", uclass );
    auto  pfound = std::lower_bound( aclass.begin(), aclass.end(), clsstr );

    if ( pfound == aclass.end() || *pfound != clsstr )
      aclass.insert( pfound, clsstr );
    return 0;
  }
  auto  Get() -> std::string
  {
    auto  output = std::string();
    auto  prefix = "";

    for ( auto& next: aclass )
    {
      (output += prefix) += next;
      prefix = ", ";
    }
    return output;
  }
};

auto  operator & (
  const std::vector<SLemmInfo<SStemInfoA>>& v1,
  const std::vector<SLemmInfo<SStemInfoA>>& v2 ) -> std::vector<SLemmInfo<SStemInfoA>>
{
  std::vector<SLemmInfo<SStemInfoA>>  output;

  for ( auto& s1: v1 )
    for ( auto& s2: v2 )
      if ( s1.nclass == s2.nclass && s1.ccstem == s2.ccstem )
      {
        SGramInfo grlist[32];
        unsigned  ngrams = 0;

        for ( auto g1 = s1.pgrams, e1 = s1.pgrams + s1.ngrams; g1 != e1; ++g1 )
          for ( auto g2 = s2.pgrams, e2 = s2.pgrams + s2.ngrams; g2 != e2; ++g2 )
            if ( g1->idForm == g2->idForm )
              grlist[ngrams++] = *g1;

        output.push_back( SStemInfoA{
          s1.ccstem,
          s1.nclass,
          s1.plemma, grlist, ngrams, s1.weight } );
      }

  return output;
}

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
    if ( !REQUIRE( mlfaruLoadWcAPI( &mlfaWc ) == 0 ) )
      abort();

    SECTION( "GetWdInfo" )
    {
      uint8_t wdinfo;

      REQUIRE( mlfaMb->GetWdInfo( nullptr, 0 ) == ARGUMENT_FAILED );
      REQUIRE( mlfaMb->GetWdInfo( &wdinfo, 1000 ) == ARGUMENT_FAILED );
      REQUIRE( mlfaMb->GetWdInfo( 0 ) == 25 );
    }
    SECTION( "GetSample" )
    {
      char  sample[256];

      REQUIRE( mlfaMb->GetSample( nullptr, 1, 0 ) == ARGUMENT_FAILED );
      REQUIRE( mlfaMb->GetSample( sample, 0, 0 ) == ARGUMENT_FAILED );
      REQUIRE( mlfaMb->GetSample( sample, 1000 ) == ARGUMENT_FAILED );
      if ( REQUIRE( mlfaMb->GetSample( sample, 0 ) > 0 ) )
        REQUIRE( sample == std::string( "определенный" ) );
      REQUIRE( mlfaMb->GetSample( 0 ) == "определенный" );
    }
    SECTION( "Lemmatize" )
    {
      auto  lemmas = decltype(mlfaMb->Lemmatize( "" )){};

      SECTION( "∙ lemmatization of correct words creates lemmas" )
      {
        SECTION( "lemmatization results of noun:m contain at least one noun:m" )
        {
          REQUIRE_NOTHROW( lemmas = mlfaMb->Lemmatize( "кринж" ) );
          if ( REQUIRE( !lemmas.empty() ) )
          {
            REQUIRE( std::find_if( lemmas.begin(), lemmas.end(), CheckPsp( 7 ) ) != lemmas.end() );
          }
        }
        SECTION( "∙ lemmatization results of noun:f contain at least one noun:f" )
        {
          REQUIRE_NOTHROW( lemmas = mlfaMb->Lemmatize( "жиза" ) );
          if ( REQUIRE( !lemmas.empty() ) )
          {
            REQUIRE( std::find_if( lemmas.begin(), lemmas.end(), CheckPsp( 13 ) ) != lemmas.end() );
          }
        }
        SECTION( "∙ lemmatization results of noun:n contain at least one noun:n" )
        {
          REQUIRE_NOTHROW( lemmas = mlfaMb->Lemmatize( "палево" ) );
          if ( REQUIRE( !lemmas.empty() ) )
          {
            REQUIRE( std::find_if( lemmas.begin(), lemmas.end(), CheckPsp( 16 ) ) != lemmas.end() );
          }
        }
        SECTION( "∙ lemmatization results of adjective contain at least one adjective" )
        {
          REQUIRE_NOTHROW( lemmas = mlfaMb->Lemmatize( "карюзный" ) );
          if ( REQUIRE( !lemmas.empty() ) )
          {
            REQUIRE( std::find_if( lemmas.begin(), lemmas.end(), CheckPsp( 25 ) ) != lemmas.end() );
          }
        }
        SECTION( "∙ different forms of unknown word produce similar lexemes" )
        {
          auto  l1 = decltype(mlfaMb->Lemmatize( "" ))();
          auto  l2 = decltype(mlfaMb->Lemmatize( "" ))();
          auto  l3 = decltype(mlfaMb->Lemmatize( "" ))();

          REQUIRE_NOTHROW( l1 = mlfaMb->Lemmatize( "частосто" ) );
          REQUIRE_NOTHROW( l2 = mlfaMb->Lemmatize( "частосту" ) );

          if ( REQUIRE( !l1.empty() ) && REQUIRE( !l2.empty() ) )
            REQUIRE( (l1 & l2).size() != 0 );

          REQUIRE_NOTHROW( l1 = mlfaMb->Lemmatize( "командировочка" ) );
          REQUIRE_NOTHROW( l2 = mlfaMb->Lemmatize( "командировочку" ) );
          REQUIRE_NOTHROW( l3 = mlfaMb->Lemmatize( "командировочек" ) );

          if ( REQUIRE( !l1.empty() )
            && REQUIRE( !l2.empty() )
            && REQUIRE( !l3.empty() ) )
          {
            REQUIRE( (l1 & l2).size() != 0 );
            REQUIRE( (l2 & l3).size() != 0 );
            REQUIRE( (l3 & l1).size() != 0 );
          }
        }
        SECTION( "∙ Mbcs and Wcs api provide same classes and forms" )
        {
          auto  l1 = decltype(mlfaMb->Lemmatize( "" ))();
          auto  l2 = decltype(mlfaWc->Lemmatize( nullptr ))();

          REQUIRE_NOTHROW( l1 = mlfaMb->Lemmatize( "частосто" ) );
          REQUIRE_NOTHROW( l2 = mlfaWc->Lemmatize( codepages::mbcstowide( codepages::codepage_utf8, "частосто" ) ) );

          if ( REQUIRE( !l1.empty() )
            && REQUIRE( !l2.empty() )
            && REQUIRE( l1.size() == l2.size() ) )
          {
            for ( auto i = 0; i != l1.size(); ++i )
            {
              REQUIRE( l1[i].nclass == l2[i].nclass );
              REQUIRE( l1[i].ccstem == l2[i].ccstem );
              REQUIRE( l1[i].ngrams == l2[i].ngrams );
            }
          }
          REQUIRE_NOTHROW( l1 = mlfaMb->Lemmatize( "частосту", sfIgnoreCapitals ) );
          REQUIRE_NOTHROW( l2 = mlfaWc->Lemmatize( codepages::mbcstowide( codepages::codepage_utf8, "частосту" ), sfIgnoreCapitals ) );

          if ( REQUIRE( !l1.empty() )
            && REQUIRE( !l2.empty() )
            && REQUIRE( l1.size() == l2.size() ) )
          {
            for ( auto i = 0; i != l1.size(); ++i )
            {
              REQUIRE( l1[i].nclass == l2[i].nclass );
              REQUIRE( l1[i].ccstem == l2[i].ccstem );
              REQUIRE( l1[i].ngrams == l2[i].ngrams );
            }
          }
        }
      }
      SECTION( "BuildForm" )
      {
      }
      SECTION( "FindMatch" )
      {
        SECTION( "∙ invalid arguments result ARGUMENT_FAILED" )
        {
          REQUIRE( mlfaMb->FindMatch( (IMatchStemMb*)nullptr, "a" ) == ARGUMENT_FAILED );
          REQUIRE( mlfaMb->FindMatch( (IMatchStemMb*)0x12345, nullptr ) == ARGUMENT_FAILED );
        }
        SECTION( "∙ empty string results no matches" )
        {
          REQUIRE( mlfaMb->FindMatch( (IMatchStemMb*)0x12345, "", 0 ) == 0 );
        }
        SECTION( "∙ too long string results WORDBUFF_FAILED" )
        {
          REQUIRE( mlfaMb->FindMatch( (IMatchStemMb*)0x12345, std::string( 1024, 'a' ) ) == WORDBUFF_FAILED );
        }
        SECTION( "∙ ? matches any one character" )
        {
          SECTION( "- in flexions it produces real word forms" )
          {
            CMatchFormsList list;

            REQUIRE_NOTHROW( mlfaMb->FindMatch( &list, "хорош?ми" ) );
            REQUIRE( list.Get() == "хорошами, хорошеми, хорошими, хорошоми" );
          }
          SECTION( "- in stems it produces template with ?" )
          {
            CMatchFormsList list;

            REQUIRE_NOTHROW( mlfaMb->FindMatch( &list, "х?рошими" ) );
            REQUIRE( list.Get() == "х?рошими" );
          }
          SECTION( "- multiple ? match multiple characters..." )
          {
            CMatchFormsList list;

            REQUIRE_NOTHROW( mlfaMb->FindMatch( &list, "хорош??и" ) );
            REQUIRE( list.Get() ==
              "хорошави, хорошаги, хорошади, хорошайи, хорошали, хорошами, хорошани, хорошапи, "
              "хорошари, хорошати, хорошеби, хорошеви, хорошеги, хорошеди, хорошеки, хорошели, "
              "хорошеми, хорошени, хорошери, хорошеси, хорошети, хорошиби, хорошиви, хорошили, "
              "хорошими, хорошини, хорошипи, хорошири, хорошиси, хорошити, хорошихи, хорошкаи, "
              "хорошкеи, хорошкии, хорошкои, хорошкуи, хорошлаи, хорошлеи, хорошлии, хорошлои, "
              "хорошнеи, хорошнии, хорошнои, хорошнуи, хорошняи, хорошоги, хорошоки, хорошоми" );
          }
          SECTION( "... or words with wildcard" )
          {
            CMatchFormsList list;

            REQUIRE_NOTHROW( mlfaMb->FindMatch( &list, "х?роше?о" ) );
            REQUIRE( list.Get() ==
              "х?рошебо, х?рошево, х?рошего, х?рошеко, х?рошело, х?рошено, х?рошеро, х?рошесо, х?рошето" );
          }
        }
        SECTION( "∙ * matches 0 and more characters" )
        {
          SECTION( "- at the beginning of string it appears in the result" )
          {
            CMatchFormsList list;

            REQUIRE_NOTHROW( mlfaMb->FindMatch( &list, "х*роший" ) );
            REQUIRE( list.Get() == "х*роший" );
          }
          SECTION( "- in the middle it occurs both as wildcard and not" )
          {
            CMatchClassList list;

            REQUIRE_NOTHROW( mlfaMb->FindMatch( &list, "хоро*ий" ) );
          }
        }
      }
    }
  }
} );

int   main()
{
  return TestItEasy::Conclusion();
}
