/**********************************************************************************************
 * dataset
 *
 * Create dataset on text collection passed.
 * Use exact morphology to markup correct and incorrect models for ranker tuning.
 **********************************************************************************************/
# include <errno.h>
# include <functional>
# include <cstring>
# include <vector>
# include <cstdio>
# include <cmath>

struct  sample
{
  unsigned  uclass;     // идентификатор класса
  unsigned  partSp;
  unsigned  ccstem;     // длина основы
  unsigned  ccflex;     // длина окончания
  unsigned  uoccur;     // количество появлений
  bool      bmatch;     // позитивность или негативность этого примера
};

auto  GetRank( const sample& smp, const std::vector<double>& cranks, const std::vector<double>& k_flex ) -> double
{
  return cranks[smp.uclass]
       * (k_flex[0] + (1.0 - k_flex[0]) * sin( atan( k_flex[1] * (smp.ccflex - k_flex[2]) ) ));
}

auto  Measure(
  const std::vector<sample>& splset,
  const std::vector<double>& cranks,
  const std::vector<double>& k_flex ) -> double
{
  double  loss = 0;

  for ( auto& next: splset )
  {
    double  flrank = GetRank( next, cranks, k_flex );

    if ( next.bmatch )
      loss += next.uoccur * (1.0 - flrank);
    else
      loss += next.uoccur * (flrank - 0.0);
  }
  return loss;
}

using MetricFunc = std::function<double(const std::vector<double>&)>;

void  Grad(
        std::vector<double>&  grad,
        std::vector<double>&  temp,
  const std::vector<double>&  rank,
  double                      cval,
  double                      step,
  MetricFunc                  metr )
{
  std::copy( rank.begin(), rank.end(), temp.begin() );

  for ( size_t i = 0; i != rank.size(); ++i )
  {
    temp[i] += step;

    auto newmetric = metr( temp );
    grad[i] = (newmetric - cval) / step;
    temp[i] = rank[i];
  }
}

void  PrintStats( FILE* to, const std::vector<double>& v, const std::vector<double>& g )
{
  const char* prefix = "[";

  for ( size_t i = 0; i != v.size(); ++i )
    {  fprintf( to, "%s%4.2f(%.0f)", prefix, v[i], g[i] );  prefix = ", ";  }

  fprintf( to, "]" );
}

struct limits
{
  double min;
  double max;
};

void  Tune(
  std::vector<double>&        aranks,
  const std::vector<limits>&  limits,
  double                      cvalue,
  double                      flstep,
  MetricFunc                  metric, bool  verbose )
{
  auto  v_grad = std::vector<double>( aranks.size() );
  auto  n_rank = std::vector<double>( aranks.size() );
  auto  stplim = flstep / 100;

  for ( int i = 0; flstep > stplim; ++i )
  {
    if ( i == 15 )
    {
      int i = 15;
    }
    Grad( v_grad, n_rank, aranks, cvalue, flstep, metric );

    if ( verbose )
      fprintf( stdout, "%d\t" "loss = %f, step = %f\n", 1 + i, cvalue, flstep );

    double grmin = 0.0;
    double grmax = 0.0;
    double grdif;

  // normalize gradient to step value
    for ( auto& next: v_grad )
    {
      grmin = std::min( grmin, next );
      grmax = std::max( grmax, next );
    }

  // check the step
    if ( (grdif = grmax - grmin) < 0.01 )
    {
      flstep = 0.0;
      continue;
    }

  // normalize the gradient to step
    for ( auto& next: v_grad )
      next /= grdif;

  // create next step
    for ( size_t i = 0; i != n_rank.size(); ++i )
      n_rank[i] = std::min( std::max( aranks[i] - v_grad[i] * flstep, limits[i].min ), limits[i].max );

    auto  newmetric = metric( n_rank );

    if ( newmetric < cvalue && (cvalue - newmetric) / cvalue > 0.001 )
    {
      std::copy( n_rank.begin(), n_rank.end(), aranks.begin() );
      cvalue = newmetric;
    } else flstep /= 2;
  }
}

void  TuneByClassRanks(
  std::vector<double>&        cranks,
  const std::vector<double>&  k_flex,
  const std::vector<sample>&  sample, bool verbose )
{
  auto  metric = Measure( sample, cranks, k_flex );
  auto  limits = std::vector<class limits>( cranks.size(), { 0.01, 1.0 } );

  Tune( cranks, limits, metric, 0.5, [&]( const std::vector<double>& clrank ) -> double
    {  return Measure( sample, clrank, k_flex );  }, verbose );
}

void  TuneByFlexPowers(
  const std::vector<double>& cranks,
  std::vector<double>&       k_flex,
  const std::vector<sample>& sample, bool verbose )
{
  auto  metric = Measure( sample, cranks, k_flex );
  auto  limits = std::vector<class limits>{
    { 0.01, 0.99 },
    { 0.01, 3.00 },
    { 0.10, 5.00 } };

  Tune( k_flex, limits, metric, 0.1, [&]( const std::vector<double>& clrank ) -> double
    {  return Measure( sample, clrank, k_flex );  }, verbose );
}

auto  GetDataset( FILE* lpfile ) -> std::vector<sample>
{
  auto  fetch = std::vector<sample>( 1 );
  char  state;
  int   scres;

  fetch.reserve( 0x1000 );

  while ( (scres = fscanf( lpfile, "cls:%u\tpsp:%u\tstm:%u\tflx:%u\tocc:%u\t%c\n",
    &fetch.back().uclass,
    &fetch.back().partSp,
    &fetch.back().ccstem,
    &fetch.back().ccflex,
    &fetch.back().uoccur, &state )) != EOF )
  {
    fetch.back().bmatch = state == '+';
    fetch.resize( fetch.size() + 1 );
  }

  while ( !fetch.empty() && fetch.back().uoccur == 0 )
    fetch.pop_back();

  return fetch;
}

bool  IsValidCplusplusKey( const char* s )
{
  return strspn( s, "abcdefghijklmnopqrstuvwxyz"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "_"
                    "0123456789" ) == strlen( s ) && (s[0] < '0' || s[0] > '9');
}

const char about[] = "tune-ranking - create class weight distribution and dump to stdout\n"
  "Usage: %s [options] dataset\n"
  "options are:\n"
  "\t"  "-ns[pace]=namespace-name, default is no namespace;\n"
  "\t"  "-va[name]=variable-name, default is 'ClassRanks';\n"
  "\t"  "-verbose - print optimization progress.\n"
  "dataset format:\n"
  "\t"  "cls:%%u\\tpsp:%%u\\tstm:%%u\\tflx:%%u\\tocc:%%u\\t[-/+]\n"
  "keys:\n"
  "\t"  "cls - class identifier;\n"
  "\t"  "psp - part-of-speach (not used now, because is descendant of class);\n"
  "\t"  "stm - the length of word stem;\n"
  "\t"  "flx - the length of scanned inflexion, >= 2;\n"
  "\t"  "occ - the occurence count in sample texts;\n"
  "\t"  "-/+ - positivite or negative sample.\n";

int   main( int argc, char* argv[] )
{
  const char* source = nullptr;
  const char* nspace = nullptr;
  const char* vaname = "ClassRanks";
  auto        datset = decltype(GetDataset( nullptr )){};
  auto        cranks = std::vector<double>();
  auto        k_flex = std::vector<double>{ 0.1, 0.3, 2.0 };
  bool        bverbs = false;

 FILE*       infile;

  for ( auto arg = argv + 1; arg != argv + argc; ++arg )
  {
    if ( **arg == '-' )
    {
      if ( strncmp( 1 + *arg, "ns=", 3 ) == 0 || strncmp( 1 + *arg, "nspace=", 7 ) == 0
        || strncmp( 1 + *arg, "ns:", 3 ) == 0 || strncmp( 1 + *arg, "nspace:", 7 ) == 0 )
      {
        nspace = *arg + 1 + strcspn( *argv, "=:" );
      }
        else
      if ( strncmp( 1 + *arg, "va=", 3 ) == 0 || strncmp( 1 + *arg, "vaname=", 7 ) == 0
        || strncmp( 1 + *arg, "va:", 3 ) == 0 || strncmp( 1 + *arg, "vaname:", 7 ) == 0 )
      {
        vaname = *arg + 1 + strcspn( *argv, "=:" );
      }
        else
      if ( strcmp( 1 + *arg, "verbose" ) == 0 ) bverbs = true;
        else
      return fprintf( stderr, "invalid switch '%s'\n", *arg );
    }
      else
    if ( source == nullptr )  source = *arg;
      else
    return fprintf( stderr, "unexpected argument '%s'\n", *arg ), EINVAL;
  }
  if ( source == nullptr )
    return fprintf( stderr, about, argv[0] ), 0;

// check the arguments
  if ( nspace != nullptr && !IsValidCplusplusKey( nspace ) )
    return fprintf( stderr, "invalid namespace name '%s'\n", nspace ), EINVAL;

  if ( !IsValidCplusplusKey( vaname ) )
    return fprintf( stderr, "invalid variable name '%s'\n", vaname ), EINVAL;

// load the dataset
  if ( (infile = fopen( source, "rt" )) == nullptr )
    return fprintf( stderr, "could not open file '%s'\n", source );

  try
  {
    datset = GetDataset( infile );
      fclose( infile );

    if ( datset.empty() )
      return fprintf( stderr, "empty dataset\n" ), EINVAL;
  }
  catch ( const std::exception& xp )
  {
    fclose( infile );
    return fprintf( stderr, "%s\n", xp.what() );
  }

// reserve space for classes
  for ( auto& next: datset )
    if ( next.uclass >= cranks.size() )
      cranks.insert( cranks.end(), next.uclass - cranks.size() + 1, 0.5 );

  TuneByFlexPowers( cranks, k_flex, datset, bverbs );
  fprintf( stdout, "====================================\n" );
  TuneByClassRanks( cranks, k_flex, datset, bverbs );

  const char* baseof = "";

  if ( nspace != nullptr )
    fprintf( stdout, "namespace %s {\n", nspace ), baseof = "  ";

  fprintf( stdout, "%sdouble  %s[%u] =\n"
                   "%s{\n", baseof, vaname, cranks.size(), baseof );

  const char* prefix = "  ";

  for ( int i = 0; i < cranks.size(); ++i )
  {
    if ( (i % 12) == 0 )
      fprintf( stdout, "%s", baseof );

    fprintf( stdout, "%s%6.4f", prefix, cranks[i] );
      prefix = (i % 12) == 11 ? ",\n  " : ", ";
  }

  fprintf( stdout, "\n};\n" );

  fprintf( stdout, "%sdouble  FlexRanker[%u] =\n"
                   "%s{\n", baseof, k_flex.size(), baseof );

  prefix = "  ";

  for ( int i = 0; i < k_flex.size(); ++i )
  {
    if ( (i % 12) == 0 )
      fprintf( stdout, "%s", baseof );

    fprintf( stdout, "%s%6.4f", prefix, k_flex[i] );
      prefix = (i % 12) == 11 ? ",\n  " : ", ";
  }

  fprintf( stdout, "\n};\n" );

  if ( nspace != nullptr )
    fprintf( stdout, "}\n", nspace );

  return 0;
}