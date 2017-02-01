# if !defined( __typedefs_h__ )
# define __typedefs_h__

# if defined( LIBMORPH_NAMESPACE )
namespace LIBMORPH_NAMESPACE {
# endif  // LIBMORPH_NAMESPACE

// Define common types used in the analyser
# if !defined( __byte_t_defined__ )
#   define  __byte_t_defined__
    typedef unsigned char   byte_t;
# endif  // !__byte_t_defined__

# if !defined( __word16_t_defined__ )
#   define __word16_t_defined__
    typedef unsigned short  word16_t;
# endif  // !__word16_t_defined__

# if !defined( __word32_t_defined__ )
#   define __word32_t_defined__
    typedef unsigned int    word32_t;
# endif  // !__word32_t_defined__

# if defined( LIBMORPH_NAMESPACE )
}
# endif  // LIBMORPH_NAMESPACE

# endif  // __typedefs_h__
