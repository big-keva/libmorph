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
# include <mlmadefs.h>
# include "../../license.h"
# include "../../tools/buildmorph.h"
# include "../../chartype.h"
# include "flexbox.h"
# include <mtc/wcsstr.h>

# define MAX_STRING_LEN    256
# define FLEX_DELIMITER   '|'

# define wfCapFirst  0x0080
# define wfCapOnly   0x0100

class Capitalization final
{
  static  bool  IsCapital( char ch )
  {
    return ch != 0 && libmorph::eng::toLoCaseMatrix[(unsigned char)ch] != ch;
  }

public:
  static  int   Type( const char* string )
  {
    return IsCapital( string[0] ) ?
           IsCapital( string[1] ) ? 2 : 1 : 0;
  }
};

class ClassTable
{
public:
  struct  class_info
  {
    uint16_t  wdinfo;
    uint16_t  fxoffs;

  public:
    bool  operator < ( const class_info& cls ) const
      {  return wdinfo < cls.wdinfo || (wdinfo == cls.wdinfo && fxoffs < cls.fxoffs);  }

  public:     // serialization
    auto  GetBufLen() const -> size_t
      {  return 2 * sizeof(uint16_t);  }

  };

public:
  auto  Add( const class_info& cls ) -> uint16_t;

public:     // serialization
  template <class O>  O*  Serialize( O* ) const;

protected:
  std::map<class_info, uint16_t>  classTab;
  unsigned                        tableLen = 0;

};

class ResolveEng
{
public:
  struct  entry_type
  {
    uint8_t     chrmin;
    uint8_t     chrmax;
    uint32_t    nlexid;
    uint16_t    oclass;
    std::string stpost;

  public:     // compare
    bool  operator <  ( const entry_type& r ) const {  return compare( r ) < 0;   }
    bool  operator == ( const entry_type& r ) const {  return compare( r ) == 0;  }

  public:     // serialization
    auto  GetBufLen() const -> size_t;
    template <class O>
    O*    Serialize( O* ) const;

  protected:  // helpers
    int   compare( const entry_type& ) const;

  };

  using stem_entry = std::pair<std::string, entry_type>;

public:
  ResolveEng();
 ~ResolveEng() = default;

public:
  auto  operator()( const char* ) -> std::vector<stem_entry>;

protected:
  auto  MapPartOfSpeech( const std::string& s ) const -> uint16_t;
  auto  FindStemPostfix( const char* string ) const -> std::string;

protected:
  std::map<std::string, uint16_t>
              partSpMapper;
  CFlexBox    inflexMapper;
  ClassTable  classesTable;

};

class CMakeEng: public ResolveEng, public buildmorph<ResolveEng>
{
  using inherited = buildmorph<ResolveEng>;

  bool  GetSwitch( std::string& out, const char* arg, const char* key ) const
  {
    size_t  keylen = strlen( key );

    if ( strncmp( arg, key, keylen ) == 0 && (arg[keylen] == '=' || arg[keylen] == ':') )
    {
      if ( out.length() != 0 )
        throw std::runtime_error( std::string( "'" ) + key + "' is already defined as '" + out + "'" );
      out = arg + 1 + keylen;
      return true;
    }
    return false;
  }

public:
  CMakeEng(): inherited( *this, libmorph_eng_license, codepages::codepage_866 )  {}

  int   Run( int argc, char* argv[] )
    {
      std::string outp_dir;
      std::string name_spc;
      std::string codepage;
      std::string unk_name;

      std::vector<const char*>  dict_set;

      for ( auto i = 1; i != argc; ++i )
      {
        if ( *argv[i] == '-' )
        {
          if ( !GetSwitch( outp_dir, 1 + argv[i], "target-dir" )
            && !GetSwitch( unk_name, 1 + argv[i], "unknown" )
            && !GetSwitch( name_spc, 1 + argv[i], "namespace" ) )
          throw std::runtime_error( "invalid switch: " + std::string( argv[i] ) );
        }
          else
        dict_set.push_back( argv[i] );
      }

    // check parameters
      if ( outp_dir.empty() )
        libmorph::LogMessage( 0, "undefined output directory was set to default '%s'\n", (outp_dir = "./").c_str() );

      if ( name_spc.empty() )
        libmorph::LogMessage( 0, "undefined 'namespace' was set to default '%s'\n", (name_spc = "__libmorpheng__").c_str() );

      if ( unk_name.empty() )
        libmorph::LogMessage( 0, "'-unknown' parameter undefined, unknown words will not be dumped\n" );

      if ( dict_set.size() == 0 )
        throw std::runtime_error( "no dictinaries specified, use --help" );

      SetUnknowns( unk_name ).
      SetNamespace( name_spc ).
      SetTargetDir( outp_dir );

//      InitTables( flex_tab, flex_idx, intr_tab, intr_idx );
      CreateDict( dict_set );
      libmorph::BinaryDumper()
        .Namespace( nspace )
        .Header( libmorph_eng_license )
        .OutDir( outdir )
        .Output( "flexTree.cpp" )
        .Dump( "flexTree", inflexMapper );
    libmorph::BinaryDumper()
        .Namespace( nspace )
        .Header( libmorph_eng_license )
        .OutDir( outdir )
        .Output( "classmap.cpp" )
        .Dump( "classmap", classesTable );

      return 0;
    }
};

// ClassTable implementation

auto  ClassTable::Add( const class_info& cls ) -> uint16_t
{
  auto  pfound = classTab.find( cls );

  if ( pfound == classTab.end() )
  {
    pfound = classTab.insert( { cls, tableLen } ).first;
    tableLen += cls.GetBufLen();
  }

  return pfound->second;
}

template <class O>
O*  ClassTable::Serialize( O* o ) const
{
  using map_iterator = decltype(classTab.cbegin());

  std::vector<map_iterator> sorted;

  for ( auto it = classTab.cbegin(); it != classTab.cend(); ++it )
    sorted.push_back( it );

  std::sort( sorted.begin(), sorted.end(), []( const map_iterator& c1, const map_iterator& c2 )
    {  return c1->second < c2->second;  } );

  for ( auto& next: sorted )
  {
    o = ::Serialize( ::Serialize( ::Serialize( ::Serialize( o,
      char(next->first.wdinfo) ), char(next->first.wdinfo >> 8) ),
      char(next->first.fxoffs) ), char(next->first.fxoffs >> 8) );
  }

  return o;
}

// ResolveEng

auto  ResolveEng::entry_type::GetBufLen() const -> size_t
{
  return sizeof(chrmin) + sizeof(chrmax) + sizeof(uint16_t)
    + ::GetBufLen( nlexid ) + (stpost.empty() ? 0 : 1 + stpost.length());
}

template <class O>
O*  ResolveEng::entry_type::Serialize( O* o ) const
{
  auto  wstore = oclass | (stpost.empty() ? 0 : wfPostSt);

  o = ::Serialize( ::Serialize( ::Serialize( ::Serialize( ::Serialize( o,
    char(chrmin) ),
    char(chrmax) ), nlexid ),
    char(wstore) ),
    char(wstore >> 8) );

  return (wstore & wfPostSt) != 0 ? ::Serialize( o, stpost ) : o;
}

int   ResolveEng::entry_type::compare( const entry_type& r ) const
{
  int   rescmp;

  if ( (rescmp = r.chrmax - chrmax) == 0 )
    if ( (rescmp = chrmin - r.chrmin) == 0 )
      if ( (rescmp = stpost.compare( r.stpost )) == 0 )
        rescmp = nlexid - r.nlexid;

  return rescmp;
}

ResolveEng::ResolveEng():
  partSpMapper{
    { "unknown",  0  },
    { "N",        1  },
    { "V",        2  },
    { "A",        3  },
    { "ADV",      4  },
    { "PN",       5  },
    { "NUM",      6  },
    { "INT",      7  },
    { "CONJ",     8  },
    { "PREP",     9  },
    { "REFL",     10 },
    { "PERS",     11 },
    { "INDEF",    12 },
    { "POSS",     13 },
    { "DEMO",     14 },
    { "REL",      15 },
    { "ART",      16 },
    { "PARTICLE", 17 },
    { "INTER",    18 },
    { "NAME",     19 },
    { "V_BE",     20 } }
{
}

inline  bool  is_space( char c )
{
  return (unsigned char)c <= 0x20;
}

inline  auto  skip_space( const char* s ) -> const char*
{
  while ( *s != '\0' && is_space( *s ) )
    ++s;
  return s;
}

auto  ResolveEng::operator ()( const char* article ) -> std::vector<stem_entry>
{
  char        szstem[0x100];
  char        szflex[0x100];
  char        partSp[64];
  uint16_t    wdinfo;
  std::string stpost = FindStemPostfix( article );
  auto        tabref = decltype(inflexMapper.AddInflex( nullptr )){};
  char*       output = szstem;

// check the length
  assert( strlen( article ) < MAX_STRING_LEN );

// get the stem
  for ( article = skip_space( article ); *article != '\0' && *article != FLEX_DELIMITER && !is_space( *article ); )
    *output++ = *article++;

  if ( *article == '\0' )
    throw std::invalid_argument( "invalid article format, '|' delimiter or space expected between stem pr part-of-speech" );
  else *output = '\0';

// check for postfix
  if ( !stpost.empty() )
    *(output -= stpost.length()) = '\0';

// get inflexion string
  for ( output = szflex; *article != '\0' && !is_space( *article ); )
    *output++ = *article++;

  if ( *article == '\0' )
    throw std::invalid_argument( "part of speech expected" );
  else *output = '\0';

// get part-of-speech
  for ( output = partSp, article = skip_space( article ); *article != '\0' && !is_space( *article ); )
    *output++ = *article++;

  if ( output == partSp )
    throw std::invalid_argument( "unexpected end of dictionary article" );
  else *output = '\0';

// convert inflexions to lower case
  for ( auto s = szflex; *s != '\0'; ++s )
    *s = libmorph::eng::toLoCaseMatrix[(unsigned char)*s];

// Get the part of speach and the inflexion
  wdinfo = MapPartOfSpeech( partSp );

  if ( szflex[0] != '\0' )  tabref = inflexMapper.AddInflex( szflex );
    else tabref = { 0, '\0', '\0' };

  if ( wdinfo == (uint16_t)-1 )
    throw std::invalid_argument( mtc::strprintf( "unresolved part-of-speech tag '%s'", partSp ) );

// apply capitalization scheme
  switch ( Capitalization::Type( szstem ) )
  {
    case 1:   wdinfo |= wfCapFirst;
              break;
    case 2:   wdinfo |= wfCapOnly;
    default:  break;
  }

// convert lexeme description to lower case
  for ( auto s = szstem; *s != '\0'; ++s )
    *s = libmorph::eng::toLoCaseMatrix[(unsigned char)*s];

  return { {
      szstem,
      {
        tabref.clower,
        tabref.cupper,
        0,
        classesTable.Add( { wdinfo, tabref.offset } ),
        stpost
      } } };
}

auto ResolveEng::MapPartOfSpeech( const std::string& ps ) const -> uint16_t
{
  auto  pfound = partSpMapper.find( ps );

  if ( pfound == partSpMapper.end() )
    throw std::invalid_argument( mtc::strprintf( "unknown part-of-speach '%s'", ps.c_str() ) );

  return pfound->second;
}

auto ResolveEng::FindStemPostfix( const char* string ) const -> std::string
{
  const char* strtop;
  const char* strend;

  if ( (strtop = strstr( string, "post:" )) != nullptr )
  {
    for ( strtop += 5; *strtop != '\0' && (unsigned char)*strtop <= 0x20; strtop++ )
      (void)NULL;
    for ( strend = strtop; (unsigned char)*strend > 0x20; strend++ )
      (void)NULL;

    return std::string( strtop, strend - strtop );
  }
  return "";
}

char about[] = "makeeng - the dictionary builder;\n"
               "Usage: %s ininame\n";

int  main(int argc, char* argv[])
{
  CMakeEng  generate;

  setbuf( stderr, NULL );

// Check the arguments
  if ( argc < 2 )
    return fprintf( stdout, about, argv[0] );

  return generate.Run( argc, argv );
}
