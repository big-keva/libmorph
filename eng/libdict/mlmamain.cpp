/******************************************************************************

    libmorpheng - dictiorary-based morphological analyser for English.
    Copyright (C) 1994-2016 Andrew Kovalenko aka Keva

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

    Commercial license is available upon request.

    Contacts:
      email: keva@meta.ua, keva@rambler.ru
      Skype: big_keva
      Phone: +7(495)648-4058, +7(926)513-2991

******************************************************************************/
# include "xmorph/wildscan.h"
# include "xmorph/charlist.h"
# include "../include/mlma1049.h"
# include "../../codepages.hpp"
# include "../chartype.h"
# include "mlmadefs.h"
# include "scanClass.hpp"
//# include "wildClass.hpp"
# include "lemmatize.hpp"
# include "buildform.hpp"
# include <cstdlib>
# include <cstring>
# include <errno.h>

# if !defined( _WIN32_WCE )
  # define  CATCH_ALL         try {
  # define  ON_ERRORS( code ) } catch ( ... ) { return (code); }
# else
  # define  CATCH_ALL
  # define  ON_ERRORS( code )
# endif  // ! _WIN32_WCE

namespace libmorph {
namespace eng {

  struct anyway_ok
  {
    template <class ... args>
    int  operator()( args... ) const  {  return 1;  }
  };

  enum  maximal: size_t
  {
    form_length = 48,               // максимальная длина слова в буквак
    utf8_length = form_length * 2,  // максимальная длина слова в байтах
    forms_count = 256,              // реально чуть меньше, но у глаголов много, до 352
    buffer_size = forms_count * utf8_length
  };

  //
  // the new api - IMlma interface class
  //
  struct  CMlmaMb: public IMlmaMb
  {
    int MLMAPROC  Attach() override {  return 0;  }
    int MLMAPROC  Detach() override {  return 0;  }

    int MLMAPROC  CheckWord( const char*    pszstr, size_t    cchstr,
                             unsigned       dwsets )                   override;
    int MLMAPROC  Lemmatize( const char*    pszstr, size_t    cchstr,
                             SLemmInfoA*    output, size_t    cchout,
                             char*          plemma, size_t    clemma,
                             SGramInfo*     pgrams, size_t    ngrams,
                             unsigned       dwsets )                   override;
    int MLMAPROC  BuildForm( char*          output, size_t    cchout,
                             lexeme_t       nlexid, formid_t  idform ) override;
    int MLMAPROC  FindForms( char*          output, size_t    cchout,
                             const char*    pszstr, size_t    cchstr,
                             formid_t       idform, unsigned  dwsets ) override;
    int MLMAPROC  CheckHelp( char*          output, size_t    cchout,
                             const char*    pszstr, size_t    cchstr ) override;
    int MLMAPROC  GetWdInfo( unsigned char* pwindo, lexeme_t  nlexid ) override;
    int MLMAPROC  FindMatch( IMlmaMatch*    pienum,
                             const char*    pszstr, size_t    cchstr ) override;

  public:     // construction
    CMlmaMb( unsigned cp = codepages::codepage_1251 ): codepage( cp ) {}

  protected:
    template <size_t N>
    int           GetJocker( uint8_t (&)[N], const char*, size_t ) const;
    template <size_t N>
    unsigned      ToCanonic( uint8_t (&)[N], const char*, size_t ) const;

  protected:  // codepage
    unsigned  codepage;

  };

  struct  CMlmaWc: public IMlmaWc
  {
    int MLMAPROC  Attach() override {  return 0;  }
    int MLMAPROC  Detach() override {  return 0;  }

    int MLMAPROC  CheckWord( const widechar*  pszstr, size_t    cchstr,
                             unsigned         dwsets )  override;
    int MLMAPROC  Lemmatize( const widechar*  pszstr, size_t    cchstr,
                             SLemmInfoW*      output, size_t    cchout,
                             widechar*        plemma, size_t    clemma,
                             SGramInfo*       pgrams, size_t    ngrams,
                             unsigned         dwsets )  override;
    int MLMAPROC  BuildForm( widechar*        output, size_t    cchout,
                             lexeme_t         nlexid, formid_t  idform )  override;
    int MLMAPROC  FindForms( widechar*        output, size_t    cchout,
                             const widechar*  pwsstr, size_t    cchstr,
                             unsigned char    idform, unsigned  dwsets )  override;
    int MLMAPROC  CheckHelp( widechar*        output, size_t    cchout,
                             const widechar*  pwsstr, size_t    cchstr )  override;
    int MLMAPROC  GetWdInfo( unsigned char*   pwindo, lexeme_t  nlexid )  override;
    int MLMAPROC  FindMatch( IMlmaMatch*      pmatch,
                             const widechar*  pszstr, size_t    cchstr )  override;
  };

  CMlmaMb mlmaMbInstance;
//  CMlmaWc mlmaWcInstance;

  // CMlmaMb implementation

  int   CMlmaMb::CheckWord( const char* pszstr, size_t  cchstr, unsigned  dwsets )
  {
    CATCH_ALL
      uint8_t locase[maximal::form_length];
      auto    scheme = ToCanonic( locase, pszstr, cchstr );

      // check source string and length
      if ( scheme == (unsigned)-1 || scheme == 0 )
        return scheme == (unsigned)-1 ? WORDBUFF_FAILED : 0;

      // get capitalization scheme; if it is invalid, do not pass time to analyses
      if ( uint8_t(scheme) == 0xff && (dwsets & sfIgnoreCapitals) == 0 )
        return 0;

      // fill scheck structure
      return Flat::ScanTree<uint8_t>( Flat::ScanList( MakeClassMatch( anyway_ok() )
        .SetCapitalization( uint16_t(scheme) )
        .SetSearchSettings( dwsets ) ), libmorpheng::stemtree, { locase, scheme >> 16 } );
    ON_ERRORS( -1 )
  }

  int   CMlmaMb::Lemmatize( const char* pszstr, size_t  cchstr,
                            SLemmInfoA* plemma, size_t  llemma,
                            char*       pforms, size_t  lforms,
                            SGramInfo*  pgrams, size_t  lgrams, unsigned  dwsets )
  {
    CATCH_ALL
      uint8_t locase[maximal::form_length];
      auto    scheme = ToCanonic( locase, pszstr, cchstr );
      int     nerror;

    // check source string and length
      if ( scheme == (unsigned)-1 || scheme == 0 )
        return scheme == (unsigned)-1 ? WORDBUFF_FAILED : 0;

    // get capitalization scheme; if it is invalid, do not pass time to analyses
      if ( uint8_t(scheme) == 0xff && (dwsets & sfIgnoreCapitals) == 0 )
        return 0;

    // create lemmatizer object
      auto  lemmatize = Lemmatizer( CapScheme( charTypeMatrix, toLoCaseMatrix, toUpCaseMatrix ),
        { plemma, llemma },
        { pforms, lforms, codepage },
        { pgrams, lgrams }, locase, dwsets );

      nerror = Flat::ScanTree<uint8_t>( Flat::ScanList( MakeClassMatch( lemmatize )
        .SetCapitalization( scheme & 0xffff )
        .SetSearchSettings( dwsets ) ),
      libmorpheng::stemtree, { locase, scheme >> 16 } );

      return nerror < 0 ? nerror : lemmatize;
    ON_ERRORS( -1 )
  }

  int   CMlmaMb::BuildForm( char* output, size_t cchout, lexeme_t nlexid, formid_t idform )
  {
    CATCH_ALL
      uint8_t         lidkey[0x10];
      const uint8_t*  ofsptr;
      auto            getofs = []( const byte_t* thedic, const fragment& str ) {  return str.empty() ? thedic : nullptr;  };

    // check the arguments
      if ( output == nullptr || cchout == 0 )
        return ARGUMENT_FAILED;

    // No original word form; algo jumps to lexeme block dictionary point by lexeme id
      if ( (ofsptr = Flat::ScanTree<uint16_t>( getofs, libmorpheng::lidstree, { lidkey, lexkeylen( lidkey, nlexid ) } )) != nullptr )
      {
        auto    dicpos = libmorpheng::stemtree + getserial( ofsptr );
        byte_t  szstem[maximal::form_length] = {};
        int     nerror;

      // fill other fields
        auto  buildform = FormBuild( { output, cchout, codepage }, CapScheme( charTypeMatrix, toLoCaseMatrix, toUpCaseMatrix ),
          nlexid,
          idform,
          szstem );

        nerror = Flat::GetTrack<uint8_t>( Flat::ViewList( MakeBuildClass( buildform ), dicpos ),
          libmorpheng::stemtree, szstem, 0, dicpos );

        return nerror < 0 ? nerror : buildform;
      }
      return 0;
    ON_ERRORS( -1 )
  }

  int   CMlmaMb::FindForms(
    char*       output, size_t    cchout,
    const char* pszstr, size_t    cchstr,
    formid_t    idform, unsigned  dwsets )
  {
    /*
    CATCH_ALL
      uint8_t locase[maximal::form_length];
      auto    scheme = ToCanonic( locase, pszstr, cchstr );
      int     nerror;

    // check the arguments
      if ( output == nullptr || cchout == 0 )
        return ARGUMENT_FAILED;

    // check source string and length
      if ( scheme == (unsigned)-1 || scheme == 0 )
        return scheme == (unsigned)-1 ? WORDBUFF_FAILED : 0;

    // get capitalization scheme; if it is invalid, do not pass time to analyses
      if ( uint8_t(scheme) == 0xff && (dwsets & sfIgnoreCapitals) == 0 )
        return 0;

    // create form builder
      auto  buildform = FormBuild( { output, cchout, codepage }, CapScheme(
        charTypeMatrix, toLoCaseMatrix,
        toUpCaseMatrix, pspMinCapValue ), 0x0000, idform, locase );

      nerror = Flat::ScanTree<uint8_t>( Flat::ScanList( MakeClassMatch( buildform )
        .SetCapitalization( uint16_t(scheme) )
        .SetSearchSettings( dwsets ) ),
      libmorphrus::stemtree, { locase, scheme >> 16 } );

      return nerror < 0 ? nerror : buildform;
    ON_ERRORS( -1 )
    */
  }

 /*
  * CheckHelp( output, cchout, pszstr, cchstr )
  *
  * Реализация через FindMatch( ... ) требует предварительной обработки шаблона к форме
  * 'либо одна звёздочка в конце, либо один знак вопроса где угодно'
  */
  int   CMlmaMb::CheckHelp( char* output, size_t  cchout, const char* pszstr, size_t  cchstr )
  {
    /*
    CATCH_ALL
      uint8_t locase[maximal::form_length];
      uint8_t smatch[maximal::form_length];
      auto    scheme = ToCanonic( locase, pszstr, cchstr );
      size_t  qtrpos;       // позиция квантора в строке
      size_t  keylen;       // длина поискового ключа
      Charset achars;
      int     nerror;

    // check source string and length
      if ( scheme == (unsigned)-1 || scheme == 0 )
        return scheme == (unsigned)-1 ? WORDBUFF_FAILED : 0;

    // check output buffer
      if ( output == nullptr || cchout == 0 )
        return ARGUMENT_FAILED;

    // провериьт наличие и рассчитать позицию квантора
      for ( auto  hasqtr = keylen = (qtrpos = size_t(-1)) + 1; keylen != (scheme >> 16); )
      {
        auto  chnext = locase[keylen++];

        if ( chnext == '*' )
        {
          qtrpos = keylen - 1;
          break;
        }
          else
        if ( chnext == '?' )
        {
          if ( hasqtr++ == 0 )  qtrpos = keylen - 1;
            else return ARGUMENT_FAILED;
        }
      }

    // проверить наличие кванторов
      if ( qtrpos != size_t(-1) && keylen != 0 )  locase[keylen] = '\0';
        else return 0;

    // scan the dictionary
      if ( (nerror = Wild::ScanTree<uint8_t>(
        MakeModelMatch( [&]( lexeme_t, const uint8_t* str, size_t len, const SGramInfo& )
          {  return achars( qtrpos < len ? str[qtrpos] : uint8_t(0) ), 0;  } ),
        libmorphrus::stemtree, { locase, scheme >> 16 }, smatch, 0 )) != 0 )
      return nerror;

      return achars( MbcsCoder( output, cchout, codepage ).object() );
    ON_ERRORS( -1 )
    */
  }

  int   CMlmaMb::GetWdInfo( unsigned char* pwinfo, lexeme_t lexkey )
  {
    CATCH_ALL
      byte_t        lidkey[0x10];
      const byte_t* ofsptr;
      auto          getofs = []( const byte_t* thedic, const fragment& str ){  return str.empty() ? thedic : nullptr;  };

    // No original word form; algo jumps to lexeme block dictionary point by lexeme id
      if ( (ofsptr = Flat::ScanTree<word16_t>( getofs, libmorpheng::lidstree, { lidkey, lexkeylen( lidkey, lexkey ) } )) != nullptr )
      {
        const byte_t* dicpos = libmorpheng::stemtree + getserial( ofsptr ) + 2; // 2 => clower && cupper
        lexeme_t      nlexid = getserial( dicpos );
        word16_t      oclass = getword16( dicpos );
        steminfo      stinfo;

        if ( nlexid != lexkey )
          return LIDSBUFF_FAILED;

        *pwinfo = stinfo.Load( libmorpheng::classmap + (oclass & 0x7fff) ).wdinfo & 0x3f;
          return 1;
      }
      return 0;
    ON_ERRORS( -1 )
  }

  int   CMlmaMb::FindMatch( IMlmaMatch*  pmatch, const char* pszstr, size_t cchstr )
  {
    /*
    CATCH_ALL
      char      cpsstr[maximal::form_length];
      uint8_t   locase[maximal::form_length];
      uint8_t   sforms[maximal::buffer_size];
      SStrMatch amatch[maximal::forms_count];
      uint8_t*  pforms;
      size_t    nmatch = 0;
      lexeme_t  lastId = 0;
      int       nerror;
      auto      reglex = [&](
        lexeme_t          nlexid,
        const uint8_t*    string,
        size_t            length,
        const SGramInfo&  grinfo )
      {
        if ( nlexid != lastId )
        {
          if ( lastId != 0 )
            pmatch->RegisterLexeme( lastId, nmatch, amatch );
          lastId = nlexid;
          pforms = sforms;
          nmatch = 0;
        }

        for ( auto prev = amatch, last = amatch + nmatch; prev != last; ++prev )
          if ( prev->id == grinfo.idForm && prev->cc == length && memcmp( prev->sz, string, length ) == 0 )
            return 0;

        amatch[nmatch++] = { strncpy( (char*)pforms, (const char*)string, length ), length, grinfo.idForm };
          pforms += length + 1;

        return 0;
      };

    // check output buffer
      if ( pmatch == nullptr )
        return ARGUMENT_FAILED;

    // check template defined
      if ( pszstr == nullptr )
        return ARGUMENT_FAILED;

    // check length defined
      if ( cchstr == (size_t)-1 )
        for ( cchstr = 0; pszstr[cchstr] != 0; ++cchstr ) (void)NULL;

    // check length defined
      if ( cchstr == 0 )
        return 0;

    // check for overflow
      if ( cchstr >= sizeof(locase) )
        return WORDBUFF_FAILED;

    // modify the codepage
      if ( codepage != codepages::codepage_1251 )
      {
        if ( (cchstr = ToInternal( cpsstr, codepage, pszstr, cchstr )) == (size_t)-1 )
          return WORDBUFF_FAILED;
        pszstr = cpsstr;
      }

    // change the word to the lower case
      CapScheme(
        charTypeMatrix, toLoCaseMatrix,
        toUpCaseMatrix, pspMinCapValue ).Get( locase, pszstr, cchstr );
      locase[cchstr] = '\0';

    // scan the dictionary
      if ( (nerror = Wild::ScanTree<uint8_t>( MakeModelMatch( reglex ),
        libmorphrus::stemtree, { locase, cchstr }, (uint8_t*)cpsstr, 0 )) != 0 )
      return nerror;

      return lastId != 0 ? pmatch->RegisterLexeme( lastId, nmatch, amatch ) : 0;
    ON_ERRORS( -1 )
    */
  }

  template <size_t N>
  auto  CMlmaMb::ToCanonic( uint8_t (& output)[N], const char* pszstr, size_t cchstr ) const -> unsigned
  {
    char  mbsstr[N];

    if ( pszstr == nullptr )
      return 0;

    if ( cchstr == (size_t)-1 )
      for ( cchstr = 0; pszstr[cchstr] != 0; ++cchstr ) (void)NULL;

    if ( cchstr == 0 )
      return 0;

    if ( codepage != codepages::codepage_1251 )
    {
      if ( (cchstr = ToInternal( mbsstr, codepage, pszstr, cchstr )) != (size_t)-1 )  pszstr = mbsstr;
        else return (unsigned)-1;
    }

    return CapScheme( charTypeMatrix, toLoCaseMatrix, toUpCaseMatrix ).Get(
      output,
      pszstr,
      cchstr );
  }

  /*
    short MLMA_API EXPORT mlmaenCheckWord( const char* lpWord, unsigned short options )
  {
    SScanPage scan;
    char      word[256];
    unsigned  size;

    CATCH_ALL
      if ( (size = GetCapScheme( lpWord, word, sizeof(word) - 1, scan.scheme ))
        == (unsigned)-1 )
          return WORDBUFF_FAILED;

      scan.lpWord = word;
      scan.nFlags = options | sfStopAfterFirst;
      scan.lpData = NULL;

      return ScanDictionary( glossary, word, size, scan, NULL );
    ON_ERRORS( -1 )
  }

  short MLMA_API EXPORT mlmaenLemmatize( const char*    lpWord,
                                         word16_t       dwsets,
                                         char*          lpLemm,
                                         lexeme_t*      lpLIDs,
                                         char*          lpGram,
                                         word16_t       ccLemm,
                                         word16_t       cdwLID,
                                         word16_t       cbGram )
  {
    SScanPage scan;
    SLemmInfo lemm;
    char      word[256];
    unsigned  size;

    CATCH_ALL
      if ( (size = GetCapScheme( lpWord, word, sizeof(word) - 1, scan.scheme ))
        == (unsigned)-1 )
          return WORDBUFF_FAILED;

      lemm.lpDest = lpLemm;   //
      lemm.ccDest = ccLemm;   //
      lemm.lpLids = lpLIDs;   //
      lemm.clLids = cdwLID;   //
      lemm.lpInfo = lpGram;   //
      lemm.cbInfo = cbGram;   //
      lemm.failed = 0;        //
      lemm.fBuilt = 0;        //

      scan.lpWord = word;
      scan.nFlags = dwsets;
      scan.lpData = &lemm;

      if ( !ScanDictionary( glossary, word, size, scan, actLemmatize ) )
        return 0;
      if ( lemm.failed != 0 ) return lemm.failed;
      return lemm.fBuilt;
    ON_ERRORS( -1 )
  }

  short   MLMA_API  EXPORT  mlmaenBuildForm( const char*    pszWord,
                                             lexeme_t       dwLexId,
                                             word16_t       options,
                                             unsigned char  nFormId,
                                             char*          pszDest,
                                             word16_t       nccDest )
  {
    CATCH_ALL
      SScanPage scan;
      SMakeInfo lemm;

      lemm.lpDest = pszDest;
      lemm.ccDest = nccDest;
      lemm.idForm = nFormId;
      lemm.wrdLID = dwLexId;
      lemm.failed = 0;
      lemm.fBuilt = 0;

      scan.nFlags = options;
      scan.lpData = &lemm;
      scan.scheme = 0;

      if ( pszWord != NULL )
      {
        char      word[256];
        unsigned  size;

        if ( (size = GetCapScheme( pszWord, word, sizeof(word) - 1, scan.scheme ))
          == (unsigned)-1 )
            return WORDBUFF_FAILED;

        scan.lpWord = word;

        if ( !ScanDictionary( glossary, word, size, scan, actBuildForm ) )
          return 0;
      }
        else
      {
      //
      //
      //

        scan.lpWord = NULL;

        if ( !DirectPageJump( lidsdict, glossary, dwLexId, scan, actBuildForm ) )
          return 0;
      }
      if ( lemm.failed != 0 )
        return lemm.failed;
      return lemm.fBuilt;
    ON_ERRORS( -1 )
  }

  short MLMA_API EXPORT mlmaenEnumWords( TEnumWords enumproc, void *lpv )
  {
    unsigned        idPage;
    word32_t        idNode;
    SLidsRef*       lpPage;
    unsigned char*  lpData;

    CATCH_ALL
      for ( idPage = 0; idPage < lidsdict.GetCount(); idPage++ )
      {
        word32_t minLid;
        word32_t maxLid;

        lpPage = lidsdict[idPage];
        lpData = lpPage->pagedata;
        minLid = GetWord32( &lpPage->minvalue );
        maxLid = GetWord32( &lpPage->maxvalue );

        for ( idNode = 0; idNode <= maxLid - minLid; idNode++ )
          if ( lpData[idNode * 3] != 0 )
            if ( !enumproc( minLid + idNode, lpv ) )
              return 0;
      }
      return 1;
    ON_ERRORS( -1 )
  }

  short MLMA_API EXPORT mlmaenCheckHelp( const char*  lpWord,
                                         char*        lpList )
  {
    CATCH_ALL
      int   size = strlen( lpWord );
      char  word[256];

      if ( size >= (int)sizeof(word) )
        return 0;
      SetLowerCase( (unsigned char*)memcpy( word, lpWord, size + 1 ) );
      return (short)WildScanDictionary( glossary, (byte08_t*)word, (byte08_t*)lpList );
    ON_ERRORS( -1 )
  }

  short MLMA_API EXPORT mlmaenListForms( lexeme_t       nlexid,
                                         char*          lpbuff,
                                         unsigned       ccbuff )
  {
    CATCH_ALL
      SLemmInfo lemm;
      SScanPage scan;

      lemm.lpDest = lpbuff;
      lemm.ccDest = ccbuff;
      lemm.failed = 0;
      lemm.fBuilt = 0;
      scan.nFlags = 0;
      scan.lpData = (void*)&lemm;
      scan.scheme = 0;

      if ( !DirectPageJump( lidsdict, glossary, nlexid, scan, actListForms ) )
        return 0;

      if ( lemm.failed != 0 )
        return lemm.failed;

      return lemm.fBuilt;
    ON_ERRORS( -1 )
  }

  short MLMA_API EXPORT mlmaenGetWordInfo( lexeme_t       nLexId,
                                           unsigned char* wdinfo )
  {
    CATCH_ALL
      SScanPage scan;

      scan.nFlags = 0;
      scan.lpData = (void*)wdinfo;
      scan.scheme = 0;

      return DirectPageJump( lidsdict, glossary, nLexId, scan, actGetWdType ) ? 1 : 0;
    ON_ERRORS( -1 )
  }

  short MLMA_API EXPORT mlmaenGetClassRef( lexeme_t         nLexId,
                                           unsigned short*  rclass )
  {
    CATCH_ALL
      SScanPage scan;

      scan.nFlags = 0;
      scan.lpData = (void*)rclass;
      scan.scheme = 0;

      return DirectPageJump( lidsdict, glossary, nLexId, scan, actGetClass ) ? 1 : 0;
    ON_ERRORS( -1 )
  }

  //
  // return word type information by the class
  //
  short MLMA_API EXPORT mlmaenGetTypeInfo( unsigned short   iclass,
                                           unsigned char*   lpinfo )
  {
    CATCH_ALL
      char* pclass = (char*)GetInfoByID( iclass );

      if ( lpinfo != NULL ) *lpinfo = (byte08_t)GetWord16( pclass );
        else return -1;
       return 1;
    ON_ERRORS( -1 )
  }
  */

}}

using namespace libmorph::eng;

int   MLMAPROC        mlmaenLoadMbAPI( IMlmaMb**  ptrAPI )
{
  if ( ptrAPI == nullptr )
    return -1;
  *ptrAPI = (IMlmaMb*)&mlmaMbInstance;
    return 0;
}

int   MLMAPROC        mlmaenLoadCpAPI( IMlmaMb**  ptrAPI, const char* codepage )
{
  (void)codepage;
  return mlmaenLoadMbAPI( ptrAPI );
}
/*
int   MLMAPROC        mlmaenLoadWcAPI( IMlmaWc**  ptrAPI )
{
  if ( ptrAPI == nullptr )
    return -1;
  *ptrAPI = (IMlmaWc*)&mlmaWcInstance;
    return 0;
}
*/