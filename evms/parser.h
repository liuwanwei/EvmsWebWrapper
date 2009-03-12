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
 * Module: parser.h
 */
 
/*
 * Change History:
 *
 * 6/2001  B. Rafanello  Initial version.
 *
 */


#ifndef PARSER_H

#define PARSER_H 1

#include "dlist.h"
#include "interpreter.h"


/*********************************************************************/
/*                                                                   */
/*   Function Name: Parse_Command_Line                               */
/*                                                                   */
/*   Descriptive Name: Parses a stream of tokens provided by the     */
/*                     scanner and screener and produces a list of   */
/*                     trees representing the commands found in the  */
/*                     stream of tokens.  The trees are constructed  */
/*                     of nodes of type Executable_Node and are      */
/*                     suitable for use with the Interpret_Commands  */
/*                     function provided by interpreter.c.           */
/*                                                                   */
/*   Input: None.                                                    */
/*                                                                   */
/*   Output: The function return value is a list of trees where each */
/*           tree represents one command found in the input stream.  */
/*                                                                   */
/*   Error Handling: Parsing errors are reported immediately to the  */
/*                   user using the facilities provided in error.h.  */
/*                   If an error is encountered, then the function   */
/*                   will return a NULL value for the list.          */
/*                                                                   */
/*   Side Effects: Tokens are read from the input stream.  Tokens are*/
/*                 disposed of after use.  Error messages may be     */
/*                 output to the user.                               */
/*                                                                   */
/*   Notes: The SetInput function of the scanner must be used prior  */
/*          to calling this function to establish the source of      */
/*          input for scanning, screening, and parsing.              */
/*                                                                   */
/*********************************************************************/                                             
#ifdef Old_Style

dlist_t Parse_Command_Line(char * Command_Line_To_Parse);

#else

dlist_t Parse_Command_Line(void);

#endif
             
#endif             
