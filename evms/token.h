/*
 *
 *   (C) Copyright IBM Corp. 2001, 2003
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Module: token.h
 */

/*
 * Change History:
 *  06/2001     B. Rafanello  Initial version 
 *
 */

/*
 *
 */

#ifndef TOKEN_H

#define TOKEN_H 1

#include <sys/types.h>

/* A boolean variable is one which is either TRUE or FALSE. */
#ifndef boolean_DEFINED
  #define boolean_DEFINED 1
  typedef u_int8_t  boolean;
#endif                     
                     
#ifndef FALSE
  #define FALSE 0
#endif
#ifndef TRUE
  #define TRUE  1
#endif

/* This defines the maximum length that any identifier may be.  Longer
   identifiers will be truncated to this length */
#define MaxIdentifierLength 256

/* Define a typecode for use with the dlist_t system. */
#define TOKEN_CODE  93484782

/* Token Characterizations */
typedef enum {
                Separator,
                Number,
                RealNumber,
                KeyWord,
                String,
                Space,
                MultiSpace,
                Tab,
                MultiTab,
                InvalidCharacter,
                EofToken,
             } TokenCharacterizations;
               
/* Structures of Interest */
typedef struct {
                  TokenCharacterizations    Characterization;   /* The "type" of token this is. */
                  uint                      TokenRow;           /* The line on which the token was found.  Useful for files of commands. */
                  uint                      TokenColumn;        /* The position of the first character of this token on the command line. */
                  uint                      TokenLength;        /* # of characters in TokenText */
                  char *                    TokenText;          /* The characters from the command line which make up this token. */
                  boolean                   In_Lookahead_Queue; /* TRUE if this token is being tracked by the lookahead system. */
               } TokenType;

             
#endif

