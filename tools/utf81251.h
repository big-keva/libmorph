# pragma once
# if !defined( __utf8to1251_h__ )
# define __utf8to1251_h__
# include <libcodes/codes.h>
# include <string>

inline  std::string utf8to1251( const char* s )
{
  size_t      utflen = codepages::utf8::strlen( s );
  std::string winstr( utflen + 1, ' ' );

  winstr.resize( codepages::mbcstombcs(
                 codepages::codepage_1251, (char*)winstr.c_str(), winstr.size(),
                 codepages::codepage_utf8, s ) );

  return winstr;
}

# endif   // __utf8to1251_h__
