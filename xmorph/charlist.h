# if !defined( __libmorph_charlist_h__ )
# define __libmorph_charlist_h__

namespace libmorph {

  class Charset
  {
    enum: size_t
    {
      element_bits = sizeof(uint32_t) * CHAR_BIT,
      charset_size = 0x100 / element_bits
    };
    uint32_t  uchars[charset_size];

  public:
    Charset()
      {  memset( uchars, 0, sizeof(uchars) );  }

  public:
    void  operator()( uint8_t c )
      {  uchars[c / element_bits] |= 1 << (c % element_bits);  }
    void  operator()( char c )
      {  return (*this)( uint8_t(c) );  }
    template <class Collector>
    int   operator()( Collector& out ) const
    {
      int   ncount = 0;

      for ( size_t o = 0; o != charset_size; ++o )
        if ( uchars[o] != 0 )
        {
          for ( size_t b = 0; b != element_bits; ++b )
            if ( (uchars[o] & (1 << b)) != 0 )
            {
              if ( out.append( char(o * element_bits + b) ) ) ++ncount;
                else return -1;
            }
        }
      return ncount;
    }
  };

}

# endif // !__libmorph_charlist_h__
