# if !defined( _mlmadefs_h_ )
# define _mlmadefs_h_

// Определить кроссплатформенные типы данных
# include "../include/mlma1033.h"

namespace __libmorpheng__
{
  // Define common types used in the analyser
  typedef unsigned char   byte08_t;
  typedef unsigned short  word16_t;
  typedef lexeme_t        word32_t;

  // Define data access macros
  # if defined( WIN32 ) || defined( WIN16 ) || defined( INTEL_SYSTEM )

  #   if !defined( GetWord16 )
  #     define GetWord16( pv )     *(word16_t*)(pv)
  #   endif  // GetWord16
  #   if !defined( GetWord32 )
  #     define GetWord32( pv )     *(word32_t*)(pv)
  #   endif  // GetWord32

  #   define Swap16( sw )
  #   define Swap32( sw )

  # elif defined( SUN )

  #   if !defined( GetWord16 )
  #     define GetWord16( pv )  ( ((byte08_t*)(pv))[0] | (((byte08_t*)(pv))[1] << 8) )
  #   endif  // GetWord16
  #   if !defined( GetWord32 )
  #     define GetWord32( pv )  ( ((byte08_t*)(pv))[0] | (((byte08_t*)(pv))[1] << 8)  \
                               | (((byte08_t*)(pv))[2] << 16) | (((byte08_t*)(pv))[3] << 24))
  #   endif  // GetWord32


  #   define Swap16( sw ) (sw) = (((sw) & 0x00FF) << 8) | ((sw) >> 8)
  #   define Swap32( sw ) (sw) = ((sw) << 24) | (((sw) & 0x0000FF00) << 8)  \
                              | (((sw) & 0x00FF0000) >> 8) | ((sw) >> 24)

  # endif // macros definitions

  // Грамматическая информация, используемая при общении с вызывающими
  // программами
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

  // Внутренняя структура, используемая при сканировании страниц
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

  // Функции min и max - свои, так как в разных компиляторах
  // используются разные способы определения этих функций

  template <class T>
  inline T minimal( T v1, T v2 )
    {  return ( v1 <= v2 ? v1 : v2 );  }

  template <class T>
  inline T maximal( T v1, T v2 )
    {  return ( v1 >= v2 ? v1 : v2 );  }

  // Функции доступа к таблицам окончаний

  inline  byte08_t* GetInfoByID( word16_t id )
    {  return classBox + GetWord16( ((word16_t*)classBox) + (id & 0x0FFF) ); }

  inline  byte08_t* GetFragByID( int id )
    {  return suffixes + GetWord16( ((word16_t*)suffixes) + id );  }

  inline int  GetSkipCount( const byte08_t* stemList )
  {
    int       cbSkip = 2;
    word16_t  idStem = GetWord16( stemList );

  // Если у слова есть псевдосуффикс, пропустить еще байт
    if ( idStem & wfSuffix )
      cbSkip++;
  // Если есть нефлективный текст в постпозиции, пропустить
  // его длину и один байт
    if ( idStem & wfPostSt )
      cbSkip += stemList[cbSkip] + 1;
  // Пропустить размер идентификатора лексемы
    return cbSkip + ((idStem & 0x3000) >> 12) + 1;
  }

  // Прототип функций, вызываемых на словаре при успешном отождествлении
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

  #define MAX_WORD_FORMS  32

// Provide windows.h definitions if needed
// Отключить использование ненужных определений - OLE, графику
// Собственно, здесь windows.h нужен, чтобы задать тип экспортируемых
// функций в соответствии с требованиями операционной системы
  # if defined( WIN32 ) || defined( WIN16 )
  #   define WIN32_LEAN_AND_MEAN
  #   include <windows.h>
  # endif

} // end __libmorpheng__ namespace

# endif // _mlmadefs_h_
