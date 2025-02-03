/******************************************************************************

    libmorpheng - dictiorary-based morphological analyser for English.
    Copyright (C) 1994-2016 Andrew Kovalenko aka Keva

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

    Commercial license is available upon request.

    Contacts:
      email: keva@meta.ua, keva@rambler.ru
      Skype: big_keva
      Phone: +7(495)648-4058, +7(926)513-2991

******************************************************************************/
# include "../../include/mlma1033.h"
# include "../codepages.hpp"
# include <algorithm>
# include <string>
# include <cstring>
# include <vector>
# include <cstdio>
# include <mtc/test-it-easy.hpp>
# include <thread>

struct  StrMatch
{
  std::string sz;
  uint8_t     id;
};

struct  LexMatch
{
  lexeme_t              nlexid;
  std::vector<StrMatch> aforms;
};

class MatchSet: public std::vector<LexMatch>, public IMlmaMatch
{
public:
  auto  ptr() const -> IMlmaMatch*  {  return (IMlmaMatch*)this;  }

public:
  int   MLMAPROC  Attach() override {  return 1;  }
  int   MLMAPROC  Detach() override {  return 1;  }
  int   MLMAPROC  RegisterLexeme( lexeme_t nlexid, int  nmatch, const SStrMatch*  pmatch ) override
    {
      push_back( { nlexid, {} } );

      for ( int i = 0; i < nmatch; ++i, ++pmatch )
        back().aforms.push_back( { { pmatch->sz, pmatch->cc }, pmatch->id } );

      return 0;
    }

public:
  auto  ToString( unsigned codepage = codepages::codepage_utf8 ) -> std::string
  {
    auto  lst = std::vector<std::string>();
    auto  out = std::string();

    for ( auto& next: *this )
      for ( auto& s: next.aforms )
      {
        auto  pfound = std::lower_bound( lst.begin(), lst.end(), s.sz );

        if ( pfound == lst.end() || *pfound != s.sz )
          lst.insert( pfound, s.sz );
      }

    for ( size_t i = 0; i != lst.size(); ++i )
      out += (i != 0 ? "/" : "") + lst[i];

    return codepages::mbcstombcs( codepage, codepages::codepage_1251, out );
  }
};

auto  FindMatch( IMlmaMb* mlma, const char* ptpl ) -> MatchSet
{
  MatchSet  match;

  mlma->FindMatch( match.ptr(), ptpl );

  return match;
}

TestItEasy::RegisterFunc  testmorpheng( []()
{
  TEST_CASE( "morph/eng" )
  {
    SECTION( "morphological analyser API may be created with different mbcs codepages" )
    {
      IMlmaMb*  mlma = nullptr;

      SECTION( "valid string keys give access to named API" )
      {
        const char* cnames[] = {
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

        for ( auto cpname = std::begin( cnames ); cpname != std::end( cnames ); ++cpname )
        {
          REQUIRE_NOTHROW( mlmaenLoadCpAPI( &mlma, *cpname ) );
            REQUIRE( mlma != nullptr );
          REQUIRE_NOTHROW( mlma = mlma != nullptr ? (mlma->Detach(), nullptr) : nullptr  );
        }
      }

      mlmaenLoadMbAPI( &mlma );//, "utf-8" );

      SECTION( "CheckWord" )
      {
        SECTION( "called with exact BYTES length of string and no options, it recognizes word" )
        {  REQUIRE( mlma->CheckWord( "check", strlen( "check" ), 0 ) == 1 );  }
        SECTION( "called with exact BYTES length of string and no options, it does not recognize unknown word" )
        {
          REQUIRE( mlma->CheckWord( "ывал", strlen( "ывал" ), 0 ) == 0 );
          REQUIRE( mlma->CheckWord( "chek", strlen( "chek" ), 0 ) == 0 );
          REQUIRE( mlma->CheckWord( "checkerr", strlen( "checkerr" ), 0 ) == 0 );
        }
        SECTION( "called with exact BYTES length of string and no options, it does not recognize word with mixed capitals" )
        {  REQUIRE( mlma->CheckWord( "VeRiFy", strlen( "VeRiFy" ), 0 ) == 0 );  }
        SECTION( "called with exact BYTES length of string and 'sfIgnoreCapitals', it recognizes word with mixed capitals" )
        {  REQUIRE( mlma->CheckWord( "VeRiFy", strlen( "VeRiFy" ), sfIgnoreCapitals ) == 1 );  }
        SECTION( "called with unknown length and no options, it does not recognize word with mixed capitals" )
        {  REQUIRE( mlma->CheckWord( "VeRiFiEd", (size_t)-1, 0 ) == 0 );  }
        SECTION( "called with unknown length and 'sfIgnoreCapitals', it recognizes word with mixed capitals" )
        {  REQUIRE( mlma->CheckWord( "VeRiFiEd", (size_t)-1, sfIgnoreCapitals ) == 1 );  }
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
            REQUIRE( strcmp( aforms, "check" ) == 0 );
            REQUIRE( strcmp( aforms + 6, "check" ) == 0 );
            REQUIRE( strcmp( aforms + 12, "check" ) == 0 );
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
      SECTION( "CheckHelp" )
      {
        char  szhelp[0x100];

        SECTION( "called with empty string it returns 0" )
        {
          REQUIRE( mlma->CheckHelp( szhelp, sizeof(szhelp), nullptr, 0 ) == 0 );
          REQUIRE( mlma->CheckHelp( szhelp, sizeof(szhelp), nullptr, (size_t)-1 ) == 0 );
          REQUIRE( mlma->CheckHelp( szhelp, sizeof(szhelp), "аааs", 0 ) == 0 );
        }
        SECTION( "without output buffer it returns ARGUMENT_FAILED" )
        {
          REQUIRE( mlma->CheckHelp( nullptr, sizeof(szhelp), "а?", (size_t)-1 ) == ARGUMENT_FAILED );
          REQUIRE( mlma->CheckHelp( szhelp, 0, "а?", (size_t)-1 ) == ARGUMENT_FAILED );
        }
        SECTION( "for dictionary templates it returns the list of characters" )
        {
          REQUIRE( mlma->CheckHelp( szhelp, "st?aw" ) == 1 );
            REQUIRE( std::string( szhelp ) == "r" );
          if ( REQUIRE( mlma->CheckHelp( szhelp, "straw*" ) == 9 ) )
          {
            REQUIRE( szhelp[0] == '\0' );
            REQUIRE( std::string( szhelp + 1 ) == "'-beiswy" );
          }
          if ( REQUIRE( mlma->CheckHelp( szhelp, "straw?d" ) == 1 ) )
            REQUIRE( std::string( szhelp ) == "e" );
          if ( REQUIRE( mlma->CheckHelp( szhelp, "fa?her-in-law" ) == 1 ) )
            REQUIRE( std::string( szhelp ) == "t" );
          if ( REQUIRE( mlma->CheckHelp( szhelp, "daughter?in-law" ) == 1 ) )
            REQUIRE( std::string( szhelp ) == "-" );
          if ( REQUIRE( mlma->CheckHelp( szhelp, "women*" ) == 2 ) )
          {
            REQUIRE( szhelp[0] == '\0' );
            REQUIRE( szhelp[1] == '\'' );
          }
          REQUIRE( mlma->CheckHelp( szhelp, "dog's*?" ) == 0 );
        }
      }
      SECTION( "FindMatch" )
      {
        auto  ms = MatchSet();

        SECTION( "it checks arguments" )
        {
          SECTION( "called with invalid arguments, it returns ARGUMENT_FAILED " )
            {  REQUIRE( mlma->FindMatch( nullptr, nullptr ) == ARGUMENT_FAILED );  }
          SECTION( "with too long string, it returns WORDBUFF_FAILED" )
            {  REQUIRE( mlma->FindMatch( ms.ptr(), std::string( 257, 'a' ).c_str() ) == WORDBUFF_FAILED );  }
        }
        SECTION( "it loads forms by template" )
        {
          SECTION( "? matches any single character:" )
          {
            SECTION( "∙ in non-flective words;" )
            {
              REQUIRE_NOTHROW( ms = FindMatch( mlma, "y?man" ) );

              if ( REQUIRE( ms.size() == 1 )
                && REQUIRE( ms.ToString() == "yuman" )
                && REQUIRE( ms.front().aforms.size() == 1 ) )
              {
                REQUIRE( ms.front().aforms.front().id == (uint8_t)-1 );
              }

              REQUIRE_NOTHROW( ms = FindMatch( mlma, "yuma?" ) );

              if ( REQUIRE( ms.size() == 2 ) )
                REQUIRE( ms.ToString() == "yuman/yumas" );
            }
            SECTION( "∙ in stems of flective words;" )
            {
              REQUIRE_NOTHROW( ms = FindMatch( mlma, "exc?ptions" ) );

              if ( REQUIRE( ms.size() == 1 )
                && REQUIRE( ms.ToString() == "exceptions" )
                && REQUIRE( ms.front().aforms.size() == 1 ) )
              {
                REQUIRE( ms.front().aforms[0].id == 1 );
              }
            }
            SECTION( "∙ in flexions of flective words;" )
            {
              REQUIRE_NOTHROW( ms = FindMatch( mlma, "exception?" ) );

              if ( REQUIRE( ms.size() == 1 )
                && REQUIRE( ms.ToString() == "exceptions" )
                && REQUIRE( ms.front().aforms.size() == 1 ) )
              {
                REQUIRE( ms.front().aforms[0].id == 1 );
              }
            }
            SECTION( "∙ in flexions of words with zero stem;" )
            {
              REQUIRE_NOTHROW( ms = FindMatch( mlma, "wome?" ) );

              if ( REQUIRE( ms.size() == 1 ) )
                REQUIRE( ms.ToString() == "women" );
            }
            SECTION( "∙ in postfixes;" )
            {
              REQUIRE_NOTHROW( ms = FindMatch( mlma, "brothers-i?-law" ) );

              if ( REQUIRE( ms.size() == 1 ) )
                REQUIRE( ms.ToString() == "brothers-in-law" );
            }
          }
          SECTION( "* matches any sequence of characters" )
          {
            SECTION( "∙ in non-flective words;" )
            {
              REQUIRE_NOTHROW( ms = FindMatch( mlma, "analogo*ly" ) );

              if ( REQUIRE( ms.size() == 1 ) )
                REQUIRE( ms.ToString() == "analogously" );
            }
            SECTION( "∙ in flective words;" )
            {
              REQUIRE_NOTHROW( ms = FindMatch( mlma, "excitabilities*" ) );

              if ( REQUIRE( ms.size() == 1 ) )
                REQUIRE( ms.ToString() == "excitabilities/excitabilities'" );
            }
            SECTION( "∙ in postfixes;" )
            {
              REQUIRE_NOTHROW( ms = FindMatch( mlma, "brother-in-law*" ) );

              if ( REQUIRE( ms.size() == 1 ) )
                REQUIRE( ms.ToString() == "brother-in-law" );
            }
          }
        }
      }
      SECTION( "EnumWords" )
      {
/*
//        FindMatches( mlma, "собака*" );
//        FindMatches( mlma, "мелоч*" );
//        FindMatches( mlma, "ху*" );
        auto  lset_1 = FindMatches( mlma, "мелоч*" );
        auto  lset_2 = LemmatizeIt( mlma, "мелочь" );
        auto  lset_3 = lset_1 & lset_2;

        REQUIRE( !lset_1.empty() );
        REQUIRE( !lset_2.empty() );
        REQUIRE( !lset_3.empty() );*/
        /*
    MLMA_METHOD( EnumWords )( MLMA_THIS
                              IMlmaEnum*      pienum,
                              const char*     pszstr, size_t    cchstr ) MLMA_PURE;
        */
      }
    }
# if 0
    if ( false )
    {
      SECTION( "checking the reversability of the dictionary" )
      {
        IMlmaMb*  mlma;

        mlmaruLoadCpAPI( &mlma, "utf-8" );

        for ( lexeme_t nlexid = 0; nlexid != 2256000; ++nlexid )
        {
          unsigned char wdinfo;

          if ( mlma->GetWdInfo( &wdinfo, nlexid ) == 0 )
            continue;

          for ( int idform = 0; idform < 256; ++idform )
          {
            char  sforms[0x100];
            int   nforms = mlma->BuildForm( sforms, sizeof(sforms), nlexid, (unsigned char)idform );

            if ( nforms == 0 )
              continue;

            for ( auto s = sforms; nforms-- > 0; )
            {
              SLemmInfoA  lemmas[8];
              int         ccform = strlen( s );
              int         nlemma = mlma->Lemmatize( s, ccform, lemmas, 8, nullptr, 0, nullptr, 0, 0 );

              REQUIRE( nlemma != 0 );
              REQUIRE( std::find_if( lemmas + 0, lemmas + nlemma, [nlexid]( const SLemmInfoA& lemm )
                {  return lemm.nlexid == nlexid;  } ) != lemmas + nlemma );

              s += (ccform + 1);
            }
          }
        }
      }
    }
# endif
  }
} );

int   main()
{
  return TestItEasy::Conclusion();
}
