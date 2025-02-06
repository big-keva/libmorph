/******************************************************************************

    libmorph - morphological analysers.

    Copyright (C) 1994-2016 Andrew Kovalenko aka Keva

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
# include "gramap.h"
# include "../tools/utf81251.h"
# include <stdexcept>
# include <cassert>
# include <string>
# include <map>

struct  gramdata
{
  unsigned  grmask;
  unsigned  grinfo;
  unsigned  mflags;
  unsigned  bflags;
};

static  std::map<std::string, gramdata> gramMapper;

# define add_key( szkey, gmask, ginfo, fmask, flags )   \
  gramMapper.emplace( utf8to1251( szkey ), gramdata{ (unsigned)(gmask), (unsigned)(ginfo), (unsigned)(fmask), (unsigned)(flags) } );

graminfo  MapInfo( const char*  pszkey, graminfo cginfo )
{
  while ( *pszkey != '\0' )
  {
    const char* pszorg;
    std::string gtoken;
    
    for ( pszorg = pszkey; *pszkey != '\0' && *pszkey != '|'; ++pszkey )
      (void)NULL;

    if ( pszkey == pszorg || !(*pszkey == '\0' || *pszkey == '|') )
      throw std::runtime_error( std::string( "invalid grammatic expression '" ) + pszorg + "'" );

    auto  pfound = gramMapper.find( gtoken = std::string( pszorg, pszkey - pszorg ) );
    
    if ( pfound == gramMapper.end() )
      throw std::runtime_error( "unknown grammatic token '" + gtoken + "'" );

    cginfo.grinfo = (cginfo.grinfo & pfound->second.grmask) | pfound->second.grinfo;
    cginfo.bflags = (cginfo.bflags & pfound->second.mflags) | pfound->second.bflags;

    if ( *pszkey == '|' )
      ++pszkey;
  }
  return cginfo;
}

void  InitRus()
{
  gramMapper.clear();

  add_key( "В",         ~gfFormMask,  3 << 12, -1, afAnimated + afNotAlive )
  add_key( "Вн",        ~gfFormMask,  3 << 12, ~(afAnimated + afNotAlive), afNotAlive )
  add_key( "Во",        ~gfFormMask,  3 << 12,  ~(afAnimated + afNotAlive),  afAnimated )
  add_key( "Д",         ~gfFormMask,  2 << 12,  -1,  afAnimated + afNotAlive )
  add_key( "И",         ~gfFormMask,  0,        -1,  afAnimated + afNotAlive )
  add_key( "П",         ~gfFormMask,  5 << 12,  -1,  afAnimated + afNotAlive )
  add_key( "П2",        ~gfFormMask,  7 << 12,  -1,  afAnimated + afNotAlive )
  add_key( "Р",         ~gfFormMask,  1 << 12,  -1,  afAnimated + afNotAlive )
  add_key( "Р2",        ~gfFormMask,  6 << 12,  -1,  afAnimated + afNotAlive )
  add_key( "Т",         ~gfFormMask,  4 << 12,  -1,  afAnimated + afNotAlive )
  add_key( "время Б",   0,            vtFuture,     -1, afAnimated + afNotAlive )
  add_key( "время Н",   0,            vtPresent,    -1, afAnimated + afNotAlive )
  add_key( "время П",   0,            vtPast,       -1, afAnimated + afNotAlive )
  add_key( "вф",        -1,           gfRetForms,   -1, 0 )
  add_key( "деепр",     gfVerbTime,   vfVerbDoing,  -1, afAnimated + afNotAlive )
  add_key( "действ",    gfVerbTime,   vfVerbActive, -1, afAnimated + afNotAlive )
  add_key( "затрудн",   -1,           0,            -1, afHardForm )
  add_key( "инфинитив", 0,            vtInfinitiv,  0,            afAnimated + afNotAlive )
  add_key( "кф",        gfVerbTime+gfVerbForm+gfGendMask+gfMultiple,   gfShortOne, -1, afAnimated + afNotAlive )
  add_key( "лицо 1",    gfVerbTime,   vbFirstFace, -1, afAnimated + afNotAlive )
  add_key( "лицо 2",    gfVerbTime,   vbSecondFace, -1, afAnimated + afNotAlive )
  add_key( "лицо 3",    gfVerbTime,   vbThirdFace, -1, afAnimated + afNotAlive )
  add_key( "нр",        0,            gfAdverb,     -1, afAnimated + afNotAlive )
  add_key( "повел",     0,            vtImperativ,  -1, afAnimated + afNotAlive )
  add_key( "проф",      -1,           0,            -1, 0 )                      
  add_key( "род",       gfVerbTime|gfVerbForm|gfMultiple, 0,      -1, 0 )
  add_key( "род ж",     gfVerbTime|gfVerbForm, 2 << 9, -1, 0 )
  add_key( "род м",     gfVerbTime|gfVerbForm, 1 << 9, -1, 0 )
  add_key( "род с",     gfVerbTime|gfVerbForm, 3 << 9, -1, 0 )
  add_key( "соед",      -1,           0,      -1, afJoiningC  )
  add_key( "ср",        0,            gfCompared,   -1, afAnimated + afNotAlive )
  add_key( "страд",     gfVerbTime,   vfVerbPassiv, -1, afAnimated + afNotAlive )
  add_key( "число",     gfVerbTime|gfVerbForm, 0, -1, afAnimated + afNotAlive )
  add_key( "число Е",   gfVerbTime|gfVerbForm, 0, -1, afAnimated + afNotAlive )
  add_key( "число М",   gfVerbTime|gfVerbForm, gfMultiple, -1, afAnimated + afNotAlive )
}

void  InitUkr()
{
  gramMapper.clear();

  add_key( "act",    gfVerbTime,   vfVerbActive, -1, afAnimated + afNotAlive )
  add_key( "face 1", gfVerbTime,   vbFirstFace, -1, afAnimated + afNotAlive )
  add_key( "face 2", gfVerbTime,   vbSecondFace, -1, afAnimated + afNotAlive )
  add_key( "face 3", gfVerbTime,   vbThirdFace, -1, afAnimated + afNotAlive )
  add_key( "fem",    gfVerbTime|gfVerbForm, 2 << 9, -1, 0 )
  add_key( "fut",    0,            vtFuture,     -1, afAnimated + afNotAlive )
  add_key( "ger",    gfVerbTime,   vfVerbDoing,  -1, afAnimated + afNotAlive )
  add_key( "imp",    0,            vtImperativ,  -1, afAnimated + afNotAlive )
  add_key( "inf",    0,            vtInfinitiv,  0,            afAnimated + afNotAlive )
  add_key( "msc",    gfVerbTime|gfVerbForm, 1 << 9, -1, 0 )
  add_key( "nwt",    gfVerbTime|gfVerbForm, 3 << 9, -1, 0 )
  add_key( "prs",    0,            vtPresent,    -1, afAnimated + afNotAlive )
  add_key( "pst",    0,            vtPast,       -1, afAnimated + afNotAlive )
  add_key( "psv",    gfVerbTime,   vfVerbPassiv, -1, afAnimated + afNotAlive )
  add_key( "sing",   gfVerbTime|gfVerbForm, 0, -1, afAnimated + afNotAlive )
  add_key( "plur",   gfVerbTime|gfVerbForm, gfMultiple, -1, afAnimated + afNotAlive )

  add_key( "В",      ~gfFormMask,  3 << 12,  -1,  afAnimated + afNotAlive )
  add_key( "Вн",     ~gfFormMask,  3 << 12,  ~(afAnimated + afNotAlive),  afNotAlive )
  add_key( "Во",     ~gfFormMask,  3 << 12,  ~(afAnimated + afNotAlive),  afAnimated )
  add_key( "Д",      ~gfFormMask,  2 << 12,  -1,  afAnimated + afNotAlive )
  add_key( "З",      ~gfFormMask,  6 << 12,  -1,  afAnimated + afNotAlive )
  add_key( "И",      ~gfFormMask,  0,        -1,  afAnimated + afNotAlive )
  add_key( "П",      ~gfFormMask,  5 << 12,  -1,  afAnimated + afNotAlive )
  add_key( "Р",      ~gfFormMask,  1 << 12,  -1,  afAnimated + afNotAlive )
  add_key( "Т",      ~gfFormMask,  4 << 12,  -1,  afAnimated + afNotAlive )
  add_key( "вф",     -1, gfRetForms,   -1, 0 )
}
