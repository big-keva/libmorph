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
# include <rus.h>
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
  bool  accent = false;

  Suffix() = default;
  Suffix( const Suffix& ) = default;
  Suffix( const char* s ): suffix{ s[0], s[1] } {}
  Suffix( const char* s, bool a ): suffix{ s[0], s[1] }, accent( a )  {}

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

  auto  Class( uint8_t partSp ) -> std::pair<AllForms, Suffix>;   /* список форм и последний символ основы */
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

auto  AllForms::Class( uint8_t partSp ) -> std::pair<AllForms, Suffix>
{
  static auto sizzlings = To1251( "жчшщ" );
  static auto consonant = To1251( "бвгджзклмнпрстфхцчшщщ" );
  static auto accentVow = To1251( "аеоуыэюя" );
  static auto letter_A  = To1251( "а" );
  static auto letter_O  = To1251( "о" );
  static auto letter_U  = To1251( "у" );
  static auto letter_E  = To1251( "е" );
  static auto letter_Y  = To1251( "й" );
  static auto inflex_OY = To1251( "ой" );

  auto  ccstem = (unsigned)-1;
  bool  accent = false;

  std::sort( pforms->aforms, pforms->aforms + pforms->nforms );

  // get stem length
  for ( auto beg = pforms->aforms + 1, end = pforms->aforms + pforms->nforms; ccstem >= 2 && beg < end; ++beg )
  {
    unsigned nmatch = 0;

    while ( nmatch < ccstem && beg[-1].pform[nmatch] != 0 && beg[-1].pform[nmatch] == beg[0].pform[nmatch] )
      ++nmatch;

    ccstem = nmatch;
  }

  // - существительные муж. на шипящую, Тв. ед. -ом
  // - существительные муж. на согласную, И. мн. -а
  if ( partSp == 7 || partSp == 38 || partSp == 40 || partSp == 42 )
  {
    for ( auto& next: *this )
      if ( next.iform == 0 )
      {
        auto  lform = strlen( next.pform );

        if ( next.pform[lform - 1] == letter_Y.front() && accentVow.find( next.pform[lform - 2] ) != accentVow.npos )
          {  accent = true;  break;  }
      }
        else
      if ( next.iform == 5 )      // окончание Тв. ед. -о? после шипящей
      {
        auto  lform = strlen( next.pform );

        if ( next.pform[lform - 2] == letter_O.front() && sizzlings.find( next.pform[lform - 3] ) != sizzlings.npos )
          {  accent = true;  break;  }
      }
        else
      if ( next.iform == 10 )     // окончание -а в И. мн.
      {
        auto  lform = strlen( next.pform );

        if ( next.pform[lform - 1] == letter_A.front() && consonant.find( next.pform[lform - 2] ) != consonant.npos )
          {  accent = true;  break;  }
      }
  }
    else
  // - прилагательные, И. муж. -ой
  if ( partSp == 25 )
  {
    for ( auto& next: *this )
      if ( next.iform == 0 )      // окончание Тв. ед. -о? после шипящей
      {
        auto  lform = strlen( next.pform );

        if ( strcmp( next.pform + lform - 2, inflex_OY.c_str() ) == 0 )
          {  accent = true;  break;  }
      }
  }

  if ( ccstem >= 2 )
  {
    for ( auto beg = pforms->aforms, end = pforms->aforms + pforms->nforms; beg < end; ++beg )
      beg->pform += ccstem;

    return { *this, Suffix( pforms->aforms[0].pform - 2, accent ) };
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
  bool                schemeB = false;

public:
  ClassVal() = default;
  ClassVal( const Suffix& suffix, const std::string& sample ):
    charSet{ { suffix, sample, 1 } },
    modProd( 1 ),
    schemeB( suffix.accent )
  {
  }
  void  AddSample( const Suffix& suffix, const std::string& sample, unsigned mdprod, bool forced = false )
  {
    auto  pfound = std::lower_bound( charSet.begin(), charSet.end(), suffix,
      []( const Sample& sample, const Suffix& suffix ){  return sample.suffix < suffix;  } );

    if ( charSet.size() != 0 && suffix.accent != schemeB )
      throw std::logic_error( "unmatching accent scheme" );

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
      // Для глаголов - парадигмы с причастиями и без
        case 1:
        case 2:
        case 5:
        case 6:
          aforms = AllForms::Build( morpho, nlexid,
            { 0,        // inf
              2,  3,    // imp
              18, 19,   // prs 1st
              22, 23,   // prs 2nt
              26, 27,   // prs 3rd,
              128, 129, 130, 131 // past m, f, n, p,
            },
            {
              1,        // возвратный inf

              4,  5,    // возвратное повелительное

              20, 21,   // prs 1st refl
              24, 25,   // prs 2nt refl
              28, 29,   // prs 3rd refl

              132, 133, 134, 135, // past m, f, n, p refl

              30,  31,  32,  33,  34,  35,  36,     // действительное причастие настоящего времени
              38,  39,  40,  41,  42,  43,  44,
              46,  47,  48,  49,  50,  51,  52,
              54,  55,  56,  57,  58,  59,  60,

              62,  63,  64,  65,  66,  67,  68,     // возвратные
              70,  71,  72,  73,  74,  75,  76,
              78,  79,  80,  81,  82,  83,  84,
              86,  87,  88,  89,  90,  91,  92,

              94,  95,  96,  97,  98,  99,  100,    // страдательные
              102, 103, 104, 105, 106, 107, 108,
              110, 111, 112, 113, 114, 115, 116,
              118, 119, 120, 121, 122, 123, 124,

              126, 127,

              136, 137, 138, 139, 140, 141, 142,    // действительные причастия
              144, 145, 146, 147, 148, 149, 150,
              152, 153, 154, 155, 156, 157, 158,
              160, 161, 162, 163, 164, 165, 166,

              168, 169, 170, 171, 172, 173, 174,    // возвратные
              176, 177, 178, 179, 180, 181, 182,
              184, 185, 186, 187, 188, 189, 190,
              192, 193, 194, 195, 196, 197, 198,

              200, 201, 201, 203, 204, 205, 206,    // страдательные
              208, 209, 210, 211, 212, 213, 214,
              216, 217, 218, 219, 220, 221, 222,
              224, 225, 226, 227, 228, 229, 230,

              232, 233
            } );
          partSp = 1;
          break;
        case 3:
        case 4:
          aforms = AllForms::Build( morpho, nlexid,
            { 0,        // inf
              2,  3,    // imp
              6,  7,    // fut 1st
              10, 11,   // fut 2nd
              14, 15,   // fut 3rd
              128, 129, 130, 131 // past m, f, n, p
            },
            {
              1,        // возвратный inf

              4,  5,    // возвратное повелительное

              8,  9,    // fut 1st refl
              12, 13,   // fut 2nt refl
              16, 17,   // fut 3rd refl

              132, 133, 134, 135, // past m, f, n, p refl

              136, 137, 138, 139, 140, 141, 142,
              144, 145, 146, 147, 148, 149, 150,
              152, 153, 154, 155, 156, 157, 158,
              160, 161, 162, 163, 164, 165, 166,

              168, 169, 170, 171, 172, 173, 174,   // возвратные
              176, 177, 178, 179, 180, 181, 182,
              184, 185, 186, 187, 188, 189, 190,
              192, 193, 194, 195, 196, 197, 198,

              200, 201, 201, 203, 204, 205, 206,    // страдательные
              208, 209, 210, 211, 212, 213, 214,
              216, 217, 218, 219, 220, 221, 222,
              224, 225, 226, 227, 228, 229, 230,

              232, 233
            } ).Renum( {
              { 6, 18 },  { 7, 19 },  { 8,  20 }, { 9,  21 },
              { 10, 22 }, { 11, 23 }, { 12, 24 }, { 13, 25 },
              { 14, 26 }, { 15, 27 }, { 16, 28 }, { 17, 29 }
            } );
          partSp = 1;
          break;
      // Для существительных - взять только полные парадигмы нативного типа склонения
        case 7:  case 8: case 9:
          aforms = AllForms::Build( morpho, nlexid, { 0, 1, 2, 5, 6, 10, 11, 12, 15, 16 } );
          aforms.Xpand( {
            { 0, 3 }, { 1, 4 }, { 10, 13 }, { 11, 14 } } );
          partSp = 7 | (partSp & wfMultiple);
          break;

      // в парадигме женского рода В. ед. всегда одинаковый, а В. мн. отличается для одуш.
      // и неодуш; для слияния парадигм отображаем все женские рода в 13 (жен.) и расширяем
      // винительный ед. и винительный мн.
        case 13: case 14: case 15:
          aforms = AllForms::Build( morpho, nlexid, { 0, 1, 2, 3, 5, 6, 10, 11, 12, 15, 16 } );
          aforms.Xpand( {
            { 3, 4 }, { 10, 13 }, { 11, 14 } } );
          partSp = 13 | (partSp & wfMultiple);
          break;

      // Средний род -= аналогично женскому, тиражирование винительного в единственном числе
      // и именительного-родительного во множественном
        case 16: case 17: case 18:
          aforms = AllForms::Build( morpho, nlexid, { 0, 1, 2, 3, 5, 6, 10, 11, 12, 15, 16 } );
          aforms.Xpand( {
            { 3, 4 }, { 10, 13 }, { 11, 14 } } );
          partSp = 16 | (partSp & wfMultiple);
          break;

        case 25: case 26: case 27:    // прилагательные
          aforms = AllForms::Build( morpho, nlexid,
            { 0,  1,  2,  3,  4,  5,  6,
              8,  9,  10, 11, 12, 13, 14,
              16, 17, 18, 19, 20, 21, 22,
              24, 25, 26, 27, 28, 29, 30 }/*,
            { 7,  15, 23, 31, 65 }*/ );
          partSp = partSp == 26 ? 25 : partSp;
          break;
  //    { "# и",    37 }, Имена собственные - игнорируем
  //    { "# им",   38 },   // Имена мужского рода
  //    { "# иж",   39 },   // Имена женского рода
        case 38: case 39:
          aforms = AllForms::Build( morpho, nlexid,
            { 0,  1,  2,  3,  5,  6,
              10, 11, 12, 13, 15, 16 } );
          break;
  //    { "# ом",   40 },   // Отчества муж. рода
  //    { "# ож",   41 },   // Отчества жен. рода
        case 40: case 41:
          aforms = AllForms::Build( morpho, nlexid,
            { 0, 1, 2, 3, 5, 6 } );
          break;
  //    { "# ф",    42 },   // Фамилии
        case 42:
          aforms = AllForms::Build( morpho, nlexid,
            { 0,  1,  2,  3,  5,  6,
              8,  9,  10, 11, 13, 14 },
            { 24, 25, 26, 27, 29, 30 } );
          break;
    // Географические названия
  //    { "# г",    43 }, - неизменяемые
  //    { "# гп",   43 }, - неизменяемые
  //    { "# гм",   44 }, - мужского рода
  //    { "# гмп",  44 },
  //    { "# гж",   45 },   // Женского рода
  //    { "# гжп",  45 },
  //    { "# гс",   46 },   // Среднего рода
  //    { "# гсп",  46 },
        case 44: case 45: case 46:
          aforms = AllForms::Build( morpho, nlexid,
            { 0,  1,  2,  3,  5,  6 } );
          break;
/*        case 47:      // множественная география
          aforms = AllForms::Build( morpho, nlexid,
            { 10,  11,  12,  13,  15,  16 } );
          break;*/
        default:
          continue;
      }

      auto  slemma = aforms.Lemma();

      // create grammar class
      if ( !aforms.empty() && (aclass = aforms.Class( partSp )).second.defined() )
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

//
// Read dictionary file and mark selected lexemes as better samples
//
template <class U>
void  GetBestSamples(
  std::vector<U>& idlSet,
  std::vector<U>& banSet,
  FILE*           source )
{
  char  szline[1024];
  bool  banned;

  while ( fgets( szline, sizeof( szline ), source ) != nullptr )
    if ( (banned = strstr( szline, "---" ) != nullptr) || strstr( szline, "+++" ) != nullptr )
    {
      auto  idlptr = strstr( szline, "LID:" );

      if ( idlptr != nullptr )
        mtc::bitset_set( banned ? banSet : idlSet, strtoul( idlptr + 4, nullptr, 10 ) );
    }
}

template <class U>
int   GetBestSamples(
  std::vector<U>& idlSet,
  std::vector<U>& banSet, std::initializer_list<const char*>  dicSet )
{
  for ( auto stpath: dicSet )
  {
    auto  infile = fopen( stpath, "rt" );

    if ( infile == nullptr )
      return fprintf( stderr, "Could not open input file '%s'", stpath );

    GetBestSamples( idlSet, banSet, infile );
      fclose( infile );
  }
  return 0;
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

  if ( mlmaruGetAPI( mtc::strprintf( "%s:1251", LIBMORPH_API_4_MAGIC ).c_str(), (void**)&morpho ) != 0 )
    return fprintf( stderr, "could not initialize the morphological analyser\n" ), EFAULT;

// get best samples
  for ( int i = 1; i < argc; ++i )
  {
    if ( (nerror = GetBestSamples( selMod, banMod, { argv[i] } )) != 0 )
      return nerror;
  }

  LoadDict( clsmap, *morpho, selMod, banMod, {
    { 128,    21018  },       // "/home/keva/dev/libmorph/lang-rus/dict/rusverbs.dic"
    { 24576,  55188  },       // "/home/keva/dev/libmorph/lang-rus/dict/masculin.dic",
    { 61440,  86160  },       // "/home/keva/dev/libmorph/lang-rus/dict/feminine.dic",
    { 90112,  99939  },       // "/home/keva/dev/libmorph/lang-rus/dict/middle.dic",
    { 106498, 151285 },       // "/home/keva/dev/libmorph/lang-rus/dict/adjects.dic",
    { 180224, 189796 } } );   // "/home/keva/dev/libmorph/lang-rus/dict/namecity.dic",
                              // "/home/keva/dev/libmorph/lang-rus/dict/adjnouns.dic",
                              // "/home/keva/dev/libmorph/lang-rus/dict/pronadjs.dic",
                              // "/home/keva/dev/libmorph/lang-rus/dict/pronouns.dic",

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
      "%s"
      "\t\t\"models\": [",
        sorter[i]->first.first,
        sorter[i]->second.modProd,
        (unsigned)i,
        sorter[i]->second.schemeB ? "\t\t\"accent\": \"b\",\n" : "" );

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
