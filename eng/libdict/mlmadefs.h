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

    Commercial license is available upоn request.

    Contacts:
      email: keva@meta.ua, keva@rambler.ru
      Skype: big_keva
      Phone: +7(495)648-4058, +7(926)513-2991

******************************************************************************/
# if !defined( __libmorph_eng_mlmadefs_h__ )
# define __libmorph_eng_mlmadefs_h__

// Определить кроссплатформенные типы данных
# include "../include/mlma1033.h"
# include "xmorph/typedefs.h"

namespace libmorpheng {
  extern unsigned char  flexTree[];
  extern unsigned char  classmap[];
  extern unsigned char  stemtree[];
  extern unsigned char  lidstree[];
}

namespace libmorph {
namespace eng
{
  // Define common types used in the analyser
  typedef unsigned char   byte08_t;
  typedef unsigned short  word16_t;
  typedef lexeme_t        word32_t;

# if !defined( mlma_grammarecord_defined )
# define mlma_grammarecord_defined
  typedef struct tagGramInfo
  {
    unsigned char  wInfo;
    unsigned char  iForm;
    unsigned short gInfo;
    unsigned char  other;
  } SGramInfo;
# endif

  // stem access class

  struct  steminfo
  {
    uint16_t  wdinfo;
    uint16_t  tfoffs;

  public:     // init
    steminfo( uint16_t oclass = (uint16_t)-1 )
      {
        if ( oclass != (uint16_t)-1 )
          Load( (const uint8_t*)libmorpheng::classmap + oclass );
      }
    steminfo( uint16_t winfo, uint16_t foffs ):
      wdinfo( winfo ),
      tfoffs( foffs ) {}
    steminfo& Load( const byte_t* pclass )
      {
        wdinfo = getword16( pclass );
        tfoffs = getword16( pclass );
        return *this;
      }
    unsigned      MinCapScheme() const
      {
        return (wdinfo >> 7) & 0x3;
      }
    const byte_t* GetFlexTable() const
      {
        return tfoffs != 0 ? tfoffs + libmorpheng::flexTree : nullptr;
      }
  };

# if 0
  //
  typedef struct tagScanPage
  {
    char*     lpWord;
    unsigned  nFlags;
    void*     lpData;
    unsigned  scheme;
  } SScanPage;

# define wfSuffix   0x8000       /* Stem has pseudo-suffix                 */
# define wfPostSt   0x4000       /* Stem has post-text definition          */
# define wfFlexes   0x4000       /* Stem has flex-table reference          */

  extern unsigned char  fxString[];
  extern unsigned char  fxTables[];
  extern unsigned char  suffixes[];
  extern unsigned char  classBox[];

  //
  //

  template <class T>
  inline T minimal( T v1, T v2 )
  {  return ( v1 <= v2 ? v1 : v2 );  }

  template <class T>
  inline T maximal( T v1, T v2 )
  {  return ( v1 >= v2 ? v1 : v2 );  }

  //
/*
  inline  byte08_t* GetInfoByID( word16_t id )
  {  return classBox + GetWord16( ((word16_t*)classBox) + (id & 0x0FFF) ); }

  inline  byte08_t* GetFragByID( int id )
  {  return suffixes + GetWord16( ((word16_t*)suffixes) + id );  }

  inline int  GetSkipCount( const byte08_t* stemList )
  {
    int       cbSkip = 2;
    word16_t  idStem = GetWord16( stemList );

    //
    if ( idStem & wfSuffix )
      cbSkip++;
    //
    //
    if ( idStem & wfPostSt )
      cbSkip += stemList[cbSkip] + 1;
    //
    return cbSkip + ((idStem & 0x3000) >> 12) + 1;
  }

  //
  typedef void (*FAction)( const byte08_t* lpStem,
                           const byte08_t* lpWord,
                           SGramInfo*      grInfo,
                           int             cgInfo,
                           SScanPage&      rfScan );

  int   FlexComp( word16_t        tfoffs,
                  const byte08_t* pszstr,
                  int             cchstr,
                  byte08_t*       pforms,
                  unsigned        dwsets,
                  const byte08_t* szpost );
  int   StemComp( const byte08_t* lpstem,
                  const byte08_t* szword,
                  int             ccword,
                  SGramInfo*      lpinfo,
                  SScanPage&      rfscan );
  bool  ScanPage( const byte08_t* page,
                  word16_t        offs,
                  const byte08_t* word,
                  int             size,
                  SScanPage&      rfScan,
                  FAction         action );
*/
# endif
#define MAX_WORD_FORMS  32

}} // end libmorph::eng namespace

# endif // !__libmorph_eng_mlmadefs_h__
