# include "../../../rus.h"
# include <algorithm>
# include <cstring>
# include <moonycode/codes.h>
# include <mtc/wcsstr.h>
# include <time.h>

extern  unsigned  charRank[32];

auto  GetNewTask( IMlmaWc* morpho,
  widechar* buffer, size_t length ) -> const widechar*;

bool  IsExcluded( const widechar* str, size_t len, uint32_t exc )
{
  while ( len-- > 0 )
    if ( (exc & (1 << (*str++ - u'а'))) != 0 )
      return true;
  return false;
}

bool  IsIncluded( const widechar* str, size_t len, uint32_t exc )
{
  while ( len-- > 0 )
    exc &= ~(1 << (*str++ - u'а'));
  return exc == 0;
}

auto  MakeMask( const widechar* str, size_t len ) -> uint32_t
{
  auto  mask = uint32_t(0);

  while ( len-- > 0 )
    mask |= (1 << (*str++ - u'а'));

  return mask;
}

auto  GetWordset( IMlmaWcXX* morpho,
  const widechar* wstempl,
  uint32_t        exclude,
  uint32_t        include ) -> std::vector<codepages::widestring>
{
  std::vector<codepages::widestring>  output;

  morpho->FindMatch( { wstempl, 5 }, [&]( lexeme_t lex, int num, const SStrMatch* ptr )
    {
      uint8_t wdinfo;

      morpho->GetWdInfo( &wdinfo, lex );

      if ( wdinfo >= 7 && wdinfo <= 23 )          // только существительные
        for ( ; num-- > 0; ++ptr )
        {
          if ( ptr->id == 0 || ptr->id == 0xff )  // в словарной форме или неизменяемые
          {
            if ( !IsExcluded( ptr->ws, ptr->cc, exclude ) && IsIncluded( ptr->ws, ptr->cc, include ) )
              output.push_back( { ptr->ws, ptr->cc } );
          }
        }

      return 0;
    } );

  return output;
}

bool  CheckWord( widechar* show, const widechar* task, const widechar* word )
{
  int  match = 1;

  show[0] = task[0] == word[0] ? task[0] :
    std::find( task + (match = 0), task + 5, word[0] ) == task + 5 ? '-' : '+';
  show[1] = task[1] == word[1] ? task[1] :
    std::find( task + (match = 0), task + 5, word[1] ) == task + 5 ? '-' : '+';
  show[2] = task[2] == word[2] ? task[2] :
    std::find( task + (match = 0), task + 5, word[2] ) == task + 5 ? '-' : '+';
  show[3] = task[3] == word[3] ? task[3] :
    std::find( task + (match = 0), task + 5, word[3] ) == task + 5 ? '-' : '+';
  show[4] = task[4] == word[4] ? task[4] :
    std::find( task + (match = 0), task + 5, word[4] ) == task + 5 ? '-' : '+';
  return show[5] = 0, match != 0;
}

unsigned  GetWordRank( const widechar* word, size_t len )
{
  unsigned  rank = 0;

  for ( size_t i = 0; i < len; ++i )
    rank += charRank[word[i] - u'а'];

  return rank;
}

IMlmaWcXX*  morpho;

int main ()
{
  mlmaruGetAPI( "utf-16", (void**)&morpho );

  auto      sample = u"шляпа";
  widechar  search[5] = { u'т', u'е', u'?', u'?', u'?' };
  auto      hypots = GetWordset( morpho, search, MakeMask( u"видойкалунсь", 12 ), 0 );
  auto      tested = std::vector<codepages::widestring>();

  for ( auto incset = 0U, excset = 0U; hypots.size() != 0; )
  {
    widechar  markup[6];
    size_t    select = size_t(-1);
    unsigned  serank = 0;
    bool      bcheck;

  // выбрать наиболее вероятную гипотезу из найденных по её символьному составу,
  // используя таблицу вероятностей символов русского текста
  // принимать во пнимание только ещё не использованные слова
    for ( size_t i = 0; i < hypots.size(); ++i )
      if ( std::find( tested.begin(), tested.end(), hypots[i] ) == tested.end() )
      {
        auto  wdrank = GetWordRank( hypots[i].c_str(), hypots[i].size() );

        if ( select == size_t(-1) || wdrank > serank )
        {
          select = i;
          serank = wdrank;
        }
      }

    if ( select == size_t(-1) )
      break;

  // сравнить с образцом
    bcheck = CheckWord( markup, sample, hypots[select].c_str() );
      tested.push_back( hypots[select] );

  // печатаем вариант
    fprintf( stdout, "%s\t\t%s",
      codepages::widetombcs( codepages::codepage_utf8, hypots[select] ).c_str(),
      codepages::widetombcs( codepages::codepage_utf8, markup ).c_str());

  // если слово найдено, завершает работу
    if ( bcheck )
      return fprintf( stdout, "\tOK\n" ), 0;

    fprintf( stdout, "\n" );

  // по результатам разметки либо добавляем буквы в требуемые, либо исключаем
    for ( auto i = 0; i != 5; ++i )
    {
      if ( markup[i] == hypots[select][i] ) search[i] = sample[i];
        else
      if ( markup[i] == '+' ) incset |= MakeMask( hypots[select].c_str() + i, 1 );
        else
      if ( markup[i] == '-' ) excset |= MakeMask( hypots[select].c_str() + i, 1 );
    }

    hypots = GetWordset( morpho, search, excset, incset );
  }
  return fprintf( stdout, "%s\n", "не угадали слово :(" ), -1;

/*  for ( auto& next: awords )
  {
    widechar  smatch[0x100];
    char      scheck[0x100];
    char      sprint[0x100];

    CheckWord( smatch, u"мотив", next.c_str() );

    codepages::widetombcs( codepages::codepage_utf8, scheck, sizeof(scheck), smatch );
    codepages::widetombcs( codepages::codepage_utf8, sprint, sizeof(sprint), next.c_str(), next.size() );

    fprintf( stdout, "%s\t%s\n", sprint, scheck );
  }*/
  return 0;
}