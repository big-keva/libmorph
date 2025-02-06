/******************************************************************************

    libmorph - morphological analysers.

    Copyright (C) 1994-2025 Andrew Kovalenko aka Keva

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
      email: keva@meta.ua, keva@rambler.ru
      Phone: +7(495)648-4058, +7(926)513-2991, +7(707)129-1418

******************************************************************************/
# if !defined( __ftables_h__ )
# define __ftables_h__
# include <algorithm>
# include <cstdint>
# include <cstring>
# include <string>
# include <tuple>

namespace libmorph {

  class  GramLevels
  {
    uint16_t  levels[3];

  protected:
    GramLevels( uint16_t u0, uint16_t u1, uint16_t u2 ): levels { u0, u1, u2 } {}

  public:
    GramLevels( const GramLevels& gl ): levels{ gl.levels[0], gl.levels[1], gl.levels[2] } {}
    GramLevels& operator = ( const GramLevels& gl )
      {
        levels[0] = gl.levels[0];
        levels[1] = gl.levels[1];
        levels[2] = gl.levels[2];
        return *this;
      }
  public:
    operator const uint16_t* () const {  return levels;  }
    const uint16_t* get() const {  return levels;  }

  public:
    uint16_t  GetUpLevels( int clevel ) const
    {
      uint16_t  destBits = 0x0000;

      for ( int i = clevel + 1; i < 3; i++ )
        destBits |= levels[i];
      return destBits;
    }
  };

  struct NounLevels: public GramLevels
  {
    using GramLevels::GramLevels;

    NounLevels(): GramLevels( 0xFFFF, 0, 0 )  {}
  };

  struct VerbLevels: public GramLevels
  {
    using GramLevels::GramLevels;

    VerbLevels(): GramLevels( 0x007F, 0x7F80, 0x8000/*gfRetForms*/ )  {}
  };

  class FlexStripper
  {
    const GramLevels& levels;
    const void*       tables;

  public:     // construction
    FlexStripper( const GramLevels& lev, const void* tab ): levels( lev ), tables( tab )  {}

  public:     // helpers
    static  std::tuple<uint8_t, uint8_t>  GetMinMaxChar( const char* tables, uint16_t     tfoffs );
    static  bool                          StringHasTail( const char* szstem, const char*  sztail );
    static  bool                          StringHasTail( std::string& rstem, const char*  sztail );

  public:     // stripper
    bool  StripStr( std::string&, uint16_t grinfo, uint16_t tfoffs, int clevel = 0 ) const;

  };

  inline  bool  FlexStripper::StringHasTail( const char* stem, const char* tail )
  {
    size_t  ccStem = strlen( stem );
    size_t  ccTail = 0x0f & *tail++;

    return ccTail <= ccStem && memcmp( stem + ccStem - ccTail, tail, ccTail ) == 0;
  }

  inline  bool  FlexStripper::StringHasTail( std::string& rstr, const char* tail )
  {
    return StringHasTail( rstr.c_str(), tail );
  }

  namespace grammatic
  {

    template <class T>
    inline  uint16_t  getword16( const T*& p )
      {
        auto blower = static_cast<const uint8_t*>( p )[0];
        auto bupper = static_cast<const uint8_t*>( p )[1];

        p = static_cast<const T*>( 2 + static_cast<const uint8_t*>( p ) );
        return (uint16_t)(blower | (bupper << 8));
      }

    inline  uint8_t         flex_flag( const uint8_t* flex )
      {  return *flex;  }
    inline  uint16_t        flex_info( const uint8_t* flex )
      {  return getword16( ++flex );  }
    inline  const uint8_t*  flex_text( const uint8_t* flex )
      {  return flex + 3;  }
    inline  uint16_t        flex_next( const uint8_t* flex )
      {  return getword16( flex += (flex[3] + 4) );  }

  }

  inline
  bool  FlexStripper::StripStr( std::string&  ststem,
                                uint16_t      grinfo,
                                uint16_t      tfoffs,
                                int           clevel ) const
  {
    const char* pTable = (const char*)tables + (tfoffs << 1); // inflexion table
    int         nItems = *pTable++ & 0x7F;                        // elements count

  // loop all the table elements
    while ( nItems > 0 )
    {
      auto  flex = (const uint8_t*)pTable;
      auto  pstr = grammatic::flex_text( flex );

    // skip to next
      pTable = (const char*)(pstr + *pstr + 1 + ((grammatic::flex_flag( flex ) & 0xC0) != 0 ? sizeof(uint16_t) : 0));
      nItems--; 

    // skip flexes with irrelevant grammar
      if ( (grinfo & levels[clevel]) != (grammatic::flex_info( flex ) & levels[clevel]) )
        continue;

      if ( ( (grinfo & levels[clevel]) == 0 ) && ( levels[clevel] != 0xFFFF ) )
        continue;

      if ( clevel < 2 && (grammatic::flex_flag( flex ) & 0x80) != 0 )
      {
        if ( StripStr( ststem, grinfo, grammatic::flex_next( flex ), clevel ) && StringHasTail( ststem, (const char*)pstr ) )
        {
          ststem.resize( ststem.length() - *pstr );
          return true;
        }
      }
        else
      {
        if ( !StringHasTail( ststem, (const char*)pstr ) )
          continue;
        ststem.resize( ststem.length() - *pstr );
        return true;
      }
    }
    if ( clevel < 2 && levels.GetUpLevels( clevel ) != 0 )
      return StripStr( ststem, grinfo, tfoffs, clevel + 1 );
    return false;
  }

  inline
  std::tuple<uint8_t, uint8_t>  FlexStripper::GetMinMaxChar( const char* tables, uint16_t tfoffs )
  {
    const uint8_t*  ptable = (const uint8_t*)tables + (tfoffs << 1);
    int             nitems = *ptable++ & 0x7F;
    int             clower = 0x100;
    int             cupper = -1;

    while ( nitems-- > 0 )
    {
      const uint8_t*  flex = ptable;
      const uint8_t*  pstr = grammatic::flex_text( flex );
      int             strl = *pstr++;

      ptable = pstr + strl + ((grammatic::flex_flag( flex ) & 0xC0) != 0 ? sizeof(unsigned short) : 0);

      if ( strl > 0 )
      {
        clower = std::min( (int)*pstr, clower ); 
        cupper = std::max( (int)*pstr, cupper ); 
      }
        else
      if ( (grammatic::flex_flag( flex ) & 0x80) == 0 )
      {
        clower = 0;
        cupper = cupper >= 0 ? cupper : 0;
      }
        else
      {
        uint8_t cl;
        uint8_t cu;

        std::tie(cl, cu) = GetMinMaxChar( tables, grammatic::flex_next( flex ) );

        clower = std::min( clower, (int)cl );
        cupper = std::max( cupper, (int)cu );
      }
    }

    return std::make_tuple( (uint8_t)clower, (uint8_t)cupper );
  }

}

# endif // __ftables_h__
