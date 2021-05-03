/*! \file
//[]================================================================== <br>
//[] Project: English Morphological Library Builder                    <br>
//[] File name: StringBox.h                                            <br>
//[] Abstract:                                                         <br>
//[]                                                                   <br>
//[]                                                                   <br>
//[] Author: K.Ignatchenko aka Kostyan                                 <br>
//[] Created: 29.04.2002                                               <br>
//[] Modifier:                                                         <br>
//[] Last modified:                                                    <br>
//[] Modifier notes:                                                   <br>
//[]==================================================================*/ 
#if !defined __stringbox_h__
#define __stringbox_h__

# include "../../include/stringmap.h"
# include <stdio.h>

# define STRING_NOT_FOUND  0x0FFFF

///////////////////////////////////////////////////
// Class - storage of strings
class CStringBox: protected stringmap<unsigned short, unsigned short>
{
  unsigned  length;   // Current size of the string box
public:
                  CStringBox(): length( 0 )
                    {
                    }
// Adds string to the string box && returns it's offset
// This offset is unique && never changes after the string 
// is added; if the string is already presents in the box, 
// just returns it's offset.
  unsigned short  AddString( const char*  string,
                             unsigned     length = (unsigned)-1 ); 
  unsigned short  GetOffset( const char*  string,
                             unsigned     length = (unsigned)-1 ) const;
  const char*     GetString( unsigned short offset );

// serialization
  unsigned        GetLength() const;
  char*           Serialize( char* );  
// Sort all the strings by offset before saving
// Return false if any error occurs during saving
  int             Put( FILE* );
  int             Put( const char* lpname );
protected:
  static int      cmp( void*&, void*& );
};

#endif // !defined __stringbox_h__
