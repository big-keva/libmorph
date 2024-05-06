# if !defined( __gramlist_h__ )
# define  __gramlist_h__
# include <namespace.h>
# include "mlmadefs.h"
# include <assert.h>

namespace LIBMORPH_NAMESPACE
{

  class gramBuffer
  {
    const steminfo&     stinfo;
    unsigned            powset;
    mutable SGramInfo*  outorg;
    mutable SGramInfo*  outptr;

  public:     // construction
    gramBuffer( const steminfo& s, unsigned m, SGramInfo* p ): stinfo( s ), powset( m ), outorg( p ), outptr( p ) {}

  public:     // API
    int     getlen() const
      {  return (int)(outptr - outorg);  }
    void    append( word16_t grinfo, byte_t bflags ) const
      {  *outptr++ = { 0, 0, grinfo, bflags };  }

  private:      // check classes
    struct anyvalue
    {
      bool  operator () ( word16_t grinfo, byte_t bflags ) const
        {  (void)grinfo;  (void)bflags;  return true;  }
    };
    struct multiple
    {
      bool  operator () ( word16_t grinfo, byte_t bflags ) const
        {  (void)bflags;  return (grinfo & gfMultiple) != 0;  }
    };
    class  mixlevel
    {
      const steminfo& stinfo;
      unsigned        powset;

    public:     // construction
      mixlevel( const steminfo& s, unsigned m ): stinfo( s ), powset( m )
        {}

    public:     // functor
      bool  operator () ( word16_t grinfo, byte_t bflags ) const
        {
          int   desire = stinfo.GetSwapLevel( grinfo, bflags );
            assert( desire >= 1 );

          return (powset & (1 << (desire - 1))) != 0;
        }
    };

  protected:  // filler
    template <class isplural, class mixlevel>
    int   FilterGram( const byte_t* thedic,
                      isplural      plural,         // plural filter, defined if wfMultiple
                      mixlevel      isswap ) const  // mixpower filter
      {
        for ( auto nforms = *thedic++; nforms-- > 0; )
        {
          word16_t  grInfo = getword16( thedic );
          byte_t    bflags = *thedic++;

          if ( plural( grInfo, bflags ) && isswap( grInfo, bflags ) )
            append( grInfo, bflags );
        }

        return getlen();
      }

  public:     // gramLoader functor
    int   operator () ( const byte_t* thedic, const fragment& thestr ) const
      {
        (void)thestr;

        if ( thestr.len == 0 )
        {
          if ( (stinfo.wdinfo & wfMultiple) != 0 )
          {
            return powset == (unsigned)-1 ? FilterGram( thedic, multiple(), anyvalue() )
                                          : FilterGram( thedic, multiple(), mixlevel( stinfo, powset ) );
          }
            else
          {
            return powset == (unsigned)-1 ? FilterGram( thedic, anyvalue(), anyvalue() )
                                          : FilterGram( thedic, anyvalue(), mixlevel( stinfo, powset ) );
          }
        }
        return 0;
      }

  };

}  // LIBMORPH_NAMESPACE

# endif  // __gramlist_h__

