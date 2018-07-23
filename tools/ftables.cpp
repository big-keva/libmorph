# include "ftables.h"
# include <string.h>
# include <algorithm>

namespace libmorph {

  inline  unsigned char   flex_flag( const unsigned char* flex )
    {  return *flex;  }
  inline  unsigned short  flex_info( const unsigned char* flex )
    {  return *(unsigned short*)(flex + 1);  }
  inline  unsigned char*  flex_text( const unsigned char* flex )
    {  return (unsigned char*)(flex + 3);  }
  inline  unsigned short  flex_next( const unsigned char* flex )
    {  return *(unsigned short*)(flex + flex[3] + 4);  }

  const uint16_t  verbLevels[3] = { 0x007F, 0x7F80, 0x8000/*gfRetForms*/ };
  const uint16_t  nounLevels[3] = { 0xFFFF, 0, 0 };

  inline uint16_t GetUpLevels( const uint16_t*  levels, int clevel )
  {
    uint16_t  destBits = 0x0000;

    for ( int i = clevel + 1; i < 3; i++ )
      destBits |= levels[i];
    return destBits;
  }

  template <class T>
  inline  uint16_t  getword16( const T*& p )
  {
    uint16_t  v = *(uint16_t*)p;
      p = (T*)(sizeof(uint16_t) + (char*)p);
    return v;
  }

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
      unsigned char*  flex = (unsigned char*)pTable;
      unsigned char*  pstr = flex_text( flex );

    // skip to next
      pTable = (const char*)(pstr + *pstr + 1 + ((flex_flag( flex ) & 0xC0) != 0 ? sizeof(uint16_t) : 0));
      nItems--; 

    // skip flexes with irrelevant grammar
      if ( (grinfo & levels[clevel]) != (flex_info( flex ) & levels[clevel]) )
        continue;

      if ( ( (grinfo & levels[clevel]) == 0 ) && ( levels[clevel] != 0xFFFF ) )
        continue;

      if ( clevel < 2 && (flex_flag( flex ) & 0x80) != 0 )
      {
        if ( StripStr( ststem, grinfo, flex_next( flex ), clevel ) && StringHasTail( ststem, (const char*)pstr ) )
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
    if ( clevel < 2 && GetUpLevels( levels, clevel ) != 0 )
      return StripStr( ststem, grinfo, tfoffs, clevel + 1 );
    return false;
  }

  std::tuple<uint8_t, uint8_t>  FlexStripper::GetMinMaxChar( const char* tables, uint16_t tfoffs )
  {
    const uint8_t*  ptable = (const uint8_t*)tables + (tfoffs << 1);
    int             nitems = *ptable++ & 0x7F;
    int             clower = 0x100;
    int             cupper = -1;

    while ( nitems-- > 0 )
    {
      const uint8_t*  flex = ptable;
      const uint8_t*  pstr = flex_text( flex );
      int             strl = *pstr++;

      ptable = pstr + strl + ((flex_flag( flex ) & 0xC0) != 0 ? sizeof(unsigned short) : 0);

      if ( strl > 0 )
      {
        clower = std::min( (int)*pstr, clower ); 
        cupper = std::max( (int)*pstr, cupper ); 
      }
        else
      if ( (flex_flag( flex ) & 0x80) == 0 )
      {
        clower = 0;
        cupper = cupper >= 0 ? cupper : 0;
      }
        else
      {
        uint8_t cl;
        uint8_t cu;

        std::tie(cl, cu) = GetMinMaxChar( tables, flex_next( flex ) );

        clower = std::min( clower, (int)cl );
        cupper = std::max( cupper, (int)cu );
      }
    }

    return std::make_tuple( (uint8_t)clower, (uint8_t)cupper );
  }

}
