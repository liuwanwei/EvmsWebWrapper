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
 * Module: interpreter.h
 */

/*
 * Change History:
 *
 * 6/2001  B. Rafanello  Initial version.
 *
 */


#ifndef INTERPRETER_H
#define INTERPRETER_H 1

#ifdef __cplusplus
extern "C"
{
#endif

#include "dlist.h"
#include <frontend.h>


/* Define a typecode for use with the dlist module. */
#define STACK_NODE  94393920

/* The interpreter takes in a tree consisting of nodes as defined below and executes them, returning the result. */
typedef enum {
               StringData,
               IntegerData,
               RealNumberData,
               Units_Node,
               Precision_Node,
               Activate_Node,
               Add_Feature_Node,
               Allocate_Node,
               Assign_Node,
               Check_Node,
               Commit_Node,
               Convert_Node,
               Create_Node,
               Deactivate_Node,
               Delete_Node,
               Echo_Node,
               Expand_Node,
               Format_Node,
               Unformat_Node,
               Shrink_Node,
               Set_Node,
               Remove_Node,
               Rename_Node,
               Replace_Node,
               Revert_Node,
               Query_Node,
               Query_Node_Type_2,
               Query_Extended_Info,
               Help_Node,
               Task_Adjust,
               Task_Node,
               Mount_Node,
               Unmount_Node,
               Topmost_Objects,
               Filter,
               Freespace_Size_Filter,
               Feature_ID,
               Feature_Name,
               Plugin_ID,
               Plugin_Name,
               Plugin_Mask,
               Probe_Node,
               Quit_Node,
               Name_Value_Pair,
               ObjectList,
               Name_To_Volume_Handle,
               Name_To_Object_Handle,
               Name_To_Region_Handle,
               Name_To_Container_Handle,
               Name_To_Segment_Handle,
               Name_To_Disk_Handle,
               Name_To_Plugin_Handle,
               Translate_Name_To_Handle
             } NodeTypes;

typedef struct {
                 NodeTypes  NodeType;
                 void *     NodeValue;
                 dlist_t    Children;
               } Executable_Node;


#ifndef DEBUG_PARSER

/*--------------------------------------------------
 * Public Global Variables
 --------------------------------------------------*/

#ifdef INTERPRETER_C

int       Verbose_Mode = 0;    /* 0 = no verbose, 1 = EVMS Engine messages reported one per line, 2 = EVMS Engine messages overlayed on top of each other. */
boolean   No_Commits = FALSE;  /* If TRUE, then don't perform commits after each command. */

#else

extern int     Verbose_Mode;
extern boolean No_Commits;

#endif

#define TIME_TO_QUIT  -2

/*********************************************************************/
/*                                                                   */
/*   Function Name: Interpret_Commands                               */
/*                                                                   */
/*   Descriptive Name: Takes a dlist_t of commands, where each       */
/*                     command is represented as a tree of           */
/*                     Executable_Node entities, and attempts to     */
/*                     interpret them, thereby executing the command */
/*                     they represent.                               */
/*                                                                   */
/*   Input: dlist_t Commands - A list of tree structures, where each */
/*                           tree structure is composed of nodes of  */
/*                           type Executable_Node.  Each tree        */
/*                           represents a command to execute.        */
/*          Interactive - If TRUE, then we are not processing a      */
/*                        command file and the Probe command (which  */
/*                        translates to an evms_rediscover() call)   */
/*                        and other commands not allowed in command  */
/*                        files are allowed.                         */
/*          Mode  - The mode in which the Engine is opened.          */
/*          Debug - The debug level to use as specified by the user. */
/*          Node_Name - The cluster nodename to administer. NULL if  */
/*                      local.                                       */
/*                                                                   */
/*   Output: The function return value is 0 if no errors occurred,   */
/*           unless a QUIT command is encountered, in which case     */
/*           TIME_TO_QUIT is returned.  If an error occurs, a        */
/*           non-zero error code will be returned.                   */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects: The state of the EVMS engine may be altered by the*/
/*                 commands being interpreted by this function.  This*/
/*                 may lead to modifications of the volumes in the   */
/*                 system.                                           */
/*                                                                   */
/*   Notes:                                                          */
/*                                                                   */
/*********************************************************************/
int Interpret_Commands(dlist_t Commands, boolean Interactive, engine_mode_t Engine_Mode, debug_level_t Debug, char * Node_Name);

/*********************************************************************/
/*                                                                   */
/*   Function Name: Commit_Changes                                   */
/*                                                                   */
/*   Descriptive Name: If any changes have been made but not yet     */
/*                     written to disk, then this function will      */
/*                     cause them to be written to disk.  This is a  */
/*                     wrapper for the evms_commit_changes function. */
/*                                                                   */
/*   Input: None.                                                    */
/*                                                                   */
/*   Output: The function return value is 0 if no errors occurred,   */
/*           a non-zero return value otherwise.  This function will  */
/*           print the status messages returned by the EVMS Engine   */
/*           during the commit process.                              */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects: This function will cause the EVMS engine to write */
/*                 any pending changes to disk.                      */
/*                                                                   */
/*   Notes:                                                          */
/*                                                                   */
/*********************************************************************/
int Commit_Changes(void);

void Report_Progress(char * Message);

#endif

#ifdef __cplusplus
}
#endif

#endif
