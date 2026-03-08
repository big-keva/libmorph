# include <errno.h>
# include <cstring>
# include <cstdlib>
# include <cstdint>
# include <vector>
# include <cstdio>

std::vector<uint8_t>  TableEntryOffset;
std::vector<double>   PartOfSpeachProb;
std::vector<
std::vector<double>>  FormIdProbMatrix;

template <class Mapper>
void  CreateTableEntryOffset( std::vector<uint8_t>& offTable, Mapper psMapper )
{
  unsigned  offset = 0;

  for ( unsigned techPsp = 0; techPsp != 256; ++techPsp )
  {
    auto  destPsp = psMapper( techPsp );

    if ( destPsp != 0 )
    {
      if ( offTable.size() < destPsp + 1 )
        offTable.insert( offTable.cend(), destPsp + 1 - offTable.size(), 0xff );

      if ( offTable[destPsp] == 0xff )
        offTable[destPsp] = offset++;
    }
  }
}

template <class C>
auto  SkipSpace( C* s ) -> C*
{
  while ( *s != '\0' && (unsigned char)*s <= 0x20 )
    ++s;
  return s;
}

auto  ToPartOfSp( uint8_t ps ) -> uint8_t
{
  switch ( ps )
  {
    case 1: case 2: case 3: case 4: case 5: case 6:
      return 1;
    case 7: case 8: case 9:
      return 7;
    case 13: case 14: case 15:
      return 13;
    case 16: case 17: case 18:
      return 16;
    case 25: case 26:
      return 25;
    case 27: case 38: case 39: case 40: case 42:
    case 44: case 45: case 46: case 47:
      return ps;
    default:
      return 0;
  }
}

int   ParseFid( uint8_t partSp, const char* szline )
{
  while ( *(szline = SkipSpace( szline )) != '\0' )
  {
    char*     strend;
    unsigned  formid = strtoul( szline, &(strend = nullptr), 0 );
    unsigned  uoccur;

    if ( strend == nullptr || *(strend = SkipSpace( strend )) != ':' )
      return fprintf( stderr, "invalid file format, colon expected after form id\n" ), EINVAL;

    if ( (uoccur = strtoul( szline = SkipSpace( strend + 1 ), &strend, 0 )) == 0 )
      return fprintf( stderr, "invalid file format, 0 frequency\n" ), EINVAL;

    if ( strend == nullptr || strchr( ",", *(strend = SkipSpace( strend )) ) == nullptr )
      return fprintf( stderr, "invalid file format, comma expected after frequency\n" ), EINVAL;

    if ( formid > 0xff )
      return fprintf( stderr, "invalid file format, form identifier is grater than 0xff\n" ), EINVAL;

    if ( formid != 0xff )
    {
      if ( FormIdProbMatrix.size() <= partSp )
        FormIdProbMatrix.resize( partSp + 1 );

      if ( FormIdProbMatrix[partSp].size() < formid + 1 )
        FormIdProbMatrix[partSp].resize( formid + 1 );

      FormIdProbMatrix[partSp][formid] += uoccur;
    }
    if ( *(szline = strend) == ',' )
      ++szline;
  }
}

int   ParsePos( const char* szline )
{
  unsigned  partSp;
  char*     strend;

  if ( (partSp = strtoul( szline = SkipSpace( szline ), &(strend = nullptr), 0 )) == 0 || partSp > 63 )
    return fprintf( stderr, "invalid file format, part of speach expected\n" ), EINVAL;

  if ( strend == nullptr || *(strend = SkipSpace( strend )) != ',' )
    return fprintf( stderr, "invalid file format, comma expected after part of speach\n" ), EINVAL;

  if ( (partSp = ToPartOfSp( partSp )) == 0 )
    return 0;

  return ParseFid( TableEntryOffset[partSp], SkipSpace( strend + 1 ) );
}

int   main()
{
  auto  infile = fopen( "/home/keva/dev/libmorph/rus/libprob/dump_pos.txt", "rt" );
  char  szline[0x1000];
  int   nerror;

  CreateTableEntryOffset( TableEntryOffset, ToPartOfSp );

  while ( fgets( szline, sizeof(szline), infile ) != nullptr )
    if ( (nerror = ParsePos( szline )) != 0 )
      return nerror;
/*
  double  flsumm = 0.0;

  for ( auto& l: PspFidMatrix )
  {
    double  lnsumm = 0.0;

    for ( auto& c: l )  lnsumm += c;

    if ( lnsumm >= 1 )
      for ( auto& c: l )  c /= lnsumm;

    flsumm += (l[0] = lnsumm);
  }

  for ( auto& l: PspFidMatrix )
    l[0] /= flsumm;
*/
  auto  output = fopen( "/home/keva/dev/libmorph/rus/libprob/psp-fid-table.cpp", "wt" );
  auto  prefix = (const char*){};
  auto  ftotal = (double)0.0;

  for ( int i = 0; i != FormIdProbMatrix.size(); ++i )
  {
    auto  ltotal = (double)0.0;

    for ( auto& next: FormIdProbMatrix[i] )
      ltotal += next;

    for ( auto& next: FormIdProbMatrix[i] )
      next /= ltotal;

    if ( PartOfSpeachProb.size() < i + 1 )
      PartOfSpeachProb.insert( PartOfSpeachProb.end(), i + 1 - PartOfSpeachProb.size(), 0.0 );

    PartOfSpeachProb[i] = ltotal;

    ftotal += ltotal;
  }

  for ( auto& next: PartOfSpeachProb )
    next /= ftotal;

// dump header and psp entry offset table
  fprintf( output,
    "namespace __libmorphrus__\n"
    "{\n"
    "  unsigned char TableEntryOffset[%d] =\n"
    "  {\n", TableEntryOffset.size() );

  prefix = "    ";

  for ( auto i = 0; i != TableEntryOffset.size(); ++i )
  {
    fprintf( output, "%s0x%02x", prefix, TableEntryOffset[i] );
    prefix = (i % 12) == 11 ? ",\n    " : ", ";
  }

  fprintf( output, "\n"
    "  };\n\n" );

// dump PartOfSpeach prob
  fprintf( output,
    "  double  PartOfSpeachProb[%d] =\n"
    "  {\n", PartOfSpeachProb.size() );

  prefix = "    ";

  for ( auto i = 0; i != PartOfSpeachProb.size(); ++i )
  {
    fprintf( output, "%s%6.4f", prefix, PartOfSpeachProb[i] );
    prefix = (i % 8) == 7 ? ",\n    " : ", ";
  }

  fprintf( output, "\n"
    "  };\n\n" );

// dump form ids probabilities
  fprintf( output,
    "  double  FormIdProbMatrix[%d][256] =\n"
    "  {\n", FormIdProbMatrix.size() );

  for ( int l = 0; l != FormIdProbMatrix.size(); ++l )
  {
    prefix = "    { ";

    for ( int i = 0; i != FormIdProbMatrix[l].size(); ++i )
    {
      fprintf( output, "%s%6.4f", prefix, FormIdProbMatrix[l][i] );
      prefix = (i % 8) == 7 ? ",\n      " : ", ";
    }

    fprintf( output, " }%s\n", l < FormIdProbMatrix.size() - 1 ? "," : "" );
  }

  fprintf( output, "  };\n" );

  fprintf( output, "}\n" );

  return fclose( output );
}