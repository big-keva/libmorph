# include <tools/buildmorph.h>
# include <tools/plaintable.h>
# include <tools/ftables.h>
# include "mtables.h"
# include "lresolve.h"
# include <tools/dumppage.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <stdarg.h>

#if defined( _MSC_VER ) && defined( _DEBUG )
  #include <crtdbg.h>
#endif

# if defined( _MSC_VER )
#   pragma warning( disable: 4237 )
# endif // _MSC_VER

using namespace libmorph;

static unsigned char mixTypes[64] =
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

static char GPL_header[] =
  "/******************************************************************************\n"
  "\n"
  "    libmorphrus - dictiorary-based morphological analyser for Russian.\n"
  "    Copyright (C) 1994-2016 Andrew Kovalenko aka Keva\n"
  "\n"
  "    This program is free software; you can redistribute it and/or modify\n"
  "    it under the terms of the GNU General Public License as published by\n"
  "    the Free Software Foundation; either version 2 of the License, or\n"
  "    (at your option) any later version.\n"
  "\n"
  "    This program is distributed in the hope that it will be useful,\n"
  "    but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
  "    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
  "    GNU General Public License for more details.\n"
  "\n"
  "    You should have received a copy of the GNU General Public License along\n"
  "    with this program; if not, write to the Free Software Foundation, Inc.,\n"
  "    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.\n"
  "\n"
  "    Commercial license is available upon request."
  "\n"
  "    Contacts:\n"
  "      email: keva@meta.ua, keva@rambler.ru\n"
  "      Skype: big_keva\n"
  "      Phone: +7(495)648-4058, +7(926)513-2991\n"
  "\n"
  "******************************************************************************/\n";

class ResolveRus
{
  array<char>         ftable;
  CReferences         findex;
  array<char>         mtable;
  mixfiles            mindex;

  array<char>         aplain;
  stringmap<unsigned> iplain;

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
public:
  int   InitTables( const ZArray& config )
    {
      const ZArray* tabset;
      file          lpfile;

      if ( aplain.Append( 16, (char*)"*inflex by Keva*" ) != 0 )
        return ENOMEM;

    // load flex tables
      if ( (tabset = config.get_zarray( "flex" )) != nullptr )
      {
        const char*   ptable;
        const char*   pindex;

        if ( (ptable = tabset->get_charstr( "table" )) == nullptr )
          return LogMessage( EINVAL, "Configuration file contains no 'flex->table' string variable!\n" );

        if ( (pindex = tabset->get_charstr( "index" )) == nullptr )
          return LogMessage( EINVAL, "Configuration file contains no 'flex->index' string variable!\n" );

        if ( LoadSource( ftable, ptable ) != 0 )
          return LogMessage( ENOENT, "Could not open file \'%s\'!\n", ptable );

        if ( (lpfile = fopen( pindex, "rb" )) == NULL )
          return LogMessage( ENOENT, "Could not open file \'%s\'!\n", pindex );

        if ( findex.FetchFrom( (FILE*)lpfile ) == NULL )
          return LogMessage( EACCES, "Could not load the flex tables references!\n" );
      }
        else
      return LogMessage( EINVAL, "Configuration file contains no 'flex->{}' section!\n" );

    // load interchange tables
      if ( (tabset = config.get_zarray( "intr" )) != nullptr )
      {
        const char*   ptable;
        const char*   pindex;

        if ( (ptable = tabset->get_charstr( "table" )) == nullptr )
          return LogMessage( EINVAL, "Configuration file contains no 'intr->table' string variable!\n" );

        if ( (pindex = tabset->get_charstr( "index" )) == nullptr )
          return LogMessage( EINVAL, "Configuration file contains no 'intr->index' string variable!\n" );

        if ( LoadSource( mtable, ptable ) != 0 )
          return LogMessage( ENOENT, "Could not open file \'%s\'!\n", ptable );

        if ( (lpfile = fopen( pindex, "rb" )) == NULL )
          return LogMessage( ENOENT, "Could not open file \'%s\'!\n", pindex );

        if ( mindex.DoLoad( mtable, (FILE*)lpfile ) == NULL )
          return LogMessage( EACCES, "Could not load the interchange references!\n" );
      }
        else
      return LogMessage( EINVAL, "Configuration file contains no 'intr->{}' section!\n" );

      return 0;
    }
  int   SaveTables( const ZArray& settings )
    {
      const char* dstdir = settings.get_charstr( "TargetDir", "./" );
      const char* nspace = settings.get_charstr( "namespace", "__libmorphrus__" );

      BinaryDumper().OutDir( dstdir ).Namespace( nspace ).Header( GPL_header ).Dump( "mxTables", serialbuff( mtable, mtable.GetLen() ) );
      BinaryDumper().OutDir( dstdir ).Namespace( nspace ).Header( GPL_header ).Dump( "flexTree", serialbuff( aplain, aplain.GetLen() ) );
      BinaryDumper().OutDir( dstdir ).Namespace( nspace ).Header( GPL_header ).Dump( "mixTypes", serialbuff( mixTypes, sizeof(mixTypes) ) );

      return 0;
    }
  int   PatchClass( rusclassinfo& rclass )
    {
      unsigned* ptrofs;
      char      sclass[0x20];

      if ( rclass.tfoffs == 0 || rclass.wdinfo == 51 )
        return 0;
      sprintf( sclass, "%x", rclass.tfoffs );

      if ( (ptrofs = iplain.Search( sclass )) == NULL )
      {
        wordtree<gramlist>  atable;
        size_t              ltable;
        size_t              theofs;
        char                szflex[0x40];
        
        if ( FillFlexTree( atable, ftable, rclass.tfoffs, 0, 0xff, szflex, 0 ) != 0 ) 
          return ENOMEM;

        ltable = atable.GetBufLen();
        theofs = aplain.GetLen();

        assert( (theofs & 0x0f) == 0 );

        if ( aplain.SetLen( (theofs + ltable + 0x0f) & ~0x0f ) == 0 )  atable.Serialize( (char*)aplain + theofs );
          else return ENOMEM;

        if ( (ptrofs = iplain.Insert( sclass, theofs )) == nullptr )
          return ENOMEM;
      }

      rclass.tfoffs = *ptrofs >> 4;
      return 0;
    }
  int   BuildStems( char* pstems, rusclassinfo* pclass, const char* string, const zarray<>& zasets )
    {
      const zarray<>* ztypes;
      char            sznorm[0x100];
      char            szdies[0x20];
      char            sztype[0x20];
      char            zindex[0x20];
      char*           strptr;

      if ( (ztypes = zasets.get_zarray( "type" )) == nullptr )
        return EINVAL;

    // get the parts
      string = GetSubtext( GetSubtext( GetSubtext( GetSubtext( string,
        sznorm ),
        szdies ),
        sztype ),
        zindex );

      for ( auto p = sznorm; *p != '\0'; ++p )
        if ( *p == 'ё' ) *p = 'е';

    // try recolve class
      if ( ResolveClassInfo( *ztypes, pstems, *pclass, sznorm, szdies, sztype, zindex, string, ftable, findex, mtable, mindex ) )
        return PatchClass( *pclass ) == 0 ? 1 : -1;

    // sheck if no alter forms
      if ( (strptr = (char*)strstr( zindex, "//" )) != NULL )
      {
        char  zapart[0x20];
        int   rescnt = 0;

        strncpy( zapart, zindex, strptr - zindex )[strptr - zindex] = '\0';

        if ( ResolveClassInfo( *ztypes, pstems, *pclass, sznorm, szdies, sztype, zapart, string, ftable, findex, mtable, mindex ) )
        {
          if ( PatchClass( *pclass++ ) == 0 ) ++rescnt;
            else return -1;
          while ( *pstems++ != '\0' )
            (void)NULL;
        }

        if ( ResolveClassInfo( *ztypes, pstems, *pclass, sznorm, szdies, sztype, strptr + 2, string, ftable, findex, mtable, mindex ) )
        {
          if ( PatchClass( *pclass++ ) == 0 ) ++rescnt;
            else return -1;
        }

        return rescnt;
      }

      return 0;
    }
};

struct  rusteminfo
{
  byte_t    chrmin;
  byte_t    chrmax;
  unsigned  nlexid;
  word16_t  oclass;
  char      szpost[0x10];

public:     // comparison
  bool  operator <  ( const rusteminfo& r ) const {  return compare( r ) < 0;   }
  bool  operator == ( const rusteminfo& r ) const {  return compare( r ) == 0;  }

public:     // serialization
  size_t  GetBufLen() const
    {
      return 2 + ::GetBufLen( nlexid ) + sizeof(word16_t) + (szpost[0] != 0 ? strlen( szpost ) + 1 : 0);
    }
  template <class O>
  O*      Serialize( O* o ) const
    {
      word16_t  wstore = oclass | (szpost[0] != 0 ? 0x8000 : 0);

      o = ::Serialize( ::Serialize( ::Serialize( ::Serialize( o,
        &chrmin, sizeof(chrmin) ),
        &chrmax, sizeof(chrmax) ), nlexid ), &wstore, sizeof(wstore) );
      return (wstore & 0x8000) != 0 ? ::Serialize( o, szpost ) : o;
    }

protected:  // helpers
  int   compare( const rusteminfo& r ) const
    {
      int   rescmp;

      if ( (rescmp = r.chrmax - chrmax) == 0 )
      if ( (rescmp = chrmin - r.chrmin) == 0 )
      if ( (rescmp = strcmp( szpost, r.szpost )) == 0 )
            rescmp = nlexid - r.nlexid;
      return rescmp;
    }
};

class BuildRus: public buildmorph<rusclassinfo, rusteminfo, ResolveRus>
{
  ResolveRus  rusmorph;

public:
  BuildRus(): buildmorph<rusclassinfo, rusteminfo, ResolveRus>( rusmorph, GPL_header, codepages::codepage_866 ), rusmorph()
    {
    }
  int   Run( const char* pszcfg )
    {
      int   nerror;

      if ( (nerror = buildmorph<rusclassinfo, rusteminfo, ResolveRus>::Initialize( pszcfg )) != 0 )
        return nerror;
      if ( (nerror = rusmorph.InitTables( settings )) != 0 )
        return nerror;
      if ( (nerror = CreateDict()) != 0 )
        return nerror;
      if ( (nerror = rusmorph.SaveTables( settings )) != 0 )
        return nerror;
      return 0;
    }
};

/*
*/
char about[] = "makerus - the dictionary builder;\n"
               "Usage: makerus ininame\n";

int   main( int argc, char* argv[] )
{
  BuildRus  generate;

  if ( argc < 2 )
    return LogMessage( 0, about );

  return generate.Run( argv[1] );
}
