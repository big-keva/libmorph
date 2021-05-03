/*! \file
//[]================================================================== <br>
//[] Project: English Morphological Library Builder                    <br>
//[] File name: makeeng.cpp                                            <br>
//[] Abstract: The default algorythm customization                     <br>
//[]                                                                   <br>
//[]                                                                   <br>
//[] Author: Andrew Kovalenko aka Keva                                 <br>
//[] Created: 29.04.2002                                               <br>
//[] Modifier:                                                         <br>
//[] Last modified:                                                    <br>
//[] Modifier notes:                                                   <br>
//[]==================================================================*/ 
# include "../../tools/dictMaker.h"
# include "../../tools/wordinfo.h"
# include "../../tools/loup1251.h"
# include "flexbox.h"
# include <sys/types.h>
# include <sys/stat.h>

# define MAX_STRING_LEN    256
# define FLEX_DELIMITER   '|'

# define wfCapFirst  0x0080
# define wfCapOnly   0x0100

inline bool IsCapital( char ch )
{
  return ch != 0 && (char)toLoCaseMatrix1251[(unsigned char)ch] != ch;
}

inline int  GetCapType( const char* string )
{
  return ( IsCapital( string[0] ) ? ( IsCapital( string[1] ) ? 2 : 1 ) : 0 );
}

class CMakeEng: public CDictMaker<CWordInfo>
{
  stringmap<unsigned short, unsigned short>
              partSpMapper;
  CFlexBox    inflexMapper;
public:  
              CMakeEng(): CDictMaker<CWordInfo>( 10, 0xF000 )
                {
                }
  virtual    ~CMakeEng()
                {
                }
// Initialize the compiler from the initialization file
  virtual int     Configure();
  virtual int     CreateMorph();
  virtual int     ResolveLine( const char*    string,
                               unsigned long  nlexid );
private:
  unsigned short  SearchPartSp( const char* pszpsp );
  unsigned short  RegisterFlex( const char* szflex );
  int             SaveFlexTabs();
};

// CMakeEng

static struct
{
  const char*     pspstr;
  unsigned short  pspval;
} pspLst[] =
{
  { "unknown",  0  },
  { "n",        1  },
  { "v",        2  },
  { "a",        3  },
  { "adv",      4  },
  { "pn",       5  },
  { "num",      6  },
  { "int",      7  },
  { "conj",     8  },
  { "prep",     9  },
  { "refl",     10 },
  { "pers",     11 },
  { "indef",    12 },
  { "poss",     13 },
  { "demo",     14 },
  { "rel",      15 },
  { "art",      16 },
  { "particle", 17 },
  { "inter",    18 },
  { "name",     19 },
  { "v_be",     20 }
};

int   CMakeEng::Configure()
{
  int     nerror;
  int     nindex;

// call nested Init()
  if ( (nerror = CDictMaker<CWordInfo>::Configure()) != 0 )
    return nerror;

// fill partSpMapper
  for ( nindex = 0; nindex < (int)(sizeof(pspLst) / sizeof(pspLst[0]));
     nindex++ )
  {
    if ( partSpMapper.Insert( pspLst[nindex].pspstr,
      pspLst[nindex].pspval ) != 0 )
        return ENOMEM;
  }
  return 0;
}

int     CMakeEng::CreateMorph()
{
  int   nerror;

  if ( (nerror = CDictMaker<CWordInfo>::CreateMorph()) != 0 )
    return nerror;
  return SaveFlexTabs();
}

static  unsigned  ulexid = 1;

int     CMakeEng::ResolveLine( const char*    string,
                               unsigned long  nlexid )
{
  CClassInfo      wdinfo;
  const char*     strorg = string;
  char*           ptrdst;
  char            szflex[256];
  char            partSp[64];

// check the length
  assert( strlen( string ) < MAX_STRING_LEN );

// skip the whitespace
  while ( *string != '\0' && (unsigned char)*string <= 0x20 )
    ++string;

// Convert lexeme description to lower case
  ptrdst = wdinfo.szstem;
  while ( *string != '\0' && *string != FLEX_DELIMITER && (unsigned char)*string > 0x20 )
    *ptrdst++ = toLoCaseMatrix1251[(unsigned char)*string++];
  *ptrdst = '\0';

// check if a skipped word
  if ( IsStrToSkip( wdinfo.szstem ) )
    return 0;

// Process flexions. Fill up grammatical class
  ptrdst = szflex;
  while ( (unsigned char)*string > 0x20 )
    *ptrdst++ = toLoCaseMatrix1251[(unsigned char)*string++];
  *ptrdst = '\0';

// skip the whitespace
  while ( *string != '\0' && (unsigned char)*string <= 0x20 )
    ++string;

// Get the part of speach
  ptrdst = partSp;
  while ( (unsigned char)*string > 0x20 )
    *ptrdst++ = toLoCaseMatrix1251[(unsigned char)*string++];
  *ptrdst = '\0';

// Map part of speech for grammatical class
  if ( (wdinfo.wdinfo = SearchPartSp( partSp )) == (unsigned short)-1 )
  {
    fprintf( stderr, "\n\tUnresolved Part-of-speech tag: %s ", (const char*)strorg );
    return 0;
  }

// Определить основной тип капитализации
  switch ( GetCapType( strorg ) )
  {
    case 1: wdinfo.wdinfo |= wfCapFirst;
            break;
    case 2: wdinfo.wdinfo |= wfCapOnly;
            break;
  }

  if ( (wdinfo.offlex = RegisterFlex( szflex )) == (unsigned short)-1 )
  {
    fprintf( stderr, "Could not register the flexions!\n" );
    return ENOMEM;
  }

  if ( nlexid == 0 )  wdinfo.nlexid = ulexid++;
    else wdinfo.nlexid = nlexid;
  return AddWordInfo( wdinfo );
}

unsigned short  CMakeEng::SearchPartSp( const char* strpsp )
{
  unsigned short* ptrval = partSpMapper.Search( strpsp );

  return ptrval != NULL ? *ptrval : (unsigned short)-1;
}

unsigned short  CMakeEng::RegisterFlex( const char* szflex )
{
  return inflexMapper.AddTable( szflex );
}

int             CMakeEng::SaveFlexTabs()
{
  array<char, char> buffer;
  unsigned          length;

  if ( buffer.SetLen( length = inflexMapper.GetStTabLen() ) != 0 )
    return ENOMEM;
  inflexMapper.SerializeSt( buffer );

  if ( DumpDicPage( "tabs0000.cpp", "fxString", (char*)buffer, length ) != 0 )
    return EFAULT;

  if ( buffer.SetLen( length = inflexMapper.GetFxTabLen() ) != 0 )
    return ENOMEM;
  inflexMapper.SerializeFx( buffer );

  return DumpDicPage( "tabs0001.cpp", "fxTables", (char*)buffer, length );
}

/////////////////////////////////////////////////////////////////////////
char about[] = "makeeng - the dictionary builder;\n"
               "Usage: makeeng ininame\n";

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                          M  A  I  N                                  //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
  CMakeEng  generate;

  setbuf( stderr, NULL );

// Check the arguments
  if ( argc < 2 )
  {
    fprintf( stderr, about );
    return -1;
  }

// Initialize the generator
  if ( generate.Initialize( argv[1] ) != 0 )
    return -1;
  return generate.CreateMorph();
}
