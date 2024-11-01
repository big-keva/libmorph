# include <moonycode/codes.h>
# include <rus/include/mlma1049.h>
# include <mtc/serialize.h>
# include <algorithm>
# include <stdexcept>
# include <vector>
# include <memory>
# include <cmath>
# include <map>

template <>
auto  Serialize( std::vector<char>* to, const void* pv, size_t ch ) -> std::vector<char>*
{
  return to->insert( to->end(), (const char*)pv, (const char*)pv + ch ), to;
}

struct  formInfo
{
  uint8_t iform;      // идентификатор формы слова
  char*   pform;      // указатель на строку

public:
  bool  operator < ( const formInfo& fi ) const
    {  return compare( fi ) < 0;  }
  bool  operator == ( const formInfo& fi ) const
    {  return compare( fi ) == 0;  }
  int   compare( const formInfo& fi ) const
    {
      int   rc;

      if ( (rc = iform - fi.iform) == 0 )
        rc = strcmp( pform, fi.pform );
      return rc;
    }
};

struct  Suprefix
{
  char      theChars[2] = { 0, 0 };
  unsigned  occCount = 0;

public:
  Suprefix() = default;
  Suprefix( const char* s ):
    theChars{ s[0], s[1] },
    occCount( 1 ) {}
  Suprefix( const Suprefix& sp ):
    theChars{ sp.theChars[0], sp.theChars[1] },
    occCount( sp.occCount ) {}

public:
  bool  operator < ( const Suprefix& two ) const
    {  return compare( two.theChars ) < 0;  }
  bool  operator == ( const Suprefix& two ) const
    {  return compare( two.theChars ) == 0;  }
  bool  operator != ( const Suprefix& two ) const
    {  return !(*this == two);  }
  int   compare( const Suprefix& two ) const
    {
      int   rc;

      if ( (rc = theChars[1] - two.theChars[1]) == 0 )
        rc = theChars[0] - two.theChars[0];
      return rc;
    }
};

class Supreset: public std::vector<Suprefix>
{
  using std::vector<Suprefix>::vector;

public:
  auto  GetScalar( const Supreset& sp ) const -> double
  {
    auto  mbeg = begin();
    auto  mend = end();
    auto  mlen = (double)0.0;
    auto  pbeg = sp.begin();
    auto  pend = sp.end();
    auto  plen = (double)0.0;
    auto  rmul = (double)0.0;

    while ( mbeg != mend && pbeg != pend )
    {
      for ( ; mbeg != mend && *mbeg < *pbeg; ++mbeg )
        mlen += mbeg->occCount * mbeg->occCount;
      for ( ; pbeg != pend && *pbeg < *mbeg; ++pbeg )
        plen += pbeg->occCount * pbeg->occCount;
      if ( mbeg != mend && pbeg != pend && *mbeg == *pbeg )
        rmul += (mbeg++)->occCount * (pbeg++)->occCount;
    }
    for ( ; mbeg != mend; ++mbeg )  mlen += mbeg->occCount * mbeg->occCount;
    for ( ; pbeg != pend; ++pbeg )  plen += pbeg->occCount * pbeg->occCount;

    return rmul / sqrt( mlen * plen );
  }
  void  AddSuprefix( const Suprefix& sp )
  {
    auto  pchars = std::lower_bound( begin(), end(), sp );

    if ( pchars == end() || *pchars != sp ) insert( pchars, sp );
      else pchars->occCount += sp.occCount;
  }
  void  AddSuprefix( const Supreset& sp )
  {
    for ( auto& sup: sp )
      AddSuprefix( sup );
  }
};

class AllForms
{
  struct  formlist
  {
    char      buffer[0x20 * 0x200];
    formInfo  aforms[0x200];
    int       nforms = 0;
  };

  static  bool  form_is_less( const formInfo& f1, const formInfo& f2 )
  {
    return f1.iform < f2.iform || (f1.iform == f2.iform && strcmp( f1.pform, f2.pform ) < 0);
  }

public:
  static  auto  Build(
    IMlmaMb&                              morpho,
    lexeme_t                              lexeme,
    const std::initializer_list<uint8_t>& forced,
    const std::initializer_list<uint8_t>& altern = {} ) -> AllForms;

  bool  empty() const {  return pforms == nullptr;  }

  auto  Class() -> std::pair<Suprefix, AllForms>;
 /*
  * Расширяет список форм слов по списку форм, пересортировывает по идентификаторам
  * для обеспечения возрастания, а формы - лексикографически.
  */
  auto  Xpand( const std::initializer_list<std::pair<uint8_t, uint8_t>>& ) -> AllForms&;
  auto  Renum( const std::initializer_list<std::pair<uint8_t, uint8_t>>& ) -> AllForms&;

  auto  Form0() -> std::string
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

    for ( ; nbuilt-- > 0; bufptr += strlen( bufptr ) + 1 )
      pforms[nforms++] = { idform, bufptr };

    forbeg += forbeg != std::end( forced ) && idform == *forbeg ? 1 : 0;
    altbeg += altbeg != std::end( altern ) && idform == *altbeg ? 1 : 0;
  }
  return aforms;
}

auto  AllForms::Class() -> std::pair<Suprefix, AllForms>
{
  auto  ccstem = (unsigned)-1;

  std::sort( pforms->aforms, pforms->aforms + pforms->nforms,
    form_is_less );

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

    return std::make_pair( Suprefix( pforms->aforms[0].pform - 2 ), *this );
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

    std::sort( pforms->aforms, pforms->aforms + pforms->nforms,
      form_is_less );
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

    std::sort( pforms->aforms, pforms->aforms + pforms->nforms,
      form_is_less );
  }
  return *this;
}

bool  AllForms::Cover( const AllForms& lc ) const
{
  auto  mbeg = begin();
  auto  mend = end();
  auto  pbeg = lc.begin();
  auto  pend = lc.end();
  int   rcmp = 0;

  while ( pbeg != pend )
  {
    while ( mbeg != mend && (rcmp = mbeg->compare( *pbeg )) < 0 )
      ++mbeg;
    if ( mbeg != mend && rcmp == 0 )  ++pbeg;
      else return false;
  }
  return pbeg == pend;
}

using ClassKey = std::pair<uint8_t, AllForms>;
using ClassVal = std::tuple<unsigned, Supreset, std::string>;
using ClassMap = std::map<ClassKey, ClassVal>;

void  LoadDict( ClassMap& mclass, IMlmaMb& morpho, const std::initializer_list<std::pair<lexeme_t, lexeme_t>>& limits )
{
//  char      szword[0x100];
//  formInfo  aforms[0x100];

  for ( auto& module: limits )
    for ( auto nlexid = module.first; nlexid <= module.second; ++nlexid )
    {
      AllForms    aforms;
      auto        aclass = std::pair<Suprefix, AllForms>();
      uint8_t     partSp;

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
             } ).Renum( {
               { 6, 18 },  { 7, 19 },  { 8,  20 }, { 9,  21 },
               { 10, 22 }, { 11, 23 }, { 12, 24 }, { 13, 25 },
               { 14, 26 }, { 15, 27 }, { 16, 28 }, { 17, 29 }
             } );
          partSp = 1;
          break;
      // Для существительных - взять только полные парадигмы нативного типа склонения
        case 7:  case 8: case 9:
          if ( (aforms = AllForms::Build( morpho, nlexid, { 0, 1, 2, 5, 6, 10, 11, 12, 15, 16 } )).empty()
            && (aforms = AllForms::Build( morpho, nlexid, { 0, 1, 2, 5, 6 } ))                    .empty() )
                aforms = AllForms::Build( morpho, nlexid, {                10, 11, 12, 15, 16 } );
          aforms.Xpand( {
            { 0, 3 }, { 1, 4 }, { 10, 13 }, { 11, 14 } } );
          partSp = 7 | (partSp & wfMultiple);
          break;

      // в парадигме женского рода В. ед. всегда одинаковый, а В. мн. отличается для одуш.
      // и неодуш; для слияния парадигм отображаем все женские рода в 13 (жен.) и расширяем
      // винительный ед. и винительный мн.
        case 13: case 14: case 15:
          if ( (aforms = AllForms::Build( morpho, nlexid, { 0, 1, 2, 3, 5, 6, 10, 11, 12, 15, 16 } )).empty()
            && (aforms = AllForms::Build( morpho, nlexid, { 0, 1, 2, 3, 5, 6 } ))                    .empty() )
                aforms = AllForms::Build( morpho, nlexid, {                   10, 11, 12, 15, 16 } );
          aforms.Xpand( {
            { 3, 4 }, { 10, 13 }, { 11, 14 } } );
          partSp = 13 | (partSp & wfMultiple);
          break;

      // Средний род -= аналогично женскому, тиражирование винительного в единственном числе
      // и именительного-родительного во множественном
        case 16: case 17: case 18:
          if ( (aforms = AllForms::Build( morpho, nlexid, { 0, 1, 2, 3, 5, 6, 10, 11, 12, 15, 16 } )).empty()
            && (aforms = AllForms::Build( morpho, nlexid, { 0, 1, 2, 3, 5, 6 } ))                    .empty() )
                aforms = AllForms::Build( morpho, nlexid, {                   10, 11, 12, 15, 16 } );
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
        case 47:      // множественная география
          aforms = AllForms::Build( morpho, nlexid,
            { 10,  11,  12,  13,  15,  16 } );
          break;
        default:
          continue;
      }

      auto  form_0 = aforms.Form0();

      // create grammar class
      if ( !aforms.empty() && !(aclass = aforms.Class()).second.empty() )
      {
        auto  clskey = std::make_pair( partSp, aclass.second );
        auto  pfound = mclass.find( clskey );

        if ( pfound == mclass.end() )
        {
          mclass.insert( { std::move( clskey ), std::make_tuple( 1U, Supreset{ std::get<0>( aclass ) },
            form_0 ) } );
        }
          else
        {
          std::get<1>( pfound->second ).AddSuprefix( std::get<0>( aclass ) );
          ++std::get<0>( pfound->second );
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
// Перебирает все модели глаголов от самой маленькой до самой большой; для каждой модели ищет
// покрывающую её полностью модель, которая бы не противоречила символам-предшественникам;
// сливает модель с саой большой найденной
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

      // найти самый длинный покрывающий класс
        for ( auto test = map.begin(); test != map.end(); ++test )
        {
          if ( test != beg
            && std::get<1>( test->second ).GetScalar( std::get<1>( beg->second ) ) > 0.7
            && test->first.second.Cover( beg->first.second ) )
          {
            if ( sel == map.end() || sel->first.second.size() < test->first.second.size() )
              sel = test;
          }
        }

      // если такой есть, сравнить буквенные последовательности
        if ( sel != map.end() )
        {
          merged = true;

          std::get<0>( sel->second ) += std::get<0>( beg->second );
          std::get<1>( sel->second ).AddSuprefix( std::get<1>( beg->second ) );

          beg = map.erase( beg );
        } else ++beg;
      } else ++beg;
  }
}

int   main()
{
  ClassMap  classmap;
  IMlmaMb*  morphrus;

  mlmaruLoadMbAPI( &morphrus );

  LoadDict( classmap, *morphrus, {
    { 128,    21018  },       // "/home/keva/dev/libmorph/rus/dict/rusverbs.dic"
    { 24576,  55188  },       // "/home/keva/dev/libmorph/rus/dict/masculin.dic",
    { 61440,  86160  },       // "/home/keva/dev/libmorph/rus/dict/feminine.dic",
    { 90112,  99939  },       // "/home/keva/dev/libmorph/rus/dict/middle.dic",
    { 106498, 151285 },       // "/home/keva/dev/libmorph/rus/dict/adjects.dic",
    { 180224, 189796 } } );   // "/home/keva/dev/libmorph/rus/dict/namecity.dic",
                              // "/home/keva/dev/libmorph/rus/dict/adjnouns.dic",
                              // "/home/keva/dev/libmorph/rus/dict/pronadjs.dic",
                              // "/home/keva/dev/libmorph/rus/dict/pronouns.dic",

  MergeMap( classmap );

//
  std::vector<ClassMap::const_iterator> sorter;
  double                                rtotal = 0.0;

  for ( auto it = classmap.begin(); it != classmap.end(); rtotal += std::get<0>( (it++)->second ) )
    sorter.push_back( it );

  std::sort( sorter.begin(), sorter.end(), []( const ClassMap::const_iterator& l1, const ClassMap::const_iterator& l2 )
    {  return std::get<0>( l1->second ) > std::get<0>( l2->second );  } );

  rtotal *= 0.99;

  sorter.resize( std::count_if( sorter.begin(), sorter.end(), [&]( const ClassMap::const_iterator& l )
    {  return (rtotal -= std::get<0>( l->second )) >= 0;  } ) );

  fprintf( stdout,
    "{\n"
    "  \"created\": %u,\n"
    "  \"classes\": [\n", (unsigned)sorter.size() );

  for ( size_t i = 0; i != sorter.size(); ++i )
  {
    const char* prefix = "[";

    fprintf( stdout,
      "\t{\n"
      "\t\t\"psp\": %u,\n"
      "\t\t\"mod\": \"%s\",\n"
      "\t\t\"cnt\": %u,\n"
      "\t\t\"idx\": %u,\n"
      "\t\t\"spf\": ",
        sorter[i]->first.first,
        codepages::mbcstombcs( codepages::codepage_utf8, codepages::codepage_1251, std::get<2>( sorter[i]->second ) ).c_str(),
        std::get<0>( sorter[i]->second ), (unsigned)i );

    for ( auto& chars: std::get<1>( sorter[i]->second ) )
    {
      fprintf( stdout, "%s\n\t\t\t{ \"n\": %u, \"s\": \"%s\" }", prefix,
        (unsigned)chars.occCount,
        codepages::mbcstombcs( codepages::codepage_utf8, codepages::codepage_1251, chars.theChars, 2 ).c_str() );
      prefix = ",";
    }

    fprintf( stdout, "\n\t\t],\n"
      "\t\t\"flx\": [" );

    prefix = "\n";

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
