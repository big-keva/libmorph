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
# if !defined( __sweets_h__ )
# define __sweets_h__
# include <stdexcept>
# include <cstdarg>
# include <cstdint>
# include <cstdio>

namespace libmorph
{

  inline  int   LogMessage( int nerror, const char* format, ... )
  {
    va_list   valist;
    va_start( valist, format );
      vfprintf( stderr, format, valist );
    va_end( valist );
    return nerror;
  }

  inline  auto  LoadSource( const std::string& szpath ) -> std::vector<char>
  {
    std::vector<char> output;
    FILE*             lpfile;

    if ( (lpfile = fopen( szpath.c_str(), "rb" )) == nullptr )
      throw std::runtime_error( "could not open file '" + szpath + "'" );

    while ( !feof( lpfile ) )
    {
      char  chbuff[0x1000];
      auto  cbread = fread( chbuff, 1, sizeof(chbuff), lpfile );

      if ( cbread != 0 )
        output.insert( output.end(), chbuff, cbread + chbuff );
    }

    fclose( lpfile );

    return output;
  }

  template <class O>
  auto    LoadObject( O& object, const std::string& szpath ) -> O&
  {
    FILE* lpfile;

    if ( (lpfile = fopen( szpath.c_str(), "rb" )) == nullptr )
      throw std::runtime_error( "could not open file '" + szpath + "'" );

    if ( object.Load( (FILE*)lpfile ) == nullptr )
    {
      fclose( lpfile );
      throw std::runtime_error( "could not load object from file '" + std::string( szpath ) + "'" );
    }

    fclose( lpfile );
    return object;
  }

  inline  char* trim( char* thestr )
  {
    char* endstr;

    while ( *thestr != '\0' && (uint8_t)*thestr <= 0x20 )
      ++thestr;
    for ( endstr = thestr; *endstr != '\0'; ++endstr )
      (void)NULL;
    while ( endstr > thestr && (uint8_t)endstr[-1] <= 0x20 )
      *--endstr = '\0';
    return thestr;
  }

  inline  std::string trim( std::string s )
  {
    while ( s.length() != 0 && (unsigned char)s.back() <= 0x20 )
      s.pop_back();
    while ( s.length() != 0 && (unsigned char)s.front() <= 0x20 )
      s.erase( 0, 1 );
    return s;
  }

}  // end libmorph namespace

# endif // __sweets_h__
