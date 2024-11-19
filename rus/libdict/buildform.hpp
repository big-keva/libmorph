# if !defined( __libmorph_rus_buildform_hpp__ )
# define __libmorph_rus_buildform_hpp__
# include "xmorph/buildforms.hpp"
# include "rus/codepages.hpp"
# include "grammap.h"

namespace libmorph {
namespace rus {

  class FormBuild
  {
    mutable MbcsCoder         pforms;       // the buffer for the forms
            const CapScheme   casing;       // capitalization scheme tool
            const lexeme_t    lexeme;
            const uint8_t*    szstem;
            const formid_t    idform;
    mutable int               nbuilt = 0;

  public:
    FormBuild(
      const MbcsCoder&  fm,
      const CapScheme&  cs,
      lexeme_t          lx,
      formid_t          fi,
      const uint8_t*    st );

  public:
    operator int() const  {  return nbuilt;  }

    int  operator()(
      lexeme_t          nlexid,
      const steminfo&   lextem,
      const fragment&   inflex,
      const fragment&   suffix,
      const SGramInfo&/*grinfo*/ ) const;

    int  operator()(
      lexeme_t        /*nlexid*/,
      const steminfo&   lextem,
      const fragment&   inflex,
      const fragment&   suffix,
      const SGramInfo*/*pgrams*/,
      size_t          /*ngrams*/ ) const;

  };

 /*
  * Meth: GetDictF orms
  * Функция синтезирует варианты флективной части слова, исходя из его
  * части речи, чередований, окончаний и грамматической информации
  * о форме.
  * Возвращает количество построенных вариантов.
  */
  int   GetDictForms( Collector&, unsigned dwsets,
    const steminfo&,
    const flexinfo&,
    const fragment& prefix,
    const fragment& suffix = {} );

 /*
  * GetWordForms(...)
  * Синтезирует варианты флективной части слова, исходя из его части речи,
  * чередований, окончаний и грамматической информации о форме.
  * Возвращает количество построенных вариантов.
  */
  int   GetWordForms( Collector&,
    const steminfo&,
    const flexinfo&,
    const fragment&   prefix,
    const fragment&   suffix = {} );

}}

# endif // !__libmorph_rus_buildform_hpp__
