/******************************************************************************

    libmorph - morphological analysers.

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
# if !defined( __sourcefile_h__ )
# define __sourcefile_h__
# include "moonycode/codes.h"
# include <stdexcept>
# include <cstdio>

class Source
{
  friend Source OpenSource( const std::string&, unsigned );

  const unsigned  encode;
  std::string     stname;
  std::string     bufstr;
  FILE*           lpfile;
  int             lineid;

public:     // open
  Source( unsigned cp = codepages::codepage_1251 ):
      encode( cp ),
      lpfile( nullptr ),
      lineid( 0 )
    {
    }
  Source( const Source& ) = delete;
  Source& operator = ( const Source& ) = delete;
  Source( Source&& s ):
      encode( s.encode ),
      stname( std::move( s.stname ) ),
      bufstr( std::move( s.bufstr ) ),
      lpfile( std::move( s.lpfile ) ),
      lineid( s.lineid )
    {
      s.lpfile = nullptr;
    }
  Source& operator = ( Source&& s )
    {
      stname = std::move( s.stname );
      bufstr = std::move( s.bufstr );
      lpfile = std::move( s.lpfile );  s.lpfile = nullptr;
      lineid = s.lineid;
      return *this;
    }
 ~Source()
    {
      if ( lpfile != nullptr )
        fclose( lpfile );
    }

public:
  Source      Open( const std::string& ) const;
  std::string Name() const  {  return stname;  }
  int         Line() const  {  return lineid;  }

public:     // get/Put
  std::string Get();
  Source&     Put( std::string&& );

};

inline  Source  OpenSource( const std::string& s, unsigned encode = codepages::codepage_1251 )
{
  Source  source( encode );

  if ( (source.lpfile = fopen( (source.stname = s).c_str(), "rb" )) == nullptr )
    throw std::runtime_error( "could not open file '" + s + "'" );

  return source;
}

inline  Source  OpenSource( const char* s, unsigned encode = codepages::codepage_1251 )
{
  return OpenSource( std::string( s ), encode );
}

inline  Source  Source::Open( const std::string& s ) const
{
  auto  rslash = stname.rbegin();
  auto  strtop = stname.rend();

  while ( rslash != strtop && *rslash != '/' && *rslash != '\\' )
    ++rslash;

  auto  folder = stname.substr( 0, strtop - rslash );

  if ( folder.length() != 0 && folder.back() != '/' && folder.back() != '\\' )
    folder += '/';

  return OpenSource( folder + s, encode );
}

// Source implementation

inline  std::string Source::Get()
{
  if ( bufstr.length() != 0 )
    return (++lineid, std::move( bufstr ));

  for ( ; ; )
  {
    char  chbuff[0x1000];
    char* buftop;
    char* bufptr;

    if ( fgets( chbuff, sizeof(chbuff), lpfile ) == nullptr )
      return "";

    for ( buftop = chbuff; *buftop != '\0' && (unsigned char)*buftop <= 0x20; ++buftop )
      (void)NULL;

    for ( bufptr = buftop; (bufptr = strstr( bufptr, "//" )) != nullptr; ++bufptr )
      if ( bufptr == buftop || (unsigned char)bufptr[-1] <= 0x20 )
        break;

    if ( bufptr == nullptr )
      for ( bufptr = buftop; *bufptr != '\0'; ++bufptr ) (void)NULL;

    while ( bufptr > buftop && (unsigned char)bufptr[-1] <= 0x20 )
      --bufptr;

    *bufptr = '\0';

    ++lineid;

    if ( *buftop == '\0' )
      continue;

    if ( codepages::codepage_1251 != encode )
    {
      codepages::mbcstombcs( codepages::codepage_1251, chbuff, sizeof(chbuff), encode, buftop, bufptr - buftop );
      buftop = chbuff;
    }

    return std::string( buftop );
  }
}

inline  Source& Source::Put( std::string&& s )
{
  if ( bufstr.length() != 0 )
    throw std::runtime_error( "source buffer overflow" );
  bufstr = std::move( s );
    --lineid;
  return *this;
}

# endif  // __sourcefile_h__
