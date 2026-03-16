/******************************************************************************

    libfuzzyrus - fuzzy morphological analyser for Russian.

    Copyright (c) 1994-2026 Andrew Kovalenko aka Keva

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

    Commercial license is available upon request.

    Contacts:
      email: keva@rambler.ru
      Phone: +7(495)648-4058, +7(926)513-2991, +7(707)129-1418

******************************************************************************/
# include <api.hpp>
# include <rus.h>
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

auto  operator & (
  const std::vector<IMlfaMbXX::lexeme>& v1,
  const std::vector<IMlfaMbXX::lexeme>& v2 ) -> std::vector<IMlfaMbXX::lexeme>
{
  std::vector<IMlfaMbXX::lexeme>  output;

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
          s1.plemma,
          s1.ccstem,
          s1.nclass, grlist, ngrams, s1.weight } );
      }

  return output;
}

template <class T>
auto  SubVector( const std::vector<T>& s, size_t pos, size_t n ) -> std::vector<T>
{
  if ( pos < s.size() ) n = std::min( pos + n, s.size() ) - pos;
    else return {};

  return { s.begin() + pos, s.begin() + pos + n };
}

TestItEasy::RegisterFunc  testmorphrusmb( []()
{
  TEST_CASE( "fuzzy/rus" )
  {
    IMlfaMbXX*  mlfaMb;
    IMlfaWcXX*  mlfaWc;

    SECTION( "fuzzy morphology may be created for native codepage (1251), unicode-16 and a set of codepages supported" )
    {
      SECTION( "∙ native (1251) is created anyway" )
      {
        REQUIRE( mlfaruGetAPI( LIBFUZZY_API_4_MAGIC ":" "windows-1251", (void**)&(mlfaMb = nullptr) ) == 0 );
        if ( REQUIRE( mlfaMb != nullptr ) )
          mlfaMb->Detach();
      }
      SECTION( "∙ named codepages are created also" )
      {
        REQUIRE( mlfaruGetAPI( LIBFUZZY_API_4_MAGIC ":" "koi8", (void**)&(mlfaMb = nullptr) ) == 0 );
        if ( REQUIRE( mlfaMb != nullptr ) )
          mlfaMb->Detach();
        REQUIRE( mlfaruGetAPI( LIBFUZZY_API_4_MAGIC ":" "utf8", (void**)&(mlfaMb = nullptr) ) == 0 );
        if ( REQUIRE( mlfaMb != nullptr ) )
          mlfaMb->Detach();
      }
      SECTION( "∙ utf16 is created" )
      {
        REQUIRE( mlfaruGetAPI( LIBFUZZY_API_4_MAGIC ":" "utf-16", (void**)&(mlfaWc = nullptr) ) == 0 );
        if ( REQUIRE( mlfaWc != nullptr ) )
          mlfaWc->Detach();
      }
      SECTION( "∙ called with invalid codepage names it return EINVAL" )
      {
        REQUIRE( mlfaruGetAPI( LIBFUZZY_API_4_MAGIC ":" "unknown codepage", (void**)&(mlfaMb = nullptr) ) == EINVAL );
      }
    }

    if ( !REQUIRE( mlfaruGetAPI( LIBFUZZY_API_4_MAGIC ":" "utf8", (void**)&mlfaMb ) == 0 ) )
      abort();
    if ( !REQUIRE( mlfaruGetAPI( LIBFUZZY_API_4_MAGIC ":" "utf-16", (void**)&mlfaWc ) == 0 ) )
      abort();

    SECTION( "GetWdInfo" )
    {
      uint8_t wdinfo;

      REQUIRE( mlfaMb->GetWdInfo( nullptr, 0 ) == ARGUMENT_FAILED );
      REQUIRE( mlfaMb->GetWdInfo( &wdinfo, 1000 ) == ARGUMENT_FAILED );
      REQUIRE( mlfaMb->GetWdInfo( 0 ) == 25 );
    }
    SECTION( "GetModels" )
    {
      char  sample[256];

      REQUIRE( mlfaMb->GetModels( nullptr, 1, 0 ) == ARGUMENT_FAILED );
      REQUIRE( mlfaMb->GetModels( sample, 0, 0 ) == ARGUMENT_FAILED );
      REQUIRE( mlfaMb->GetModels( sample, 1000 ) == ARGUMENT_FAILED );
      if ( REQUIRE( mlfaMb->GetModels( sample, 0 ) > 0 ) )
        REQUIRE( sample == std::string( "пустынный" ) );
      REQUIRE( SubVector( mlfaMb->GetModels( 0 ), 0, 5 ) == std::vector<std::string>{
			  "пустынный",
        "правильный",
        "здоровый",
        "масличный",
        "грамотный" } );
    }
    SECTION( "Lemmatize" )
    {
      auto  lemmas = decltype(mlfaMb->Lemmatize( "" )){};

      SECTION( "∙ lemmatization of correct words creates lemmas" )
      {
        SECTION( "∙ lemmatization builds lemmas" )
        {
          REQUIRE_NOTHROW( lemmas = mlfaMb->Lemmatize( "криворожский", sfIgnoreCapitals ) );
          if ( REQUIRE( !lemmas.empty() ) )
          {
            for ( auto& next: lemmas )
              fprintf( stdout, "%4.2f\t%2d\t%2d\t%s\n",
                next.weight,
                next.nclass, next.pgrams->wdInfo, next.lemma.c_str() );
          }
        }
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
            for ( size_t i = 0; i != l1.size(); ++i )
            {
              REQUIRE( l1[i].nclass == l2[i].nclass );
              REQUIRE( l1[i].ngrams == l2[i].ngrams );
            }
          }
          REQUIRE_NOTHROW( l1 = mlfaMb->Lemmatize( "частосту", sfIgnoreCapitals ) );
          REQUIRE_NOTHROW( l2 = mlfaWc->Lemmatize( codepages::mbcstowide( codepages::codepage_utf8, "частосту" ), sfIgnoreCapitals ) );

          if ( REQUIRE( !l1.empty() )
            && REQUIRE( !l2.empty() )
            && REQUIRE( l1.size() == l2.size() ) )
          {
            for ( size_t i = 0; i != l1.size(); ++i )
            {
              REQUIRE( l1[i].nclass == l2[i].nclass );
              REQUIRE( l1[i].ngrams == l2[i].ngrams );
            }
          }
        }
      }
      SECTION( "BuildForm" )
      {
        auto  lemmas = mlfaMb->Lemmatize( "криворожский" );

        if ( REQUIRE( lemmas.size() != 0 )
          && REQUIRE( lemmas.front().pgrams->wdInfo == 25 ))
        {
          char  szform[0x40];
          int   nforms;

          if ( REQUIRE_NOTHROW( nforms = mlfaMb->BuildForm( szform, sizeof(szform),
            lemmas.front().lemma.c_str(), lemmas.front().ccstem, lemmas.front().nclass, 5 ) ) )
          {
            if ( REQUIRE( nforms == 1 ) )
              REQUIRE( szform == std::string( "криворожским" ) );
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
