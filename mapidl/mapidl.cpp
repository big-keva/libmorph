# include <algorithm>
# include <cstring>
# include <cstdlib>
# include <cstdio>
# include <cstdint>
# include <climits>
# include <cerrno>
# include <vector>

using element_t = unsigned;
using lexid_map = std::vector<element_t>;

enum: size_t
{
  element_bits = sizeof(element_t) * CHAR_BIT,
  mapsize_step = 0x10000
};

void  InsertLexId( lexid_map& map, unsigned lex )
{
  size_t  uindex = (size_t)(lex / element_bits);
  size_t  ushift = (size_t)(lex % element_bits);
  size_t  lxmask = 1 << ushift;

  if ( map.size() <= uindex )
  {
    map.reserve( (uindex + 1 + mapsize_step) & ~mapsize_step );
    map.resize( uindex + 1 );
  }
  if ( (map[uindex] & lxmask) != 0 )
    fprintf( stderr, "Lexical identifier %u is already used!\n", lex );
  else map[uindex] |= lxmask;
}

auto  GetMinLexId( const lexid_map& map ) -> unsigned
{
  auto  top = map.begin();
  auto  end = map.end();

  while ( top != end && *top == 0 )
    ++top;

  for ( auto off = 0, bit = 1; top != end && off != element_bits; ++off )
    if ( (*top & bit) != 0 )
      return off + (top - map.begin()) * element_bits;

  return -1;
}

auto  GetMaxLexId( const lexid_map& map ) -> unsigned
{
  auto  top = map.rbegin();
  auto  end = map.rend();

  while ( top != end && *top == 0 )
    ++top;

  for ( int off = element_bits - 1; top != end && off >= 0; --off )
    if ( (*top & (1 << off)) != 0 )
      return off + (map.rend() - top - 1) * element_bits;

  return -1;
}

bool  TestIfLexId( const lexid_map& map, unsigned lex )
{
  size_t  uindex = (size_t)(lex / element_bits);
  size_t  ushift = (size_t)(lex % element_bits);

  return map.size() > uindex && (map[uindex] & (1 << ushift)) != 0;
}

auto  GetNextHole( const lexid_map& map, unsigned lex ) -> unsigned
{
  size_t  uindex = (size_t)(lex / element_bits);
  size_t  ushift = (size_t)(lex % element_bits);

// check for hole in same group value
  for ( auto uvalue = map[uindex]; ushift != element_bits; ++ushift )
  {
    if ( (uvalue & (1 << ushift)) == 0 )
      return ushift + uindex * element_bits;
  }

  for ( auto itnext = map.begin() + uindex + 1; itnext != map.end(); ++itnext )
    if ( *itnext != (element_t)-1 )
    {
      auto uvalue = *itnext;

      for ( ushift = 0; ushift != element_bits; ++ushift )
        if ( (uvalue & (1 << ushift)) == 0 )
          return ushift + (itnext - map.begin()) * element_bits;
    }

  return (unsigned)-1;
}

void  PrintLexMap( FILE* out, const lexid_map& map )
{
  auto  minlex = GetMinLexId( map );
  auto  maxlex = GetMaxLexId( map );

// check for lexemes
  if ( minlex == (unsigned)-1 || maxlex == (unsigned)-1 )
    return (void)fprintf( out, "No lexemes present, no lines with LEXID:nnn string found\n" );

// type the minimal and the maximal lexeme
  fprintf( out, "Minimal lexeme: %u (%08x)\n"
                "Maximal lexeme: %u (%08x)\n", minlex, minlex, maxlex, maxlex );

// type the holes
  while ( minlex < maxlex )
  {
    auto  l_hole = GetNextHole( map, minlex + 1 );
    auto  h_hole = l_hole + 1;

    if ( l_hole == (unsigned)-1 )
      break;

    while ( h_hole < maxlex && !TestIfLexId( map, h_hole ) )
      ++h_hole;

    fprintf( out, "\tHole of %u items, %u (%08x) - %u (%08x)\n",
      h_hole - l_hole, l_hole, l_hole, h_hole - 1, h_hole - 1 );

    minlex = h_hole;
  }
}

// Dictionary scanner implementation

//
// Map the dictionary string to the string lexical identifier
//
unsigned  GetStringLID( const char* string )
{
  for ( auto str = string + 1; (str = strstr( str, "LID:" )) != nullptr; ++str )
  {
    if ( (unsigned char)str[-1] <= 0x20 )
    {
      char* end = nullptr;
      auto  lid = strtol( str += 4, &end, 0 );

      if ( lid != 0 && end != nullptr )
        return lid;
    }
  }
  return 0;
}

//
// Scan the dictionary and create the lexeme map
//
auto  GetLexemeMap( lexid_map& map, FILE* src ) -> const lexid_map&
{
  char  buffer[1024];

  for ( auto lineid = 1; fgets( buffer, sizeof(buffer), src ) != nullptr; ++lineid )
  {
    auto  nlexid = GetStringLID( buffer );

    if ( nlexid != 0 )
      InsertLexId( map, nlexid );
  }

  return map;
}

char  about[] = "dictMap - create the russian morphological dictionary "
                    " source lexical map.\n"
                "Usage: dictMap filename, where 'filename' is the morphological dictionary.\n";

int   main( int argc, char* argv[] )
{
  FILE* lpfile;

// check program arguments
  if ( argc < 2 )
    return fputs( about, stderr ), -1;

// try open dictionary
  if ( (lpfile = fopen( argv[1], "rb" )) == nullptr )
    return (fprintf( stderr, "Could not open file %s!\n", argv[1] ), -2);

  try
  {
    lexid_map lexmap;

    PrintLexMap( stdout, GetLexemeMap( lexmap, lpfile ) );

    return (fclose( lpfile ), 0);
  }
  catch ( const std::exception& x )
  {
    return (fclose( lpfile ), fprintf( stderr, "%s\n", x.what() ), EFAULT);
  }
}

