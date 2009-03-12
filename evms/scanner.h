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
 * Module: scanner.h
 */

/*
 * Change History:
 *  05/15/2001  B. Rafanello  Initial version begun
 *
 */

/*
 *
 */

/* This module defines the interface to the scanner, which is the front end of
   the syntactic analyzer used by the LVM command.  The scanner is an FSA whose
   states and actions are as indicated in the table below:

                                                                Token Type
   State   Characters that trigger a           State to         Output To
   Name           Transition                   Transition to    Screener
   ----------------------------------------------------------------------------

   Start  --
          ' '                               -> SingleSpace
          '\t'                              -> SingleTab
          ',',':','{','}' , '=' , '(' , ')' -> Start                    => "Separator"
          '-'                               -> OptionCheck
          '+'                               -> PositiveNumberCheck
          '0' .. '9'                        -> IsNumber
          '"'                               -> IsString
          '/'                               -> IsPathName
          'A' .. 'Z' , 'a' .. 'z'           -> IsKeyWord
          EOF                               -> EndState
                                            -> Error;

   OptionCheck --
          '-'                               -> Start                    => "Separator"
                                            -> IsNegativeNumber;

   IsNegativeNumber --
          '0' .. '9'                        -> IsNumber
                                            -> Start                    => "Separator";

   IsNumber --
          '0' .. '9'                        -> IsNumber
          '.'                               -> IsRealNumber
          'A' .. 'Z', 'a' .. 'z'            -> IsKeyWord                => "Number"
                                            -> SeparatorOrError         => "Number";

   IsRealNumber --
          '0' .. '9'                        -> IsRealNumber
          'A' .. 'Z', 'a' .. 'z'            -> IsKeyWord                => "RealNumber"
                                            -> SeparatorOrError         => "RealNumber";

   SeparatorOrError --
          ',',':','}'                       -> Start                    => "Separator"
          ' '                               -> SingleSpace
          '\t'                              -> SingleTab
          EOF                               -> EndState
                                            -> Error;

   PositiveNumberCheck --
          '0' .. '9'                        -> IsNumber
                                            -> EndState                 => "Invalid Character";

   IsPathName --
          '/' , 'A' .. 'Z' , 'a' .. 'z'     -> IsPathName
          '0' .. '9'                        -> IsPathName
          '_' , '-'                         -> IsPathName
                                            -> SeparatorOrError         => "String";

   IsKeyWord --
          'A' .. 'Z' , 'a' .. 'z'           -> IsKeyWord
          '0' .. '9'                        -> IsKeyWord
          '_' , '-'                         -> IsKeyWord
          '/'                               -> IsPathName
                                            -> SeparatorOrError         => "Keyword";

   EndOfString --
          '"'                               -> IsString
                                            -> Start                    => "String";
   IsString --
          '"'                               -> EndOfString
          accept any character other than
            the single quote mark as being
            part of the string              -> IsString;

   SingleSpace --
          ' '                               -> IsMultiSpace
                                            -> Start                    => "Space";

   IsMultiSpace --
          ' '                               -> IsMultiSpace
                                            -> Start                    => "MultiSpace";

   SingleTab --
          '\t'                              -> IsMultiTab
                                            -> Start                    => "Tab";

   IsMultiTab --
          ' '                               -> IsMultiTab
                                            -> Start                    => "MultiTab";

   Error --
                                            -> EndState                 => "ERROR";

   EndState --
                                                                        => "EOF";

   ----------------------------------------------------------------------------

   The scanner maintains a buffer.  Each time a character is used in a
   transition, it is placed into the buffer.  The buffer is cleared each
   time a transition to the Start state is made.  When the scanner reaches
   a state where it outputs a value (as indicated in the table), the output
   consists of two parts: the contents of the buffer, and a characterization
   of the contents of the buffer.  In the table above, only the characterization
   is shown in the output column.  In those cases where output occurs on a
   transition to the start state, the output takes place before the transition
   to the start state.  Each of the items "output" by the scanner is appended
   to a linked list, which is returned to the caller when scanning has been
   completed.  Thus, the scanner returns a linked list of tokens.                 */


#ifndef SCANNER_H

#define SCANNER_H 1

#include "token.h"
#include "dlist.h"

/*********************************************************************/
/*                                                                   */
/*   Function Name: GetToken                                         */
/*                                                                   */
/*   Descriptive Name: Returns a token derived from the source set   */
/*                     by the SetInput function.                     */
/*                                                                   */
/*   Input: None.                                                    */
/*                                                                   */
/*   Output: The function return value is a token.                   */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects: None.                                             */
/*                                                                   */
/*   Notes:                                                          */
/*                                                                   */
/*********************************************************************/
TokenType * GetToken(void);




/* This structure is used to track the replacement parameters which can be
   accessed using the $(#), where # is the entry in the Replacement_Parameters
   array.                                                                      */
typedef struct {
                 boolean     Is_Filename;
                 char *      Text;
               } Substitution_Table_Entry;

typedef struct {
                 uint                       count;
                 Substitution_Table_Entry * Replacement_Parameters;
               } Substitution_Table_Type;


/*********************************************************************/
/*                                                                   */
/*   Function Name: SetInput                                         */
/*                                                                   */
/*   Descriptive Name: Sets the input source for drawing characters  */
/*                     used to build tokens.                         */
/*                                                                   */
/*   Input: boolean IsFile - if TRUE, then the following parameter   */
/*                           is interpreted as the name of a file to */
/*                           be used for input.                      */
/*          char * FilenameOrString - If IsFile is TRUE, then this   */
/*                           is the name of the file to use for      */
/*                           input.  If IsFile is FALSE, then this   */
/*                           is a pointer to a buffer containing a   */
/*                           NULL terminated string which will be    */
/*                           used as input for building tokens.      */
/*          char ** Replacement_Parameter_Table - An array of string */
/*                           pointers where each entry in the array  */
/*                           points to a string which can be         */
/*                           referenced by the notation $(#).        */
/*                                                                   */
/*   Output: The function returns TRUE if it succeeded, FALSE        */
/*           otherwise.                                              */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:                                                   */
/*                                                                   */
/*   Notes:                                                          */
/*                                                                   */
/*********************************************************************/
boolean SetInput(boolean IsFile, char * FilenameOrString, Substitution_Table_Type * Replacement_Parameter_Table);

#endif

