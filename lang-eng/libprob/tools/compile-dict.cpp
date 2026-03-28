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
# include <tools/dumppage.h>
# include <moonycode/codes.h>
# include <mtc/patricia.h>
# include <mtc/json.h>

constexpr size_t INFLEX_LEN = 0x20;

template <>
std::string*  Serialize( std::string* o, const void* p, size_t l )
{
  return *o += std::string( (const char*)p, l ), o;
}

struct  ClassRef
{
  unsigned  uClsId;
  unsigned  mPower;
  uint8_t   formid;

  size_t  GetBufLen() const
    {  return ::GetBufLen( uClsId ) + ::GetBufLen( mPower ) + 1;  }
  template <class O>
  O*      Serialize( O* o ) const
    {  return ::Serialize( ::Serialize( ::Serialize( o, uClsId ), mPower ), &formid, 1 );  }
};

struct FlexNode
{
  mtc::charstr  stflex;
  uint8_t       idform;

public:
  FlexNode( uint8_t f, std::string&& s ):
    stflex( std::move( s ) ),
    idform( f ) {}

public:
  size_t  GetBufLen() const
    {
      return stflex.length() + sizeof(uint8_t) * 2;
    }
  template <class O>
  O*      Serialize( O* o ) const
    {
      uint8_t ccflex = stflex.size();

      return
        ::Serialize(
        ::Serialize(
        ::Serialize( o, &idform, 1 ), &ccflex, 1 ), stflex.c_str(), ccflex );
    }
};

struct ClassMap: std::vector<std::string>
{
  template <class O>
  O*      Serialize( O* o ) const
    {
      for ( auto next = begin(); next != end(); ++next )
        o = ::Serialize( o, next->c_str(), next->length() );

      return o;
    }
};

struct  Model
{
  std::string suffix;
  std::string sample;
  unsigned    weight;

  template <class O>
  O*  Serialize( O* o ) const
  {
    return ::Serialize( ::Serialize( ::Serialize( o, suffix.c_str(), 2 ),
      weight ),
      sample );
  }
};

char* revert( char* beg, char* end )
{
  auto  s = beg;

  for ( --end; beg < end; ++beg, --end )
    std::swap( *beg, *end );

  return s;
}

auto  Get1251Str( const mtc::zmap& z, const char* k ) -> std::string
{
  auto  pszstr = z.get_charstr( k );
  auto  pwsstr = z.get_widestr( k );

  if ( pszstr != nullptr )
    return codepages::mbcstombcs( codepages::codepage_1251, codepages::codepage_utf8, *pszstr );
  if ( pwsstr != nullptr )
    return codepages::widetombcs( codepages::codepage_1251, *pwsstr );
  throw std::invalid_argument( mtc::strprintf( "failed to get key '%s' string", k ) );
}

# define STR_TO_TEXT( x ) #x
# define MAKE_STRING( x ) STR_TO_TEXT( x )

const char about[] = "compile-dict v2.0 - create russian probability morphology dictionary\n"
  "Usage: %s [options] source output\n"
  "options are:\n"
  "\t"  "-ns[pace]=namespace-name, default is \"" MAKE_STRING( LIBMORPH_NAMESPACE ) "\".\n";

int   main( int argc, char* argv[] )
{
  auto  source = (const char*)nullptr;
  auto  output = (const char*)nullptr;
  auto  nspace = (const char*)nullptr;
  auto  jsn = mtc::zmap();
  auto  inp = (FILE*)nullptr;
  auto  arr = (const mtc::array_zmap*)nullptr;
  auto  flxdic = mtc::patricia::tree<std::vector<ClassRef>>();
  auto  clsmap = ClassMap();

  for ( int i = 1; i != argc; ++i )
  {
    if ( *argv[i] == '-' )
    {
      if ( strncmp( 1 + argv[i], "ns=", 3 ) == 0 || strncmp( 1 + argv[i], "nspace=", 7 ) == 0 )
      {
        nspace = 1 + strchr( 1 + argv[i], '=' );
      }
        else
      return fprintf( stderr, "invalid switch '%s'\n", argv[i] ), EINVAL;
    }
      else
    if ( source == nullptr )  source = argv[i];
      else
    if ( output == nullptr )  output = argv[i];
      else
    return fprintf( stderr, "invalid argument '%s'\n", argv[i] ), EINVAL;
  }

  if ( nspace == nullptr )
    nspace = MAKE_STRING( LIBMORPH_NAMESPACE );

  if ( source == nullptr )
    return fprintf( stderr, about, 1 + strrchr( argv[0], '/' ) ), 0;

  if ( output == nullptr )
    return fprintf( stderr, about, 1 + strrchr( argv[0], '/' ) ), 0;

  if ( (inp = fopen( source, "rt" )) == nullptr )
    return fprintf( stderr, "could not open file '%s'\n", source ), ENOENT;

// parse the input
  try
  {
    mtc::json::Parse( inp, jsn );
    fclose( inp );
  }
  catch ( const mtc::json::parse::error& err )
  {
    fprintf( stderr, "error parsing source file %s, line %u: %s\n",
      source, err.get_json_lineid(), err.what() );
    return fclose( inp ), EINVAL;
  }

// check class list
  if ( (arr = jsn.get_array_zmap( "classes" )) == nullptr )
    return fputs( "input source does not contain 'classes' class array\n", stderr ), EINVAL;

  if ( arr->empty() )
    return fputs( "input source does not have any classes in class array\n", stderr ), EINVAL;

// create inflextion dictionary
  for ( auto& cls: *arr )
  {
    auto  partSp = cls.get_int32( "partSp", 0 );
    auto  cindex = cls.get_int32( "index", -1 );
    auto  pModel = cls.get_array_zmap( "models" );
    auto  inflex = cls.get_array_zmap( "inflex" );
    bool  accent = cls.get_charstr( "accent", "a" ) == "b";
    auto  models = std::vector<Model>();
    auto  flexet = std::vector<FlexNode>();

    try
    {
    // check loaded class for models and inflex
      if ( pModel == nullptr || pModel->empty() )
        return fprintf( stderr, "could not find 'models' for class %d\n", cindex ), EINVAL;

      if ( inflex == nullptr || inflex->empty() )
        return fprintf( stderr, "could not find 'inflex' for class %d\n", cindex ), EINVAL;

    // load models list
      for ( auto& model: *pModel )
      {
        auto  suffix = Get1251Str( model, "suffix" );
        auto  sample = Get1251Str( model, "sample" );
        auto  weight = model.get_int32( "weight", -1 );

      // check model format
        if ( suffix.length() != 2 )
          return fprintf( stderr, "class '%d': invalid model stem final char: '%s'\n", cindex,
            codepages::mbcstombcs( codepages::codepage_utf8, codepages::codepage_1251, suffix ).c_str() ), EINVAL;
        if ( weight == -1 )
          fprintf( stderr, "class '%d': undefined model weight\n", cindex );
        if ( sample.empty() )
          fprintf( stderr, "class '%d': warning: model string not specified\n", cindex );

      // register model
        models.push_back( {
          std::move( suffix ),
          std::move( sample ), unsigned(weight) } );
      }

    // load inflexion set
      for ( auto& flex: *inflex )
      {
        char  szflex[INFLEX_LEN];
        int   idform = flex.get_int32( "id", -1 );
        auto  szform = Get1251Str( flex, "sz" );

      // check inflextion class
        if ( idform == -1 )
          return fprintf( stderr, "invalid inflexion, no 'id' integer field\n" ), EINVAL;

        if ( szform.length() > sizeof(szflex) - 2 )
          return fprintf( stderr, "inflexion too long, recompile with increased INFLEX_LEN constant\n" ), EINVAL;

      // prepare reverted flexion
        revert( strcpy( szflex, szform.c_str() ), szflex + szform.length() );

      // append suprefixes and register models
        for ( auto& model: models )
        {
          decltype(flxdic)::value_type* pvalue;

          szflex[szform.length() + 0] = model.suffix[1];
          szflex[szform.length() + 1] = model.suffix[0];

          if ( (pvalue = flxdic.Search( szflex, szform.length() + 2 )) != nullptr )
            pvalue->push_back( { (unsigned)clsmap.size(), model.weight, (uint8_t)idform } );
          else flxdic.Insert( szflex, szform.length() + 2, { { (unsigned)clsmap.size(), model.weight, (uint8_t)idform } } );
        }

        flexet.emplace_back( (uint8_t)idform, std::move( szform ) );
      }

    // store class itself
      clsmap.resize( clsmap.size() + 1 );

    // reorder models
      std::sort( models.begin(), models.end(), []( const Model& a, const Model& b )
        {
          int   rc;

          if ( (rc = b.weight - a.weight) == 0 )
            rc = a.sample.compare( b.sample );

          return rc < 0;
        });

      ::Serialize( ::Serialize( ::Serialize( ::Serialize( &clsmap.back(), uint8_t(partSp) ),
        uint8_t(accent ? 1 : 0) ),
        flexet ),
        models );
    }
    catch ( const std::runtime_error& xp )
    {
      return fprintf( stderr, "class '%d' error: %s\n", cindex, xp.what() ), EINVAL;
    }
  }

  libmorph::BinaryDumper  dumper;
  const char*             prefix;
  unsigned                offset = 0;

  dumper.Namespace( nspace ).Output( output );

// dump class offset table
  dumper.Print(
    "  unsigned  ClassNumber = %u;\n"
    "\n"
    "  unsigned  ClassOffset[] =\n"
    "  {\n", clsmap.size() );

  prefix = "    ";

  for ( size_t i = 0; i != clsmap.size(); ++i )
  {
    dumper.Print( "%s0x%06x", prefix, offset );
    prefix = (i % 6) == 5 ? ",\n    " : ", ";
    offset += clsmap[i].size();
  }

  dumper.Print( "\n"
    "  };\n" );

// dump tables and inflexions
  dumper
    .Dump( "ClassTables", clsmap )
    .Dump( "ReverseDict", flxdic );

  return 0;
}