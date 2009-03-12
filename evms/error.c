/*
 *
 *   (C) Copyright IBM Corp. 2001, 2004
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
 * Module: screener.c
 */

/*
 * Change History:
 *
 * 6/2001  B. Rafanello  Initial version.
 *
 */

/*
 *
 * This file will need to change in order to provide proper national
 * language support.  But, for development purposes, this will do.
 *
 */

/* Identify this file. */
#define ERROR_C      1

/*--------------------------------------------------
 * Necessary include files
 --------------------------------------------------*/

#define __USE_GNU
#include <stdio.h>
#include <string.h>		/* strerror */
#include "token.h"		/* TokenType, Token, MaxIdentifierLength */
#include "error.h"
#include "evms.h"
#include "dlist.h"
#include <errno.h>
//#include "../../Engine/handlemgr.h"

/*--------------------------------------------------
 * Private constants
 --------------------------------------------------*/
#define HANDLE_MANAGER_NOT_INITIALIZED   221
#define HANDLE_MANAGER_BAD_HANDLE        222


/*--------------------------------------------------
 * Private functions
 --------------------------------------------------*/



/*--------------------------------------------------
 * Public Functions
 --------------------------------------------------*/

void
Report_Scanner_Error(Scanner_Errors Error, char *Parameter1,
		     char *Parameter2, uint Column, uint Row)
{

    switch (Error) {
    case Bad_Command_File:
	My_Printf("Bad command file specified: %s\n", Parameter1);
	break;
    case Invalid_Character:
	My_Printf("The character %c is not valid in \"%s\"\n", *Parameter2,
		  Parameter1);
	break;
    case Invalid_Character_In_Substitution:
	My_Printf
	    ("The character %s is not valid in a substitution command.\n",
	     Parameter1);
	break;
    case Invalid_Substitution:
	My_Printf
	    ("An substitution command that is not valid was encountered.");
	break;
    case Scanner_Out_Of_Memory:
	My_Printf
	    ("Out of memory condition encountered.  Current Buffer: \"%s\"  Current Character: %c\n",
	     Parameter1, *Parameter2);
	break;
    default:
	My_Printf("Unknown error.\n");
    }

    return;

}

void
Report_Parser_Error(Parser_Errors Error, char *Filename, TokenType * Token)
{

    switch (Error) {
    case Engine_Open_Failure:
	My_Printf("Unable to open the EVMS Engine.\n");
	break;
    case SetInput_Failure:
	My_Printf("Unable to set the input source for parsing.\n");
    case Out_Of_Memory:
	My_Printf("Out of memory.\n");
	break;
    case Expecting_Command_Start:
	My_Printf("Expecting the start of a new command.\n");
	break;
    case Expecting_Command_Name:
	My_Printf("Expecting command name.\n");
	break;
    case Expecting_Command_Name_Or_File:
	My_Printf("Expecting a command name or a filename.\n");
	break;
    case Expecting_Command_File_Name:
	My_Printf("Expecting the name of a command file.\n");
	break;
    case Expecting_Command_Separator:
	My_Printf("Expecting the command separator.\n");
	break;
    case Expecting_Colon_Separator:
	My_Printf("Expecting a colon.\n");
	break;
    case Expecting_Comma_Separator:
	My_Printf("Expecting a comma.\n");
	break;
    case Expecting_Allocate_Or_Assign_Command:
	My_Printf
	    ("Expecting either the 'Allocate' or 'Assign' keyword.\n");
    case Expecting_Create_Or_Check_Command:
	My_Printf("Expecting the 'Create' or 'Check' commands.\n");
	break;
    case Expecting_Delete_Command:
	My_Printf("Expecting the 'Delete' command.\n");
	break;
    case Expecting_Format_Command:
	My_Printf("Expecting the 'Format' command.\n");
	break;
    case Expecting_Feature_Or_Segment_Keyword:
	My_Printf
	    ("Expecting either the 'Feature' or 'Segment' keyword.\n");
	break;
    case Expecting_Manager_Keyword:
	My_Printf("Expecting the 'Manager' keyword.\n");
	break;
    case Expecting_Filter_Identifier:
	My_Printf("Expecting the name of a Query Filter.\n"
		  "The following filters are available: Available, Container, Convertible, "
		  "Expandable, EQ, GT, LT, List Options, Object, Plugin, Region, Segment, "
		  "Shrinkable, Unclaimed, or Volume.\n");
	break;
    case Expecting_Expand_Command:
	My_Printf("Expecting the 'Expand' command.\n");
	break;
    case Expecting_Probe_Command:
	My_Printf("Expecting the 'Probe' command.\n");
	break;
    case Expecting_Shrink_Command:
	My_Printf("Expecting the 'Shrink' command.\n");
	break;
    case Expecting_Shrink_Parameters:
	My_Printf
	    ("Expecting the parameters required to shrink an object/volume.\n");
	break;
    case Expecting_Revert_Or_Rename:
	My_Printf
	    ("Expecting either the 'Remove', 'Revert', or 'Rename' keywords.\n");
	break;
    case Expecting_Help_Command:
	My_Printf("Expecting the 'Help' command.\n");
	break;
    case Expecting_Query_Command:
	My_Printf("Expecting the 'Query' command.\n");
	break;
    case Expecting_Creation_Parameters:
	My_Printf
	    ("Expecting 'Volume', 'Object', 'Container', or 'Region'.\n");
	break;
    case Expecting_Name_Or_Compatibility:
	My_Printf
	    ("Expecting the 'Compatibility' keyword or the 'Name' keyword.\n");
	break;
    case Expecting_Compatibility_Keyword:
	My_Printf("Expecting the 'Compatibility' keyword.\n");
	break;
    case Expecting_Volume_Parameter:
	My_Printf("Expecting the 'Volume' parameter.\n");
	break;
    case Expecting_Object_Parameter:
	My_Printf("Expecting the 'Object' parameter.\n");
	break;
    case Expecting_Container_Parameter:
	My_Printf("Expecting the 'Container' parameter.\n");
	break;
    case Expecting_Expand_Parameters:
	My_Printf("Expecting the parameters for an Expand operation .\n");
	break;
    case Expecting_Query_Parameters:
	My_Printf
	    ("Expecting either 'Plugins', 'Volumes', 'Regions', 'Objects', 'Containers', "
	     "'Disks', 'Segments', 'Acceptable', 'Children', 'Parent', 'Expand Points', "
	     "'Shrink Points', or 'Extended Info' keywords.\n");
	break;
    case Expecting_Query_Expand_Points_Keywords:
	My_Printf("Expecting the 'Expand Points' keywords.\n");
	break;
    case Expecting_Query_Shrink_Points_Keywords:
	My_Printf("Expecting the 'Shrink Points' keywords.\n");
	break;
    case Expecting_Plugins_Or_Parent_Parameter:
	My_Printf("Expecting the 'Plugins' or 'Parent' keyword.\n");
	break;
    case Expecting_Volumes_Parameter:
	My_Printf("Expecting the 'Volumes' keyword.\n");
	break;
    case Expecting_Regions_Parameter:
	My_Printf("Expecting the 'Regions' keyword.\n");
	break;
    case Expecting_Disks_Parameter:
	My_Printf("Expecting the 'Disks' keyword.\n");
	break;
    case Expecting_Segments_Parameter:
	My_Printf("Expecting the 'Segments' keyword.\n");
	break;
    case Expecting_Objects_Parameter:
	My_Printf("Expecting the 'Objects' keyword.\n");
	break;
    case Expecting_Acceptable_Parameter:
	My_Printf("Expecting the 'Acceptable' keyword.\n");
	break;
    case Expecting_List_Options_Keywords:
	My_Printf("Expecting the 'List Options' keywords.\n");
	break;
    case Expecting_Volume_Or_Object:
	My_Printf("Expecting a volume or object to be specified.\n");
	break;
    case Expecting_Containers_Or_Children_Parameter:
	My_Printf("Expecting the 'Containers' or 'Children' keywords.\n");
	break;
    case Expecting_Help_Parameters:
	My_Printf("Expecting the topic to provide help information on.\n");
	break;
    case Expecting_Plugin_Type_Parameter:
	My_Printf
	    ("Expecting either 'Feature', 'Region Manager', 'Segment Manager', 'Device Manager', "
	     "'Filesystem Interface Module', 'Distributed Lock Manager', or 'Cluster Management'.\n");
	break;
    case Expecting_Feature_Identifier:
	My_Printf("Expecting the identifier of a feature.\n");
	break;
    case Expecting_Plugin_Identifier:
	My_Printf("Expecting the identifier of a plugin module.\n");
	break;
    case Expecting_Object_Identifier:
	My_Printf("Expecting the identifier of a storage object.\n");
	break;
    case Expecting_Region_Or_Object:
	My_Printf("Expecting the identifier of a region or object.\n");
	break;
    case Expecting_Offset:
	My_Printf("Expecting an offset to be specified.\n");
	break;
    case Expecting_Start_KeyWord:
	My_Printf("Expecting the 'Start' keyword.\n");
	break;
    case Expecting_End_KeyWord:
	My_Printf("Expecting the 'End' keyword.\n");
	break;
    case Expecting_Offset_Parameters:
	My_Printf
	    ("Expecting either 'Start', 'End', or a numeric offset.\n");
	break;
    case Expecting_Size_Parameters:
	My_Printf("Expecting a numeric value.\n");
	break;
    case Expecting_Size_Value:
	My_Printf("Expecting a numeric value.\n");
	break;
    case Expecting_EQ_Keyword:
	My_Printf("Expecting the 'EQ' keyword.\n");
	break;
    case Expecting_LT_Keyword:
	My_Printf("Expecting the 'LT' keyword.\n");
	break;
    case Expecting_GT_Keyword:
	My_Printf("Expecting the 'GT' keyword.\n");
	break;
    case Expecting_GT_LT_Parameter:
	My_Printf("Expecting either the 'GT' or 'LT' keywords.\n");
	break;
    case Expecting_Units_Parameter:
	My_Printf
	    ("Expecting a unit of measure.  Supported units of measure are:\n"
	     "     For disks: Kilobytes (KB), Megabytes (MB), Gigabytes (GB), Terrabytes (TB), Sectors.\n"
	     "     For time: Hours (h), Minutes (min), Seconds (s), Milliseconds (ms), Microseconds (us)\n");
	break;
    case Expecting_Disk_Units_Parameter:
	My_Printf
	    ("Expecting a unit of measure.  Supported units of measure are:\n"
	     "     For disks: Kilobytes (KB), Megabytes (MB), Gigabytes (GB), Terrabytes (TB), Sectors.\n");
	break;
    case Expecting_Number_Or_String:
	My_Printf("Expecting either a numeric value or a string.\n");
	break;
    case Expecting_Number:
	My_Printf("Expecting a numeric value.\n");
	break;
    case Expecting_Name:	/* Name = "string" */
	My_Printf("Expecting a name.\n");
	break;
    case Expecting_Field_Name:
	My_Printf("Expecting an extended info field name.\n");
	break;
    case Expecting_Name_Value:	/* Name portion of Name Value Pair */
	My_Printf("Expecting the name portion of a Name Value pair.\n");
	break;
    case Expecting_Equal_Sign_Separator:
	My_Printf("Expecting an equal sign.\n");
	break;
    case Expecting_Open_Brace_Separator:
	My_Printf("Expecting an open brace.\n");
	break;
    case Expecting_Closing_Brace_Separator:
	My_Printf("Expecting a closing brace.\n");
	break;
    case Expecting_Open_Paren_Separator:
	My_Printf("Expecting an open parenthesis.\n");
	break;
    case Expecting_Closing_Paren_Separator:
	My_Printf("Expecting a closing parenthesis.\n");
	break;
    case Expecting_Name_Value_Pair:
	My_Printf("Expecting a name-value pair.\n");
	break;
    case Expecting_Query_Volume_Parameters:
	My_Printf("Expecting the parameters for a volume query.\n");
	break;
    case Expecting_Query_Plugin_Parameters:
	My_Printf("Expecting the parameters for a plugin query.\n");
	break;
    case Expecting_Query_Region_Parameters:
	My_Printf("Expecting the parameters for a region query.\n");
	break;
    case Expecting_Query_Acceptable_Parameters:
	My_Printf("Expecting the parameters for an Acceptable query.\n");
	break;
    case Expecting_Query_Acceptable_Create:
	My_Printf
	    ("Expecting the type of item to create: 'Volume', 'EVMS Object', 'Region', 'Segment', or 'Container'.\n");
	break;
    case Expecting_Region_Creation_Parameters:
	My_Printf("Expecting the parameters for a region creation.\n");
	break;
    case Expecting_Object_List:
	My_Printf("Expecting a list of storage objects or regions.\n");
	break;
    case Expecting_Query_Disks_Parameters:
	My_Printf("Expecting the parameters for a disk query.\n");
	break;
    case Expecting_Volume_Identifier:
	My_Printf("Expecting the identifier of a volume.\n");
	break;
    case Expecting_Region_Identifier:
	My_Printf("Expecting the identifier of a region.\n");
	break;
    case Expecting_Container_Identifier:
	My_Printf("Expecting the identifier of a container.\n");
	break;
    case Expecting_Unclaimed_Identifier:
	My_Printf("Expecting the 'Unclaimed' keyword.\n");
	break;
    case Expecting_Disk_Identifier:
	My_Printf("Expecting the identifier of a disk.\n");
	break;
    case Expecting_Expandable_Shrinkable_Identifier:
	My_Printf
	    ("Expecting the 'Expandable' or 'Shrinkable' keywords.\n");
	break;
    case Expecting_Query_Extended_Info_Keywords:
	My_Printf("Expecting the 'Extended Info' keywords.\n");
	break;
    case Expecting_Freespace_Identifier:
	My_Printf("Expecting the 'Freespace' keyword.\n");
	break;
    case Expecting_EVMS_Identifier:
	My_Printf
	    ("Expecting the identifier for a plug-in, volume, EVMS object, region, container, segment, or disk.\n");
    case Internal_Error_Encountered:
	My_Printf("An internal error occurred.\n");
	break;
    case Bad_Command_File_Name:
	My_Printf("An invalid command file was specified.\n");
	break;
    case Wrong_Filter:
	My_Printf("The specified filter is not allowed.\n");
	break;
    case Duplicate_Filter:
	My_Printf
	    ("A filter has been specified more than once for the current Query.\n");
	break;
    case Expecting_Commit_Parameters:
	My_Printf("Expecting the 'Hold' or 'Release' keywords.\n");
	break;
    default:
	My_Printf("Unknown Error.\n");
	break;
    }

    if (Filename != NULL)
	My_Printf("Command File being processed: %s\n", Filename);

    if (Token != NULL) {
	My_Printf("Current Token: %s\n", Token->TokenText);
	My_Printf("Token found at column %u, line %u.\n",
		  Token->TokenColumn, Token->TokenRow);
    }

    return;

}


void Report_Standard_Error(int Error)
{
    char *Error_Text;		/* Used with strerror */

    if (Error < 0)
	Error = -Error;

    switch (Error) {
    case INVALID_OPTION:
	My_Printf("One of the options specified was not correct.\n");
	break;
    case INTERNAL_ERROR_BOD:
	My_Printf
	    ("Internal error: Bad Option Descriptor returned by EVMS Engine.\n");
	break;
    case EINVAL:
	My_Printf("A command line parameter was not valid.\n");
	break;
    case ENOENT:
	My_Printf("The specified item could not be found.\n");
	break;
    case DLIST_CORRUPTED:
	My_Printf("Internal error: Corrupted dlist_t.\n");
	break;
    case DLIST_BAD:
	My_Printf("Internal error: Bad dlist_t.\n");
	break;
    case DLIST_NOT_INITIALIZED:
	My_Printf("Internal error: Uninitialized dlist_t.\n");
	break;
    case DLIST_EMPTY:
	My_Printf("Internal error: Empty dlist_t.\n");
	break;
    case DLIST_ITEM_SIZE_WRONG:
	My_Printf
	    ("Internal error: Item size does not match dlist_t entry.\n");
	break;
    case DLIST_BAD_ITEM_POINTER:
	My_Printf("Internal error: Bad item pointer for dlist_t.\n");
	break;
    case DLIST_ITEM_SIZE_ZERO:
	My_Printf
	    ("Internal error: Items of size 0 not allowed in DLISTs.\n");
	break;
    case DLIST_ITEM_TAG_WRONG:
	My_Printf
	    ("Internal error: Item tag does not agree with what is in the dlist_t.\n");
	break;
    case DLIST_END_OF_LIST:
	My_Printf
	    ("Internal error: End of dlist_t reached earlier than expected.\n");
	break;
    case DLIST_ALREADY_AT_START:
	My_Printf("Internal error: Already at start of dlist_t.\n");
	break;
    case DLIST_BAD_HANDLE:
	My_Printf
	    ("Internal error: Bad handle for an item in a dlist_t.\n");
	break;
    case DLIST_INVALID_INSERTION_MODE:
	My_Printf
	    ("Internal error: Invalid insertion mode for a dlist_t.\n");
	break;
    case HANDLE_MANAGER_NOT_INITIALIZED:
	My_Printf
	    ("Internal error: The handle manager has not been initialized yet and a handle has been requested.\n");
	break;
    case HANDLE_MANAGER_BAD_HANDLE:
	My_Printf("The specified handle is invalid.\n");
	break;
    case INVALID_TASK:
	My_Printf
	    ("The task specified either does not exist or is invalid at this time.\n");
	break;
    default:
	Error_Text = strerror(Error);
	if (Error_Text == NULL) {
	    My_Printf("Unknown error.\n");
	} else {
	    My_Printf("%s\n", Error_Text);
	}
	break;
    }

    return;
}
