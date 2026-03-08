/**********************************************************************************************
 * dataset
 *
 * Create dataset on text collection passed.
 * Use exact morphology to markup correct and incorrect models for ranker tuning.
 **********************************************************************************************/
# include "../../include/mlma1049.h"
# include "mtc/arbitrarymap.h"
# include "moonycode/codes.h"
# include <namespace.h>
# include "../scandict.hpp"
# include "../lemmatize.hpp"
# include <algorithm>

namespace LIBMORPH_NAMESPACE
{
  extern unsigned char  ClassTable[];
  extern unsigned char  InflexDict[];
}

struct  sample
{
  unsigned  uclass;     // идентификатор класса
  uint8_t   partSp;
  size_t    ccstem;     // длина основы
  size_t    ccflex;     // длина окончания
  unsigned  uoccur;     // количество появлений
  bool      bmatch;     // позитивность или негативность этого примера
};

using datamap = mtc::arbitrarymap<std::vector<sample>>;

auto  ToPartOfSp( uint8_t ps ) -> uint8_t
{
  switch ( ps & 0x3f )
  {
    case 1: case 2: case 3: case 4: case 5: case 6:
      return 1;
    case 7: case 8: case 9:
      return 7 | (ps & wfMultiple);
    case 13: case 14: case 15:
      return 13 | (ps & wfMultiple);
    case 16: case 17: case 18:
      return 16 | (ps & wfMultiple);
    case 25: case 26:
      return 25;
    case 27: case 38: case 39: case 40: case 42:
    case 44: case 45: case 46: case 47:
      return ps;
    default:
      return 0;
  }
}

//
// DoNeedForm
// Returns true if the form is flective and in list of analysed parts of speech
//
bool  DoNeedForm( const SGramInfo& gr )
{
  return gr.idForm != (uint8_t)-1
      && ToPartOfSp( gr.wdInfo ) != 0;
}

auto  GetStemLen( lexeme_t lexeme, IMlmaMb& morpho ) -> size_t
{
  char  buffer[0x100];
  char* pstore = buffer;
  auto  ccstem = (unsigned)-1;

  for ( int i = 0; i != 256; ++i )
  {
    auto  nbuilt = morpho.BuildForm( pstore, buffer + sizeof (buffer) - pstore, lexeme, (uint8_t)i );

    if ( nbuilt <= 0 )
      continue;

    for ( auto p = pstore; nbuilt-- > 0; p += strlen( p ) + 1 )
    {
      unsigned  ccnext = 0;

      while ( ccnext != ccstem && p[ccnext] != 0 && p[ccnext] == buffer[ccnext] )
        ++ccnext;
      ccstem = std::min( ccnext, ccstem );
    }

    if ( pstore == buffer )
      pstore = buffer + strlen( buffer ) + 1;
  }
  assert( ccstem != (unsigned)-1 );
  return ccstem;
}

void  MakeSample(
  datamap&    output,
  const char* string,
  size_t      length,
  IMlmaMb&    morpho )
{
  SLemmInfoA  lemmas[32];
  SGramInfo   gramms[64];
  char        buffer[0x100];
  auto        samptr = decltype(output.Search( nullptr, 0 ))( nullptr );
  auto        nlemma = morpho.Lemmatize( string, length,
    lemmas, sizeof(lemmas) / sizeof(lemmas[0]),
    buffer, sizeof(buffer) / sizeof(buffer[0]),
    gramms, sizeof(gramms) / sizeof(gramms[0]), sfIgnoreCapitals );

// если есть отождествления, то отобрать те флективные, которые входят в список
// анализируемых в нечёткой морфологии, лемматизировать их вероятностным образом
// и создать записи для настройки
  for ( auto i = decltype(nlemma)( 0 ); i != nlemma; ++i )
    if ( DoNeedForm( *lemmas[i].pgrams ) )
    {
      auto  ccstem = GetStemLen( lemmas[i].nlexid, morpho );
      auto  oldcls = (unsigned)-1;

      if ( samptr == nullptr )
        samptr = output.Insert( { string, length } );

      LIBMORPH_NAMESPACE::ScanTree( [&](
          const char* plemma,
          size_t      clemma,
          size_t      cchstr,
          unsigned    uclass,
          unsigned    uoccur,
          uint8_t     idform ) -> bool
        {
          if ( oldcls != uclass )
          {
            auto  pclass = __libmorphrus__::GetClass( uclass );
            auto  pfound = std::find_if( samptr->begin(), samptr->end(), [&]( const sample& sm )
              {  return sm.uclass == uclass;  } );
            auto  partSp = (uint8_t)*pclass++;

            if ( pfound == samptr->end() )
              pfound = samptr->insert( pfound, { uclass, partSp, clemma + 2, cchstr - clemma, 1, false } );

          // если это 'та' модель, что надо - пометить её
            pfound->bmatch |=
                pfound->ccstem == ccstem
             && partSp == ToPartOfSp( lemmas[i].pgrams->wdInfo );

            oldcls = uclass;
          }

          return true;
        }, (const char*)LIBMORPH_NAMESPACE::InflexDict, string + length - 1, length );
    }
}

void  GetSamples( datamap& output, FILE* source, IMlmaMb& morpho )
{
  auto  buffer = std::vector<char>( 1024 * 1024 );
  auto  cbread = decltype(fread( nullptr, 0, 0, nullptr)){};

  do
  {
    if ( (cbread = fread( buffer.data(), 1, buffer.size(), source )) > 0 )
    {
      auto  pbeg = buffer.data();
      auto  pend = buffer.data() + cbread;

      while ( pbeg != pend )
      {
        auto  ptop = decltype(pbeg){};
        auto  pget = decltype(output.Search( nullptr, 0 )){};

      // skip until the cyrillic section
        while ( pbeg != pend && (unsigned char)*pbeg <= 0xC0 )
          ++pbeg;

      // get next term
        for ( ptop = pbeg; pbeg != pend && ((unsigned char)*pbeg >= 0xC0 || *pbeg == '-'); ++pbeg )
          (void)NULL;

      // check of got
        if ( pbeg <= ptop )
          continue;

      // search in the map; if present, just increment count
        if ( (pget = output.Search( ptop, pbeg - ptop )) != nullptr )
        {
          for ( auto& sample: *pget )
            ++sample.uoccur;
        } else MakeSample( output, ptop, pbeg - ptop, morpho );
      }
    }
  } while ( cbread == buffer.size() );
}

int   main()
{
  IMlmaMb*  morpho;
  datamap   sample;

  mlmaruLoadMbAPI( &morpho );

  GetSamples( sample, fopen( "/home/keva/dev/moonycode/text/kondrashov.txt", "rb" ), *morpho );

  auto  output = fopen( "dataset.txt", "wb" );

  for ( auto& it: sample )
    for ( auto& sa: it.second )
    {
      fprintf( output,
         "cls:%u\t"
         "psp:%u\t"
         "stm:%u\t"
         "flx:%u\t"
         "occ:%u\t"
         "%s\n",
        sa.uclass,
        sa.partSp,
        sa.ccstem,
        sa.ccflex,
        sa.uoccur,
        sa.bmatch ? "+" : "-" );
    }

  fclose( output );

  return 0;
}