/******************************************************************************

    libfuzzyrus - fuzzy morphological analyser for Russian.

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
# include "../include/mlma1049.h"
# include <string>
# include <cstdint>
# include <cassert>
# include <cstdio>
# include <algorithm>
# include <list>
# include <map>

template <class C>
inline  C* next( C* s )
{
  while ( *s++ != 0 )
    (void)0;
  return s;
}

class Libmorphrus
{
  IMlmaMb*  mlma;

public:
  Libmorphrus(): nlexid( 0 )
    {
      mlmaruLoadCpAPI( &mlma, "1251" );
    }

public:
  std::string Get() const;

protected:
  std::map<formid_t, std::string> Cut( std::map<formid_t, std::string>&& ) const;
  std::map<formid_t, std::string> Get( lexeme_t ) const;
  std::string                     Map( lexeme_t lex, const std::map<formid_t, std::string>& ) const;

protected:
  mutable lexeme_t  nlexid;

};

class DumpedQueue
{
  FILE* lpfile;

public:
  DumpedQueue( FILE* f ): lpfile( f ) {}

public:
  std::string Get() const;

};

std::string Libmorphrus::Get() const
{
  while ( nlexid <= 0x3ffff )
  {
    auto    formap = Get( nlexid++ );
    uint8_t wdinfo;

  // если формы есть, вернуть строку
    if ( formap.size() != 0 )
      return Map( nlexid - 1, formap );

  // если ни одной формы не построено, сместиться на следующий идентификатор лексемы
    while ( nlexid <= 0x3ffff && mlma->GetWdInfo( &wdinfo, nlexid ) == 0 )
      ++nlexid;
  }
  return "";
}

std::map<formid_t, std::string> Libmorphrus::Cut( std::map<formid_t, std::string>&& map ) const
{
  size_t      ccstem = (size_t)-1;
  const char* szbase = nullptr;

  if ( map.size() == 0 )
    return std::move( map );

  for ( auto beg = map.begin(); ccstem != 0 && beg != map.end(); ++beg )
    if ( ccstem == (size_t)-1 )
    {
      ccstem = beg->second.length();
      szbase = beg->second.c_str();
    }
      else
    {
      while ( ccstem > 0 && strncmp( szbase, beg->second.c_str(), ccstem ) != 0 )
        --ccstem;
    }

  for ( auto beg = map.begin(); ccstem != 0 && ++beg != map.end(); )
    beg->second.erase( 0, ccstem );

  return std::move( map );
}

std::map<formid_t, std::string> Libmorphrus::Get( lexeme_t nlexid ) const
{
  std::map<formid_t, std::string> map;
  char                            buf[0x100];
  int                             fms;

  for ( int formid = 0; formid < 0x100; ++formid )
    if ( (fms = mlma->BuildForm( buf, sizeof(buf), nlexid, (formid_t)formid )) > 0 )
      for ( char* ptr = buf; fms-- > 0; ptr = next( ptr ) )
        map.insert( { (formid_t)formid, ptr } );

  return std::move( Cut( std::move( map ) ) );
}

std::string Libmorphrus::Map( lexeme_t lex, const std::map<formid_t, std::string>& map ) const
{
  std::string out;
  char        str[0x100];
  const char* div = "";

  sprintf( str, "%x\t", lex );  out = str;

  for ( auto beg = map.begin(); beg != map.end(); ++beg, div = "/" )
  {
    sprintf( str, "%s%s|%02x", div, beg->second.c_str(), (unsigned)beg->first );
    out += str;
  }

  return std::move( out );
}

std::string DumpedQueue::Get() const
{
  std::string output;
  char        szline[0x400];

  for ( ; ; )
  {
    char* pszend;

    if ( fgets( szline, sizeof(szline), lpfile ) == nullptr )
      return output;
    
    for ( pszend = szline; *pszend != '\0'; ++pszend )
      (void)NULL;
    if ( pszend > szline && pszend[-1] == '\n' )
      --pszend;
    output += std::string( szline, pszend - szline );

    if ( *pszend == '\n' )
      return output;
  }
}

template <class Source>
void  DumpDict( const Source& sq )
{
  std::string st;

  for ( ; (st = sq.Get()) != ""; )
    (void)fprintf( stdout, "%s\n", st.c_str() );
}

template <class Source>
void  LoadList( std::list<std::string>& vec, Source& src, size_t lim )
{
  while ( vec.size() < lim )
  {
    auto  str = src.Get();

    if ( str.length() != 0 )  vec.push_back( std::move( str ) );
      else break;
  }
}

template <class Src1, class Src2>
void  DumpDiff( const Src1& src1, const Src2& src2 )
{
  for ( ; ; )
  {
    auto  s1 = src1.Get();
    auto  s2 = src2.Get();

    if ( s1.length() == 0 && s2.length() == 0 )
      break;

    if ( strcmp( s1.c_str(), s2.c_str() ) != 0 )
    {
      std::list<std::string>  v1;
      std::list<std::string>  v2;
      
      v1.push_back( std::move( s1 ) );
      v2.push_back( std::move( s2 ) );

      LoadList( v1, src1, 0x400 );
      LoadList( v2, src2, 0x400 );

      while ( v1.size() != 0 && v2.size() != 0 )
      {
      // skip matching lines
        while ( v1.size() != 0 && v2.size() != 0 && strcmp( v1.front().c_str(), v2.front().c_str() ) == 0 )
        {
          v1.pop_front();
          v2.pop_front();
        }

      // dump left diff
        while ( v1.size() != 0 )
        {
          auto  st = v1.front();
          auto  pf = std::find_if( v2.begin(), v2.end(), [&]( const std::string& sn )
            {  return strcmp( st.c_str(), sn.c_str() ) == 0;  } );
            
          if ( pf != v2.end() )
            break;

          fprintf( stdout, "<< %s\n", st.c_str() );
            v1.pop_front();
        }

      // dump right diff
        while ( v2.size() != 0 )
        {
          auto  st = v2.front();
          auto  pf = std::find_if( v1.begin(), v1.end(), [&]( const std::string& sn )
            {  return strcmp( st.c_str(), sn.c_str() ) == 0;  } );

          if ( pf != v1.end() )
            break;

          fprintf( stdout, ">> %s\n", st.c_str() );
            v2.pop_front();
        }

        LoadList( v1, src1, v2.size() );
        LoadList( v2, src2, v1.size() );
      }

    // if have something in left list, check match to right file
      if ( v1.size() == 0 && v2.size() == 0 )
        continue;

      assert( v1.size() != 0 || v2.size() != 0 );

      return (void)fprintf( stdout, "!!! diff too big !!!\n" );
    }
  }
}

const char about[] = "dicdump - check/build dictionary dump\n"
                    "usage: dicdump mode\n"
                    "options are:\n"
                      "\tcheck=dump"  "\t"  "- check current dictionary match to control dump 'dump';\n"
                      "\tbuild     "  "\t"  "- dump current dictionary\n";

int   main( int argc, char* argv[] )
{
  if ( argc < 2 )
    return (fprintf( stdout, about ), 0);

  if ( strcmp( argv[1], "build" ) == 0 )
    return (DumpDict( Libmorphrus() ), 0);

  if ( strncmp( argv[1], "check:", 6 ) == 0 || strncmp( argv[1], "check=", 6 ) == 0 )
  {
    FILE* pin = fopen( argv[1] + 6, "rt" );

    if ( pin == nullptr )
      return (fprintf( stderr, "Could not open file '%s'\n", argv[1] + 6 ), EINVAL);

    DumpDiff( DumpedQueue( pin ), Libmorphrus() );

    fclose( pin );
    return 0;
  }

  return (fprintf( stderr, "unknown parameter '%s'\n", argv[1] ), EINVAL);
}

