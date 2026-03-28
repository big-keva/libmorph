/******************************************************************************

    libmorpheng - dictionary-based morphological analyser for English.

    Copyright (C) 1994-2025 Andrew Kovalenko aka Keva

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
      email: keva@meta.ua, keva@rambler.ru
      Phone: +7(495)648-4058, +7(926)513-2991, +7(707)129-1418

******************************************************************************/
# include "../../../eng.h"
# include "../../../api.hpp"
# include <xmorph/codepages.hpp>
# include <mtc/test-it-easy.hpp>
# include <algorithm>
# include <string>
# include <cstring>
# include <vector>
# include <cstdio>
# include <thread>

template <class CharType>
class MatchSet: public IMlmaMatch, public std::vector<std::basic_string<CharType>>
{
  static  auto  GetCharStr( const std::string& s ) -> const std::string&
  {
    return s;
  }
  static  auto  GetCharStr( const mtc::widestr& s ) -> std::string
  {
    return codepages::widetombcs( codepages::codepage_utf8, s );
  }
public:
  int   MLMAPROC  Attach() override {  return 1;  }
  int   MLMAPROC  Detach() override {  return 1;  }
  int   MLMAPROC  AddLexeme( lexeme_t nlexid, int  nmatch, const SStrMatch*  pmatch ) override
  {
    for ( auto beg = pmatch, end = pmatch + nmatch; beg != end; ++beg )
      std::vector<std::basic_string<CharType>>::emplace_back( (const CharType*)beg->sz );
    return 0;
  }
  auto  clear() -> IMlmaMatch*
  {
    std::vector<std::basic_string<CharType>>::clear();
    return this;
  }
public:
  auto  ToString() -> std::string
  {
    auto  out = std::string();
    auto  pre = (const char*)"";

    for ( auto& next: *this )
    {
      out += mtc::strprintf( "%s%s", pre, GetCharStr( next ).c_str() );
      pre = "/";
    }

    return out;
  }
};

TestItEasy::RegisterFunc  testmorpheng( []()
{
  TEST_CASE( "morph/eng" )
  {
    SECTION( "morphological analyser API may be created with different mbcs codepages" )
    {
      IMlmaMbXX*  mlma = nullptr;

      SECTION( "valid string keys give access to named API" )
      {
        std::string cnames[] = {
          "Windows-1251",
          "Windows",
          "1251",
          "Win-1251",
          "Win",
          "Windows 1251",
          "Win 1251",
          "ansi",
          "koi-8",
          "koi8",
          "20866",
          "dos",
          "oem",
          "866",
          "28595",
          "iso-88595",
          "iso-8859-5",
          "10007",
          "mac",
          "65001",
          "utf-8",
          "utf8"
        };

        for ( auto& cpname: cnames )
        {
          REQUIRE_NOTHROW( mlmaenGetAPI( (LIBMORPH_API_4_MAGIC ":" + cpname).c_str(), (void**)&mlma ) );
            REQUIRE( mlma != nullptr );
          REQUIRE_NOTHROW( mlma = mlma != nullptr ? (mlma->Detach(), nullptr) : nullptr  );
        }
      }
    }
    SECTION( "multibyte api processes words" )
    {
      IMlmaMbXX*  mlma = nullptr;

      mlmaenGetAPI( LIBMORPH_API_4_MAGIC ":" "utf-8", (void**)&mlma );

      SECTION( "CheckWord" )
      {
        SECTION( "called with exact BYTES length of string and no options, it recognizes word" )
        {
          REQUIRE( mlma->CheckWord( "check", mtc::w_strlen( "check" ), 0 ) == 1 );
        }
        SECTION( "called with exact BYTES length of string and no options, it does not recognize unknown word" )
        {
          REQUIRE( mlma->CheckWord( "ывал", mtc::w_strlen( "ывал" ), 0 ) == 0 );
          REQUIRE( mlma->CheckWord( "chek", mtc::w_strlen( "chek" ), 0 ) == 0 );
          REQUIRE( mlma->CheckWord( "checkerr", mtc::w_strlen( "checkerr" ), 0 ) == 0 );
        }
        SECTION( "called with exact BYTES length of string and no options, it does not recognize word with mixed capitals" )
        {
          REQUIRE( mlma->CheckWord( "VeRiFy", mtc::w_strlen( "VeRiFy" ), 0 ) == 0 );
        }
        SECTION( "called with exact BYTES length of string and 'sfIgnoreCapitals', it recognizes word with mixed capitals" )
        {
          REQUIRE( mlma->CheckWord( "VeRiFy", mtc::w_strlen( "VeRiFy" ), sfIgnoreCapitals ) == 1 );
        }
        SECTION( "called with unknown length and no options, it does not recognize word with mixed capitals" )
        {
          REQUIRE( mlma->CheckWord( "VeRiFiEd", (size_t)-1, 0 ) == 0 );
        }
        SECTION( "called with unknown length and 'sfIgnoreCapitals', it recognizes word with mixed capitals" )
        {
          REQUIRE( mlma->CheckWord( "VeRiFiEd", (size_t)-1, sfIgnoreCapitals ) == 1 );
        }
      }
      SECTION( "GetWdInfo" )
      {
        unsigned char wdinfo;

        SECTION( "it returns the technical part-of-speach for any lexeme" )
        {
          REQUIRE( mlma->GetWdInfo( &wdinfo, 46863 ) != 0 );
          REQUIRE( wdinfo == 1 );
        }
        SECTION( "for invalid lexemes, it returns 0" )
        {
          REQUIRE( mlma->GetWdInfo( &wdinfo, 0 ) == 0 );
        }
      }
      SECTION( "Lemmatize" )
      {
        SLemmInfoA  alemms[0x20];
        char        aforms[0xf0];
        SGramInfo   agrams[0x40];

        SECTION( "unknown words are not lemmatized with any parameters" )
        {
          REQUIRE( mlma->Lemmatize( "f", -1, nullptr, 0, nullptr, 0, nullptr, 0, 0 ) == 0 );
          REQUIRE( mlma->Lemmatize( "aaa", -1, nullptr, 0, nullptr, 0, nullptr, 0, 0 ) == 0 );
          REQUIRE( mlma->Lemmatize( "ccode", -1, nullptr, 0, nullptr, 0, nullptr, 0, 0 ) == 0 );

          REQUIRE( mlma->Lemmatize( "f", alemms, aforms, agrams ) == 0 );
          REQUIRE( mlma->Lemmatize( "aaa", alemms, aforms, agrams ) == 0 );
          REQUIRE( mlma->Lemmatize( "ccode", alemms, aforms, agrams ) == 0 );
        }
        SECTION( "lemmatization without strings gets lemmas" )
        {
          if ( REQUIRE( mlma->Lemmatize( "check", -1, alemms, 0x20, nullptr, 0, nullptr, 0, 0 ) == 3 ) )
          {
            REQUIRE( std::find_if( alemms, alemms + 3, []( const SLemmInfoA& lemm )
              {  return lemm.nlexid == 4356;  } ) != alemms + 3 );
            REQUIRE( std::find_if( alemms, alemms + 3, []( const SLemmInfoA& lemm )
              {  return lemm.nlexid == 46863;  } ) != alemms + 3 );
            REQUIRE( std::find_if( alemms, alemms + 3, []( const SLemmInfoA& lemm )
              {  return lemm.nlexid == 108063;  } ) != alemms + 3 );
          }
        }
        SECTION( "lemmatization without grammatical descriptions just builds forms" )
        {
          if ( REQUIRE( mlma->Lemmatize( "check", -1, nullptr, 0, aforms, 0xf0, nullptr, 0, 0 ) == 3 ) )
          {
            REQUIRE( mtc::w_strcmp( aforms, "check" ) == 0 );
            REQUIRE( mtc::w_strcmp( aforms + 6, "check" ) == 0 );
            REQUIRE( mtc::w_strcmp( aforms + 12, "check" ) == 0 );
          }
        }
        SECTION( "complete lemmatization builds strings, gets lemmas and grammatical descriptions" )
        {
          REQUIRE( mlma->Lemmatize( "check",
            alemms,
            aforms,
            agrams ) == 3 );
        }
        SECTION( "overflows result errors:" )
        {
          SECTION( "LIDSBUFF_FAILED with not enough buffer for lemmas" )
          {
            REQUIRE( mlma->Lemmatize( "check", -1,
                alemms, 0x02,
                aforms, 0xf0,
                agrams, 0x40, 0 ) == LIDSBUFF_FAILED );
            REQUIRE( mlma->Lemmatize( "check",
              { alemms, 0x02 },
              { aforms, 0xf0 },
              { agrams, 0x40 }, 0 ) == LIDSBUFF_FAILED );
            REQUIRE( mlma->Lemmatize( "check",
              { alemms, 0x02 },
                aforms,
                agrams ) == LIDSBUFF_FAILED );
          }
          SECTION( "LEMMBUFF_FAILED with not enough buffer for froms" )
          {
            REQUIRE( mlma->Lemmatize( "check",
                alemms,
              { aforms, 0xf },
                agrams ) == LEMMBUFF_FAILED );
          }
          SECTION( "GRAMBUFF_FAILED with not enough buffer for grammas" )
          {
            REQUIRE( mlma->Lemmatize( "check",
                alemms,
                aforms,
              { agrams, 0x02 } ) == GRAMBUFF_FAILED );
          }
        }
        SECTION( "by default it is case-sensitive" )
        {
          REQUIRE( mlma->Lemmatize( "andrew",
              alemms,
              aforms,
              agrams ) == 0 );
          REQUIRE( mlma->Lemmatize( "Andrew",
              alemms,
              aforms,
              agrams ) == 1 );
        }
        SECTION( "with sfIgnoreCapitals it is case-insensitive" )
        {
          REQUIRE( mlma->Lemmatize( "andrew",
              alemms,
              aforms,
              agrams, sfIgnoreCapitals ) == 1 );
        }
        SECTION( "plural nouns are lemmatized to plural nominative" )
        {
          auto  lemmas = decltype( mlma->Lemmatize( "" ) )();

          REQUIRE_NOTHROW( lemmas = mlma->Lemmatize( "scissors" ) );

          if ( REQUIRE( lemmas.size() == 2 ) )
          {
            REQUIRE( std::string( lemmas[0].plemma ) == "scissor" );
            REQUIRE( std::string( lemmas[1].plemma ) == "scissors" );
          }
        }
        SECTION( "irregular forms are also processed" )
        {
          auto  lemmas = decltype( mlma->Lemmatize( "" ) )();

          if ( REQUIRE( (lemmas = mlma->Lemmatize( "man" )).size() == 2 ) )
          {
            REQUIRE( std::string( lemmas[0].plemma ) == "man" );
            REQUIRE(             (lemmas[0].pgrams->wdInfo & 0x3f) == 2 );    // V
            REQUIRE( std::string( lemmas[1].plemma ) == "man" );
            REQUIRE(             (lemmas[1].pgrams->wdInfo & 0x3f) == 1 );    // N
          }
          if ( REQUIRE( (lemmas = mlma->Lemmatize( "men" )).size() == 1 ) )
          {
            REQUIRE( std::string( lemmas[0].plemma ) == "man" );
            REQUIRE(             (lemmas[0].pgrams->wdInfo & 0x3f) == 1 );    // N
          }
          if ( REQUIRE( (lemmas = mlma->Lemmatize( "woman" )).size() == 2 ) )
          {
            REQUIRE( std::string( lemmas[0].plemma ) == "woman" );
            REQUIRE(             (lemmas[0].pgrams->wdInfo & 0x3f) == 2 );    // V
            REQUIRE( std::string( lemmas[1].plemma ) == "woman" );
            REQUIRE(             (lemmas[1].pgrams->wdInfo & 0x3f) == 1 );    // N
          }
          if ( REQUIRE( (lemmas = mlma->Lemmatize( "women" )).size() == 1 ) )
          {
            REQUIRE( std::string( lemmas[0].plemma ) == "woman" );
            REQUIRE(             (lemmas[0].pgrams->wdInfo & 0x3f) == 1 );    // N
          }
        }
        SECTION( "forms with postfixes are analyzed and built" )
        {
          auto  lemmas = decltype( mlma->Lemmatize( "" ) )();

          REQUIRE_NOTHROW( lemmas = mlma->Lemmatize( "fathers-in-law" ) );

          if ( REQUIRE( lemmas.size() == 1 ) )
            REQUIRE( std::string( lemmas.front().plemma ) == "father-in-law" );
        }
        SECTION( "dictionary form is built in minimal correct capitalization" )
        {
          if ( REQUIRE( mlma->Lemmatize( "Andrew",
            alemms,
            aforms,
            agrams ) == 1 ) )
          {
            REQUIRE( std::string( aforms ) == "Andrew" );
          }

          REQUIRE( mlma->Lemmatize( "andrew",
            alemms,
            aforms,
            agrams ) == 0 );

          if ( REQUIRE( mlma->Lemmatize( "andrew",
            alemms,
            aforms,
            agrams, sfIgnoreCapitals ) == 1 ) )
          {
            REQUIRE( std::string( aforms ) == "Andrew" );
          }
        }
        SECTION( "verb BE has irregular form list" )
        {
          auto  lemmas = decltype( mlma->Lemmatize( "" ) )();

          REQUIRE_NOTHROW( lemmas = mlma->Lemmatize( "was" ) );

          if ( REQUIRE( lemmas.size() == 1 ) )
            REQUIRE( std::string( lemmas.front().plemma ) == "be" );
        }
        SECTION( "cxx wrapper also provides lexemes" )
        {
          auto  lemmas = mlma->Lemmatize( "human" );

          if ( REQUIRE( lemmas.size() == 2 ) )
          {
            REQUIRE( std::string( lemmas[0].plemma ) == "human" );
            REQUIRE( std::string( lemmas[1].plemma ) == "human" );
          }
        }
      }
      SECTION( "BuildForm" )
      {
        char      aforms[0xf0];
        lexeme_t  lVerbBe = 141312;
        lexeme_t  lAndrew = 131093;
        lexeme_t  lFather = 58586;

        SECTION( "being called with empty buffer, it returns invalid argument" )
          {  REQUIRE( mlma->BuildForm( nullptr, 0xf0, 5, 0 ) == ARGUMENT_FAILED );  }

        SECTION( "being called with invalid lexeme, it returns 0" )
          {  REQUIRE( mlma->BuildForm( aforms, 5, 0 ) == 0 );  }

        SECTION( "being called with small buffer and correct lexeme, it returns LEMMBUFF_FAILED" )
          {  REQUIRE( mlma->BuildForm( aforms, 0x03, lVerbBe, 5 ) == LEMMBUFF_FAILED );  }

        SECTION( "with correct buffer, it builds forms" )
        {
          auto  out = decltype(mlma->BuildForm( lVerbBe, 5 ))();

          REQUIRE_NOTHROW( out = mlma->BuildForm( lVerbBe, 5 ) );

          if ( REQUIRE( out.size() == 1 ) )
            REQUIRE( out[0] == "were" );
        }

        SECTION( "for non-flective words, it builds form '0xff'" )
        {
          REQUIRE( mlma->BuildForm( aforms, 24840 /* substitutional */, 1 ) == 0 );
          REQUIRE( mlma->BuildForm( aforms, 24840 /* substitutional */, 0xff ) == 1 );
          REQUIRE( std::string( aforms ) ==  "substitutional" );
        }

        SECTION( "for personal names, it builds minimal valid capitalization" )
        {
          REQUIRE( mlma->BuildForm( aforms, lAndrew, 1 ) == 1 );
          REQUIRE( std::string( aforms ) == "Andrew's" );
        }

        SECTION( "for suffixed words, it appends suffix" )
        {
          REQUIRE( mlma->BuildForm( aforms, lFather, 1 ) == 1 );
          REQUIRE( std::string( aforms ) == "fathers-in-law" );
        }

        SECTION( "for plural-only words it builds plural forms only" )
        {
          if ( REQUIRE( mlma->BuildForm( 88893, 1 ).size() != 0 ) )
            REQUIRE( mlma->BuildForm( 88893, 1 ).front() == "scissors" );

          REQUIRE( mlma->BuildForm( 88893, 0 ).size() == 0 );
        }
      }
      SECTION( "FindForms" )
      {
        char  aforms[0x100];

        SECTION( "with invalid arguments it returns ARGUMENT_INVALID" )
        {
          REQUIRE( mlma->FindForms( nullptr, 0xff, "word", (size_t)-1, 1, 0 ) == ARGUMENT_FAILED );
        }
        SECTION( "with empty string it returns 0" )
        {
          REQUIRE( mlma->FindForms( aforms, { nullptr, 10 }, 1 ) == 0 );
          REQUIRE( mlma->FindForms( aforms, "", 1 ) == 0 );
        }
        SECTION( "for unknown words it returns 0" )
        {
          REQUIRE( mlma->FindForms( aforms, "aaa", 0 ) == 0 );
        }
        SECTION( "for known words it returns count of forms build" )
        {
          auto  out = decltype( mlma->FindForms( "market", 1 ) )();

          REQUIRE_NOTHROW( out = mlma->FindForms( "market", 1 ) );

          if ( REQUIRE( out.size() == 2 ) )
          {
            REQUIRE( out[0] == "markets" );
            REQUIRE( out[1] == "markets" );
          }
        }
        SECTION( "for non-flective words, it builds form '0xff'" )
        {
          REQUIRE( mlma->FindForms( aforms, "marriable", 0 ) == 0 );
          REQUIRE( mlma->FindForms( aforms, "marriable", 0xff ) == 1 );
          REQUIRE( std::string( aforms ) ==  "marriable" );
        }

        SECTION( "for personal names, it builds minimal valid capitalization" )
        {
          REQUIRE( mlma->FindForms( aforms, "andrew", 0 ) == 0 );
          REQUIRE( mlma->FindForms( aforms, "andrew", 1, sfIgnoreCapitals ) == 1 );
          REQUIRE( std::string( aforms ) == "Andrew's" );
        }

        SECTION( "for suffixed words, it appends suffix" )
        {
          REQUIRE( mlma->FindForms( aforms, "daughters-in-law", 0 ) == 1 );
          REQUIRE( std::string( aforms ) == "daughter-in-law" );
        }
      }
      SECTION( "FindMatch" )
      {
        auto  ms = MatchSet<char>();

        SECTION( "it checks arguments" )
        {
          SECTION( "called with invalid arguments, it returns ARGUMENT_FAILED " )
            {  REQUIRE( mlma->FindMatch( nullptr, nullptr, 0 ) == ARGUMENT_FAILED );  }
          SECTION( "with too long string, it returns WORDBUFF_FAILED" )
            {  REQUIRE( mlma->FindMatch( &ms, std::string( 257, 'a' ).c_str() ) == WORDBUFF_FAILED );  }
        }
        SECTION( "it loads forms by template" )
        {
          SECTION( "? matches any single character:" )
          {
            SECTION( "∙ in non-flective words;" )
            {
              REQUIRE_NOTHROW( mlma->FindMatch( ms.clear(), "y?man" ) );

              if ( REQUIRE( ms.size() == 1 ) )
                REQUIRE( ms.ToString() == "yuman" );

              REQUIRE_NOTHROW( mlma->FindMatch( ms.clear(), "yuma?" ) );

              if ( REQUIRE( ms.size() == 2 ) )
                REQUIRE( ms.ToString() == "yuman/yumas" );
            }
            SECTION( "∙ in stems of flective words;" )
            {
              REQUIRE_NOTHROW( mlma->FindMatch( ms.clear(), "exc?ptions" ) );

              if ( REQUIRE( ms.size() == 1 ) )
                REQUIRE( ms.ToString() == "exceptions" );
            }
            SECTION( "∙ in flexions of flective words;" )
            {
              REQUIRE_NOTHROW( mlma->FindMatch( ms.clear(), "exception?" ) );

              if ( REQUIRE( ms.size() == 1 ) )
                REQUIRE( ms.ToString() == "exceptions" );
            }
            SECTION( "∙ in flexions of words with zero stem;" )
            {
              REQUIRE_NOTHROW( mlma->FindMatch( ms.clear(), "wome?" ) );

              if ( REQUIRE( ms.size() == 1 ) )
                REQUIRE( ms.ToString() == "women" );
            }
            SECTION( "∙ in postfixes;" )
            {
              REQUIRE_NOTHROW( mlma->FindMatch( ms.clear(), "brothers-i?-law" ) );

              if ( REQUIRE( ms.size() == 1 ) )
                REQUIRE( ms.ToString() == "brothers-in-law" );
            }
          }
          SECTION( "* matches any sequence of characters" )
          {
            SECTION( "∙ in non-flective words;" )
            {
              REQUIRE_NOTHROW( mlma->FindMatch( ms.clear(), "analogo*ly" ) );

              if ( REQUIRE( ms.size() == 1 ) )
                REQUIRE( ms.ToString() == "analogously" );
            }
            SECTION( "∙ in flective words;" )
            {
              REQUIRE_NOTHROW( mlma->FindMatch( ms.clear(), "excitabilities*" ) );

              if ( REQUIRE( ms.size() == 2 ) )
                REQUIRE( ms.ToString() == "excitabilities/excitabilities'" );
            }
            SECTION( "∙ in postfixes;" )
            {
              REQUIRE_NOTHROW( mlma->FindMatch( ms.clear(), "brother-in-law*" ) );

              if ( REQUIRE( ms.size() == 1 ) )
                REQUIRE( ms.ToString() == "brother-in-law" );
            }
          }
        }
      }
    }

    SECTION( "widechar api also processes words" )
    {
      IMlmaWcXX*  mlma = nullptr;

      mlmaenGetAPI( LIBMORPH_API_4_MAGIC ":" "utf-16", (void**)&mlma );

      SECTION( "CheckWord" )
      {
        SECTION( "called with exact BYTES length of string and no options, it recognizes word" )
        {
          REQUIRE( mlma->CheckWord( u"check", mtc::w_strlen( u"check" ), 0 ) == 1 );
        }
        SECTION( "called with exact BYTES length of string and no options, it does not recognize unknown word" )
        {
          REQUIRE( mlma->CheckWord( u"ывал",      mtc::w_strlen( u"ывал" ), 0 ) == 0 );
          REQUIRE( mlma->CheckWord( u"chek",      mtc::w_strlen( u"chek" ), 0 ) == 0 );
          REQUIRE( mlma->CheckWord( u"checkerr",  mtc::w_strlen( u"checkerr" ), 0 ) == 0 );
        }
        SECTION( "called with exact BYTES length of string and no options, it does not recognize word with mixed capitals" )
        {
          REQUIRE( mlma->CheckWord( u"VeRiFy", mtc::w_strlen( u"VeRiFy" ), 0 ) == 0 );
        }
        SECTION( "called with exact BYTES length of string and 'sfIgnoreCapitals', it recognizes word with mixed capitals" )
        {
          REQUIRE( mlma->CheckWord( u"VeRiFy", mtc::w_strlen( "uVeRiFy" ), sfIgnoreCapitals ) == 1 );
        }
        SECTION( "called with unknown length and no options, it does not recognize word with mixed capitals" )
        {
          REQUIRE( mlma->CheckWord( u"VeRiFiEd", 0 ) == 0 );
        }
        SECTION( "called with unknown length and 'sfIgnoreCapitals', it recognizes word with mixed capitals" )
        {
          REQUIRE( mlma->CheckWord( u"VeRiFiEd", sfIgnoreCapitals ) == 1 );
        }
      }
      SECTION( "GetWdInfo" )
      {
        unsigned char wdinfo;

        SECTION( "it returns the technical part-of-speach for any lexeme" )
        {
          REQUIRE( mlma->GetWdInfo( &wdinfo, 46863 ) != 0 );
          REQUIRE( wdinfo == 1 );
        }
        SECTION( "for invalid lexemes, it returns 0" )
        {
          REQUIRE( mlma->GetWdInfo( &wdinfo, 0 ) == 0 );
        }
      }
      SECTION( "Lemmatize" )
      {
        SLemmInfoW  alemms[0x20];
        widechar    aforms[0xf0];
        SGramInfo   agrams[0x40];

        SECTION( "unknown words are not lemmatized with any parameters" )
        {
          REQUIRE( mlma->Lemmatize( u"f", -1, nullptr, 0, nullptr, 0, nullptr, 0, 0 ) == 0 );
          REQUIRE( mlma->Lemmatize( u"aaa", -1, nullptr, 0, nullptr, 0, nullptr, 0, 0 ) == 0 );
          REQUIRE( mlma->Lemmatize( u"ccode", -1, nullptr, 0, nullptr, 0, nullptr, 0, 0 ) == 0 );

          REQUIRE( mlma->Lemmatize( u"f", alemms, aforms, agrams ) == 0 );
          REQUIRE( mlma->Lemmatize( u"aaa", alemms, aforms, agrams ) == 0 );
          REQUIRE( mlma->Lemmatize( u"ccode", alemms, aforms, agrams ) == 0 );
        }
        SECTION( "lemmatization without strings gets lemmas" )
        {
          if ( REQUIRE( mlma->Lemmatize( u"check", -1, alemms, 0x20, nullptr, 0, nullptr, 0, 0 ) == 3 ) )
          {
            REQUIRE( std::find_if( alemms, alemms + 3, []( const SLemmInfoW& lemm )
              {  return lemm.nlexid == 4356;  } ) != alemms + 3 );
            REQUIRE( std::find_if( alemms, alemms + 3, []( const SLemmInfoW& lemm )
              {  return lemm.nlexid == 46863;  } ) != alemms + 3 );
            REQUIRE( std::find_if( alemms, alemms + 3, []( const SLemmInfoW& lemm )
              {  return lemm.nlexid == 108063;  } ) != alemms + 3 );
          }
        }
        SECTION( "lemmatization without grammatical descriptions just builds forms" )
        {
          if ( REQUIRE( mlma->Lemmatize( u"check", -1, nullptr, 0, aforms, 0xf0, nullptr, 0, 0 ) == 3 ) )
          {
            REQUIRE( mtc::w_strcmp( aforms, "check" ) == 0 );
            REQUIRE( mtc::w_strcmp( aforms + 6, "check" ) == 0 );
            REQUIRE( mtc::w_strcmp( aforms + 12, "check" ) == 0 );
          }
        }
        SECTION( "complete lemmatization builds strings, gets lemmas and grammatical descriptions" )
        {
          REQUIRE( mlma->Lemmatize( u"check",
            alemms,
            aforms,
            agrams ) == 3 );
        }
        SECTION( "overflows result errors:" )
        {
          SECTION( "LIDSBUFF_FAILED with not enough buffer for lemmas" )
          {
            REQUIRE( mlma->Lemmatize( u"check", -1,
                alemms, 0x02,
                aforms, 0xf0,
                agrams, 0x40, 0 ) == LIDSBUFF_FAILED );
            REQUIRE( mlma->Lemmatize( u"check",
              { alemms, 0x02 },
              { aforms, 0xf0 },
              { agrams, 0x40 }, 0 ) == LIDSBUFF_FAILED );
            REQUIRE( mlma->Lemmatize( u"check",
              { alemms, 0x02 },
                aforms,
                agrams ) == LIDSBUFF_FAILED );
          }
          SECTION( "LEMMBUFF_FAILED with not enough buffer for froms" )
          {
            REQUIRE( mlma->Lemmatize( u"check",
                alemms,
              { aforms, 0xf },
                agrams ) == LEMMBUFF_FAILED );
          }
          SECTION( "GRAMBUFF_FAILED with not enough buffer for grammas" )
          {
            REQUIRE( mlma->Lemmatize( u"check",
                alemms,
                aforms,
              { agrams, 0x02 } ) == GRAMBUFF_FAILED );
          }
        }
        SECTION( "by default it is case-sensitive" )
        {
          REQUIRE( mlma->Lemmatize( u"andrew",
              alemms,
              aforms,
              agrams ) == 0 );
          REQUIRE( mlma->Lemmatize( u"Andrew",
              alemms,
              aforms,
              agrams ) == 1 );
        }
        SECTION( "with sfIgnoreCapitals it is case-insensitive" )
        {
          REQUIRE( mlma->Lemmatize( u"andrew",
              alemms,
              aforms,
              agrams, sfIgnoreCapitals ) == 1 );
        }
        SECTION( "plural nouns are lemmatized to plural nominative" )
        {
          auto  lemmas = decltype( mlma->Lemmatize( u"" ) )();

          REQUIRE_NOTHROW( lemmas = mlma->Lemmatize( u"scissors" ) );

          if ( REQUIRE( lemmas.size() == 2 ) )
          {
            REQUIRE( mtc::widestr( lemmas[0].plemma ) == u"scissor" );
            REQUIRE( mtc::widestr( lemmas[1].plemma ) == u"scissors" );
          }
        }
        SECTION( "irregular forms are also processed" )
        {
          auto  lemmas = decltype( mlma->Lemmatize( u"" ) )();

          if ( REQUIRE( (lemmas = mlma->Lemmatize( u"man" )).size() == 2 ) )
          {
            REQUIRE( mtc::widestr( lemmas[0].plemma ) == u"man" );
            REQUIRE(              (lemmas[0].pgrams->wdInfo & 0x3f) == 2 );    // V
            REQUIRE( mtc::widestr( lemmas[1].plemma ) == u"man" );
            REQUIRE(              (lemmas[1].pgrams->wdInfo & 0x3f) == 1 );    // N
          }
          if ( REQUIRE( (lemmas = mlma->Lemmatize( u"men" )).size() == 1 ) )
          {
            REQUIRE( mtc::widestr( lemmas[0].plemma ) == u"man" );
            REQUIRE(              (lemmas[0].pgrams->wdInfo & 0x3f) == 1 );    // N
          }
          if ( REQUIRE( (lemmas = mlma->Lemmatize( u"woman" )).size() == 2 ) )
          {
            REQUIRE( mtc::widestr( lemmas[0].plemma ) == u"woman" );
            REQUIRE(              (lemmas[0].pgrams->wdInfo & 0x3f) == 2 );    // V
            REQUIRE( mtc::widestr( lemmas[1].plemma ) == u"woman" );
            REQUIRE(              (lemmas[1].pgrams->wdInfo & 0x3f) == 1 );    // N
          }
          if ( REQUIRE( (lemmas = mlma->Lemmatize( u"women" )).size() == 1 ) )
          {
            REQUIRE( mtc::widestr( lemmas[0].plemma ) == u"woman" );
            REQUIRE(              (lemmas[0].pgrams->wdInfo & 0x3f) == 1 );    // N
          }
        }
        SECTION( "forms with postfixes are analyzed and built" )
        {
          auto  lemmas = decltype( mlma->Lemmatize( u"" ) )();

          REQUIRE_NOTHROW( lemmas = mlma->Lemmatize( u"fathers-in-law" ) );

          if ( REQUIRE( lemmas.size() == 1 ) )
            REQUIRE( mtc::widestr( lemmas.front().plemma ) == u"father-in-law" );
        }
        SECTION( "dictionary form is built in minimal correct capitalization" )
        {
          if ( REQUIRE( mlma->Lemmatize( u"Andrew",
            alemms,
            aforms,
            agrams ) == 1 ) )
          {
            REQUIRE( mtc::widestr( aforms ) == u"Andrew" );
          }

          REQUIRE( mlma->Lemmatize( u"andrew",
            alemms,
            aforms,
            agrams ) == 0 );

          if ( REQUIRE( mlma->Lemmatize( u"andrew",
            alemms,
            aforms,
            agrams, sfIgnoreCapitals ) == 1 ) )
          {
            REQUIRE( mtc::widestr( aforms ) == u"Andrew" );
          }
        }
        SECTION( "verb BE has irregular form list" )
        {
          auto  lemmas = decltype( mlma->Lemmatize( u"" ) )();

          REQUIRE_NOTHROW( lemmas = mlma->Lemmatize( u"was" ) );

          if ( REQUIRE( lemmas.size() == 1 ) )
            REQUIRE( mtc::widestr( lemmas.front().plemma ) == u"be" );
        }
        SECTION( "cxx wrapper also provides lexemes" )
        {
          auto  lemmas = mlma->Lemmatize( u"human" );

          if ( REQUIRE( lemmas.size() == 2 ) )
          {
            REQUIRE( mtc::widestr( lemmas[0].plemma ) == u"human" );
            REQUIRE( mtc::widestr( lemmas[1].plemma ) == u"human" );
          }
        }
      }
      SECTION( "BuildForm" )
      {
        widechar  aforms[0xf0];
        lexeme_t  lVerbBe = 141312;
        lexeme_t  lAndrew = 131093;
        lexeme_t  lFather = 58586;

        SECTION( "being called with empty buffer, it returns invalid argument" )
          {  REQUIRE( mlma->BuildForm( nullptr, 0xf0, 5, 0 ) == ARGUMENT_FAILED );  }

        SECTION( "being called with invalid lexeme, it returns 0" )
          {  REQUIRE( mlma->BuildForm( aforms, 5, 0 ) == 0 );  }

        SECTION( "being called with small buffer and correct lexeme, it returns LEMMBUFF_FAILED" )
          {  REQUIRE( mlma->BuildForm( aforms, 0x03, lVerbBe, 5 ) == LEMMBUFF_FAILED );  }

        SECTION( "with correct buffer, it builds forms" )
        {
          auto  out = decltype(mlma->BuildForm( lVerbBe, 5 ))();

          REQUIRE_NOTHROW( out = mlma->BuildForm( lVerbBe, 5 ) );

          if ( REQUIRE( out.size() == 1 ) )
            REQUIRE( out[0] == u"were" );
        }

        SECTION( "for non-flective words, it builds form '0xff'" )
        {
          REQUIRE( mlma->BuildForm( aforms, 24840 /* substitutional */, 1 ) == 0 );
          REQUIRE( mlma->BuildForm( aforms, 24840 /* substitutional */, 0xff ) == 1 );
          REQUIRE( mtc::widestr( aforms ) == u"substitutional" );
        }

        SECTION( "for personal names, it builds minimal valid capitalization" )
        {
          REQUIRE( mlma->BuildForm( aforms, lAndrew, 1 ) == 1 );
          REQUIRE( mtc::widestr( aforms ) == u"Andrew's" );
        }

        SECTION( "for suffixed words, it appends suffix" )
        {
          REQUIRE( mlma->BuildForm( aforms, lFather, 1 ) == 1 );
          REQUIRE( mtc::widestr( aforms ) == u"fathers-in-law" );
        }

        SECTION( "for plural-only words it builds plural forms only" )
        {
          if ( REQUIRE( mlma->BuildForm( 88893, 1 ).size() != 0 ) )
            REQUIRE( mlma->BuildForm( 88893, 1 ).front() == u"scissors" );

          REQUIRE( mlma->BuildForm( 88893, 0 ).size() == 0 );
        }
      }
      SECTION( "FindForms" )
      {
        widechar  aforms[0x100];

        SECTION( "with invalid arguments it returns ARGUMENT_INVALID" )
        {
          REQUIRE( mlma->FindForms( nullptr, 0xff, u"word", (size_t)-1, 1, 0 ) == ARGUMENT_FAILED );
        }
        SECTION( "with empty string it returns 0" )
        {
          REQUIRE( mlma->FindForms( aforms, { nullptr, 10 }, 1 ) == ARGUMENT_FAILED );
          REQUIRE( mlma->FindForms( aforms, u"", 1 ) == 0 );
        }
        SECTION( "for unknown words it returns 0" )
        {
          REQUIRE( mlma->FindForms( aforms, u"aaa", 0 ) == 0 );
        }
        SECTION( "for known words it returns count of forms build" )
        {
          auto  out = decltype( mlma->FindForms( u"market", 1 ) )();

          REQUIRE_NOTHROW( out = mlma->FindForms( u"market", 1 ) );

          if ( REQUIRE( out.size() == 2 ) )
          {
            REQUIRE( out[0] == u"markets" );
            REQUIRE( out[1] == u"markets" );
          }
        }
        SECTION( "for non-flective words, it builds form '0xff'" )
        {
          REQUIRE( mlma->FindForms( aforms, u"marriable", 0 ) == 0 );
          REQUIRE( mlma->FindForms( aforms, u"marriable", 0xff ) == 1 );
          REQUIRE( mtc::widestr( aforms ) == u"marriable" );
        }

        SECTION( "for personal names, it builds minimal valid capitalization" )
        {
          REQUIRE( mlma->FindForms( aforms, u"andrew", 0 ) == 0 );
          REQUIRE( mlma->FindForms( aforms, u"andrew", 1, sfIgnoreCapitals ) == 1 );
          REQUIRE( mtc::widestr( aforms ) == u"Andrew's" );
        }

        SECTION( "for suffixed words, it appends suffix" )
        {
          REQUIRE( mlma->FindForms( aforms, u"daughters-in-law", 0 ) == 1 );
          REQUIRE( mtc::widestr( aforms ) == u"daughter-in-law" );
        }
      }
      SECTION( "FindMatch" )
      {
        auto  ms = MatchSet<widechar>();

        SECTION( "it checks arguments" )
        {
          SECTION( "called with invalid arguments, it returns ARGUMENT_FAILED " )
            {  REQUIRE( mlma->FindMatch( nullptr, nullptr, 0 ) == ARGUMENT_FAILED );  }
          SECTION( "with too long string, it returns WORDBUFF_FAILED" )
            {  REQUIRE( mlma->FindMatch( &ms, mtc::widestr( 257, 'a' ).c_str() ) == WORDBUFF_FAILED );  }
        }
        SECTION( "it loads forms by template" )
        {
          SECTION( "? matches any single character:" )
          {
            SECTION( "∙ in non-flective words;" )
            {
              REQUIRE_NOTHROW( mlma->FindMatch( ms.clear(), u"y?man" ) );

              if ( REQUIRE( ms.size() == 1 ) )
                REQUIRE( ms.ToString() == "yuman" );

              REQUIRE_NOTHROW( mlma->FindMatch( ms.clear(), u"yuma?" ) );

              if ( REQUIRE( ms.size() == 2 ) )
                REQUIRE( ms.ToString() == "yuman/yumas" );
            }
            SECTION( "∙ in stems of flective words;" )
            {
              REQUIRE_NOTHROW( mlma->FindMatch( ms.clear(), u"exc?ptions" ) );

              if ( REQUIRE( ms.size() == 1 ) )
                REQUIRE( ms.ToString() == "exceptions" );
            }
            SECTION( "∙ in flexions of flective words;" )
            {
              REQUIRE_NOTHROW( mlma->FindMatch( ms.clear(), u"exception?" ) );

              if ( REQUIRE( ms.size() == 1 ) )
                  REQUIRE( ms.ToString() == "exceptions" );
            }
            SECTION( "∙ in flexions of words with zero stem;" )
            {
              REQUIRE_NOTHROW( mlma->FindMatch( ms.clear(), u"wome?" ) );

              if ( REQUIRE( ms.size() == 1 ) )
                REQUIRE( ms.ToString() == "women" );
            }
            SECTION( "∙ in postfixes;" )
            {
              REQUIRE_NOTHROW( mlma->FindMatch( ms.clear(), u"brothers-i?-law" ) );

              if ( REQUIRE( ms.size() == 1 ) )
                REQUIRE( ms.ToString() == "brothers-in-law" );
            }
          }
          SECTION( "* matches any sequence of characters" )
          {
            SECTION( "∙ in non-flective words;" )
            {
              REQUIRE_NOTHROW( mlma->FindMatch( ms.clear(), u"analogo*ly" ) );

              if ( REQUIRE( ms.size() == 1 ) )
                REQUIRE( ms.ToString() == "analogously" );
            }
            SECTION( "∙ in flective words;" )
            {
              REQUIRE_NOTHROW( mlma->FindMatch( ms.clear(), u"excitabilities*" ) );

              if ( REQUIRE( ms.size() == 2 ) )
                REQUIRE( ms.ToString() == "excitabilities/excitabilities'" );
            }
            SECTION( "∙ in postfixes;" )
            {
              REQUIRE_NOTHROW( mlma->FindMatch( ms.clear(), u"brother-in-law*" ) );

              if ( REQUIRE( ms.size() == 1 ) )
                REQUIRE( ms.ToString() == "brother-in-law" );
            }
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
