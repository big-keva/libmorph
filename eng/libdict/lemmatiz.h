# if !defined( _lemmatiz_h_ )
# define _lemmatiz_h_

# include "mlmadefs.h"

namespace __libmorpheng__
{
  #if !defined( lemmatize_errors_defined )
    #define lemmatize_errors_defined
    #define LEMMBUFF_FAILED -1
    #define LIDSBUFF_FAILED -2
    #define GRAMBUFF_FAILED -3
    #define WORDBUFF_FAILED -4
  #endif

  typedef struct tagLemmInfo
  {
    char*     lpDest;   // Указатель на массив нормальных форм слова
    word16_t  ccDest;   // Количество символов, влезающее в lpDest
    word32_t* lpLids;   // Указатель на массив лексических номеров
    word16_t  clLids;   // Количество элементов, влезающее в lpLids
    char*     lpInfo;   // Указатель на массив описаний отождествившихся форм
    word16_t  cbInfo;   // Количество байт, влезающих в lpInfo
    short     failed;   // Номер слетевшего по переполнению параметра
    word16_t  fBuilt;   // Количество построенных нормальных форм
  } SLemmInfo;

  typedef struct tagMakeInfo
  {
    char*         lpDest;   // Указатель на массив построенных форм слова
    word16_t      ccDest;   // Количество символов, влезающее в lpDest
    byte08_t      idForm;   // Идентификатор формы
    word32_t      wrdLID;   // Лексический номер обрабатываемого слова
    short         failed;   // Признак переполнения выходного массива
    word16_t      fBuilt;   // Количество форм
  } SMakeInfo;

  void  actLemmatize( const byte08_t* lpStem,
                      const byte08_t* lpWord,
                      SGramInfo*      grInfo,
                      int             cgInfo,
                      SScanPage&      rfScan );
  void  actBuildForm( const byte08_t* lpStem,
                      const byte08_t* lpWord,
                      SGramInfo*      grInfo,
                      int             cgInfo,
                      SScanPage&      rfScan );
  void  actListForms( const byte08_t* lpStem,
                      const byte08_t* lpWord,
                      SGramInfo*      grInfo,
                      int             cgInfo,
                      SScanPage&      rfScan );

  void  actGetWdType( const byte08_t* lpStem,
                      const byte08_t* lpWord,
                      SGramInfo*      grInfo,
                      int             cgInfo,
                      SScanPage&      rfScan );
  void  actGetClass(  const byte08_t* lpStem,
                      const byte08_t* lpWord,
                      SGramInfo*      grInfo,
                      int             cgInfo,
                      SScanPage&      rfScan );
} // end __libmorpheng__ namespace

# endif // _lemmatiz_h_
