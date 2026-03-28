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
# include <eng.h>
# include <moonycode/codes.h>
# include <mtc/serialize.h>
# include <mtc/bitset.h>
# include <algorithm>
# include <stdexcept>
# include <vector>
# include <memory>
# include <cmath>
# include <map>
#include <mtc/utf.hpp>

template <>
auto  Serialize( std::vector<char>* to, const void* pv, size_t ch ) -> std::vector<char>*
{
  return to->insert( to->end(), (const char*)pv, (const char*)pv + ch ), to;
}

struct Suffix
{
  char  suffix[2] = { 0, 0 };

  Suffix() = default;
  Suffix( const Suffix& ) = default;
  Suffix( const char* s ): suffix{ s[0], s[1] } {}

  bool  operator == ( const Suffix& s ) const {  return compare( s ) == 0;  }
  bool  operator != ( const Suffix& s ) const {  return compare( s ) != 0;  }
  bool  operator <  ( const Suffix& s ) const {  return compare( s ) <  0;  }

  bool  defined() const
  {
    return suffix[0] != 0 && suffix[1] != 0;
  }
  int   compare( const Suffix& s ) const
  {
    int   rc;

    return (rc = uint8_t(suffix[1]) - uint8_t(s.suffix[1])) != 0 ? rc :
      uint8_t(suffix[0]) - uint8_t(s.suffix[0]);
  }
};

class AllForms
{
  struct  formInfo
  {
    uint8_t iform;      // идентификатор формы слова
    char*   pform;      // указатель на строку

  public:
    bool  operator < ( const formInfo& fi ) const   {  return compare( fi ) < 0;  }
    bool  operator == ( const formInfo& fi ) const  {  return compare( fi ) == 0;  }
    int   compare( const formInfo& fi ) const
    {
      int   rc;

      return (rc = iform - fi.iform) == 0 ? strcmp( pform, fi.pform ) : rc;
    }
  };

  struct  formlist
  {
    char      buffer[0x20 * 0x200];
    formInfo  aforms[0x200];
    int       nforms = 0;

    auto  size() const -> size_t {  return nforms;  }
    auto  begin() const -> const formInfo* {  return aforms;  }
    auto  end() const -> const formInfo* {  return aforms + nforms;  }
  };

public:
  static  auto  Build(
    IMlmaMb&                              morpho,
    lexeme_t                              lexeme,
    const std::initializer_list<uint8_t>& forced,
    const std::initializer_list<uint8_t>& altern = {} ) -> AllForms;

  bool  empty() const {  return pforms == nullptr;  }

  auto  Class() -> std::pair<AllForms, Suffix>;   /* список форм и последний символ основы */
 /*
  * Расширяет список форм слов по списку форм, пересортировывает по идентификаторам
  * для обеспечения возрастания, а формы - лексикографически.
  */
  auto  Xpand( const std::initializer_list<std::pair<uint8_t, uint8_t>>& ) -> AllForms&;
  auto  Renum( const std::initializer_list<std::pair<uint8_t, uint8_t>>& ) -> AllForms&;

  auto  Forms() const -> const formlist&
    {  return pforms != nullptr ? *pforms.get() : throw std::logic_error( "formlist not defined" );  }
  auto  Lemma() -> std::string
    {  return pforms != nullptr && pforms->nforms != 0 ? pforms->aforms[0].pform : "?";  }

  bool  Cover( const AllForms& ) const;

public:     // operators
  bool  operator < ( const AllForms& lc ) const
  {
    auto  mbeg = begin();
    auto  mend = end();
    auto  pbeg = lc.begin();
    auto  pend = lc.end();
    int   rcmp = 0;

    while ( mbeg != mend && pbeg != pend && (rcmp = mbeg->compare( *pbeg )) == 0 )
    {
      ++mbeg;
      ++pbeg;
    }
    if ( rcmp == 0 )
      rcmp = (mbeg != mend) - (pbeg != pend);
    return rcmp < 0;
  }

public: // iterator
  auto  begin() const -> const formInfo*
    {  return pforms != nullptr ? pforms->aforms : nullptr;  }
  auto  end() const -> const formInfo*
    {  return pforms != nullptr ? pforms->aforms + pforms->nforms : nullptr;  }
  auto  size() const -> size_t
    {  return pforms != nullptr ? pforms->nforms : 0;  }

protected:
  std::shared_ptr<formlist> pforms;

};

auto  To1251( const char* str ) -> std::string
{
  return codepages::mbcstombcs( codepages::codepage_1251, codepages::codepage_utf8, str );
}

// AllForms implementation

auto  AllForms::Build(
    IMlmaMb&                              morpho,
    lexeme_t                              nlexid,
    const std::initializer_list<uint8_t>& forced,
    const std::initializer_list<uint8_t>& altern ) -> AllForms
{
  AllForms  aforms;
  uint8_t   partSp;

  if ( morpho.GetWdInfo( &partSp, nlexid ) > 0 ) aforms.pforms = std::make_shared<formlist>();
    else return aforms;

  auto  bufptr = aforms.pforms->buffer;
  auto  bufend = aforms.pforms->buffer + sizeof(aforms.pforms->buffer);
  auto  pforms = aforms.pforms->aforms;
  auto& nforms = aforms.pforms->nforms;
  auto  forbeg = std::begin( forced );
  auto  altbeg = std::begin( altern );

  while ( forbeg != std::end( forced ) || altbeg != std::end( altern ) )
  {
    uint8_t idform =
      forbeg != std::end( forced ) ? (altbeg != std::end( altern ) ? std::min( *forbeg, *altbeg ) : *forbeg) : *altbeg;

    int     nbuilt = morpho.BuildForm( bufptr, bufend - bufptr, nlexid, idform );

    if ( nbuilt < 0 )
      throw std::invalid_argument( "not enough space in buffer to build form" );

    if ( nbuilt == 0 && forbeg != std::end( forced ) && idform == *forbeg )
      return {};

    while ( nbuilt-- > 0 )
    {
      pforms[nforms++] = { idform, bufptr };

      for ( ; (*bufptr = codepages::chartolower( codepages::codepage_1251, *bufptr )) != 0; ++bufptr )
        if ( *bufptr == '-' ) return {};

      ++bufptr;
    }

    forbeg += forbeg != std::end( forced ) && idform == *forbeg ? 1 : 0;
    altbeg += altbeg != std::end( altern ) && idform == *altbeg ? 1 : 0;
  }
  return aforms;
}

auto  AllForms::Class() -> std::pair<AllForms, Suffix>
{
  auto  ccstem = (unsigned)-1;

  if ( pforms->nforms <= 1 )
    return {};
  
  std::sort( pforms->aforms, pforms->aforms + pforms->nforms );

  // get stem length
  for ( auto beg = pforms->aforms + 1, end = pforms->aforms + pforms->nforms; ccstem >= 2 && beg < end; ++beg )
  {
    unsigned nmatch = 0;

    while ( nmatch < ccstem && beg[-1].pform[nmatch] != 0 && beg[-1].pform[nmatch] == beg[0].pform[nmatch] )
      ++nmatch;

    ccstem = nmatch;
  }

  if ( ccstem >= 2 )
  {
    for ( auto beg = pforms->aforms, end = pforms->aforms + pforms->nforms; beg < end; ++beg )
      beg->pform += ccstem;

    return { *this, Suffix( pforms->aforms[0].pform - 2 ) };
  }
  return { {}, {} };
}

auto  AllForms::Xpand( const std::initializer_list<std::pair<uint8_t, uint8_t>>& forms ) -> AllForms&
{
  if ( pforms != nullptr )
  {
    for ( int i = 0, e = pforms->nforms; i != e; ++i )
    {
      auto  idf = pforms->aforms[i].iform;

      for ( auto& exp: forms )
        if ( exp.first == idf )
          pforms->aforms[pforms->nforms++] = { exp.second, pforms->aforms[i].pform };
    }

    std::sort( pforms->aforms, pforms->aforms + pforms->nforms );
  }
  return *this;
}

auto  AllForms::Renum( const std::initializer_list<std::pair<uint8_t, uint8_t>>& with ) -> AllForms&
{
  if ( pforms != nullptr )
  {
    for ( int i = 0, e = pforms->nforms; i != e; ++i )
    {
      auto  idf = pforms->aforms[i].iform;

      for ( auto& exp: with )
        if ( exp.first == idf )
          pforms->aforms[i].iform = exp.second;
    }

    std::sort( pforms->aforms, pforms->aforms + pforms->nforms );
  }
  return *this;
}

bool  AllForms::Cover( const AllForms& lc ) const
{
  auto  mbeg = begin();
  auto  pbeg = lc.begin();
  int   rcmp = 0;

  for ( ; mbeg != end() && pbeg != lc.end(); )
  {
    while ( mbeg != end() && (rcmp = mbeg->compare( *pbeg )) < 0 )
      ++mbeg;
    if ( mbeg == end() || rcmp != 0 )
      return false;
    ++mbeg;
    ++pbeg;
  }
  return pbeg == lc.end();
}

struct ClassVal
{
  struct Sample
  {
    Suffix      suffix;
    std::string sample;
    unsigned    mdProd;
  };

  std::vector<Sample> charSet;
  unsigned            modProd = 0;

public:
  ClassVal() = default;
  ClassVal( const Suffix& suffix, const std::string& sample ):
    charSet{ { suffix, sample, 1 } },
    modProd( 1 )
  {
  }
  void  AddSample( const Suffix& suffix, const std::string& sample, unsigned mdprod, bool forced = false )
  {
    auto  pfound = std::lower_bound( charSet.begin(), charSet.end(), suffix,
      []( const Sample& sample, const Suffix& suffix ){  return sample.suffix < suffix;  } );

    if ( pfound == charSet.end() || pfound->suffix != suffix )
    {
      charSet.insert( pfound, { suffix, sample, mdprod } );
        modProd += mdprod;
    }
      else
    {
      if ( forced )
        pfound->sample = sample;
      pfound->mdProd += mdprod;
        modProd += mdprod;
    }
  }
};

using ClassKey = std::pair<uint8_t, AllForms>;

using ClassMap = std::map<ClassKey, ClassVal>;
using LexLimit = std::pair<lexeme_t, lexeme_t>;
using DicLimit = const std::initializer_list<LexLimit>;

void  LoadDict(
  ClassMap&                   mclass,     // grammatical class collector
  IMlmaMb&                    morpho,     // morphology api
  const std::vector<uint64_t> selMod,
  const std::vector<uint64_t> banMod,
  const DicLimit&             limits )    // dictionary limits
{
  for ( auto& module: limits )
    for ( auto nlexid = module.first; nlexid <= module.second; ++nlexid )
    {
      AllForms    aforms;
      auto        aclass = std::pair<AllForms, Suffix>();
      uint8_t     partSp;

      // check suppressed lexemes
      if ( mtc::bitset_get( banMod, nlexid ) )
        continue;

      // check valuable part of speach
      if ( morpho.GetWdInfo( &partSp, nlexid ) <= 0 )
        continue;

      switch ( partSp & 0x3f )
      {
        case 1:     // существительные
          aforms = AllForms::Build( morpho, nlexid, { 0, 1, 2, 3 } );
          break;
        case 2:     // глагол
          aforms = AllForms::Build( morpho, nlexid, { 0, 1, 2, 3, 4 } );
          break;
        case 3:     // прилагательные
        case 4:     // наречие
          aforms = AllForms::Build( morpho, nlexid, { 0 }, { 1, 2 } );
          break;
        case 17:    // имена
          aforms = AllForms::Build( morpho, nlexid, { 0, 1 } );
          break;
        default:
          continue;
      }

      auto  slemma = aforms.Lemma();

      // create grammar class
      if ( !aforms.empty() && (aclass = aforms.Class()).second.defined() )
      {
        auto  clskey = std::make_pair( partSp, aclass.first );
        auto  pfound = mclass.find( clskey );

        if ( pfound == mclass.end() )
        {
          mclass.emplace( std::move( clskey ),
            ClassVal( { aclass.second, slemma } ) );
        }
          else
        {
          pfound->second.AddSample( std::get<1>( aclass ), slemma, 1,
            mtc::bitset_get( selMod, nlexid ) );
        }
      }
    }
}

//# include <mtc/patricia.h>
//# include <mtc/interfaces.h>

char* revert( char* s )
{
  auto  p = s;
  auto  e = s;

  while ( *e != 0 ) ++e;

  for ( --e; p < e; ++p, --e )
    std::swap( *p, *e );

  return s;
}

void  PrintDiff( const AllForms& l, const AllForms& r )
{
  auto  lbeg = l.begin();
  auto  rbeg = r.begin();

  while ( lbeg != l.end() || rbeg != r.end() )
  {
    auto  GetFormStr = []( const char* fm ) -> std::string
    {
      return *fm != '\0' ? codepages::mbcstombcs( codepages::codepage_utf8, codepages::codepage_1251, fm ) : "''";
    };

  // print unmatched left forms
    while ( lbeg != l.end() && (rbeg == r.end() || *lbeg < *rbeg) )
    {
      printf( "  %-3d  %-16s\n",
        lbeg->iform,
        GetFormStr( lbeg->pform ).c_str() );
      ++lbeg;
    }
  // print unmatched right forms
    while ( rbeg != r.end() && (lbeg == l.end() || *rbeg < *lbeg) )
    {
      printf( "                            %-3d  %s\n",
        rbeg->iform,
        GetFormStr( rbeg->pform ).c_str() );
      ++rbeg;
    }
  // print matching strings
    while ( lbeg != l.end() && rbeg != r.end() && *lbeg == *rbeg )
    {
      auto  lstr = mtc::strprintf( "%-3d  %s", lbeg->iform, GetFormStr( lbeg->pform ).c_str() );
      auto  llen = mtc::utf::strlen( lstr.c_str() );

      lstr += std::string( 21 - llen, ' ' );

      printf( "  %s     %-3d  %s\n",
        lstr.c_str(),
        rbeg->iform,
        GetFormStr( rbeg->pform ).c_str() );
      ++lbeg;
      ++rbeg;
    }
  }
}

//
// Перебирает все модели глаголов от самой маленькой до самой большой; для каждой модели ищет
// покрывающую её полностью модель, которая бы не противоречила символам-предшественникам;
// сливает модель с самой большой найденной
//
void  MergeMap( ClassMap& map )
{
  for ( bool merged = true; merged; )
  {
    merged = false;

    for ( auto beg = map.begin(); beg != map.end(); )
      if ( std::get<0>( beg->first ) == 1 )     // только для глаголов
      {
        auto  sel = map.end();

      // найти самый длинный покрываемый класс
        for ( auto test = map.begin(); test != map.end(); ++test )
          if ( test != beg
            && std::get<0>( test->first ) == 1
            && std::get<1>( beg->first ).Cover( std::get<1>( test->first ) ) )
          {
            if ( sel == map.end() || sel->first.second.size() < test->first.second.size() )
              sel = test;
          }

      // если найден, напечатать
        if ( sel != map.end() )
        {
          auto& lSamples = beg->second.charSet;
          auto& rSamples = sel->second.charSet;
/*
          PrintDiff( std::get<1>( beg->first ), std::get<1>( sel->first ) );

          for ( auto& mod: lSamples )
            fprintf( stdout, "%s ", codepages::mbcstombcs( codepages::codepage_utf8, codepages::codepage_1251, mod.sSample ).c_str() );
          fprintf( stdout, "\n  vs\n" );

          for ( auto& mod: rSamples )
            fprintf( stdout, "%s ", codepages::mbcstombcs( codepages::codepage_utf8, codepages::codepage_1251, mod.sSample ).c_str() );
          fprintf( stdout, "\n" );
*/
          for ( auto& next: rSamples )
            beg->second.AddSample( next.suffix, next.sample, next.mdProd );

          map.erase( sel );
          merged = true;
          /*
          // если такой есть, сравнить хвосты основ
          for ( auto lbeg = lSamples.begin(), rbeg = rSamples.begin(); lbeg != lSamples.end() && rbeg != rSamples.end(); )
          {
            while ( lbeg != lSamples.end() && (uint8_t)lbeg->keyChar < (uint8_t)rbeg->keyChar )
              ++lbeg;
            if ( lbeg == lSamples.end() )
              break;
            while ( rbeg != rSamples.end() && (uint8_t)rbeg->keyChar < (uint8_t)lbeg->keyChar )
              ++rbeg;
            if ( rbeg != rSamples.end() && lbeg->keyChar == rbeg->keyChar )
            {
              int i = 0;
              ++lbeg;
              ++rbeg;
            }
          }
          */
        }

/*        if ( sel != map.end() )
        {
//          beg = map.erase( beg );
        } else */++beg;
      } else ++beg;
  }
}

int   main( int argc, char* argv[] )
{
  ClassMap  clsmap;
  IMlmaMb*  morpho;
  auto      selMod = std::vector<uint64_t>();
  auto      banMod = std::vector<uint64_t>();
  int       nerror;

  if ( mlmaenGetAPI( mtc::strprintf( "%s:1251", LIBMORPH_API_4_MAGIC ).c_str(), (void**)&morpho ) != 0 )
    return fprintf( stderr, "could not initialize the morphological analyser\n" ), EFAULT;

  LoadDict( clsmap, *morpho, selMod, banMod, {
    { 128,    29173  },       // "/home/keva/dev/libmorph/lang-eng/dict/adj.dic"
    { 30720,  34774  },       // "/home/keva/dev/libmorph/lang-eng/dict/adverb.dic",
    { 36864,  103012 },       // "/home/keva/dev/libmorph/lang-eng/dict/noun.dic",
    { 106496, 119146 },       // "/home/keva/dev/libmorph/lang-eng/dict/verb.dic",
    { 131072, 131603 } } );   // "/home/keva/dev/libmorph/lang-eng/dict/names.dic",

  MergeMap( clsmap );

// choose verbs to select verb models
  std::vector<ClassMap::const_iterator> sorter;
  double                                rtotal = 0.0;

  for ( auto it = clsmap.begin(); it != clsmap.end(); rtotal += (it++)->second.modProd )
    sorter.push_back( it );

  std::sort( sorter.begin(), sorter.end(), []( const ClassMap::const_iterator& l1, const ClassMap::const_iterator& l2 )
    {  return l1->second.modProd > l2->second.modProd;  } );

  while ( sorter.size() != 0 && sorter.back()->second.modProd < 1 )
    sorter.pop_back();

  rtotal *= 0.95;

  sorter.resize( std::count_if( sorter.begin(), sorter.end(), [&]( const ClassMap::const_iterator& l )
    {  return (rtotal -= l->second.modProd) >= 0;  } ) );

/*  sorter.resize( std::count_if( sorter.begin(), sorter.end(), [&]( const ClassMap::const_iterator& l )
    {  return l->second.uOccurs * 100 > sorter.front()->second.uOccurs;  } ) );*/

  fprintf( stdout,
    "{\n"
    "  \"created\": %u,\n"
    "  \"classes\": [\n", (unsigned)sorter.size() );

  for ( size_t i = 0; i != sorter.size(); ++i )
  {
    const char* prefix = "\n";

  // print class header
    fprintf( stdout,
      "\t{\n"
      "\t\t\"partSp\": %u,\n"
      "\t\t\"rate\": %u,\n"
      "\t\t\"index\": %u,\n"
      "\t\t\"models\": [",
        sorter[i]->first.first,
        sorter[i]->second.modProd,
        (unsigned)i );

  // print the models
    for ( auto& model: sorter[i]->second.charSet )
    {
      fprintf( stdout, "%s\t\t\t{ \"suffix\": \"%s\", \"weight\": %u, \"sample\": \"%s\" }", prefix,
        codepages::mbcstombcs( codepages::codepage_utf8, codepages::codepage_1251, model.suffix.suffix, 2 ).c_str(),
        model.mdProd,
        codepages::mbcstombcs( codepages::codepage_utf8, codepages::codepage_1251, model.sample ).c_str() );
      prefix = ",\n";
    }

  // print the flexions
    fprintf( stdout,
      "%s"
      "\t\t],\n"
      "\t\t\"inflex\": [", prefix = "\n" );

    for ( auto& next: sorter[i]->first.second )
    {
      fprintf( stdout, "%s\t\t\t{ \"id\": %u, \"sz\": \"%s\" }",
        prefix, next.iform, *next.pform != 0 ? codepages::mbcstombcs( codepages::codepage_utf8, codepages::codepage_1251, next.pform ).c_str() : "" );
      prefix = ",\n";
    }

    fputs(
      "\n"
      "\t\t]\n"
      "\t},\n", stdout );
  }

  fprintf( stdout,
    "  ]\n"
    "}\n" );

  return 0;
}
