# if !defined( __libmorph_eng_buildform_hpp__ )
# define __libmorph_eng_buildform_hpp__
# include "mlmadefs.h"
# include "xmorph/buildforms.hpp"
# include "xmorph/capsheme.h"
# include "../../codepages.hpp"

namespace libmorph {
namespace eng {

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
   * GetWordForms(...)
   * Синтезирует варианты флективной части слова, исходя из его части речи,
   * чередований, окончаний и грамматической информации о форме.
   * Возвращает количество построенных вариантов.
   */
  int   GetWordForms( Collector&,
    const fragment& prefix,
    const fragment& suffix,
    const steminfo& stinfo, formid_t );

  /*
  * Meth: GetDictF orms
  * Функция синтезирует варианты флективной части слова, исходя из его
  * части речи, чередований, окончаний и грамматической информации
  * о форме.
  * Возвращает количество построенных вариантов.
  */
  int   GetDictForms( Collector&,
    const fragment& prefix,
    const fragment& suffix,
    const steminfo& stinfo );

}}

# endif // !__libmorph_eng_buildform_hpp__
