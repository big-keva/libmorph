# include <tools/dumppage.h>
# include "lresolve.h"
# include "../licenseGPL.h"
# include "tools/plaintable.h"
# include "tools/classtable.h"
# include "tools/ftables.h"
# include "tools/sweets.h"
# include "mtables.h"
# include <map>

unsigned char mixTypes[64] =
{
  0x00,                                 /* Несуществующий тип слова     */
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01,   /* 1..6: глаголы                */
/* Существительные мужского рода */
  0x02,                                 /* 7: # м                       */
  0x03,                                 /* 8: # мо                      */
  0x04,                                 /* 9: # м//мо, # мо//м          */
  0x05,                                 /* 10: м с                      */
  0x06,                                 /* 11: мо жо                    */
  0x06,                                 /* 12: мо со                    */
/* Существительные женского рода */
  0x05,                                 /* 13: # ж                      */
  0x06,                                 /* 14: # жо                     */
  0x07,                                 /* 15: # ж//жо, # жо//ж         */
/* Существительные среднего рода */
  0x05,                                 /* 16: # с                      */
  0x06,                                 /* 17: # со                     */
  0x07,                                 /* 18: # с//со, # со//с         */
/* Существительные общего рода   */
  0x05,                                 /* 19: м//ж ж                   */
  0x06,                                 /* 20: мо//жо жо                */
/* Существительные муж./ср. рода */
  0x05,                                 /* 21: # м//с                   */
  0x06,                                 /* 22: мо//со со                */
/* Существительные жен./ср. рода */
  0x05,                                 /* 23: # ж//с, # с//ж           */
/* Существительные множ. числа   */
  0x05,                                 /* 24: мн. ж//м, мн. м//ж       */
/* Прилагательные                */
  0x08,                                 /* 25: # п                      */
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00
};

struct  lexemedump
{
  byte_t      chrmin;
  byte_t      chrmax;
  lexeme_t    nlexid;
  word16_t    oclass;
  std::string stpost;

public:     // comparison
  bool  operator <  ( const lexemedump& r ) const {  return compare( r ) < 0;   }
  bool  operator == ( const lexemedump& r ) const {  return compare( r ) == 0;  }

public:
  auto  GetBufLen() const -> size_t
  {
    return 2 + ::GetBufLen( nlexid ) + sizeof(word16_t) + (stpost.length() != 0 ? stpost.length() + 1 : 0);
  }
  template <class O>
  O*    Serialize( O* o ) const
  {
    word16_t  wstore = oclass | (stpost.length() != 0 ? 0x8000 : 0);

    o = ::Serialize( ::Serialize( ::Serialize( ::Serialize( o,
      &chrmin, sizeof(chrmin) ),
      &chrmax, sizeof(chrmax) ), nlexid ), &wstore, sizeof(wstore) );

    return (wstore & 0x8000) != 0 ? ::Serialize( o, stpost ) : o;
  }

protected:  // helpers
  int   compare( const lexemedump& r ) const
    {
      int   rescmp;

      if ( (rescmp = r.chrmax - chrmax) == 0 )
      if ( (rescmp = chrmin - r.chrmin) == 0 )
      if ( (rescmp = strcmp( stpost.c_str(), r.stpost.c_str() )) == 0 )
            rescmp = nlexid - r.nlexid;
      return rescmp;
    }
};

class ResolveRus
{
  using stemdata = std::pair<std::string, lexemedump>;

  std::vector<char>         ftable;
  libmorph::TableIndex      findex;

  std::vector<char>         mtable;
  libmorph::rus::Alternator mindex;

  std::vector<char>         aplain;
  std::map<size_t, size_t>  iplain;

  classtable<morphclass>    clsset;

protected:
  template <size_t N>
  const char* GetSubtext( const char* source, char (&output)[N] )
    {
      if ( source != nullptr )
      {
        char* outptr = output;

        while ( outptr < output + N && *source != '\0' && (unsigned char)*source > 0x20 )
          *outptr++ = *source++;
        if ( outptr >= output + N )
          return nullptr;
        for ( *outptr = '\0'; *source != '\0' && (unsigned char)*source <= 0x20; ++source )
          (void)NULL;
      }
      return source;
    }

protected:
  template <class O>
  void  LoadObject( O& o, const std::string& szpath ) const
    {
      FILE* lpfile;

      if ( (lpfile = fopen( szpath.c_str(), "rb" )) == nullptr )
        throw std::runtime_error( "could not open file '" + std::string( szpath ) + "'" );

      if ( o.Load( (FILE*)lpfile ) == nullptr )
        {
          fclose( lpfile );
          throw std::runtime_error( "could not load object from file '" + std::string( szpath ) + "'" );
        }
      fclose( lpfile );
    }

public:
  void  InitTables( const std::string&  flex_table, const std::string& flex_index,
                    const std::string&  intr_table, const std::string& intr_index );
  void  SaveTables( const std::string&  output_dir, const std::string& name_space );

  /*
    MapTables( class )

    Ищет уже отображённые таблицы в patricia-представление таблицы окончаний; если не существует
    такого - добавляет отображение, и меняет ссылку на компактные таблицы окончаний ссылкой на
    развёрнутые
  */
  morphclass  MapTables( const morphclass& rclass )
    {
      morphclass  mclass( rclass );

      if ( mclass.tfoffs != 0 && mclass.wdinfo != 51 )
      {
        auto  it = iplain.find( mclass.tfoffs );

        if ( it == iplain.end() )
        {
          auto    atable = libmorph::CreatePlainTable( ftable.data(), rclass.tfoffs );
          size_t  theofs = aplain.size();

          assert( (theofs & 0x0f) == 0 );

          aplain.insert( aplain.end(), atable.begin(), atable.end() );
          aplain.resize( (aplain.size() + 0x0f) & ~0x0f );

          iplain.insert( { mclass.tfoffs, theofs } );
          mclass.tfoffs = static_cast<uint16_t>(theofs >> 4);
        }
          else
        mclass.tfoffs = static_cast<uint16_t>(it->second >> 4);
      }
      return mclass;
    }
  stemdata  MapLexStem( const lexemeinfo& li )
    {
      lexemedump  stinfo;

      stinfo.chrmin = li.chrmin;
      stinfo.chrmax = li.chrmax;
      stinfo.oclass = clsset.AddClass( MapTables( li.mclass ) );
      stinfo.stpost = li.stpost;

      return std::make_pair( li.ststem, stinfo );
    }
  std::vector<stemdata> BuildStems( const char* string )
    {
      std::vector<stemdata>   keyset;
      lexemeinfo              lexinf;
      char                    sznorm[0x100];
      char                    szdies[0x20];
      char                    sztype[0x20];
      char                    zindex[0x20];
      char*                   strptr;

    // get the parts
      string = GetSubtext( GetSubtext( GetSubtext( GetSubtext( string,
        sznorm ),
        szdies ),
        sztype ),
        zindex );

    // заменить 'ё' на 'е'
      std::replace( sznorm, sznorm + strlen( sznorm ), 0xB8, 0xe5 );

    // try recolve class
      if ( (lexinf = ResolveClassInfo( sznorm, szdies, sztype, zindex, string,
        ftable.data(), findex,
        mtable.data(), mindex )).mclass != nullclass )
      {
        keyset.push_back( MapLexStem( lexinf ) );
        return keyset;
      }

    // sheck if no alter forms
      if ( (strptr = (char*)strstr( zindex, "//" )) != NULL )
      {
        char  zapart[0x20];

        strncpy( zapart, zindex, strptr - zindex )[strptr - zindex] = '\0';

        if ( (lexinf = ResolveClassInfo( sznorm, szdies, sztype, zapart, string, ftable.data(), findex, mtable.data(), mindex )).mclass != nullclass )
          keyset.push_back( MapLexStem( lexinf ) );

        if ( (lexinf = ResolveClassInfo( sznorm, szdies, sztype, strptr + 2, string, ftable.data(), findex, mtable.data(), mindex )).mclass != nullclass )
          keyset.push_back( MapLexStem( lexinf ) );
      }

      return keyset;
    }
};

# include <tools/buildmorph.h>

class BuildRus: public buildmorph<lexemedump, ResolveRus>
{
  using inherited = buildmorph<lexemedump, ResolveRus>;

  bool  GetSwitch( std::string& out, const char* arg, const char* key ) const
  {
    size_t  keylen = strlen( key );

    if ( strncmp( arg, key, keylen ) != 0 )
      return false;

    if ( arg[keylen] != '=' && arg[keylen] != ':' )
      return false;

    if ( out.length() != 0 )
      throw std::runtime_error( std::string( "'" ) + key + "' is already defined as '" + out + "'" );

    out = arg + 1 + keylen;
      return true;
  };

public:
  BuildRus(): buildmorph<lexemedump, ResolveRus>( rusmorph, libmorph_rus_license, codepages::codepage_866 ), rusmorph()
    {}
  template <class Args>
  int   Run( Args& args )
    {
      std::string flex_tab;
      std::string flex_idx;
      std::string intr_tab;
      std::string intr_idx;

      std::string outp_dir;
      std::string name_spc;
      std::string codepage;
      std::string unk_name;

      std::vector<const char*>  dict_set;

      for ( auto arg = args.begin(); arg != args.end(); ++arg )
      {
        if ( **arg == '-' )
        {
          if ( !GetSwitch( flex_tab, 1 + *arg, "flex-table" )
            && !GetSwitch( flex_idx, 1 + *arg, "flex-index" )
            && !GetSwitch( intr_tab, 1 + *arg, "intr-table" )
            && !GetSwitch( intr_idx, 1 + *arg, "intr-index" )
            && !GetSwitch( outp_dir, 1 + *arg, "target-dir" )

            && !GetSwitch( unk_name, 1 + *arg, "unknown" )
            && !GetSwitch( name_spc, 1 + *arg, "namespace" )
            && !GetSwitch( codepage, 1 + *arg, "codepage" ) )
          throw std::runtime_error( "invalid switch: " + std::string( *arg ) );
        }
          else
        dict_set.push_back( *arg );
      }

    // check parameters
      if ( outp_dir.empty() )
        libmorph::LogMessage( 0, "undefined output directory was set to default '%s'\n", (outp_dir = "./").c_str() );

      if ( name_spc.empty() )
        libmorph::LogMessage( 0, "undefined 'namespace' was set to default '%s'\n", (name_spc = "__libmorphrus__").c_str() );

      if ( unk_name.empty() )
        libmorph::LogMessage( 0, "'-unknown' parameter undefined, unknown words will not be dumped\n" );

      if ( flex_tab.empty() || flex_idx.empty() || intr_tab.empty() || intr_idx.empty() )
        throw std::runtime_error( "no flexion/interchange tables specified, use --help" );

      if ( dict_set.size() == 0 )
        throw std::runtime_error( "no dictinaries specified, use --help" );

      SetUnknowns( unk_name ).
      SetNamespace( name_spc ).
      SetTargetDir( outp_dir );

      rusmorph.InitTables( flex_tab, flex_idx, intr_tab, intr_idx );
        CreateDict( dict_set );
      rusmorph.SaveTables( outp_dir, name_spc );

      return 0;
    }

protected:
  ResolveRus  rusmorph;

};

// ResolveRus implementation

void  ResolveRus::InitTables( const std::string&  flex_table, const std::string& flex_index,
                              const std::string&  intr_table, const std::string& intr_index )
{
  static const char amagic[] = "*inflex by Keva*";

  aplain.insert( aplain.end(), amagic, amagic + 16 );

// load flex ad mix tables
  ftable = libmorph::LoadSource( flex_table.c_str() );
    LoadObject( findex, flex_index );
  mtable = libmorph::LoadSource( intr_table.c_str() );
    LoadObject( mindex, intr_index );
}

void  ResolveRus::SaveTables( const std::string& outdir, const std::string& nspace )
{
  size_t  length;

  if ( (length = clsset.GetBufLen()) != (size_t)-1 )
    libmorph::LogMessage( 0, "info: type dictionary size is %d bytes.\n", length );
  else throw std::runtime_error( "fault to calculate the type dictionary size!" );

  libmorph::BinaryDumper().OutDir( outdir ).Namespace( nspace ).Header( libmorph_rus_license ).
    Dump( "mxTables", mtc::serialbuf( mtable.data(), mtable.size() ) );
  libmorph::BinaryDumper().OutDir( outdir ).Namespace( nspace ).Header( libmorph_rus_license ).
    Dump( "flexTree", mtc::serialbuf( aplain.data(), aplain.size() ) );
  libmorph::BinaryDumper().OutDir( outdir ).Namespace( nspace ).Header( libmorph_rus_license ).
    Dump( "mixTypes", mtc::serialbuf( mixTypes, sizeof(mixTypes) ) );
  libmorph::BinaryDumper().OutDir( outdir ).Namespace( nspace ).Header( libmorph_rus_license ).
    Dump( "classmap", clsset );
}

std::vector<char>         LoadCommandFile( const char* szpath )
{
  std::vector<char> output;
  FILE*             lpfile = nullptr;

  try
  {
    if ( (lpfile = fopen( szpath, "rb" )) == nullptr )
      throw std::runtime_error( "could not open the responce file '" + std::string( szpath ) + "'" );

    while ( !feof( lpfile ) )
    {
      char  chbuff[0x400];
      auto  cbread = fread( chbuff, 1, sizeof(chbuff), lpfile );

      if ( cbread != 0 )
        output.insert( output.end(), chbuff, cbread + chbuff );
    }

    output.push_back( 0 );

    fclose( lpfile );

    return output;
  }
  catch ( ... )
  {
    if ( lpfile != nullptr )
      fclose( lpfile );
    throw;
  }
}

std::vector<const char*>  ReadCommandLine( int argc, char* argv[] )
{
  std::vector<const char*>  out;

  for ( auto arg = argv + 1; arg < argv + argc; ++arg )
    if ( **arg != '@' )
      out.push_back( *arg );

  return out;
}

std::vector<const char*>  ReadCommandFile( std::vector<const char*>&& cmd, std::vector<char>& buf )
{
  std::replace_if( buf.begin(), buf.end(), []( char ch ){  return (unsigned char)ch <= 0x20;  }, '\0' );

  for ( auto top = buf.begin(); top != buf.end(); )
    if ( *top != '\0' )
    {
      auto  end = top;
      auto  pos = cmd.end();

      if ( *top == '-' )
        while ( end != buf.end() && *end != '\0' && *end != '=' && *end != ':' ) ++end;
      else
        while ( end != buf.end() && *end != '\0' ) ++end;

    // search key in the list
      for ( auto p = cmd.begin(); p != cmd.end() && pos == cmd.end(); ++p )
        if ( strncmp( buf.data() + (top - buf.begin()), *p, end - top ) == 0 )
          pos = p;

    // check of overriden in args
      if ( pos == cmd.end() )
        cmd.push_back( buf.data() + (top - buf.begin()) );

      while ( top != buf.end() && *top != '\0' ) ++top;
      while ( top != buf.end() && *top == '\0' ) ++top;
    }

  return std::move( cmd );
}

/*
*/
char about[] = "makerus - the dictionary builder;\n"
               "Usage: makerus [@options_pack] [options]\n"
               "Options pack is a file with all the options listed, and command-line options override it's contents if used.\n"
               "Options are:\n"
               "\t" "-flex-table=compiled_inflexion_tables_file_name\n"
               "\t" "-flex-index=compiled-inflexion_tables_index_file_name\n"
               "\t" "-intr-table=compiled_interchange_tables_file_name\n"
               "\t" "-intr-index=compiled-interchange_tables_index_file_name\n"
               "\t" "-target-dir=directory_to_dump_the_dictionaries\n"
               "\t" "-unknown=unknown_words_file_path\n"
               "\t" "-namespace=namespace_name_to_contain_dictionaries, default is __libmorphrus__\n"
               "\t" "-codepage=codepage_name_for_dictionaries_sources\n"
               ;

# include <tools/serialize.h>

int   main( int argc, char* argv[] )
{
  try
  {
    BuildRus          generate;
    auto              commands = ReadCommandLine( argc, argv );
    std::vector<char> respfile;

    if ( argc < 2 )
      return libmorph::LogMessage( 0, about );

    if ( *argv[1] == '@' )
    {
      respfile = LoadCommandFile( argv[1] + 1 );
      commands = ReadCommandFile( std::move( commands ), respfile );
    }

    return generate.Run( commands );
  }
  catch ( const std::runtime_error& x )
  {
    return libmorph::LogMessage( EFAULT, "%s\n", x.what() );
  }
}
