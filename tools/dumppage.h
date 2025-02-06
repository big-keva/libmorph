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
# pragma once
# if !defined( __dumppage_h__ )
# define __dumppage_h__
# include "mtc/serialize.h"
# include <stdexcept>
# include <cstdarg>
# include <cstring>
# include <cstdio>
# include <string>

namespace libmorph
{

  class dumpsource
  {
    FILE*       lpfile;
    unsigned    stored;
    int         nchars;

  public:     // construction
    dumpsource( FILE* p, int maxchars = 12 ): lpfile( p ), stored( 0 ), nchars( maxchars )
      {}
    dumpsource* ptr() const
      {
        return (dumpsource*)this;
      }
    void  putbuff( const void* p, size_t l )
      {
        for ( auto pch = static_cast<const uint8_t*>( p ), end = pch + l; pch != end; ++pch, ++stored )
          fprintf( lpfile, "%s0x%02x", stored == 0 ? "" : (stored % nchars) == 0 ? ",\n    " : ",", *pch );
      }
  };

  class BinaryDumper
  {
    std::string outdir;
    std::string nspace;
    std::string szname;
    std::string header;
    FILE*       lpfile;

  public:     // construction
    BinaryDumper(): lpfile( nullptr )
      {}
   ~BinaryDumper()
      {
        if ( lpfile != nullptr )
        {
          if ( !nspace.empty() )
            fprintf( lpfile, "\n"  "}  // end namespace\n\n" );
          fclose( lpfile );
        }
      }

    BinaryDumper& OutDir( const char* d )           {  outdir = d;  return *this;  }
    BinaryDumper& OutDir( const std::string& d )    {  outdir = d;  return *this;  }
    BinaryDumper& Header( const char* h )           {  header = h;  return *this;  }
    BinaryDumper& Header( const std::string& h )    {  header = h;  return *this;  }
    BinaryDumper& Namespace( const char* n )        {  nspace = n;  return *this;  }
    BinaryDumper& Namespace( const std::string& n ) {  nspace = n;  return *this;  }
    BinaryDumper& Output( const char* f )           {  szname = f;  return *this;  }
    BinaryDumper& Output( const std::string& f )    {  szname = f;  return *this;  }

  public:
    BinaryDumper& Print( const char* format, ... )
      {
        FILE*   pf;

        if ( (pf = OpenFile( nullptr )) != nullptr )
        {
          va_list valist;
          va_start( valist, format );
            vfprintf( pf, format, valist );
          va_end( valist );
        }
        return *this;
      }
    template <class serializable>
    BinaryDumper& Dump( const char* vaname, const serializable& serial )
      {
        FILE* pf;

        if ( (pf = OpenFile( vaname )) != NULL )
        {
          fprintf( pf, "  unsigned char %s[] =\n"  "  {\n"  "    ", vaname );
            serial.Serialize( dumpsource( pf ).ptr() );
          fprintf( pf, "\n"  "  };\n" );
        }
        return *this;
      }
  protected:  // helper
    FILE* OpenFile( const char* vaname )
      {
        if ( lpfile == nullptr )
        {
          std::string stpath;
          const char* sslash = "";

          if ( !outdir.empty() && strchr( "/\\", outdir.back() ) == nullptr )
            sslash = "/";

          stpath = outdir + sslash + (szname.empty() ? vaname : szname);

          if ( (lpfile = fopen( stpath.c_str(), "wt" )) == NULL )
            throw std::runtime_error( "coult not create file '" + stpath + "'" );

        // print header if available
          if ( !header.empty() )
            fprintf( lpfile, "%s\n", header.c_str() );

        // print namespace if available
          if ( !nspace.empty() )
            fprintf( lpfile, "namespace %s\n"  "{\n"  "\n", nspace.c_str() );
        }
        return lpfile;
      }
  };

  template <class dirstr, class nspstr, class varstr, class serializable>
  int       DumpBinary( dirstr pszdir, nspstr nspace, varstr vaname, serializable& serial )
  {
    return BinaryDumper().OutDir( pszdir ).Namespace( nspace ).Dump( vaname, serial );
  }

} // end 'libmorph' namespace

template <>
libmorph::dumpsource* Serialize( libmorph::dumpsource* o, const void* p, size_t l )
{
  if ( o != nullptr )
    o->putbuff( p, l );
  return o;
}

# endif // __dumppage_h__
