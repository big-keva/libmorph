# include "../../tools/dumppage.h"
# include "moonycode/codes.h"
# include "mtc/patricia.h"
# include "mtc/json.h"

template <>
std::string*  Serialize( std::string* o, const void* p, size_t l )
{
  return *o += std::string( (const char*)p, l ), o;
}

struct  ClassRef
{
  unsigned  uClsId;
  unsigned  uOccur;
  uint8_t   formid;

  size_t  GetBufLen() const
    {  return ::GetBufLen( uClsId ) + ::GetBufLen( uOccur ) + 1;  }
  template <class O>
  O*      Serialize( O* o ) const
    {  return ::Serialize( ::Serialize( ::Serialize( o, uClsId ), uOccur ), &formid, 1 );  }
};

struct FlexNode
{
  mtc::charstr  stflex;
  uint8_t       idform;

public:
  FlexNode( uint8_t f, const char* s, size_t l ):
    stflex( s, l ),
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

struct ClassMap: public std::vector<std::string>
{
  template <class O>
  O*      Serialize( O* o ) const
    {
      for ( auto next = begin(); next != end(); ++next )
        o = ::Serialize( o, next->c_str(), next->length() );

      return o;
    }
};

char* revert( char* beg, char* end )
{
  auto  s = beg;

  for ( --end; beg < end; ++beg, --end )
    std::swap( *beg, *end );

  return s;
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
  auto  cranks = std::vector<unsigned>();

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
    struct  prefix
    {
      unsigned  occCount;
      char      twoChars[2];
    };

    auto  partSp = cls.get_int32( "psp", 0 );
    auto  clscnt = cls.get_int32( "cnt", 0 );
    auto  supset = cls.get_array_zmap( "spf" );
    auto  inflex = cls.get_array_zmap( "flx" );
    auto  supref = std::vector<prefix>();
    auto  lclass = std::vector<FlexNode>();

  // check valid class
    if ( supset == nullptr || supset->empty() )
      return fprintf( stderr, "class contains no 'spf' array of suprefixes!\n" ), EINVAL;

    if ( inflex == nullptr || inflex->empty() )
      return fprintf( stderr, "class contains no 'flx' array of suprefixes!\n" ), EINVAL;

  // load suprefix list
    for ( auto& supp: *supset )
    {
      auto  add = prefix();
      auto  mbs = supp.get_charstr( "s" );
      auto  wcs = supp.get_widestr( "s" );
      char  prf[3];

    // check occurence count
      if ( (add.occCount = supp.get_int32( "n", 0 )) == 0 )
        return fprintf( stderr, "prefix occurence value is invalid or not found\n" ), EINVAL;

      if ( mbs == nullptr && wcs == nullptr )
        return fprintf( stderr, "prefix string is not found\n" ), EINVAL;

      if ( wcs != nullptr )
      {
        if ( codepages::widetombcs( codepages::codepage_1251, prf, sizeof(prf), wcs->data(), wcs->size() ) != 2 )
          return fprintf( stderr, "invalid prefix length != 2\n" ), EINVAL;
      }
        else
      {
        if ( codepages::mbcstombcs( codepages::codepage_1251, prf, sizeof(prf), codepages::codepage_utf8, mbs->data(), mbs->size() ) != 2 )
          return fprintf( stderr, "invalid prefix length != 2\n" ), EINVAL;
      }

      add.twoChars[0] = prf[0];
      add.twoChars[1] = prf[1];

      supref.emplace_back( add );
    }

  // create inflexion set
    for ( auto& flex: *inflex )
    {
      char  szflex[0x20];
      int   idform = flex.get_int32( "id", -1 );
      auto  szform = flex.get_charstr( "sz" );
      auto  wsform = flex.get_widestr( "sz" );
      auto  ccflex = (size_t){};

    // check inflextion class
      if ( idform == -1 )
        return fprintf( stderr, "invalid inflexion, no 'id' integer field\n" ), EINVAL;

      if ( szform != nullptr )
      {
        ccflex = codepages::mbcstombcs(
          codepages::codepage_1251, szflex, sizeof(szflex) - 2,
          codepages::codepage_utf8, szform->c_str(), szform->size() );
      }
        else
      if ( wsform != nullptr )
      {
        ccflex = codepages::widetombcs(
          codepages::codepage_1251, szflex, sizeof(szflex) - 2, wsform->c_str(), wsform->size() );
      }
        else
      return fprintf( stderr, "invalid inflexion, no 'sz' string field\n" ), EINVAL;

      if ( ccflex > sizeof(szflex) - 2 )
        return fprintf( stderr, "inflexion too long, recompile with increased INFLEX_LEN constant\n" ), EINVAL;

      lclass.emplace_back( (uint8_t)idform,
        szflex,
        ccflex );

    // prepare reverted flexion
      revert( szflex, szflex + ccflex );

    // append suprefixes and register models
      for ( auto& sup: supref )
      {
        auto  pvalue = (decltype(flxdic)::value_type*){};

        szflex[ccflex + 0] = sup.twoChars[1];
        szflex[ccflex + 1] = sup.twoChars[0];

        if ( (pvalue = flxdic.Search( szflex, ccflex + 2 )) != nullptr )
          pvalue->push_back( { (unsigned)clsmap.size(), sup.occCount, (uint8_t)idform } );
        else flxdic.Insert( szflex, ccflex + 2, { { (unsigned)clsmap.size(), sup.occCount, (uint8_t)idform } } );
      }
    }

  // register class rank in class ranks map
    cranks.push_back( clscnt );

  // store class itself
    clsmap.resize( clsmap.size() + 1 );

    ::Serialize( ::Serialize( &clsmap.back(), partSp ), lclass );
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

// dump class ranks
  dumper.Print( "\n"
                "  unsigned  ClassWeight[] =\n"
                "  {\n" );

  prefix = "    ";

  for ( size_t i = 0; i != cranks.size(); ++i )
  {
    dumper.Print( "%s0x%06x", prefix, cranks[i] );
    prefix = (i % 6) == 5 ? ",\n    " : ", ";
  }

  dumper.Print(
    "\n"
    "  };\n" );

  return 0;
}