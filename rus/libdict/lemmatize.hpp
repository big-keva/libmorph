# if !defined( __libmorph_rus_lemmatize_hpp__ )
# define __libmorph_rus_lemmatize_hpp__
# include "xmorph/buildforms.hpp"
# include "codepages.hpp"
# include "grammap.h"

namespace libmorph {
namespace rus {

 /*
  * Lemmatizer - create lemmatization result, build normal (dictionary) form for
  * a stem passed;
  * the output data - the references to the arrays to be filled by the lemmatizer
  */
  class Lemmatizer
  {
    template <class T>
    struct  target
    {
      T*  beg = nullptr;
      T*  cur = nullptr;
      T*  lim = nullptr;

    public:
      target() = default;
      target( T* p, size_t l ): beg( p ), cur( p ), lim( p + l )  {}

      bool  operator == ( nullptr_t ) const {  return beg == nullptr;  }
      bool  operator != ( nullptr_t ) const {  return !(*this == nullptr);  }
    };

  public:
    Lemmatizer(
      const CapScheme&          cs,
      const target<SLemmInfoA>& lm,
      const MbcsCoder&          fm,
      const target<SGramInfo>&  gr, const uint8_t* st, unsigned us );

  public:
    operator int() const  {  return nbuilt;  }

    int  operator()(
      lexeme_t          nlexid,
      const steminfo&   lextem,
      const fragment&   inflex,
      const fragment&   suffix,
      const SGramInfo&  grinfo ) const;

    int  operator()(
      lexeme_t          nlexid,
      const steminfo&   lextem,
      const fragment&   inflex,
      const fragment&   suffix,
      const SGramInfo*  grbuff,
      size_t            ngrams ) const;

  protected:
            const CapScheme     casing;       // capitalization scheme tool
    mutable target<SLemmInfoA>  plemms;       // the output buffer for descriptions
    mutable MbcsCoder           pforms;       // the buffer for the forms
    mutable target<SGramInfo>   pgrams;       // the buffer for grammar descriptions
    mutable int                 nbuilt = 0;
            const uint8_t*      szstem;
            unsigned            dwsets;

  };

}} // end namespace

# endif // !__libmorph_rus_lemmatize_hpp__
