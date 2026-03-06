/******************************************************************************

    libmorphukr - dictionary-based morphological analyser for Ukrainian.

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
# if !defined( _libmorph_ukr_h_ )
# define _libmorph_ukr_h_

# include "mlma-api.h"
# include "mlfa-api.h"

# if !defined( sfHardForms)
#   define sfHardForms       0x0004    /* Затрудненные словоформы         */
# endif   /* !sfHardForms */

# if !defined( nfAdjVerbs )
#   define nfAdjVerbs        0x0100    /* Нормализация до причастия       */
# endif   /* !nfAdjVerbs  */

/**************************************************************************/
/*            Грамматическая информация из таблиц окончаний               */
/*                                                                        */
/*                                *******                                 */
/*                                                                        */
/* Склоняющиеся слова, младшие 2 байта. Грамматическая информация.        */
/*                                                                        */
/* @+++++++ ++++++++ - возвратная форма (глаголы и прилагательные)        */
/* +@@@++++ ++++++++ - падеж                                              */
/* ++++@+++ ++++++++ - число                                              */
/* +++++@@+ ++++++++ - род                                                */
/* +++++++@ ++++++++ - краткая форма (для прил. и прич.)                  */
/* ++++++++ @+++++++ - сравнительная степень                              */
/* ++++++++ +@@+++++ - причастие, деепричастие, страдательное причастие   */
/* ++++++++ +++@@+++ - лицо (для глаголов и их форм)                      */
/* ++++++++ +++++@@@ - временн'ая характеристика глагольной формы         */
/*                                                                        */
/* Ниже документирован дополнительный "информационный" байт, сопутству-   */
/* ющий каждому элементу таблиц окончаний:                                */
/*                                                                        */
/*          +++++++@ - окончание для одушевленного имени                  */
/*          ++++++@+ - окончание для неодушевленного имени                */
/**************************************************************************/

# if !defined( russian_gram_info_defined )
#   define russian_gram_info_defined

#   define wfMultiple     0x40          /* Множественное число   */

#   define afAnimated     0x01          /*                       */
#   define afNotAlive     0x02          /*                       */
#   define afHardForm     0x04          /* Затрудненная форма    */


#   define gfRetForms     0x8000        /* Возвратная форма      */
#   define gfCaseMask     0x7000        /* Маска для падежей     */
#   define gfMultiple     0x0800        /* Множественное число   */
#   define gfGendMask     0x0600        /* Род                   */
#   define gfShortOne     0x0100        /* Краткая форма         */
#   define gfCompared     0x0080        /* Сравнительная степень */
#   define gfVerbForm     0x0060        /* Причастная информация */
#   define gfAdverb       0x0040        /* Наречие от прил.      */
#   define gfVerbFace     0x0018        /* Лицо                  */
#   define gfVerbTime     0x0007        /* Время                 */

#   define vtInfinitiv    0x0001        /* Инфинитив             */
#   define vtImperativ    0x0002        /* Повелит. наклонение   */
#   define vtFuture       0x0003        /* Будущее время         */
#   define vtPresent      0x0004        /* Настоящее время       */
#   define vtPast         0x0005        /* Прошедшее время       */

#   define vbFirstFace    0x0008        /* Первое лицо           */
#   define vbSecondFace   0x0010        /* Второе лицо           */
#   define vbThirdFace    0x0018        /* Третье лицо           */

#   define vfVerb         0x0000        /* Глагольная форма      */
#   define vfVerbActive   0x0020        /* Действит. причастие   */
#   define vfVerbPassiv   0x0040        /* Страд. причастие      */
#   define vfVerbDoing    0x0060        /* Деепричастие          */

# endif  /* russian_gram_info_defined */

# if defined( __cplusplus )
extern "C" {
# endif /* __cplusplus */

  int   MLMAPROC  mlmaukGetAPI( const char* apiKey, void** ppvAPI );
  int   MLMAPROC  mlfaukGetAPI( const char* apiKey, void** ppvAPI );

# if defined( __cplusplus )
}
# endif /* __cplusplus */

# endif  /* !_libmorph_ukr_h_ */
