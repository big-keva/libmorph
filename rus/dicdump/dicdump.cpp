# include "../include/mlma1049.h"
# include <string>
# include <cstdint>
# include <cstdio>
# include <sstream>
# include <iostream>
# include <iomanip>
# include <map>

template <class C>
inline  C* next( C* s )
{
  while ( *s++ != 0 )
    (void)0;
  return s;
}

class StringQueue
{
public:
  virtual std::string Get() = 0;
};

class Libmorphrus: public StringQueue
{
  IMlmaMb*  mlma;

public:
  Libmorphrus(): nlexid( 0 )
    {
      mlmaruLoadCpAPI( &mlma, "866" );
    }

public:
  virtual std::string Get() override final;

protected:
  std::map<formid_t, std::string> Cut( std::map<formid_t, std::string>&& ) const;
  std::map<formid_t, std::string> Get( lexeme_t ) const;
  std::string                     Map( lexeme_t lex, const std::map<formid_t, std::string>& ) const;

protected:
  lexeme_t  nlexid;
};

std::string Libmorphrus::Get()
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
  std::stringstream out;
  const char*       div = "";

  out << std::hex << lex << '\t';

  for ( auto beg = map.begin(); beg != map.end(); ++beg, div = "/" )
    out << div
        << beg->second
        << '|'
        << std::setw( 2 ) << std::setfill( '0' ) << std::hex << (unsigned)beg->first;

  out << std::ends;

  return std::move( out.str() );
}

void  DumpDiff( StringQueue& s1, StringQueue& s2 )
{
  std::string str1;
  std::string str2;

  for ( ; ; )
  {
    str1 = s1.Get();
    str2 = s2.Get();

    if ( str1 != str2 )
    {
      return (void)fprintf( stdout, "<< %s\n"
                                    ">> %s\n", str1.c_str(), str2.c_str() );
    }
  }
}

int   main( int argc, char* argv[] )
{
  Libmorphrus morpho;
  Libmorphrus source;
  std::string stnext;

  while ( (stnext = morpho.Get()).length() != 0 )
    fprintf( stdout, "%s\n", stnext.c_str() );

  return 0;
}

