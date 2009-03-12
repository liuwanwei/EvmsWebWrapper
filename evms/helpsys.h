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
 * Module: helpsys.h
 */

/*
 * Change History:
 *
 * 6/2001  B. Rafanello  Initial version.
 *
 */


#ifndef HELPSYS_H

#define HELPSYS_H 1

#include "interpreter.h"


#ifndef DEBUG_PARSER

/*********************************************************************/
/*                                                                   */
/*   Function Name: Display_Help                                     */
/*                                                                   */
/*   Descriptive Name: Displays the help text for a given command.   */
/*                                                                   */
/*   Input: Executable_Node * Current_Node - This should be the help */
/*                            node from the interpreter.  It will    */
/*                            indicate which specific command, if    */
/*                            any, help should be provided for.      */
/*                                                                   */
/*   Output: Text is displayed to the user.                          */
/*                                                                   */
/*   Error Handling: There should be no errors.                      */
/*                                                                   */
/*   Side Effects: None.                                             */
/*                                                                   */
/*   Notes:                                                          */
/*                                                                   */
/*********************************************************************/
void Display_Help(Executable_Node * Current_Node);

#endif

#endif

