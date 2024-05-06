# if !defined( __scanlist_h__ )
# define  __scanlist_h__
# include <namespace.h>
# include "gramlist.h"
# include <assert.h>

namespace LIBMORPH_NAMESPACE
{
  template <class collector, class stemcheck>
  class  listTracer
  {
    collector&    output;
    const byte_t* dicpos;

  public:     // initialization
    listTracer( collector& a, const byte_t* p, const byte_t* d ): output( a ), dicpos( d )
      {  (void)p;  }

  public:     // scaner
    int   operator () ( const byte_t* pstems, const byte_t* ptrace, unsigned ltrace ) const
      {
        int   nstems = getserial( pstems );

        while ( nstems-- > 0 )
        {
          const byte_t* thepos = pstems;
          lexeme_t      nlexid = getserial( pstems += 2 );
          word16_t      oclass = getword16( pstems );
          const byte_t* szpost;

        // check postfix
          if ( (oclass & 0x8000) != 0 ) pstems += 1 + *(szpost = pstems);
            else szpost = nullptr;

        // check match
          if ( thepos == dicpos )
            return 1 + output.InsertStem( nlexid, stemcheck( (oclass & ~0x8000) + classmap ), szpost, ptrace + ltrace, nullptr, 0 );
        }

        return (int)(pstems > dicpos);
      }
  };

  template <class collector, class stemcheck>
  class  listLookup
  {
    collector&  output;

  public:     // initialization
    listLookup( collector& a ): output( a ) {}

  public:     // scaner
    int   operator ()( const byte_t* pstems, const fragment& thestr ) const
      {
        SGramInfo fxlist[0x40];     // Массив отождествлений на окончаниях
        unsigned  ucount = getserial( pstems );

        while ( ucount-- > 0 )
        {
          byte_t        chrmin = *pstems++;
          byte_t        chrmax = *pstems++;
          lexeme_t      nlexid = getserial( pstems );
          word16_t      oclass = getword16( pstems );
          const byte_t* szpost;
          size_t        ccflex = thestr.len;
          stemcheck     stinfo;
          int           nforms;
          int           nerror;

        // check postfix
          if ( (oclass & 0x8000) != 0 )
          {
            const byte_t* flestr;
            const byte_t* matstr;
            size_t        ccpost;
            int           rescmp = 0;

            ccpost = *(szpost = pstems);
              pstems += 1 + ccpost;

            if ( ccpost > ccflex )
              continue;

            for ( flestr = thestr.str + ccflex - ccpost, ccflex -= ccpost, matstr = szpost + 1;
              ccpost-- > 0 && (rescmp = *flestr++ - *matstr++) == 0; ) (void)0;

            if ( rescmp != 0 )
              continue;
          }
            else
          szpost = nullptr;

        // оценить, может ли хотя бы потенциально такое окончание быть у основ начиная с этой и далее
          if ( ccflex > 0 )
          {
            if ( *thestr.str > chrmax ) break;
            if ( *thestr.str < chrmin ) continue;
          }

        // check capitalization scheme
          if ( !output.VerifyCaps( stinfo.Load( classmap + (oclass & 0x7fff) ) ) )
            continue;

        // check if non-flective
          if ( stinfo.GetFlexTable() == nullptr )
          {
            if ( ccflex == 0 )
            {
              SGramInfo grprep = { 0, 0, stinfo.tfoffs, 0 };  stinfo.tfoffs = 0;

              if ( (nerror = output.InsertStem( nlexid, stinfo, szpost, thestr.str, &grprep, 1 )) != 0 )
                return nerror;
            }
            continue;
          }

        // Теперь - обработка флективных слов. Сначала обработаем более
        // частый случай - флективное слово без чередований в основе.
        // Для этого достаточно проверить, не выставлен ли флаг наличия
        // чередований, так как при попадании в эту точку слово заведомо
        // будет флективным.
          if ( stinfo.GetSwapTable() == nullptr )
          {
            gramBuffer  grlist( stinfo, (unsigned)-1, fxlist );

            if ( (nforms = LinearScanDict<byte_t, int>( grlist, stinfo.GetFlexTable(), { thestr.str, ccflex } )) == 0 )
              continue;

            if ( (nerror = output.InsertStem( nlexid, stinfo, szpost, thestr.str, fxlist, nforms )) != 0 )
              return nerror;
          }
            else
        // Ну и, наконец, последний случай - слово с чередованиями в основе. Никаких специальных проверок здесь делать
        // уже не требуется, так как попадание в эту точку само по себе означает наличие чередований в основе
          {
            const byte_t* mixtab = stinfo.GetSwapTable();     // Собственно таблица
            int           mixcnt = *mixtab++;                 // Количество чередований
            int           mindex;
            int           nforms;

            for ( mindex = nforms = 0; mindex < mixcnt; ++mindex, mixtab += 1 + (0x0f & *mixtab) )
            {
              const byte_t* curmix = mixtab;
              size_t        mixlen = 0x0f & *curmix;
              unsigned      powers = *curmix++ >> 4;
              const byte_t* flextr = thestr.str;
              size_t        flexcc = thestr.len;
              gramBuffer    grlist( stinfo, powers, fxlist );
              size_t        cmplen;
              int           rescmp;

            // перейти к следующей строке чередования
            // сравнить чередования
              if ( (cmplen = flexcc) > mixlen )
                cmplen = mixlen;

              for ( rescmp = 0, mixlen -= cmplen, flexcc -= cmplen; cmplen-- > 0 && (rescmp = *curmix++ - *flextr++) == 0; )
                (void)0;
              if ( rescmp == 0 )
                rescmp = mixlen > 0;

              if ( rescmp > 0 ) break;
              if ( rescmp < 0 ) continue;

            // Построить массив грамматических отождествлений в предположении, что используется правильная ступень чередования основы
              if ( (nforms = LinearScanDict<byte_t, int>( grlist, stinfo.GetFlexTable(), { flextr, flexcc } )) == 0 )
                continue;

              if ( (nerror = output.InsertStem( nlexid, stinfo, szpost, thestr.str, fxlist, nforms )) != 0 )
                return nerror;
            }
          }
        }
        return 0;
      }
  };

}  // LIBMORPH_NAMESPACE

# endif  // __scandict_h__

