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
 * Module: interpreter.c
 */

/*
 * Change History:
 *
 * 7/2001  B. Rafanello  Initial version.
 *
 */

/*
 *
 */

/* Identify this file. */
#define INTERPRETER_C

#ifndef DEBUG_PARSER

/*--------------------------------------------------
 * Necessary include files
 --------------------------------------------------*/

#include <stdlib.h>		/* malloc */
#include <stdio.h>
#include <limits.h>
#include <ctype.h>		/* toupper */
#include <assert.h>
#include <string.h>
#include <termios.h>
#include <sys/ioctl.h>
#include "dlist.h"
#include "evms.h"
#include "error.h"
#include "frontend.h"
#include "interpreter.h"
#include "screener.h"
#include "helpsys.h"

#include "../Protocol.h"
#include "ObjectStructs.h"
#include "PrivateData.h"

extern ui_callbacks_t Callback_Functions;	/* Used to pass call back functions to the EVMS Engine.  These
						   functions allow the EVMS Engine to provide status and error messages
						   for display to the user during complicated operations.               */

/*--------------------------------------------------
 * Private constants
 --------------------------------------------------*/
#define STRING_POINTER   20294390
#define INTEGER_VALUE    38928921
#define REAL_NUMBER      93482372
#define LIST             77834894
#define HANDLE           19827435
#define HANDLE_INFO      81342783
#define PLUGIN_ID_MASK   18923789
#define BOOLEAN_VALUE    54938587

/*--------------------------------------------------
 * Private types
 --------------------------------------------------*/
typedef enum {
    GT,
    LT,
    EQ,
} Size_Filter_Types;

typedef struct {
    Size_Filter_Types Filter_Type;
    u_int64_t Upper_Limit;
    u_int64_t Lower_Limit;
    boolean Use_Freespace;
} Size_Filter_Structure;


/*--------------------------------------------------
 * Private variables
 --------------------------------------------------*/
static dlist_t Stack = NULL;
static boolean Interactive_Mode = TRUE;
static engine_mode_t Engine_Mode = ENGINE_READWRITE;
static debug_level_t Debug_Level = -1;
static char *Node_Name = NULL;
static task_handle_t Task = 0;
static task_action_t Task_Action;
static option_descriptor_t **Option_Descriptors;
static int Option_Count;
static boolean *Option_Usage;
static handle_array_t *Object_Array;
static value_unit_t Current_Units;
static int Last_Message_Length = 0;
/* Hack for Freespace filter. */
static boolean Freespace_Not_Size = FALSE;
/* Hack for List Plug-in Options filter. */
static boolean List_Options = FALSE;
/* Hack for Get Acceptable filter. */
static boolean Get_Acceptable = FALSE;
static boolean Use_Selected_Objects_Filter;
static char *ByteStr = "bytes";
static char *PetabyteStr = "PB";


/*--------------------------------------------------
 * Private Function Prototypes
 --------------------------------------------------*/
static void Check_For_Error(int Error);
static void Pop(TAG Item_Type, uint Item_Size, ADDRESS Buffer);
static void Push(TAG Item_Type, uint Item_Size, ADDRESS Item);
static void Peek(TAG * Item_Type, uint * Item_Size);
static int Process_Node(ADDRESS Object,
			TAG ObjectTag,
			ADDRESS ObjectHandle, ADDRESS Parameters);
static int Print_Object_Info(ADDRESS Object,
			     TAG ObjectTag,
			     ADDRESS ObjectHandle, ADDRESS Parameters);
static int Do_Revert(Executable_Node * Current_Node);
static int Do_Create(Executable_Node * Current_Node);
static int Do_Assign(Executable_Node * Current_Node);
static int Do_Activate(Executable_Node * Current_Node);
static int Do_Deactivate(Executable_Node * Current_Node);
static int Do_Add_Feature(Executable_Node * Current_Node);
static int Do_Format(Executable_Node * Current_Node);
static int Do_Allocation(Executable_Node * Current_Node);
static int Do_Remove(Executable_Node * Current_Node);
static int Do_Echo(Executable_Node * Current_Node);
static int Do_Delete(Executable_Node * Current_Node);
static int Do_Unformat(Executable_Node * Current_Node);
static int Do_Convert(Executable_Node * Current_Node);
static int Do_Rename(Executable_Node * Current_Node);
static int Do_Replace(Executable_Node * Current_Node);
static int Do_Expand(Executable_Node * Current_Node);
static int Do_Set(Executable_Node * Current_Node);
static int Do_Check(Executable_Node * Current_Node);
static int Do_Shrink(Executable_Node * Current_Node);
static int Do_Task(Executable_Node * Current_Node);
static int Do_Mount(Executable_Node * Current_Node);
static int Do_Unmount(Executable_Node * Current_Node);
static int Do_Help(Executable_Node * Current_Node);
static int Do_Translate_Name_To_Handle(Executable_Node * Current_Node);
static int Do_Name_To_Volume_Handle(Executable_Node * Current_Node);
static int Do_Name_To_Plugin_Handle(Executable_Node * Current_Node);
static int Do_Name_To_Object_Handle(Executable_Node * Current_Node);
static int Do_Name_To_Region_Handle(Executable_Node * Current_Node);
static int Do_Name_To_Container_Handle(Executable_Node * Current_Node);
static int Do_Name_To_Segment_Handle(Executable_Node * Current_Node);
static int Do_Name_To_Disk_Handle(Executable_Node * Current_Node);
static int Do_Plugin_Mask(Executable_Node * Current_Node);
static int Do_Query_Type_2(Executable_Node * Current_Node);
static int Do_Query(Executable_Node * Current_Node);
static int Do_Query_Acceptable_Expand_Filter(Executable_Node *
					     Current_Node);
static int Do_Query_Extended_Info(Executable_Node * Current_Node);
static int Do_Task_Adjust(Executable_Node * Current_Node);
static int Do_Topmost_Objects(Executable_Node * Current_Node);
static int Do_Probe(Executable_Node * Current_Node);
static int Do_Volume_Filter(Executable_Node * Current_Node);
static int Do_Convertible_Filter(Executable_Node * Current_Node);
static int Do_Object_Filter(Executable_Node * Current_Node);
static int Do_Plugin_Filter(Executable_Node * Current_Node);
static int Do_Container_Filter(Executable_Node * Current_Node);
static int Do_Region_Filter(Executable_Node * Current_Node);
static int Do_Plugin_Type_Filter(Executable_Node * Current_Node);
static int Do_Disk_Filter(Executable_Node * Current_Node);
static int Do_Segment_Filter(Executable_Node * Current_Node);
static int Do_GT_Filter(Executable_Node * Current_Node);
static int Do_LT_Filter(Executable_Node * Current_Node);
static int Do_EQ_Filter(Executable_Node * Current_Node);
static int Do_Expand_Filter(Executable_Node * Current_Node);
static int Do_Shrink_Filter(Executable_Node * Current_Node);
static int Do_Query_Acceptable_Create_Filter(Executable_Node *
					     Current_Node);
static int Do_Unclaimed_Filter(Executable_Node * Current_Node);
static int Do_Freespace_Filter(Executable_Node * Current_Node);
static int Do_Freespace_Size_Filter(Executable_Node * Current_Node);
static int Do_List_Options_Filter(Executable_Node * Current_Node);
static int Do_Precision(Executable_Node * Current_Node);
static int Do_Unit_Conversion(Executable_Node * Current_Node);
static int Do_IntegerData(Executable_Node * Current_Node);
static int Do_RealNumber(Executable_Node * Current_Node);
static int Do_Feature_Name(Executable_Node * Current_Node);
static int Do_Feature(Executable_Node * Current_Node,
		      object_handle_t Handle, boolean Forward);
static int Do_Feature_ID(Executable_Node * Current_Node);
static int Do_Plugin_ID(Executable_Node * Current_Node);
static int Do_ObjectList(Executable_Node * Current_Node);
static int Do_Name_Value_Pair(Executable_Node * Current_Node);
static boolean Scan_For_Volumes(ADDRESS Object,
				TAG ObjectTag,
				ADDRESS ObjectHandle,
				ADDRESS Parameters,
				boolean * FreeMemory, uint * Error);
static boolean Scan_For_Convertible(ADDRESS Object,
				    TAG ObjectTag,
				    ADDRESS ObjectHandle,
				    ADDRESS Parameters,
				    boolean * FreeMemory, uint * Error);
static boolean Scan_For_Plugin(ADDRESS Object,
			       TAG ObjectTag,
			       ADDRESS ObjectHandle,
			       ADDRESS Parameters,
			       boolean * FreeMemory, uint * Error);
static boolean Scan_For_Containers(ADDRESS Object,
				   TAG ObjectTag,
				   ADDRESS ObjectHandle,
				   ADDRESS Parameters,
				   boolean * FreeMemory, uint * Error);
static boolean Scan_For_Objects(ADDRESS Object,
				TAG ObjectTag,
				ADDRESS ObjectHandle,
				ADDRESS Parameters,
				boolean * FreeMemory, uint * Error);
static boolean Scan_For_Segments(ADDRESS Object,
				 TAG ObjectTag,
				 ADDRESS ObjectHandle,
				 ADDRESS Parameters,
				 boolean * FreeMemory, uint * Error);
static boolean Scan_For_Selected_Objects(ADDRESS Object,
					 TAG ObjectTag,
					 ADDRESS ObjectHandle,
					 ADDRESS Parameters,
					 boolean * FreeMemory,
					 uint * Error);
static boolean Scan_For_Regions(ADDRESS Object, TAG ObjectTag,
				ADDRESS ObjectHandle, ADDRESS Parameters,
				boolean * FreeMemory, uint * Error);
static boolean Scan_For_Plugins(ADDRESS Object, TAG ObjectTag,
				ADDRESS ObjectHandle, ADDRESS Parameters,
				boolean * FreeMemory, uint * Error);
static boolean Scan_For_Disks(ADDRESS Object, TAG ObjectTag,
			      ADDRESS ObjectHandle, ADDRESS Parameters,
			      boolean * FreeMemory, uint * Error);
static boolean Scan_For_Size(ADDRESS Object, TAG ObjectTag,
			     ADDRESS ObjectHandle, ADDRESS Parameters,
			     boolean * FreeMemory, uint * Error);
static boolean Scan_For_Unclaimed(ADDRESS Object, TAG ObjectTag,
				  ADDRESS ObjectHandle, ADDRESS Parameters,
				  boolean * FreeMemory, uint * Error);
static boolean Scan_For_Freespace(ADDRESS Object, TAG ObjectTag,
				  ADDRESS ObjectHandle, ADDRESS Parameters,
				  boolean * FreeMemory, uint * Error);
static boolean Scan_For_Expandable(ADDRESS Object, TAG ObjectTag,
				   ADDRESS ObjectHandle,
				   ADDRESS Parameters,
				   boolean * FreeMemory, uint * Error);
static boolean Scan_For_Shrinkable(ADDRESS Object, TAG ObjectTag,
				   ADDRESS ObjectHandle,
				   ADDRESS Parameters,
				   boolean * FreeMemory, uint * Error);
static boolean Kill_Nodes(ADDRESS Object, TAG ObjectTag,
			  ADDRESS ObjectHandle, ADDRESS Parameters,
			  boolean * FreeMemory, uint * Error);
static boolean Check_Child_For_Disk(object_handle_t Handle,
				    object_handle_t Disk_Handle,
				    uint * Error);
static boolean Check_Child_For_Region(object_handle_t Handle,
				      object_handle_t Region_Handle,
				      uint * Error);
static boolean Check_Child_For_Segment(object_handle_t Handle,
				       object_handle_t Segment_Handle,
				       uint * Error);
static boolean Check_Child_For_Container(object_handle_t Handle,
					 object_handle_t Container_Handle,
					 uint * Error);
static int Get_Option_Descriptors(void);
static void Clear_Option_Descriptors(void);
static double Convert_Real(u_int32_t Index, double value);
static int64_t Convert_Int(u_int32_t Index, int64_t value);
static void Display_Option_Descriptors(boolean Print_Default_Value);
static int Do_Object_And_Options(Executable_Node * Current_Node);
static int Setup_Task(plugin_handle_t Handle, task_action_t Task_Type,
		      boolean Report_Errors);
static int Cleanup_Task(void);
static void Print_Extended_Info(extended_info_t * Info);
static void Print_Units(value_unit_t Units);
static void Print_Value(u_int Indent,
			boolean No_Conversions,
			value_type_t Value_Type,
			value_unit_t Units,
			value_format_t Suggested_Format, value_t * Value);
static void Print_Expand_Shrink_Move_Slide_Info(task_action_t Start,
						task_action_t End,
						char *Object_Name,
						object_handle_t
						Object_Handle,
						my_advance_info_t * advance);
static void Display_Plugin_Tasks(object_handle_t Handle, my_advance_info_t * advance);
static char *Determine_Units(u_int64_t Value, double *Result);	/* Input value is in bytes. */


/*--------------------------------------------------
 * Public Functions Available
 --------------------------------------------------*/

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
int Commit_Changes(void)
{

    int Return_Value;

    if (Verbose_Mode == 0) {
	/* Don't output EVMS Engine Commit Status Messages. */
	Callback_Functions.status = NULL;
	return evms_commit_changes();
    }

    /* We are in verbose mode 1 or 2. */

    My_Printf("All pending changes are now being committed to disk.\n");

    Callback_Functions.status = Report_Progress;
    Return_Value = evms_commit_changes();

    My_Printf("\n");

    Last_Message_Length = 0;

    return Return_Value;
}


/*********************************************************************/
/*                                                                   */
/*   Function Name: Interpret_Commands                               */
/*                                                                   */
/*   Descriptive Name: Takes a dlist_t of commands, where each command */
/*                     is represented as a tree of Executable_Node   */
/*                     entities, and attempts to interpret them,     */
/*                     thereby executing the command they represent. */
/*                                                                   */
/*   Input: dlist_t Commands - A list of tree structures, where each */
/*                           tree structure is composed of nodes of  */
/*                           type Executable_Node.  Each tree        */
/*                           represents a command to execute.        */
/*          Interactive - If TRUE, then we are not processing a      */
/*                        command file. Commands not allowed in      */
/*                        command files are allowed.                 */
/*          Mode  - The mode in which the Engine is opened.          */
/*          Debug - The debug level to use as specified by the user. */
/*          Name - The cluster nodename to administer. NULL if local.*/
/*                                                                   */
/*   Output: The function return value is 0 if no errors occurred,   */
/*           a non-zero return value otherwise.                      */
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
int
Interpret_Commands(dlist_t Commands, boolean Interactive,
		   engine_mode_t Mode, debug_level_t Debug, char *Name)
{
    int Error;
    int Cleanup_Error;

    Interactive_Mode = Interactive;
    Engine_Mode = Mode;
    Debug_Level = Debug;
    Node_Name = Name;
    Get_Acceptable = FALSE;
    Freespace_Not_Size = FALSE;
    List_Options = FALSE;


    Error = ForEachItem(Commands, Process_Node, NULL, TRUE);

    /* Free up the memory used to represent the commands we just finished interpreting. */
    Cleanup_Error = PruneList(Commands, Kill_Nodes, NULL);
    if (!Cleanup_Error)
	Cleanup_Error = DestroyList(&Commands, TRUE);

    if ((!Error) && Cleanup_Error)
	Error = Cleanup_Error;

    return Error;
}

/*
 *  Added a typedef that allows the push/pop/peek routines to
 *  be able to save the size of the item that is being inserted
 *  and removed from the dlist.  This was necessary especially in
 *  Peek() ... who is expected to return the Item_Size and Item_Tag
 *  values ... though the function description doesn't say so.
 */
typedef struct dlist_node_s {
    u_int32_t item_size;
    void *object;
} dlist_node_t;



/*--------------------------------------------------
 * Private Functions
 --------------------------------------------------*/
static void Check_For_Error(int Error)
{
    /* Assume that Error == 0 means success, i.e. no error. */
    if (Error) {
	Report_Standard_Error(Error);
	abort();
    }

    return;
}

static void Pop(TAG Item_Type, uint Item_Size, ADDRESS Buffer)
{
    int Error;
    dlist_node_t *node;

    Error = GoToEndOfList(Stack);
    Check_For_Error(Error);

    Error = ExtractObject(Stack, Item_Type, NULL, (void *) &node);
    Check_For_Error(Error);

    if (!node) {
	Report_Standard_Error(DLIST_BAD_ITEM_POINTER);
	abort();
    }

    memcpy(Buffer, node->object, node->item_size);
    free(node->object);
    free(node);

    return;
}

static void Push(TAG Item_Type, uint Item_Size, ADDRESS Item)
{

    ADDRESS NotNeeded;
    ADDRESS Object;
    int Error;
    dlist_node_t *node;

    /* Has the stack been created yet? */
    if (Stack == NULL) {
	Stack = CreateList();
	if (Stack == NULL) {
	    Report_Standard_Error(ENOMEM);
	    exit(ENOMEM);
	}
    }

    node = malloc(sizeof(dlist_node_t));
    if (node == NULL) {
	Report_Standard_Error(ENOMEM);
	exit(ENOMEM);
    }

    Object = malloc(Item_Size);
    if (Object == NULL) {
	free(node);
	Report_Standard_Error(ENOMEM);
	exit(ENOMEM);
    }

    memcpy(Object, Item, Item_Size);

    node->item_size = Item_Size;
    node->object = Object;

    Error =
	InsertObject(Stack, node, Item_Type, NULL, AppendToList, TRUE,
		     &NotNeeded);
    Check_For_Error(Error);

    return;
}

static void Peek(TAG * Item_Type, uint * Item_Size)
{
    int Error;
    dlist_node_t *node;

    Error = GoToEndOfList(Stack);
    Check_For_Error(Error);

    Error = BlindGetObject(Stack, Item_Type, NULL, FALSE, (void *) &node);
    Check_For_Error(Error);

    if (!node) {
	Report_Standard_Error(DLIST_BAD_ITEM_POINTER);
	abort();
    }

    *Item_Size = node->item_size;

    return;
}

static boolean
Kill_Nodes(ADDRESS Object,
	   TAG ObjectTag,
	   ADDRESS ObjectHandle,
	   ADDRESS Parameters, boolean * FreeMemory, uint * Error)
{
    Executable_Node *Current_Node = (Executable_Node *) Object;

    /* Sanity Check */
    if (ObjectTag != STACK_NODE) {
	Report_Standard_Error(DLIST_ITEM_TAG_WRONG);
	abort();
    }

    /* Assume success. */
    *Error = DLIST_SUCCESS;

    /* Free the node and it node value, as appropriate. */
    switch (Current_Node->NodeType) {
    case StringData:
    case IntegerData:
    case RealNumberData:
    case Name_To_Volume_Handle:
    case Name_To_Object_Handle:
    case Name_To_Region_Handle:
    case Name_To_Container_Handle:
    case Name_To_Segment_Handle:
    case Name_To_Disk_Handle:
    case Name_To_Plugin_Handle:
    case Translate_Name_To_Handle:
	free(Current_Node->NodeValue);
	break;
    default:
	if ((Current_Node->NodeValue != NULL)
	    && (strlen((char *) Current_Node->NodeValue) == 1))
	    free(Current_Node->NodeValue);
	break;

    }

    /* Now delete any children. */
    if (Current_Node->Children != NULL)
	*Error = PruneList(Current_Node->Children, Kill_Nodes, NULL);

    /* Have DLIST delete the node for us. */
    *FreeMemory = TRUE;

    return TRUE;
}


static int
Process_Node(ADDRESS Object,
	     TAG ObjectTag, ADDRESS ObjectHandle, ADDRESS Parameters)
{
    Executable_Node *Current_Node = (Executable_Node *) Object;
    int Error = 0;

    /* Sanity Check */
    if (ObjectTag != STACK_NODE) {
	Report_Standard_Error(DLIST_ITEM_TAG_WRONG);
	abort();
    }

    /* Call the appropriate function to process the current node. */
    switch (Current_Node->NodeType) {
    case StringData:
	Push(STRING_POINTER, sizeof(char *), &(Current_Node->NodeValue));
	break;
    case IntegerData:
	Error = Do_IntegerData(Current_Node);
	break;
    case RealNumberData:
	Error = Do_RealNumber(Current_Node);
	break;
    case Units_Node:
	Error = Do_Unit_Conversion(Current_Node);
	break;
    case Precision_Node:
	Error = Do_Precision(Current_Node);
	break;
    case Echo_Node:
	Error = Do_Echo(Current_Node);
	break;
    case Create_Node:
	Error = Do_Create(Current_Node);
	if ((!Error) && (!No_Commits)) {
	    Error = Commit_Changes();
	    if (Error)
		Report_Standard_Error(Error);
	}
	break;
    case Activate_Node:
	Error = Do_Activate(Current_Node);
	if ((!Error) && (!No_Commits)) {
	    Error = Commit_Changes();
	    if (Error)
		Report_Standard_Error(Error);
	}
	break;
    case Deactivate_Node:
	Error = Do_Deactivate(Current_Node);
	if ((!Error) && (!No_Commits)) {
	    Error = Commit_Changes();
	    if (Error)
		Report_Standard_Error(Error);
	}
	break;
    case Check_Node:
	Error = Do_Check(Current_Node);
	if ((!Error) && (!No_Commits)) {
	    Error = Commit_Changes();
	    if (Error)
		Report_Standard_Error(Error);
	}
	break;
    case Commit_Node:
	if (Current_Node->NodeValue == HoldStr) {
	    No_Commits = TRUE;
	} else {

	    if (Current_Node->NodeValue == ReleaseStr)
		No_Commits = FALSE;

	    Error = Commit_Changes();
	    if (Error)
		Report_Standard_Error(Error);
	}
	break;
    case Allocate_Node:
	Error = Do_Allocation(Current_Node);
	if ((!Error) && (!No_Commits)) {
	    Error = Commit_Changes();
	    if (Error)
		Report_Standard_Error(Error);
	}
	break;
    case Format_Node:
	Error = Do_Format(Current_Node);
	if ((!Error) && (!No_Commits)) {
	    Error = Commit_Changes();
	    if (Error)
		Report_Standard_Error(Error);
	}
	break;
    case Assign_Node:
	Error = Do_Assign(Current_Node);
	if ((!Error) && (!No_Commits)) {
	    Error = Commit_Changes();
	    if (Error)
		Report_Standard_Error(Error);
	}
	break;
    case Remove_Node:
	Error = Do_Remove(Current_Node);
	if ((!Error) && (!No_Commits)) {
	    Error = Commit_Changes();
	    if (Error)
		Report_Standard_Error(Error);
	}
	break;
    case Delete_Node:
	Error = Do_Delete(Current_Node);
	if ((!Error) && (!No_Commits)) {
	    Error = Commit_Changes();
	    if (Error)
		Report_Standard_Error(Error);
	}
	break;
    case Unformat_Node:
	Error = Do_Unformat(Current_Node);
	if ((!Error) && (!No_Commits)) {
	    Error = Commit_Changes();
	    if (Error)
		Report_Standard_Error(Error);
	}
	break;
    case Expand_Node:
	Error = Do_Expand(Current_Node);
	if ((!Error) && (!No_Commits)) {
	    Error = Commit_Changes();
	    if (Error)
		Report_Standard_Error(Error);
	}
	break;
    case Set_Node:
	Error = Do_Set(Current_Node);
	if ((!Error) && (!No_Commits)) {
	    Error = Commit_Changes();
	    if (Error)
		Report_Standard_Error(Error);
	}
	break;
    case Shrink_Node:
	Error = Do_Shrink(Current_Node);
	if ((!Error) && (!No_Commits)) {
	    Error = Commit_Changes();
	    if (Error)
		Report_Standard_Error(Error);
	}
	break;
    case Rename_Node:
	Error = Do_Rename(Current_Node);
	if ((!Error) && (!No_Commits)) {
	    Error = Commit_Changes();
	    if (Error)
		Report_Standard_Error(Error);
	}
	break;
    case Replace_Node:
	Error = Do_Replace(Current_Node);
	if ((!Error) && (!No_Commits)) {
	    Error = Commit_Changes();
	    if (Error)
		Report_Standard_Error(Error);
	}
	break;
    case Revert_Node:
	Error = Do_Revert(Current_Node);
	if ((!Error) && (!No_Commits)) {
	    Error = Commit_Changes();
	    if (Error)
		Report_Standard_Error(Error);
	}
	break;
    case Task_Node:
	Error = Do_Task(Current_Node);
	if ((!Error) && (!No_Commits)) {
	    Error = Commit_Changes();
	    if (Error)
		Report_Standard_Error(Error);
	}
	break;
    case Mount_Node:
	Error = Do_Mount(Current_Node);
	if ((!Error) && (!No_Commits)) {
	    Error = Commit_Changes();
	    if (Error)
		Report_Standard_Error(Error);
	}
	break;
    case Unmount_Node:
	Error = Do_Unmount(Current_Node);
	if ((!Error) && (!No_Commits)) {
	    Error = Commit_Changes();
	    if (Error)
		Report_Standard_Error(Error);
	}
	break;
    case Query_Node:
	Error = Do_Query(Current_Node);
	break;
    case Query_Node_Type_2:
	Error = Do_Query_Type_2(Current_Node);
	break;
    case Query_Extended_Info:
	Error = Do_Query_Extended_Info(Current_Node);
	break;
    case Quit_Node:
	Error = TIME_TO_QUIT;
	break;
    case Task_Adjust:
	Error = Do_Task_Adjust(Current_Node);
	break;
    case Topmost_Objects:
	Error = Do_Topmost_Objects(Current_Node);
	break;
    case Probe_Node:
	Error = Do_Probe(Current_Node);
	break;
    case Help_Node:
	Error = Do_Help(Current_Node);
	break;
    case Convert_Node:
	Error = Do_Convert(Current_Node);
	if ((!Error) && (!No_Commits)) {
	    Error = Commit_Changes();
	    if (Error)
		Report_Standard_Error(Error);
	}
	break;
    case Add_Feature_Node:
	Error = Do_Add_Feature(Current_Node);
	if ((!Error) && (!No_Commits)) {
	    Error = Commit_Changes();
	    if (Error)
		Report_Standard_Error(Error);
	}
	break;
    case Filter:
	if (Current_Node->NodeValue == VolumeStr)
	    Error = Do_Volume_Filter(Current_Node);
	else if (Current_Node->NodeValue == ObjectStr)
	    Error = Do_Object_Filter(Current_Node);
	else if (Current_Node->NodeValue == ContainerStr)
	    Error = Do_Container_Filter(Current_Node);
	else if (Current_Node->NodeValue == RegionStr)
	    Error = Do_Region_Filter(Current_Node);
	else if (Current_Node->NodeValue == TypeStr)
	    Error = Do_Plugin_Type_Filter(Current_Node);
	else if (Current_Node->NodeValue == PluginStr)
	    Error = Do_Plugin_Filter(Current_Node);
	else if (Current_Node->NodeValue == DiskStr)
	    Error = Do_Disk_Filter(Current_Node);
	else if (Current_Node->NodeValue == SegmentStr)
	    Error = Do_Segment_Filter(Current_Node);
	else if (Current_Node->NodeValue == GTStr)
	    Error = Do_GT_Filter(Current_Node);
	else if (Current_Node->NodeValue == LTStr)
	    Error = Do_LT_Filter(Current_Node);
	else if (Current_Node->NodeValue == EQStr)
	    Error = Do_EQ_Filter(Current_Node);
	else if (Current_Node->NodeValue == ExpandableStr)
	    Error = Do_Expand_Filter(Current_Node);
	else if (Current_Node->NodeValue == ExpandStr)
	    Error = Do_Query_Acceptable_Expand_Filter(Current_Node);
	else if (Current_Node->NodeValue == ShrinkStr)
	    Error = Do_Shrink_Filter(Current_Node);
	else if (Current_Node->NodeValue == CreateStr)
	    Error = Do_Query_Acceptable_Create_Filter(Current_Node);
	else if ((Current_Node->NodeValue == UnclaimedStr) ||
		 (Current_Node->NodeValue == AvailableStr))
	    Error = Do_Unclaimed_Filter(Current_Node);
	else if (Current_Node->NodeValue == FreespaceStr)
	    Error = Do_Freespace_Filter(Current_Node);
	else if (Current_Node->NodeValue == LOStr)
	    Error = Do_List_Options_Filter(Current_Node);
	else if (Current_Node->NodeValue == ConvertibleStr)
	    Error = Do_Convertible_Filter(Current_Node);
	else {
	    My_Printf("Internal error: Unknown filter encountered.\n");
	    Error = EINVAL;
	}
	break;
    case Freespace_Size_Filter:
	Error = Do_Freespace_Size_Filter(Current_Node);
	break;
    case Feature_ID:
	Error = Do_Feature_ID(Current_Node);
	break;
    case Plugin_ID:
	Error = Do_Plugin_ID(Current_Node);
	break;
    case Feature_Name:
    case Plugin_Name:
	Error = Do_Feature_Name(Current_Node);
	break;
    case Plugin_Mask:
	Error = Do_Plugin_Mask(Current_Node);
	break;
    case Name_Value_Pair:
	Error = Do_Name_Value_Pair(Current_Node);
	break;
    case ObjectList:
	Error = Do_ObjectList(Current_Node);
	break;
    case Name_To_Volume_Handle:
	Error = Do_Name_To_Volume_Handle(Current_Node);
	break;
    case Name_To_Object_Handle:
	Error = Do_Name_To_Object_Handle(Current_Node);
	break;
    case Name_To_Region_Handle:
	Error = Do_Name_To_Region_Handle(Current_Node);
	break;
    case Name_To_Container_Handle:
	Error = Do_Name_To_Container_Handle(Current_Node);
	break;
    case Name_To_Segment_Handle:
	Error = Do_Name_To_Segment_Handle(Current_Node);
	break;
    case Name_To_Disk_Handle:
	Error = Do_Name_To_Disk_Handle(Current_Node);
	break;
    case Name_To_Plugin_Handle:
	Error = Do_Name_To_Plugin_Handle(Current_Node);
	break;
    case Translate_Name_To_Handle:
	Error = Do_Translate_Name_To_Handle(Current_Node);
	break;
    default:
	break;

    }

    return Error;

}


static int Do_Allocation(Executable_Node * Current_Node)
{
    Executable_Node *Child_Node;
    int Error = 0;
    object_handle_t Handle;
    handle_object_info_t *Object_Info;

    /* We need to get the handle of the plug-in to use.  This means that we need to get the
       evms object info for the object that the user specified.  In the evms object info
       will be the handle of the plug-in that controls that object.                          */

    Error = GoToStartOfList(Current_Node->Children);
    Check_For_Error(Error);

    Error =
	GetObject(Current_Node->Children, STACK_NODE, NULL, FALSE,
		  (ADDRESS) & Child_Node);
    Check_For_Error(Error);

    /* The object list for this ObjectList node should have a single entry.  We need that entry. */
    Error = GoToStartOfList(Child_Node->Children);
    Check_For_Error(Error);

    Error =
	GetObject(Child_Node->Children, STACK_NODE, NULL, FALSE,
		  (ADDRESS) & Child_Node);
    Check_For_Error(Error);

    /* Now we should have the node we need.  If we process the node using Process_Node, then we should
       find the handle of the object we need on the stack.                                             */
    Error = Process_Node(Child_Node, STACK_NODE, NULL, NULL);
    if (!Error) {

	/* Now get the handle from the stack. */
	Pop(HANDLE, sizeof(object_handle_t), &Handle);

	/* Now get the info on the object. */
	Error = evms_get_info(Handle, &Object_Info);

	/* Is the object specified of the proper type? */
	if (Error ||
	    ((Object_Info->type != REGION)
	     && (Object_Info->type != SEGMENT))) {
	    My_Printf
		("The object specified for use with the Allocate command is incorrect.\n");

	    if (!Error)
		evms_free(Object_Info);

	    return EINVAL;
	}

	/* Now we can use the handle of the plug-in that controls this object. */
	Task_Action = EVMS_Task_Create;
	if (Object_Info->type == REGION)
	    Error =
		Do_Feature(Current_Node, Object_Info->info.region.plugin,
			   TRUE);
	else
	    Error =
		Do_Feature(Current_Node, Object_Info->info.segment.plugin,
			   TRUE);

	evms_free(Object_Info);
    }

    return Error;
}


static int Do_Format(Executable_Node * Current_Node)
{
    int Error = 0;

    /* Set the Task_Action variable so that our children will know what kind of task this is. */
    Task_Action = EVMS_Task_mkfs;

    /* Process our children. */
    Error = ForEachItem(Current_Node->Children, Process_Node, NULL, TRUE);

    return Error;

}


static int Do_Set(Executable_Node * Current_Node)
{

    /* Set the task type. */
    Task_Action = EVMS_Task_Set_Info;

    return Do_Object_And_Options(Current_Node);
}


static int Do_Check(Executable_Node * Current_Node)
{
    /* Set the task type. */
    Task_Action = EVMS_Task_fsck;

    return Do_Object_And_Options(Current_Node);

}


static int Do_Object_And_Options(Executable_Node * Current_Node)
{
    Executable_Node *Child_Node;
    int Error = 0;
    object_handle_t Handle;

    /* We need to get the handle of the object to expand. */
    Error = GoToStartOfList(Current_Node->Children);
    Check_For_Error(Error);

    Error =
	ExtractObject(Current_Node->Children, STACK_NODE, NULL,
		      (ADDRESS) & Child_Node);
    Check_For_Error(Error);

    /* Now we should have the node we need.  If we process the node using Process_Node, then we should
       find the handle of the object we need on the stack.                                             */
    Error = Process_Node(Child_Node, STACK_NODE, NULL, NULL);
    if (!Error) {

	/* Now get the handle from the stack. */
	Pop(HANDLE, sizeof(object_handle_t), &Handle);

	Error = Do_Feature(Current_Node, Handle, FALSE);

    }

    return Error;
}


static int Do_Expand(Executable_Node * Current_Node)
{
    Executable_Node *Child_Node;
    int Error = 0;
    object_handle_t Handle;
    object_type_t Handle_Type;

    /* We need to get the handle of the object to expand. */
    Error = GoToStartOfList(Current_Node->Children);
    Check_For_Error(Error);

    Error =
	ExtractObject(Current_Node->Children, STACK_NODE, NULL,
		      (ADDRESS) & Child_Node);
    Check_For_Error(Error);

    /* Now we should have the node we need.  If we process the node using Process_Node, then we should
       find the handle of the object we need on the stack.                                             */
    Error = Process_Node(Child_Node, STACK_NODE, NULL, NULL);
    if (!Error) {

	/* Now get the handle from the stack. */
	Pop(HANDLE, sizeof(object_handle_t), &Handle);

	/* Set the task type. */

	/* If Handle is the handle of a container, then we use the EVMS_Task_Expand_Container.
	   For other objects, we use EVMS_Task_Expand.                                         */
	Error = evms_get_handle_object_type(Handle, &Handle_Type);
	if (!Error) {

	    Task_Action = EVMS_Task_Expand;

	    Error = Do_Feature(Current_Node, Handle, FALSE);
	}

    }

    return Error;
}


static int Do_Shrink(Executable_Node * Current_Node)
{
    Executable_Node *Child_Node;
    int Error = 0;
    object_handle_t Handle;
    handle_object_info_t *Handle_Info = NULL;

    /* We need to get the handle of the object to shrink. */
    Error = GoToStartOfList(Current_Node->Children);
    Check_For_Error(Error);

    Error =
	ExtractObject(Current_Node->Children, STACK_NODE, NULL,
		      (ADDRESS) & Child_Node);
    Check_For_Error(Error);

    /* Now we should have the node we need.  If we process the node using Process_Node, then we should
       find the handle of the object we need on the stack.                                             */
    Error = Process_Node(Child_Node, STACK_NODE, NULL, NULL);
    if (!Error) {

	/* Now get the handle from the stack. */
	Pop(HANDLE, sizeof(object_handle_t), &Handle);

	/* If Handle is the handle of a container, then we use the EVMS_Task_Expand_Container.
	   For other objects, we use EVMS_Task_Expand.                                         */
	Error = evms_get_info(Handle, &Handle_Info);
	if (!Error) {

	    /* Set the task type. */
	    Task_Action = EVMS_Task_Shrink;

	    Error = Do_Feature(Current_Node, Handle, FALSE);

	}

    }

    if (Handle_Info != NULL)
	evms_free(Handle_Info);

    return Error;

}


static int Do_Task(Executable_Node * Current_Node)
{
    Executable_Node *Child_Node;
    int Error = 0;
    int Index;
    char *Task_Name = NULL;
    object_handle_t Handle;
    function_info_array_t *Available_Tasks;

    /* We need to get the task name and the handle of the target object. */
    Error = GoToStartOfList(Current_Node->Children);
    Check_For_Error(Error);

    /* Get the task name. */
    Error =
	ExtractObject(Current_Node->Children, STACK_NODE, NULL,
		      (ADDRESS) & Child_Node);
    Check_For_Error(Error);

    /* Now we should have the node we need.  If we process the node using Process_Node, then we should
       find a pointer to the task name on the stack.                                                    */
    Error = Process_Node(Child_Node, STACK_NODE, NULL, NULL);
    if (!Error) {

	/* Now get the task name from the stack. */
	Pop(STRING_POINTER, sizeof(char *), &Task_Name);

	/* Get the name of the target object. */
	Error =
	    ExtractObject(Current_Node->Children, STACK_NODE, NULL,
			  (ADDRESS) & Child_Node);
	Check_For_Error(Error);

	/* Now we should have the node we need.  If we process the node using Process_Node, then we should
	   find the handle of the object we need on the stack.                                             */
	Error = Process_Node(Child_Node, STACK_NODE, NULL, NULL);
	if (!Error) {

	    /* Now get the handle from the stack. */
	    Pop(HANDLE, sizeof(object_handle_t), &Handle);

	    /* Set the task type. */

	    /* We need to get the available tasks for the target object. */
	    Error = evms_get_plugin_functions(Handle, &Available_Tasks);
	    if (!Error) {

		for (Index = 0; Index < Available_Tasks->count; Index++) {

		    if (strcmp
			(Available_Tasks->info[Index].name,
			 Task_Name) == 0) {
			/* We have identified the task to use. */
			Task_Action =
			    Available_Tasks->info[Index].function;

			/* Perform the task. */
			Error = Do_Feature(Current_Node, Handle, FALSE);

			/* exit the for loop. */
			break;

		    }

		}

		/* Did we find a match? */
		if (Index >= Available_Tasks->count) {
		    /* An invalid task was specified. */
		    Report_Standard_Error(INVALID_TASK);
		}

	    }

	}

    }

    return Error;
}


static int Do_Mount(Executable_Node * Current_Node)
{
    Executable_Node *Child_Node;
    int Error = 0;
    char *Mount_Point = NULL;
    char *Mount_Options = NULL;
    object_handle_t Handle;

    /* We need to get the volume handle and the mount point. */
    Error = GoToStartOfList(Current_Node->Children);
    Check_For_Error(Error);

    /* Get the volume handle. */
    Error =
	ExtractObject(Current_Node->Children, STACK_NODE, NULL,
		      (ADDRESS) & Child_Node);
    Check_For_Error(Error);

    /* Now we should have the node we need.  If we process the node using
       Process_Node, then we should find the volume handle on the stack. */
    Error = Process_Node(Child_Node, STACK_NODE, NULL, NULL);
    if (!Error) {

	/* Now get the volume handle from the stack. */
	Pop(HANDLE, sizeof(object_handle_t), &Handle);

	/* Get the mount point. */
	Error =
	    ExtractObject(Current_Node->Children, STACK_NODE, NULL,
			  (ADDRESS) & Child_Node);
	Check_For_Error(Error);

	/* Now we should have the node we need.  If we process the node using
	   Process_Node, then we should find the pointer to the mount point on the
	   stack. */
	Error = Process_Node(Child_Node, STACK_NODE, NULL, NULL);
	if (!Error) {

	    /* Now get the mount point from the stack. */
	    Pop(STRING_POINTER, sizeof(char *), &Mount_Point);

	    /* Get the mount options if they were given. */
	    Error =
		ExtractObject(Current_Node->Children, STACK_NODE, NULL,
			      (ADDRESS) & Child_Node);
	    if (Error != DLIST_EMPTY) {

		Check_For_Error(Error);

		/* Now we should have the node we need.  If we process the node using
		   Process_Node, then we should find the pointer to the mount options
		   on the stack. */
		Error = Process_Node(Child_Node, STACK_NODE, NULL, NULL);
		if (!Error) {

		    /* Now get the mount options from the stack. */
		    Pop(STRING_POINTER, sizeof(char *), &Mount_Options);
		}

	    } else {
		Error = DLIST_SUCCESS;
	    }

	    if (!Error) {
		Error = evms_mount(Handle, Mount_Point, Mount_Options);
	    }

	}

    }

    return Error;
}


static int Do_Unmount(Executable_Node * Current_Node)
{
    Executable_Node *Child_Node;
    int Error = 0;
    object_handle_t Handle;

    /* We need to get the volume handle. */
    Error = GoToStartOfList(Current_Node->Children);
    Check_For_Error(Error);

    /* Get the volume handle. */
    Error =
	ExtractObject(Current_Node->Children, STACK_NODE, NULL,
		      (ADDRESS) & Child_Node);
    Check_For_Error(Error);

    /* Now we should have the node we need.  If we process the node using
       Process_Node, then we should find the volume handle on the stack. */
    Error = Process_Node(Child_Node, STACK_NODE, NULL, NULL);
    if (!Error) {

	/* Now get the volume handle from the stack. */
	Pop(HANDLE, sizeof(object_handle_t), &Handle);

	Error = evms_unmount(Handle);
    }

    return Error;
}


static double Convert_Real(u_int32_t Index, double value)
{

    if ((Current_Units == EVMS_Unit_None) ||
	(Current_Units == Option_Descriptors[Index]->unit))
	return value;

    if (Current_Units == EVMS_Unit_Bytes) {
	switch (Option_Descriptors[Index]->unit) {
	case EVMS_Unit_Bytes:
	    return value;
	    break;
	case EVMS_Unit_Kilobytes:
	    return (value / 1024.0);
	    break;
	case EVMS_Unit_Megabytes:
	    return (value / 1048576.0);
	    break;
	case EVMS_Unit_Gigabytes:
	    return (value / 1073741824.0);
	    break;
	case EVMS_Unit_Terabytes:
	    return (value / (1073741824.0 * 1024.0));
	    break;
	case EVMS_Unit_Petabytes:
	    return (value / (1073741824.0 * 1024.0 * 1024.0));
	    break;
	default:
	    My_Printf
		("Option Error:  The units specified for the %s option are incorrect.\n",
		 Option_Descriptors[Index]->name);
	    exit(EINVAL);
	    break;
	}

    } else if (Current_Units == EVMS_Unit_Microseconds) {
	switch (Option_Descriptors[Index]->unit) {
	case EVMS_Unit_Milliseconds:
	    return (value / 1000.0);
	    break;
	case EVMS_Unit_Microseconds:
	    return value;
	    break;
	default:
	    My_Printf
		("Fatal Error:  The units specified for the %s option are incorrect.\n",
		 Option_Descriptors[Index]->name);
	    exit(EINVAL);
	    break;
	}

    }

    My_Printf("Internal error: Value for Current_Units is not valid.\n");
    abort();

    return EPERM;

}


static int64_t Convert_Int(u_int32_t Index, int64_t value)
{
    if ((Current_Units == EVMS_Unit_None) ||
	(Current_Units == Option_Descriptors[Index]->unit))
	return value;

    if (Current_Units == EVMS_Unit_Bytes) {
	switch (Option_Descriptors[Index]->unit) {
	case EVMS_Unit_Bytes:
	    return value;
	    break;
	case EVMS_Unit_Kilobytes:
	    return (value / 1024);
	    break;
	case EVMS_Unit_Megabytes:
	    return (value / 1048576);
	    break;
	case EVMS_Unit_Gigabytes:
	    return (value / 1073741824LL);
	    break;
	case EVMS_Unit_Terabytes:
	    return (value / (1073741824LL * 1024LL));
	    break;
	case EVMS_Unit_Petabytes:
	    return (value / (1073741824LL * 1024LL * 1024LL));
	    break;
	case EVMS_Unit_Sectors:
	    return (value / 512);
	    break;
	default:
	    My_Printf
		("Option Error: The units specified for the %s option are incorrect.\n",
		 Option_Descriptors[Index]->name);
	    exit(EINVAL);
	    break;
	}

    } else if (Current_Units == EVMS_Unit_Bytes) {
	switch (Option_Descriptors[Index]->unit) {
	case EVMS_Unit_Milliseconds:
	    return (value / 1000);
	    break;
	case EVMS_Unit_Microseconds:
	    return value;
	    break;
	default:
	    My_Printf
		("Option Error: The units specified for the %s option are incorrect.\n",
		 Option_Descriptors[Index]->name);
	    exit(EINVAL);
	    break;
	}

    }

    My_Printf("Internal error: Value for Current_Units is not valid.\n");
    abort();

    return EPERM;
}


static int Do_Name_Value_Pair(Executable_Node * Current_Node)
{
    int Error;
    char *Option_Name;
    char *String_Value;
    u_int32_t String_Length;
    int64_t Big_Integer;
    double Big_Real;
    u_int32_t i;
    u_int32_t j;
    u_int32_t Option_Index;
    TAG Item_Tag;
    uint Item_Size;
    uint List_Size;
    boolean Value_Is_List = FALSE;
    value_list_t *Value_List = NULL;
    value_t Value;
    task_effect_t effect;
    boolean Option_Found;

    /* Find out how many children we have.  One child will be the option name, the rest will be the
       values for the list we need to produce.                                                      */
    Error = GetListSize(Current_Node->Children, &List_Size);
    Check_For_Error(Error);

    /* Clear the Current_Units field.  It will be set by our children if the user specified a unit for the value. */
    Current_Units = EVMS_Unit_None;

    /* Process our children in reverse order so that we can get what we want on the stack
       in a more convenient order.                                                         */
    Error = ForEachItem(Current_Node->Children, Process_Node, NULL, FALSE);
    if (!Error) {

	/* The top item on the stack should be the name of the option to set. */
	Pop(STRING_POINTER, sizeof(char *), &Option_Name);

	/* Now we must find the option specified by the user. */
	Option_Found = FALSE;
	for (Option_Index = 0;
	     (Option_Index < Option_Count) && (!Option_Found);
	     Option_Index++) {
	    if (strcasecmp
		(Option_Name, Option_Descriptors[Option_Index]->name)
		== 0) {
		Option_Found = TRUE;
		break;
	    }

	}


	if (Option_Found) {
	    /* We have a match!  Lets see what type of value this option expects. */

	    /* Is a list expected? */
	    if (Option_Descriptors[Option_Index]->
		flags & EVMS_OPTION_FLAGS_VALUE_IS_LIST) {
		Value_Is_List = TRUE;

		/* The number of items in the list will be one less than the number of children we have. */
		List_Size--;

	    } else {

		/* Since no list is expected, the number of children must be two. */
		if (List_Size != 2) {
		    My_Printf("Too many values specified for option %s.\n",
			      Option_Name);
		    return EINVAL;
		}

		/* We will be returning one value for this option. */
		List_Size = 1;
	    }

	    /* Allocate memory. */
	    Value_List =
		(value_list_t *) malloc(sizeof(value_list_t) +
					(List_Size * sizeof(value_t)));
	    Value_List->count = List_Size;

	    /* Initialize Value. */
	    Value.list = Value_List;

	    switch (Option_Descriptors[Option_Index]->type) {
	    case EVMS_Type_String:	/* char*     */
		for (i = 0; i < List_Size; i++) {
		    Peek(&Item_Tag, &Item_Size);

		    if (Item_Tag != STRING_POINTER) {
			My_Printf
			    ("Improper value specified for option %s.\n",
			     Option_Name);
			Error = EINVAL;
			break;
		    }

		    Pop(STRING_POINTER, Item_Size, &String_Value);

		    Value_List->value[i].s = String_Value;
		}
		break;
	    case EVMS_Type_Boolean:	/* boolean   */
		for (i = 0; i < List_Size; i++) {
		    Peek(&Item_Tag, &Item_Size);

		    if (Item_Tag != STRING_POINTER) {
			My_Printf
			    ("Improper value specified for option %s.\n",
			     Option_Name);
			Error = EINVAL;
			break;
		    }

		    Pop(STRING_POINTER, Item_Size, &String_Value);

		    /* Convert String_Value to upper case. */
		    String_Length = strlen(String_Value);
		    for (j = 0; j < String_Length; j++)
			String_Value[j] = toupper(String_Value[j]);

		    if ((strcmp("TRUE", String_Value) == 0) ||
			((String_Length == 1) && (String_Value[0] == 'T')))
		    {
			Value_List->value[i].b = TRUE;
		    } else if ((strcmp("FALSE", String_Value) == 0) ||
			       ((String_Length == 1)
				&& (String_Value[0] == 'F'))) {
			Value_List->value[i].b = FALSE;
		    } else {
			My_Printf
			    ("Improper value specified for option %s.\n",
			     Option_Name);
			Error = EINVAL;
			break;
		    }

		}
		break;
	    case EVMS_Type_Char:	/* char      */
	    case EVMS_Type_Unsigned_Char:	/* unsigned char */
		for (i = 0; i < List_Size; i++) {
		    Peek(&Item_Tag, &Item_Size);

		    if (Item_Tag != STRING_POINTER) {
			My_Printf
			    ("Improper value specified for option %s.\n",
			     Option_Name);
			Error = EINVAL;
			break;
		    }

		    Pop(STRING_POINTER, Item_Size, &String_Value);

		    String_Length = strlen(String_Value);
		    if (String_Length != 1) {
			My_Printf
			    ("Improper value specified for option %s.\n",
			     Option_Name);
			Error = EINVAL;
			break;
		    }

		    if (Option_Descriptors[Option_Index]->type ==
			EVMS_Type_Char)
			Value_List->value[i].c = String_Value[0];
		    else
			Value_List->value[i].uc = String_Value[0];

		}
		break;
	    case EVMS_Type_Real32:	/* float     */
		for (i = 0; i < List_Size; i++) {
		    Peek(&Item_Tag, &Item_Size);

		    if (Item_Tag == STRING_POINTER) {
			My_Printf
			    ("Improper value specified for option %s.\n",
			     Option_Name);
			Error = EINVAL;
			break;
		    }

		    if (Item_Tag == INTEGER_VALUE) {
			Pop(INTEGER_VALUE, Item_Size, &Big_Integer);
			Big_Real = (double) Big_Integer;
		    } else
			Pop(REAL_NUMBER, Item_Size, &Big_Real);

		    Value_List->value[i].r32 =
			(float) Convert_Real(Option_Index, Big_Real);
		}
		break;
	    case EVMS_Type_Real64:	/* double    */
		for (i = 0; i < List_Size; i++) {
		    Peek(&Item_Tag, &Item_Size);

		    if (Item_Tag == STRING_POINTER) {
			My_Printf
			    ("Improper value specified for option %s.\n",
			     Option_Name);
			Error = EINVAL;
			break;
		    }

		    if (Item_Tag == INTEGER_VALUE) {
			Pop(INTEGER_VALUE, Item_Size, &Big_Integer);
			Big_Real = (double) Big_Integer;
		    } else
			Pop(REAL_NUMBER, Item_Size, &Big_Real);


		    Value_List->value[i].r64 =
			Convert_Real(Option_Index, Big_Real);
		}
		break;
	    case EVMS_Type_Int:	/* int       */
		for (i = 0; i < List_Size; i++) {
		    Peek(&Item_Tag, &Item_Size);

		    if (Item_Tag == STRING_POINTER) {
			My_Printf
			    ("Improper value specified for option %s.\n",
			     Option_Name);
			Error = EINVAL;
			break;
		    }

		    if (Item_Tag == REAL_NUMBER) {
			Pop(REAL_NUMBER, Item_Size, &Big_Real);
			Big_Integer = (int64_t) Big_Real;
		    } else
			Pop(INTEGER_VALUE, Item_Size, &Big_Integer);

		    /* Convert to the proper units for this option. */
		    Big_Integer = Convert_Int(Option_Index, Big_Integer);

		    if ((Big_Integer > INT_MAX) || (Big_Integer < INT_MIN)) {
			My_Printf
			    ("Improper value specified for option %s.\n",
			     Option_Name);
			Error = EINVAL;
			break;
		    }

		    Value_List->value[i].i = (int) Big_Integer;
		}
		break;
	    case EVMS_Type_Int8:	/* int8_t    */
		for (i = 0; i < List_Size; i++) {
		    Peek(&Item_Tag, &Item_Size);

		    if (Item_Tag == STRING_POINTER) {
			My_Printf
			    ("Improper value specified for option %s.\n",
			     Option_Name);
			Error = EINVAL;
			break;
		    }

		    if (Item_Tag == REAL_NUMBER) {
			Pop(REAL_NUMBER, Item_Size, &Big_Real);
			Big_Integer = (int64_t) Big_Real;
		    } else
			Pop(INTEGER_VALUE, Item_Size, &Big_Integer);

		    /* Convert to the proper units for this option. */
		    Big_Integer = Convert_Int(Option_Index, Big_Integer);

		    if ((Big_Integer > 127) || (Big_Integer < -128)) {
			My_Printf
			    ("Improper value specified for option %s.\n",
			     Option_Name);
			Error = EINVAL;
			break;
		    }

		    Value_List->value[i].i8 = (int8_t) Big_Integer;
		}
		break;
	    case EVMS_Type_Int16:	/* int16_t   */
		for (i = 0; i < List_Size; i++) {
		    Peek(&Item_Tag, &Item_Size);

		    if (Item_Tag == STRING_POINTER) {
			My_Printf
			    ("Improper value specified for option %s.\n",
			     Option_Name);
			Error = EINVAL;
			break;
		    }

		    if (Item_Tag == REAL_NUMBER) {
			Pop(REAL_NUMBER, Item_Size, &Big_Real);
			Big_Integer = (int64_t) Big_Real;
		    } else
			Pop(INTEGER_VALUE, Item_Size, &Big_Integer);

		    /* Convert to the proper units for this option. */
		    Big_Integer = Convert_Int(Option_Index, Big_Integer);

		    if ((Big_Integer > 32767) || (Big_Integer < -32768)) {
			My_Printf
			    ("Improper value specified for option %s.\n",
			     Option_Name);
			Error = EINVAL;
			break;
		    }

		    Value_List->value[i].i16 = (int16_t) Big_Integer;
		}
		break;
	    case EVMS_Type_Int32:	/* int32_t   */
		for (i = 0; i < List_Size; i++) {
		    Peek(&Item_Tag, &Item_Size);

		    if (Item_Tag == STRING_POINTER) {
			My_Printf
			    ("Improper value specified for option %s.\n",
			     Option_Name);
			Error = EINVAL;
			break;
		    }

		    if (Item_Tag == REAL_NUMBER) {
			Pop(REAL_NUMBER, Item_Size, &Big_Real);
			Big_Integer = (int64_t) Big_Real;
		    } else
			Pop(INTEGER_VALUE, Item_Size, &Big_Integer);

		    /* Convert to the proper units for this option. */
		    Big_Integer = Convert_Int(Option_Index, Big_Integer);

		    if ((Big_Integer > 2147483647)
			|| (Big_Integer < -2147483648LL)) {
			My_Printf
			    ("Improper value specified for option %s.\n",
			     Option_Name);
			Error = EINVAL;
			break;
		    }

		    Value_List->value[i].i32 = (int32_t) Big_Integer;
		}
		break;
	    case EVMS_Type_Int64:	/* int64_t   */
		for (i = 0; i < List_Size; i++) {
		    Peek(&Item_Tag, &Item_Size);

		    if (Item_Tag == STRING_POINTER) {
			My_Printf
			    ("Improper value specified for option %s.\n",
			     Option_Name);
			Error = EINVAL;
			break;
		    }

		    if (Item_Tag == REAL_NUMBER) {
			Pop(REAL_NUMBER, Item_Size, &Big_Real);
			Big_Integer = (int64_t) Big_Real;
		    } else
			Pop(INTEGER_VALUE, Item_Size, &Big_Integer);

		    /* Convert to the proper units for this option. */
		    Big_Integer = Convert_Int(Option_Index, Big_Integer);

		    Value_List->value[i].i64 = Big_Integer;
		}
		break;
	    case EVMS_Type_Unsigned_Int:	/* uint      */
		for (i = 0; i < List_Size; i++) {
		    Peek(&Item_Tag, &Item_Size);

		    if (Item_Tag == REAL_NUMBER) {
			Pop(REAL_NUMBER, Item_Size, &Big_Real);
			if (Big_Real < 0)
			    Error = EINVAL;
			else
			    Big_Integer = (int64_t) Big_Real;
		    } else if (Item_Tag == STRING_POINTER)
			Error = EINVAL;
		    else
			Pop(INTEGER_VALUE, Item_Size, &Big_Integer);

		    if (Error) {
			My_Printf
			    ("Improper value specified for option %s.\n",
			     Option_Name);
			break;
		    }

		    /* Convert to the proper units for this option. */
		    Big_Integer = Convert_Int(Option_Index, Big_Integer);

		    if ((Big_Integer > 4294967295U) || (Big_Integer < 0)) {
			My_Printf
			    ("Improper value specified for option %s.\n",
			     Option_Name);
			Error = EINVAL;
			break;
		    }

		    Value_List->value[i].ui = (u_int) Big_Integer;
		}
		break;
	    case EVMS_Type_Unsigned_Int8:	/* u_int8_t  */
		for (i = 0; i < List_Size; i++) {
		    Peek(&Item_Tag, &Item_Size);

		    if (Item_Tag == STRING_POINTER)
			Error = EINVAL;
		    else if (Item_Tag == REAL_NUMBER) {
			Pop(REAL_NUMBER, Item_Size, &Big_Real);
			if (Big_Real < 0)
			    Error = EINVAL;
			else
			    Big_Integer = (int64_t) Big_Real;
		    } else
			Pop(INTEGER_VALUE, Item_Size, &Big_Integer);

		    if (Error) {
			My_Printf
			    ("Improper value specified for option %s.\n",
			     Option_Name);
			break;
		    }

		    /* Convert to the proper units for this option. */
		    Big_Integer = Convert_Int(Option_Index, Big_Integer);

		    if ((Big_Integer > 255) || (Big_Integer < 0)) {
			My_Printf
			    ("Improper value specified for option %s.\n",
			     Option_Name);
			Error = EINVAL;
			break;
		    }

		    Value_List->value[i].ui = (u_int8_t) Big_Integer;
		}
		break;
	    case EVMS_Type_Unsigned_Int16:	/* u_int16_t */
		for (i = 0; i < List_Size; i++) {
		    Peek(&Item_Tag, &Item_Size);

		    if (Item_Tag == STRING_POINTER)
			Error = EINVAL;
		    else if (Item_Tag == REAL_NUMBER) {
			Pop(REAL_NUMBER, Item_Size, &Big_Real);
			if (Big_Real < 0)
			    Error = EINVAL;
			else
			    Big_Integer = (int64_t) Big_Real;
		    } else
			Pop(INTEGER_VALUE, Item_Size, &Big_Integer);

		    if (Error) {
			My_Printf
			    ("Improper value specified for option %s.\n",
			     Option_Name);
			break;
		    }

		    /* Convert to the proper units for this option. */
		    Big_Integer = Convert_Int(Option_Index, Big_Integer);

		    if ((Big_Integer > 65535) || (Big_Integer < 0)) {
			My_Printf
			    ("Improper value specified for option %s.\n",
			     Option_Name);
			Error = EINVAL;
			break;
		    }

		    Value_List->value[i].ui16 = (u_int16_t) Big_Integer;
		}
		break;
	    case EVMS_Type_Unsigned_Int32:	/* u_int32_t */
		for (i = 0; i < List_Size; i++) {
		    Peek(&Item_Tag, &Item_Size);

		    if (Item_Tag == STRING_POINTER)
			Error = EINVAL;
		    else if (Item_Tag == REAL_NUMBER) {
			Pop(REAL_NUMBER, Item_Size, &Big_Real);
			if (Big_Real < 0)
			    Error = EINVAL;
			else
			    Big_Integer = (int64_t) Big_Real;
		    } else
			Pop(INTEGER_VALUE, Item_Size, &Big_Integer);

		    if (Error) {
			My_Printf
			    ("Improper value specified for option %s.\n",
			     Option_Name);
			break;
		    }

		    /* Convert to the proper units for this option. */
		    Big_Integer = Convert_Int(Option_Index, Big_Integer);

		    if ((Big_Integer > 4294967295U) || (Big_Integer < 0)) {
			My_Printf
			    ("Improper value specified for option %s.\n",
			     Option_Name);
			Error = EINVAL;
			break;
		    }

		    Value_List->value[i].ui32 = (u_int32_t) Big_Integer;
		}
		break;
	    case EVMS_Type_Unsigned_Int64:	/* u_int64_t */
		for (i = 0; i < List_Size; i++) {
		    Peek(&Item_Tag, &Item_Size);

		    if (Item_Tag == STRING_POINTER)
			Error = EINVAL;
		    else if (Item_Tag == REAL_NUMBER) {
			Pop(REAL_NUMBER, Item_Size, &Big_Real);
			if (Big_Real < 0)
			    Error = EINVAL;
			else
			    Big_Integer = (int64_t) Big_Real;
		    } else
			Pop(INTEGER_VALUE, Item_Size, &Big_Integer);

		    if (Error) {
			My_Printf
			    ("Improper value specified for option %s.\n",
			     Option_Name);
			break;
		    }

		    /* Convert to the proper units for this option. */
		    Big_Integer = Convert_Int(Option_Index, Big_Integer);

		    Value_List->value[i].ui64 = (u_int64_t) Big_Integer;
		}
		break;
	    default:
		Report_Standard_Error(INTERNAL_ERROR_BOD);
		abort();
		break;
	    }

	    if (!Error) {
		if (Value_Is_List)
		    Error =
			evms_set_option_value(Task, Option_Index, &Value,
					      &effect);
		else
		    Error =
			evms_set_option_value(Task, Option_Index,
					      &(Value_List->value[0]),
					      &effect);

		if (!Error) {

		    /* Indicate that we set this option. */
		    Option_Usage[Option_Index] = TRUE;

		    /* Check to see if the plug-in altered the value specified by the user. */
		    if (effect & EVMS_Effect_Inexact) {
			My_Printf
			    ("The value for option %s was adjusted.\n",
			     Option_Descriptors[Option_Index]->name);
		    }

		} else
		    Report_Standard_Error(Error);

	    }

	    free(Value_List);

	} else {
	    My_Printf("The option %s is not valid in this context.\n",
		      Option_Name);
	    Error = EINVAL;
	}

    }

    return Error;
}


static int
Do_Feature(Executable_Node * Current_Node, object_handle_t Handle,
	   boolean Forward)
{
    int Error;
    int Cleanup_Error = 0;
    u_int32_t i;
    ADDRESS Not_Needed = NULL;
    handle_array_t *Object_List = NULL;
    handle_object_info_t *Object_Info = NULL;
    dlist_t Return_Values;

    Error = Setup_Task(Handle, Task_Action, TRUE);
    if (!Error) {

	/* Now that we have created the task structure and obtained descriptions of all of the
	   available options for this task, we can now process our children.                   */
	Error =
	    ForEachItem(Current_Node->Children, Process_Node, NULL,
			Forward);
	if (!Error) {

	    /* Since all children finished successfully, we can perform our final checks before invoking the task. */
	    Clear_Option_Descriptors();
	    Error = Get_Option_Descriptors();
	    for (i = 0; (i < Option_Count) && (!Error); i++) {

		if (Option_Usage[i] &&
		    (Option_Descriptors[i]->
		     flags & EVMS_OPTION_FLAGS_INACTIVE)) {
		    Error = INVALID_OPTION;
		    Report_Standard_Error(Error);
		}

	    }

	    /* If there were no errors, then lets do it. */
	    if (!Error) {
		if (!Get_Acceptable) {
		    /* Invoke the task and see what happens! */
		    Error = evms_invoke_task(Task, &Object_List);
		    if (Error)
			Report_Standard_Error(Error);
		    else if (Object_List != NULL)
			if (Task_Action == EVMS_Task_Create
			    || Task_Action == EVMS_Task_Create_Container) {

			    for (i = 0; i < Object_List->count; i++) {
				Error =
				    evms_get_info(Object_List->handle[i],
						  &Object_Info);
				if (Error) {
				    Report_Standard_Error(Error);
				}

				switch (Object_Info->type) {
				case SEGMENT:
				    My_Printf
					("The create command created segment: %s\n",
					 Object_Info->info.segment.name);
				    break;
				case CONTAINER:
				    My_Printf
					("The create command created container: %s\n",
					 Object_Info->info.container.name);
				    break;
				case REGION:
				    My_Printf
					("The create command created region: %s\n",
					 Object_Info->info.region.name);
				    break;
				case EVMS_OBJECT:
				    My_Printf
					("The create command created object: %s\n",
					 Object_Info->info.object.name);
				    break;
				default:
				    break;
				}
				evms_free(Object_Info);
			    }
			}
		    evms_free(Object_List);

		} else {
		    /* We need to get the acceptable objects for this task in the state it is currently in
		       and then place these tasks in a list.  The List is pushed on the stack for us and
		       should be waiting for us to pop it off.  Once we populate the list, we must push
		       it back onto the stack so that others may access it.                                 */
		    Pop(LIST, sizeof(dlist_t), &Return_Values);

		    /* Get the acceptable objects for the task. */
		    Error =
			evms_get_acceptable_objects(Task, &Object_List);

		    if ((!Error) && (Object_List != NULL)) {

			/* We need to convert each handle in the Object_List to a handle_object_info_t entry
			   so that any parents of this node can process it.                                  */
			for (i = 0; i < Object_List->count; i++) {
			    Error =
				evms_get_info(Object_List->handle[i],
					      &Object_Info);
			    if (Error) {
				Report_Standard_Error(Error);
				break;
			    }

			    /* Add the info structure to the Return_Values list. */
			    Error =
				InsertObject(Return_Values, Object_Info,
					     HANDLE_INFO, NULL,
					     AppendToList, TRUE,
					     &Not_Needed);
			    Check_For_Error(Error);
			}

			if (Object_List != NULL)
			    evms_free(Object_List);

			/* Now put the Return_Values list back on the stack. */
			Push(LIST, sizeof(dlist_t), &Return_Values);

		    }

		}

	    }

	}

	if (!Get_Acceptable) {
	    Cleanup_Error = Cleanup_Task();
	    if (!Error)
		Error = Cleanup_Error;
	}

    }

    return Error;

}


static int Do_Feature_ID(Executable_Node * Current_Node)
{
    object_handle_t Handle;
    u_int64_t Number;
    plugin_id_t ID;
    int Error = 0;

    /* Get the ID specified by the user. */
    Number = (u_int64_t) atoll((char *) Current_Node->NodeValue);

    if (Number <= 4294967295U) {

	ID = (plugin_id_t) Number;

	Error = evms_get_plugin_by_ID(ID, &Handle);
	if (!Error) {

	    Error = Do_Feature(Current_Node, Handle, FALSE);

	} else
	    Report_Standard_Error(Error);

    } else {
	My_Printf("The feature/plug-in ID specified was out of range.\n");
	Error = EINVAL;
    }

    return Error;

}


static int Do_Feature_Name(Executable_Node * Current_Node)
{
    object_handle_t Handle;
    int Error = 0;

    Error = evms_get_plugin_by_name(Current_Node->NodeValue, &Handle);
    if (!Error) {

	Error = Do_Feature(Current_Node, Handle, FALSE);

    } else
	Report_Standard_Error(Error);

    return Error;

}


static int Do_ObjectList(Executable_Node * Current_Node)
{
    task_effect_t Task_Result;
    declined_handle_array_t *Bad_Object_List;
    int Error = 0;
    int Error2 = 0;
    char *Error_Message;
    u_int32_t Child_Count;
    u_int32_t i;
    handle_object_info_t *Bad_Object_Info;
    char *Bad_Object_Name;

    /* Process our children. */
    Error = ForEachItem(Current_Node->Children, Process_Node, NULL, TRUE);
    if (!Error) {

	/* We should have one handle on the stack for each child we have.  We must
	   take these handles off of the stack and put them into a handle_array_t
	   for passing the EVMS Engine.                                            */

	/* How many children do we have? */
	Error = GetListSize(Current_Node->Children, &Child_Count);
	Check_For_Error(Error);

	if (Child_Count > 0) {
	    /* Allocate memory for the handle_array_t object. */
	    Object_Array =
		(handle_array_t *) malloc(sizeof(handle_array_t) +
					  sizeof(object_handle_t) *
					  Child_Count);
	    if (Object_Array != NULL) {

		/* Now we must fill in the Object_Array. */
		Object_Array->count = Child_Count;
		for (Child_Count = 0; Child_Count < Object_Array->count;
		     Child_Count++) {
		    Pop(HANDLE, sizeof(object_handle_t),
			&(Object_Array->handle[Child_Count]));
		}

		/* Now that the object array is complete, lets send it to the EVMS Engine. */
		Error =
		    evms_set_selected_objects(Task, Object_Array,
					      &Bad_Object_List,
					      &Task_Result);
		if (Error || ((Bad_Object_List != NULL)
			      && (Bad_Object_List->count > 0))) {

		    /* Walk the declined handle array and report errors. */
		    if ((Bad_Object_List != NULL) &&
			(Bad_Object_List->count > 0)) {
			for (i = 0; i < Bad_Object_List->count; i++) {

			    Error2 =
				evms_get_info(Bad_Object_List->declined[i].
					      handle, &Bad_Object_Info);
			    if (!Error2) {
				switch (Bad_Object_Info->type) {
				case VOLUME:
				    Bad_Object_Name =
					Bad_Object_Info->info.volume.name;
				    break;
				case CONTAINER:
				    Bad_Object_Name =
					Bad_Object_Info->info.container.
					name;
				    break;
				case EVMS_OBJECT:
				    Bad_Object_Name =
					Bad_Object_Info->info.object.name;
				    break;
				case REGION:
				    Bad_Object_Name =
					Bad_Object_Info->info.region.name;
				    break;
				case SEGMENT:
				    Bad_Object_Name =
					Bad_Object_Info->info.segment.name;
				    break;
				case DISK:
				    Bad_Object_Name =
					Bad_Object_Info->info.disk.name;
				    break;
				case PLUGIN:
				    Bad_Object_Name =
					Bad_Object_Info->info.plugin.
					short_name;
				    break;
				default:
				    Bad_Object_Name = NULL;
				    break;
				}
				Error_Message =
				    strerror(Bad_Object_List->declined[i].
					     reason);
				My_Printf
				    ("%s was rejected with error: %s\n",
				     Bad_Object_Name, Error_Message);
			    }

			}

		    }

		    if (!Error)
			Error = EINVAL;

		}

	    } else {
		Report_Standard_Error(ENOMEM);
		exit(ENOMEM);
	    }

	}

    }

    return Error;

}


static int Get_Option_Descriptors(void)
{
    u_int32_t i;
    int Error = 0;

    /* Set Option_Descriptors to all NULL. */
    for (i = 0; i < Option_Count; i++)
	Option_Descriptors[i] = NULL;

    for (i = 0; (i < Option_Count) && (!Error); i++) {
	Error =
	    evms_get_option_descriptor(Task, i, &(Option_Descriptors[i]));
    }

    if (Error)
	Report_Standard_Error(Error);

    return Error;
}


static void Clear_Option_Descriptors(void)
{
    u_int32_t i;

    for (i = 0; (i < Option_Count) && (Option_Descriptors[i] != NULL); i++)
	evms_free(Option_Descriptors[i]);

    return;
}


static int Do_Probe(Executable_Node * Current_Node)
{
    int Error = 0;
    boolean changes_pending;

    Error = evms_changes_pending(&changes_pending, NULL);
    if (!Error) {
	if (changes_pending)
	    Error = Commit_Changes();
    } else {
	// BUGBUG add code to handle an error from evms_changes_pending()
    }

    if (!Error)
	Error = evms_close_engine();

    if (!Error)
	Error =
	    evms_open_engine(Node_Name, Engine_Mode, &Callback_Functions,
			     Debug_Level, DEFAULT_LOG_FILE);

    if (Error)
	Report_Standard_Error(Error);

    return Error;

}


static int Do_IntegerData(Executable_Node * Current_Node)
{
    int64_t Number;
    int Return_Value = 0;

    Number = (int64_t) atoll((char *) Current_Node->NodeValue);
    Push(INTEGER_VALUE, sizeof(int64_t), &Number);

    /* Process our children. */
    Return_Value =
	ForEachItem(Current_Node->Children, Process_Node, NULL, TRUE);

    return Return_Value;
}


static int Do_Precision(Executable_Node * Current_Node)
{
    int64_t Number;
    uint Child_Count;
    int Return_Value = 0;
    boolean Use_Default_For_Tolerance;

    Number = (int64_t) atoll((char *) Current_Node->NodeValue);
    Push(INTEGER_VALUE, sizeof(int64_t), &Number);

    /* Process our children. */
    Return_Value =
	ForEachItem(Current_Node->Children, Process_Node, NULL, TRUE);

    /* If we have a child, then it processed the INTEGER_VALUE that we put on the stack.
       After processing that value, it put the result back on the stack.  We want to
       leave it there.  We need to indicate to our parent, though, whether we had a
       child or not.  If we had a child, then the user specified a unit of measure
       to go with the precision value they specified.  If we had no child, then our
       parent must assume some default condition.                                         */
    if (Return_Value == 0) {
	/* Find out how many children we have. */
	Return_Value = GetListSize(Current_Node->Children, &Child_Count);
	Check_For_Error(Return_Value);

	if (Child_Count > 0)
	    Use_Default_For_Tolerance = FALSE;
	else
	    Use_Default_For_Tolerance = TRUE;

	Push(BOOLEAN_VALUE, sizeof(boolean), &Use_Default_For_Tolerance);

    }

    return Return_Value;

}


static int Do_RealNumber(Executable_Node * Current_Node)
{

    double Real_Number;
    int Return_Value = 0;

    errno = 0;
    Real_Number = strtod((char *) Current_Node->NodeValue, NULL);
    if (errno != 0) {
	Report_Standard_Error(EINVAL);
	Return_Value = EINVAL;
    } else {
	Push(REAL_NUMBER, sizeof(double), &Real_Number);

	/* Process our children. */
	Return_Value =
	    ForEachItem(Current_Node->Children, Process_Node, NULL, TRUE);
    }

    return Return_Value;
}


static int Do_Revert(Executable_Node * Current_Node)
{
    object_handle_t Handle;
    int Error;

    /* Process our children first! */
    Error = ForEachItem(Current_Node->Children, Process_Node, NULL, TRUE);
    if (!Error) {
	Pop(HANDLE, sizeof(object_handle_t), &Handle);

	Error = evms_delete(Handle);
	if (Error)
	    Report_Standard_Error(Error);

    }

    return Error;
}


static int Do_Echo(Executable_Node * Current_Node)
{
    char *Message = NULL;
    int Error;

    /* Process our children first! */
    Error = ForEachItem(Current_Node->Children, Process_Node, NULL, TRUE);
    if (!Error) {
	Pop(STRING_POINTER, sizeof(char *), &Message);

	if (Message != NULL) {
	    My_Printf("%s\n", Message);
	} else {
	    My_Printf("\n");
	}

    }

    return Error;
}


static int Do_Translate_Name_To_Handle(Executable_Node * Current_Node)
{
    object_handle_t Handle;
    int Error;

    Error =
	evms_get_object_handle_for_name(VOLUME | EVMS_OBJECT | REGION |
					CONTAINER | SEGMENT | DISK,
					Current_Node->NodeValue, &Handle);

    if (!Error)
	Push(HANDLE, sizeof(object_handle_t), &Handle);
    else {
	My_Printf("Error translating %s to a handle.\n",
		  (char *) Current_Node->NodeValue);
	Report_Standard_Error(Error);
    }

    return Error;
}

static int Do_Name_To_Volume_Handle(Executable_Node * Current_Node)
{
    object_handle_t Handle;
    int Error;

    Error =
	evms_get_object_handle_for_name(VOLUME, Current_Node->NodeValue,
					&Handle);

    if (!Error)
	Push(HANDLE, sizeof(object_handle_t), &Handle);
    else {
	My_Printf("Error translating %s to a handle.\n",
		  (char *) Current_Node->NodeValue);
	Report_Standard_Error(Error);
    }

    return Error;
}


static int Do_Plugin_ID(Executable_Node * Current_Node)
{
    object_handle_t Handle;
    u_int64_t Number;
    plugin_id_t ID;
    int Error = 0;

    /* Get the ID specified by the user. */
    Number = (u_int64_t) atoll((char *) Current_Node->NodeValue);

    if (Number <= 4294967295U) {

	ID = (plugin_id_t) Number;

	Error = evms_get_plugin_by_ID(ID, &Handle);
	if (!Error)
	    Push(HANDLE, sizeof(object_handle_t), &Handle);
	else
	    Report_Standard_Error(Error);

    } else {
	My_Printf("The feature/plug-in ID specified was out of range.\n");
	Error = EINVAL;
    }

    return Error;
}


static int Do_Name_To_Plugin_Handle(Executable_Node * Current_Node)
{
    object_handle_t Handle;
    int Error;

    Error = evms_get_plugin_by_name(Current_Node->NodeValue, &Handle);

    if (!Error)
	Push(HANDLE, sizeof(object_handle_t), &Handle);
    else {
	My_Printf("Error translating %s to a handle.\n",
		  (char *) Current_Node->NodeValue);
	Report_Standard_Error(Error);
    }

    return Error;
}


static int Do_Name_To_Object_Handle(Executable_Node * Current_Node)
{
    object_handle_t Handle;
    int Error;

    Error =
	evms_get_object_handle_for_name(EVMS_OBJECT | REGION | SEGMENT |
					DISK, Current_Node->NodeValue,
					&Handle);

    if (!Error)
	Push(HANDLE, sizeof(object_handle_t), &Handle);
    else {
	My_Printf("Error translating %s to a handle.\n",
		  (char *) Current_Node->NodeValue);
	Report_Standard_Error(Error);
    }

    return Error;
}

static int Do_Name_To_Region_Handle(Executable_Node * Current_Node)
{
    object_handle_t Handle;
    int Error;

    Error =
	evms_get_object_handle_for_name(REGION, Current_Node->NodeValue,
					&Handle);

    if (!Error)
	Push(HANDLE, sizeof(object_handle_t), &Handle);
    else {
	My_Printf("Error translating %s to a handle.\n",
		  (char *) Current_Node->NodeValue);
	Report_Standard_Error(Error);
    }

    return Error;
}

static int Do_Name_To_Container_Handle(Executable_Node * Current_Node)
{
    object_handle_t Handle;
    int Error;

    Error =
	evms_get_object_handle_for_name(CONTAINER, Current_Node->NodeValue,
					&Handle);

    if (!Error)
	Push(HANDLE, sizeof(object_handle_t), &Handle);
    else {
	My_Printf("Error translating %s to a handle.\n",
		  (char *) Current_Node->NodeValue);
	Report_Standard_Error(Error);
    }

    return Error;
}

static int Do_Name_To_Segment_Handle(Executable_Node * Current_Node)
{
    object_handle_t Handle;
    int Error;

    Error =
	evms_get_object_handle_for_name(SEGMENT, Current_Node->NodeValue,
					&Handle);

    if (!Error)
	Push(HANDLE, sizeof(object_handle_t), &Handle);
    else {
	My_Printf("Error translating %s to a handle.\n",
		  (char *) Current_Node->NodeValue);
	Report_Standard_Error(Error);
    }

    return Error;
}

static int Do_Name_To_Disk_Handle(Executable_Node * Current_Node)
{
    object_handle_t Handle = 0;
    int Error = 0;

    Error =
	evms_get_object_handle_for_name(DISK, Current_Node->NodeValue,
					&Handle);

    if (!Error)
	Push(HANDLE, sizeof(object_handle_t), &Handle);
    else {
	My_Printf("Error translating %s to a handle.\n",
		  (char *) Current_Node->NodeValue);
	Report_Standard_Error(Error);
    }

    return Error;
}

static int Do_Unit_Conversion(Executable_Node * Current_Node)
{
    int64_t Number;
    double Real_Number;
    TAG Item_Tag;
    uint Item_Size;
    int Return_Value = 0;

    /* We must get the top item on the stack.  It should be an integer or a real number. */
    Peek(&Item_Tag, &Item_Size);

    switch (Item_Tag) {
    case INTEGER_VALUE:
	Pop(INTEGER_VALUE, Item_Size, &Number);
	if (Current_Node->NodeValue == MegabyteStr) {
	    Number = Number * 1048576L;
	    Current_Units = EVMS_Unit_Bytes;
	} else if (Current_Node->NodeValue == GigabyteStr) {
	    Number = Number * 1073741824L;
	    Current_Units = EVMS_Unit_Bytes;
	} else if (Current_Node->NodeValue == KilobyteStr) {
	    Number = Number * 1024L;
	    Current_Units = EVMS_Unit_Bytes;
	} else if (Current_Node->NodeValue == TerrabyteStr) {
	    Number = Number * 1073741824L * 1024;
	    Current_Units = EVMS_Unit_Bytes;
	} else if (Current_Node->NodeValue == HoursStr) {
	    Number = Number * 1000000 * 3600;
	    Current_Units = EVMS_Unit_Microseconds;
	} else if (Current_Node->NodeValue == MinutesStr) {
	    Number = Number * 1000000 * 60;
	    Current_Units = EVMS_Unit_Microseconds;
	} else if (Current_Node->NodeValue == SecondsStr) {
	    Number = Number * 1000000;
	    Current_Units = EVMS_Unit_Microseconds;
	} else if (Current_Node->NodeValue == MillisecondsStr) {
	    Number = Number * 1000;
	    Current_Units = EVMS_Unit_Microseconds;
	} else if (Current_Node->NodeValue == MicrosecondsStr) {
	    Current_Units = EVMS_Unit_Microseconds;
	} else {
	    Return_Value = DLIST_CORRUPTED;
	    Current_Units = EVMS_Unit_None;
	}

	if (!Return_Value)
	    Push(INTEGER_VALUE, sizeof(u_int64_t), &Number);
	break;
    case REAL_NUMBER:
	Pop(REAL_NUMBER, Item_Size, &Real_Number);

	if (Current_Node->NodeValue == MegabyteStr) {
	    Real_Number = Real_Number * 1048576L;
	    Current_Units = EVMS_Unit_Bytes;
	} else if (Current_Node->NodeValue == GigabyteStr) {
	    Real_Number = Real_Number * 1073741824L;
	    Current_Units = EVMS_Unit_Bytes;
	} else if (Current_Node->NodeValue == KilobyteStr) {
	    Real_Number = Real_Number * 1024L;
	    Current_Units = EVMS_Unit_Bytes;
	} else if (Current_Node->NodeValue == TerrabyteStr) {
	    Real_Number = Real_Number * 1073741824L * 1024;
	    Current_Units = EVMS_Unit_Bytes;
	} else if (Current_Node->NodeValue == HoursStr) {
	    Real_Number = Real_Number * 1000000 * 3600;
	    Current_Units = EVMS_Unit_Microseconds;
	} else if (Current_Node->NodeValue == MinutesStr) {
	    Real_Number = Real_Number * 1000000 * 60;
	    Current_Units = EVMS_Unit_Microseconds;
	} else if (Current_Node->NodeValue == SecondsStr) {
	    Real_Number = Real_Number * 1000000;
	    Current_Units = EVMS_Unit_Microseconds;
	} else if (Current_Node->NodeValue == MillisecondsStr) {
	    Real_Number = Real_Number * 1000;
	    Current_Units = EVMS_Unit_Microseconds;
	} else if (Current_Node->NodeValue == MicrosecondsStr) {
	    Current_Units = EVMS_Unit_Microseconds;
	} else {
	    Return_Value = DLIST_CORRUPTED;
	    Current_Units = EVMS_Unit_None;
	}

	if (!Return_Value)
	    Push(REAL_NUMBER, sizeof(double), &Real_Number);
	break;
    default:
	Return_Value = DLIST_CORRUPTED;
	break;
    }

    return Return_Value;

}


static int Do_Activate(Executable_Node * Current_Node)
{
    object_handle_t Handle;
    int Error = 0;

    /* Process our children first! */
    Error = ForEachItem(Current_Node->Children, Process_Node, NULL, TRUE);
    if (!Error) {

	Pop(HANDLE, sizeof(object_handle_t), &Handle);
	Error = evms_activate(Handle);
	if (Error)
	    Report_Standard_Error(Error);

    }

    return Error;
}


static int Do_Assign(Executable_Node * Current_Node)
{
    int Error = 0;

    /* Set the Task_Action variable so that our children will know what kind of task this is. */
    Task_Action = EVMS_Task_Assign_Plugin;

    /* Process our children. */
    Error = ForEachItem(Current_Node->Children, Process_Node, NULL, TRUE);

    return Error;

}


static int Do_Add_Feature(Executable_Node * Current_Node)
{
    int Error = 0;

    /* Set the Task_Action variable so that our children will know what kind of task this is. */
    Task_Action = EVMS_Task_Add_Feature;

    /* Process our children. */
    Error = ForEachItem(Current_Node->Children, Process_Node, NULL, TRUE);

    return Error;

}


static int Do_Create(Executable_Node * Current_Node)
{
    object_handle_t Handle;
    int Error = 0;
    char *Name = NULL;

    if (Current_Node->NodeValue == (void *) ObjectStr)
	Task_Action = EVMS_Task_Create;
    else if (Current_Node->NodeValue == (void *) ContainerStr)
	Task_Action = EVMS_Task_Create_Container;
    else if (Current_Node->NodeValue == (void *) RegionStr)
	Task_Action = EVMS_Task_Create;

    /* Process our children. */
    Error = ForEachItem(Current_Node->Children, Process_Node, NULL, TRUE);
    if (!Error) {

	/* What are we going to create? */
	if (Current_Node->NodeValue == (void *) VolumeStr) {
	    Pop(STRING_POINTER, sizeof(char *), &Name);
	    Pop(HANDLE, sizeof(object_handle_t), &Handle);
	    Error = evms_create_volume(Handle, Name);
	    if (Error)
		Report_Standard_Error(Error);

	} else if (Current_Node->NodeValue == (void *) CompatibilityStr) {
	    Pop(HANDLE, sizeof(object_handle_t), &Handle);
	    Error = evms_create_compatibility_volume(Handle);
	    if (Error)
		Report_Standard_Error(Error);
	}

    }

    return Error;
}


static int Do_Deactivate(Executable_Node * Current_Node)
{
    object_handle_t Handle;
    int Error = 0;

    /* Process our children first! */
    Error = ForEachItem(Current_Node->Children, Process_Node, NULL, TRUE);
    if (!Error) {

	Pop(HANDLE, sizeof(object_handle_t), &Handle);
	Error = evms_deactivate(Handle);
	if (Error)
	    Report_Standard_Error(Error);

    }

    return Error;
}


static int Do_Delete(Executable_Node * Current_Node)
{
    object_handle_t Handle;
    int Error = 0;

    /* Process our children first! */
    Error = ForEachItem(Current_Node->Children, Process_Node, NULL, TRUE);
    if (!Error) {

	Pop(HANDLE, sizeof(object_handle_t), &Handle);
	Error = evms_destroy(Handle);
	if (Error)
	    Report_Standard_Error(Error);

    }

    return Error;
}


static int Do_Unformat(Executable_Node * Current_Node)
{
    object_handle_t Handle;
    int Error = 0;

    /* Process our children first! */
    Error = ForEachItem(Current_Node->Children, Process_Node, NULL, TRUE);
    if (!Error) {

	Pop(HANDLE, sizeof(object_handle_t), &Handle);
	Error = evms_unmkfs(Handle);
	if (Error)
	    Report_Standard_Error(Error);

    }

    return Error;
}


static int Do_Rename(Executable_Node * Current_Node)
{
    int Error = 0;
    object_handle_t Handle;
    char *Name = NULL;

    /* Process our children first! */
    Error = ForEachItem(Current_Node->Children, Process_Node, NULL, TRUE);
    if (!Error) {

	Pop(STRING_POINTER, sizeof(char *), &Name);
	Pop(HANDLE, sizeof(object_handle_t), &Handle);
	Error = evms_set_volume_name(Handle, Name);
	if (Error)
	    Report_Standard_Error(Error);

    }

    return Error;
}


static int Do_Replace(Executable_Node * Current_Node)
{
    int Error = 0;
    object_handle_t Source;
    object_handle_t Target;

    /* Process our children first! */
    Error = ForEachItem(Current_Node->Children, Process_Node, NULL, TRUE);
    if (!Error) {

	Pop(HANDLE, sizeof(object_handle_t), &Target);
	Pop(HANDLE, sizeof(object_handle_t), &Source);
	Error = evms_replace(Source, Target);
	if (Error)
	    Report_Standard_Error(Error);

    }

    return Error;
}


static int Do_Convert(Executable_Node * Current_Node)
{
    object_handle_t Handle;
    int Error = 0;
    char *Name = NULL;

    /* Process our children. */
    Error = ForEachItem(Current_Node->Children, Process_Node, NULL, TRUE);
    if (!Error) {

	/* What are we going to create? */
	if (Current_Node->NodeValue == (void *) EVMSStr) {
	    Pop(STRING_POINTER, sizeof(char *), &Name);
	    Pop(HANDLE, sizeof(object_handle_t), &Handle);
	    Error = evms_convert_to_evms_volume(Handle, Name);
	    if (Error)
		Report_Standard_Error(Error);

	} else if (Current_Node->NodeValue == (void *) CompatibilityStr) {
	    Pop(HANDLE, sizeof(object_handle_t), &Handle);
	    Error = evms_convert_to_compatibility_volume(Handle);
	    if (Error)
		Report_Standard_Error(Error);
	}

    }

    return Error;

}


static int Do_Query_Extended_Info(Executable_Node * Current_Node)
{
    int Error = 0;
    char *Field_Name = NULL;
    object_handle_t Handle;
    extended_info_array_t *Info;
    u_int Index;
    u_int Child_Count = 0;

    /* We should have either 1 or 2 children. */
    Error = GetListSize(Current_Node->Children, &Child_Count);
    Check_For_Error(Error);

    if ((Child_Count < 1) || (Child_Count > 2)) {
	My_Printf
	    ("Internal error: Incorrect number of children for a Query_Extended_Info node.\n");
	abort();
    }

    /* Process any children. */
    Error = ForEachItem(Current_Node->Children, Process_Node, NULL, TRUE);
    if (!Error) {
	if (Child_Count == 2)
	    Pop(STRING_POINTER, sizeof(char *), &Field_Name);

	Pop(HANDLE, sizeof(object_handle_t), &Handle);

	Error = evms_get_extended_info(Handle, Field_Name, &Info);
	if (!Error) {
	    if ((Info != NULL) && (Info->count > 0)) {
		for (Index = 0; Index < Info->count; Index++)
		    Print_Extended_Info(&(Info->info[Index]));
	    } else {
		My_Printf
		    ("No extended information is available for the specified item.\n");
	    }

	    if (Info != NULL)
		evms_free(Info);

	} else {
	    My_Printf
		("Error getting extended info for the specified item.  Error is:\n");
	    Report_Standard_Error(Error);
	}

    }

    return Error;

}


static int Do_Query(Executable_Node * Current_Node)
{
    dlist_t Return_Values;
    handle_array_t *Object_List;
    handle_object_info_t *Object_Info;
    int Error = 0;
    boolean Convert_Handle_Array_To_List = TRUE;
    ADDRESS NotNeeded;
    unsigned int index;

    /* Create the list used to hold the return values. */
    Return_Values = CreateList();
    if (Return_Values == NULL) {
	Report_Standard_Error(ENOMEM);
	exit(ENOMEM);
    }

    /* Get the appropriate list of candidates. */
    if (Current_Node->NodeValue == VolumesStr) {
	Error = evms_get_volume_list(0, 0, 0, &Object_List);
    } else if (Current_Node->NodeValue == PluginsStr) {
	Error = evms_get_plugin_list(0, 0, &Object_List);
    } else if (Current_Node->NodeValue == RegionsStr) {
	Error = evms_get_object_list(REGION, 0, 0, 0, 0, &Object_List);
    } else if (Current_Node->NodeValue == ObjectsStr) {
	Error = evms_get_object_list(0, 0, 0, 0, 0, &Object_List);
    } else if (Current_Node->NodeValue == ContainersStr) {
	Error = evms_get_container_list(0, 0, 0, &Object_List);
    } else if (Current_Node->NodeValue == DisksStr) {
	Error = evms_get_object_list(DISK, 0, 0, 0, 0, &Object_List);
    } else if (Current_Node->NodeValue == SegmentsStr) {
	Error = evms_get_object_list(SEGMENT, 0, 0, 0, 0, &Object_List);
    } else
	Convert_Handle_Array_To_List = FALSE;

    if ((Convert_Handle_Array_To_List) && (!Error)) {
	/* We need to convert each handle in the Object_List to a handle_object_info_t entry
	   so that any children of this node can process it.                                 */
	for (index = 0; index < Object_List->count; index++) {
	    Error =
		evms_get_info(Object_List->handle[index], &Object_Info);
	    if (Error) {
		Report_Standard_Error(Error);
		break;
	    }

	    /* Add the info structure to the Return_Values list. */
	    Error =
		InsertObject(Return_Values, Object_Info, HANDLE_INFO, NULL,
			     AppendToList, TRUE, &NotNeeded);
	    Check_For_Error(Error);
	}

	evms_free(Object_List);

    }

    if (!Error) {

	/* Put the list object on the stack. */
	Push(LIST, sizeof(dlist_t), &Return_Values);

	/* Process any children. */
	Error =
	    ForEachItem(Current_Node->Children, Process_Node, NULL, TRUE);
	if (!Error) {

	    /* Remove the list object from the stack. */
	    Pop(LIST, sizeof(dlist_t), &Return_Values);

	    /* Print the contents of each object in the Return_Values list. */
	    My_Printf("\n");
	    Error =
		ForEachItem(Return_Values, Print_Object_Info, NULL, TRUE);
	    List_Options = FALSE;

	}

    }

    Freespace_Not_Size = FALSE;

    /* Dispose of the Return_Values list. */
    Check_For_Error(DestroyList(&Return_Values, FALSE));

    return Error;

}


static int Do_Query_Type_2(Executable_Node * Current_Node)
{
    dlist_t Return_Values;
    object_handle_t Object_Handle;
    handle_array_t *Object_List;
    handle_array_t Tmp_Array;
    handle_object_info_t *Object_Info;
    expand_handle_array_t *Expand_Points;
    shrink_handle_array_t *Shrink_Points;
    int Error = 0;
    ADDRESS NotNeeded;
    unsigned int index;

    /* Create the list used to hold the return values. */
    Return_Values = CreateList();
    if (Return_Values == NULL) {
	Report_Standard_Error(ENOMEM);
	exit(ENOMEM);
    }

    /* Process any children. */
    Error = ForEachItem(Current_Node->Children, Process_Node, NULL, TRUE);
    if (!Error) {

	/* There should be the handle of the object we are interested in on the stack.  Get the handle. */
	Pop(HANDLE, sizeof(object_handle_t), &Object_Handle);

	if (Current_Node->NodeValue == EPStr) {
	    Error = evms_get_expand_points(Object_Handle, &Expand_Points);
	    if (!Error) {

		if ((Expand_Points != NULL) && (Expand_Points->count != 0)) {

		    for (index = 0; index < Expand_Points->count; index++) {
			Error =
			    evms_get_info(Expand_Points->
					  expand_point[index].object,
					  &Object_Info);
			if (Error) {
			    Report_Standard_Error(Error);
			    break;
			}

			/* Add the info structure to the Return_Values list. */
			Error =
			    InsertObject(Return_Values, Object_Info,
					 HANDLE_INFO, NULL, AppendToList,
					 TRUE, &NotNeeded);
			Check_For_Error(Error);

			// added by wwliu
			if(0 != InsertPrivateData(
			 Expand_Points->expand_point[index].object,
			 Expand_Points->expand_point[index].max_expand_size))
			{
				printf("wwliu : preserve shrink point error\n");
			}

		    }

		}

	    } else {
		My_Printf("The specified object can not be expanded.\n");
	    }

	} else if (Current_Node->NodeValue == SPStr) {
	    Error = evms_get_shrink_points(Object_Handle, &Shrink_Points);
	    if (!Error) {

		if ((Shrink_Points != NULL) && (Shrink_Points->count != 0)) {

		    for (index = 0; index < Shrink_Points->count; index++) {
			Error =
			    evms_get_info(Shrink_Points->
					  shrink_point[index].object,
					  &Object_Info);
			if (Error) {
			    Report_Standard_Error(Error);
			    break;
			}

			/* Add the info structure to the Return_Values list. */
			Error =
			    InsertObject(Return_Values, Object_Info,
					 HANDLE_INFO, NULL, AppendToList,
					 TRUE, &NotNeeded);
			Check_For_Error(Error);

			// added by wwliu 
			if(0 != InsertPrivateData(
			 Shrink_Points->shrink_point[index].object,
			 Shrink_Points->shrink_point[index].max_shrink_size))
			{
				printf("wwliu : preserve shrink point error\n");
			}

		    }

		}

	    } else {
		My_Printf
		    ("Error getting the shrink points for the specified object.\n");
		Report_Standard_Error(Error);
	    }

	} else {
	    Error = evms_get_info(Object_Handle, &Object_Info);
	    if (!Error) {

		if (Current_Node->NodeValue == ChildrenStr) {
		    /* Child case */
		    switch (Object_Info->type) {
		    case VOLUME:
			Object_List = NULL;
			Error =
			    evms_get_info(Object_Info->info.volume.object,
					  &Object_Info);
			if (!Error) {
			    Error =
				InsertObject(Return_Values, Object_Info,
					     HANDLE_INFO, NULL,
					     AppendToList, TRUE,
					     &NotNeeded);
			    Check_For_Error(Error);
			} else
			    Report_Standard_Error(Error);
			break;
		    case CONTAINER:
			Object_List =
			    Object_Info->info.container.objects_consumed;
			break;
		    case REGION:
			Object_List =
			    Object_Info->info.region.child_objects;
			break;
		    case EVMS_OBJECT:
			Object_List =
			    Object_Info->info.object.child_objects;
			break;
		    case SEGMENT:
			Object_List =
			    Object_Info->info.segment.child_objects;
			break;
		    case DISK:
			Object_List = Object_Info->info.disk.child_objects;
			break;
		    default:
			Object_List = NULL;
			break;
		    }

		} else {
		    /* Parent case */
		    switch (Object_Info->type) {
		    case VOLUME:
			Object_List = NULL;
			break;
		    case CONTAINER:
			Object_List =
			    Object_Info->info.container.objects_produced;
			break;
		    case REGION:
		    case SEGMENT:
		    case DISK:
		    case EVMS_OBJECT:
			Object_List =
			    Object_Info->info.object.parent_objects;
			if (Object_List->count == 0
			    && Object_Info->info.object.volume != 0) {
			    Tmp_Array.count = 1;
			    Tmp_Array.handle[0] =
				Object_Info->info.object.volume;
			    Object_List = &Tmp_Array;
			}
			break;
		    default:
			Object_List = NULL;
			break;
		    }

		}

		if (Object_List != NULL) {

		    for (index = 0; index < Object_List->count; index++) {
			Error =
			    evms_get_info(Object_List->handle[index],
					  &Object_Info);
			if (Error) {
			    Report_Standard_Error(Error);
			    break;
			}

			/* Add the info structure to the Return_Values list. */
			Error =
			    InsertObject(Return_Values, Object_Info,
					 HANDLE_INFO, NULL, AppendToList,
					 TRUE, &NotNeeded);
			Check_For_Error(Error);
		    }

		}

	    } else
		Report_Standard_Error(Error);

	}

    }

    if (!Error) {
	/* Print the contents of each object in the Return_Values list. */
	My_Printf("\n");
	Error = ForEachItem(Return_Values, Print_Object_Info, NULL, TRUE);
	List_Options = FALSE;

    }

    Freespace_Not_Size = FALSE;

    /* Dispose of the Return_Values list. */
    Check_For_Error(DestroyList(&Return_Values, FALSE));

    return Error;

}


static int Do_Help(Executable_Node * Current_Node)
{
    Display_Help(Current_Node);
    return 0;
}


/* Create the specified task, place its handle in the global variable Task,
   set the global variable Task_Action, Allocate the Option_Descriptors array,
   allocate the Option_Usage array, fill in the Option_Descriptors array.      */
static int
Setup_Task(plugin_handle_t Handle, task_action_t Task_Type,
	   boolean Report_Errors)
{
    unsigned int i;
    int Error = 0;
    int Dont_Care;
    object_type_t type;

    /* Initialize the global task variables. */
    Task = 0;
    Option_Descriptors = NULL;
    Option_Count = 0;
    Option_Usage = NULL;


    /* A different set_info command is directed at Containers */
    if (Task_Type == EVMS_Task_Set_Info &&
	evms_get_handle_object_type(Handle, &type) == 0
	&& type == CONTAINER) {
	Task_Type = EVMS_Task_Set_Container_Info;
    }


    /* Create the task. */
    Error = evms_create_task(Handle, Task_Type, &Task);
    if (!Error) {

	/* Since we successfully created the task, save what type of task we created. */
	Task_Action = Task_Type;

	/* Get the options for the task we created. */
	Error = evms_get_option_count(Task, &Option_Count);
	if ((!Error) && (Option_Count > 0)) {

	    Option_Descriptors =
		(option_descriptor_t **)
		malloc(sizeof(option_descriptor_t *) * Option_Count);
	    if (Option_Descriptors != NULL) {

		Option_Usage =
		    (boolean *) malloc(sizeof(boolean) * Option_Count);
		if (Option_Usage != NULL) {

		    /* We will mark all options as being unused.  As options are set by each name-value pair
		       in our children, they will mark which options they set.  When control returns to this
		       function, we can run this list and check each option that is in use to make sure that
		       it is active.  If it is not, then an improper set of options was specified and we can
		       abort the task operation with an error.                                               */
		    for (i = 0; i < Option_Count; i++)
			Option_Usage[i] = FALSE;

		    Error = Get_Option_Descriptors();

		} else
		    Check_For_Error(ENOMEM);

	    } else
		Check_For_Error(ENOMEM);

	} else {
	    /* There were no options or we had an error. */
	    Option_Count = 0;

	    if (Error && Report_Errors)
		Report_Standard_Error(Error);

	}

    } else {
	/* We could not create the task! */
	Task = 0;
	if (Report_Errors)
	    Report_Standard_Error(Error);
    }

    if (Error) {
	Dont_Care = Cleanup_Task();
    }

    return Error;

}


/* Free the Option_Usage array, free all of the Option_Descriptors, free
   the Option_Descriptors array, destroy the task.                          */
static int Cleanup_Task(void)
{
    int Error = 0;

    if (Option_Usage != NULL) {
	free(Option_Usage);
	Option_Usage = NULL;
    }

    if (Option_Descriptors != NULL) {

	Clear_Option_Descriptors();

	free(Option_Descriptors);

	Option_Descriptors = NULL;
	Option_Count = 0;
    }

    if (Task != 0) {
	Error = evms_destroy_task(Task);
	Task = 0;
	if (Error)
	    Report_Standard_Error(Error);
    }


    return Error;
}


static void
Print_Expand_Shrink_Move_Slide_Info(task_action_t Start,
				    task_action_t End,
				    char *Object_Name,
				    object_handle_t Object_Handle,
				    my_advance_info_t * advance)
{
    task_action_t Current_Task;
    int Error = DLIST_SUCCESS;
    boolean Display_Default_Values = FALSE;

    my_operation_type my_op_type = MY_OP_NULL;

    /* Expand  Move Slide Shrink */
    for (Current_Task = Start; Current_Task <= End; Current_Task++) {

	    my_op_type = MY_OP_NULL;

	Error = Setup_Task(Object_Handle, Current_Task, FALSE);
	if (!Error) {

	    switch (Current_Task) {
	    case EVMS_Task_Set_Info:

		my_op_type = MY_OP_SETINFO;

		if (Option_Count > 0) {
			/*
		    My_Printf
			("The following options for this %s may be changed:\n",
			 Object_Name);
			 */
		    Display_Default_Values = TRUE;
		}
		break;
	    case EVMS_Task_Expand:

		my_op_type = MY_OP_EXPAND;
		/*
		if (Option_Count > 0) {
		    My_Printf
			("This %s may be expanded using the Expand command.\n",
			 Object_Name);
		    My_Printf
			("The following options are available for expanding this %s:\n",
			 Object_Name);
		} else {
		    My_Printf
			("This %s may be expanded using the Expand command.\n\n",
			 Object_Name);
		}
		*/

		break;
	    case EVMS_Task_Shrink:

		my_op_type = MY_OP_SHRINK;

		/*
		if (Option_Count > 0) {
		    My_Printf
			("This %s may be shrunk using the Shrink command.\n",
			 Object_Name);
		    My_Printf
			("The following options are available for shrinking this %s:\n",
			 Object_Name);
		} else {
		    My_Printf
			("This %s may be shrunk using the Shrink command.\n\n",
			 Object_Name);
		}
		*/
		break;
	    case EVMS_Task_Slide:
		
		my_op_type = MY_OP_SLIDE;

		/*
		if (Option_Count > 0) {
		    My_Printf
			("This %s may be relocated on disk using the Slide command.\n",
			 Object_Name);
		    My_Printf
			("The following options are available for relocating this %s:\n",
			 Object_Name);
		} else {
		    My_Printf
			("This %s may be relocated on disk using the Slide command.\n\n",
			 Object_Name);
		}
		*/
		break;
	    case EVMS_Task_Move:

		my_op_type = MY_OP_MOVE;

		/*
		if (Option_Count > 0) {
		    My_Printf
			("The contents of this %s may be moved to another region using the\n",
			 Object_Name);
		    My_Printf("Move command.");
		    My_Printf
			("The following options are available for moving the contents of this %s:\n",
			 Object_Name);
		} else {
		    My_Printf
			("The contents of this %s may be moved to another storage object using the Move Command.\n\n",
			 Object_Name);
		}
		*/
		break;
	    default:
		My_Printf
		    ("Internal error: Possible memory corruption.\n\n");
		abort();
		break;
	    }

	    if(MY_OP_NULL != my_op_type)
	    {
		int op_pos;
	    	op_pos = MY_ADD_OPERATION(advance, my_op_type, Option_Count);

		// clear new operation flag after used.
		my_op_type = MY_OP_NULL;

		if(op_pos < 0)
		{
			abort();
		}

	    	if (Option_Count > 0) 
		{
			MY_ADD_OPTION_DESCRIPTORS(advance->supported_operations[op_pos]->option, Option_Descriptors, Option_Count);
			// Display_Option_Descriptors(Display_Default_Values);
	    	}
	    }


	}

	Error = Cleanup_Task();
    }

    return;

}


static char *Determine_Units(u_int64_t Value, double *Result)
{				/* Input value is in bytes. */
    char *Return_Value;
    unsigned int Count;

    *Result = Value;
    for (Count = 0; (*Result >= 1024) && (Count <= 5); Count++)
	*Result = *Result / 1024;

    switch (Count) {
    case 0:
	Return_Value = ByteStr;
	break;
    case 1:
	Return_Value = KilobyteStr;
	break;
    case 2:
	Return_Value = MegabyteStr;
	break;
    case 3:
	Return_Value = GigabyteStr;
	break;
    case 4:
	Return_Value = TerrabyteStr;
	break;
    default:
	Return_Value = PetabyteStr;
	break;
    }

    return Return_Value;
}

int SendObjectData(my_object_info_t * output)
{
    if(NULL == output)
    {
	    return  -1;
    }

    // MY_SEND_EVMS_OBJECT(&output);
    char buffer[NET_BUFFER_LEN + 1];
    int  len  = NET_BUFFER_LEN;

    if(0 == MY_PACK_EVMS_OBJECT(output, buffer, &len))
    {

	SendEvmsFrame(g_sock_fd, 
		     DEFAULT_RETCODE, 
		     buffer, 
		     len, 
		     EVMS_OBJECT_INFO);
    }


    if(output->basic)
    {
	    MY_FREE_BASIC_INFO_T(output->basic);
	    output->basic = NULL;
    }
    if(output->advance)
    {
	    MY_FREE_ADVANCE_INFO_T(output->advance);
	    output->advance = NULL;
    }

    return 0;
}

/* This will be changed in the future to allow easier integration of
   national language support                                             */
static int
Print_Object_Info(ADDRESS Object,
		  TAG ObjectTag, ADDRESS ObjectHandle, ADDRESS Parameters)
{
    handle_object_info_t *Object_Info = (handle_object_info_t *) Object;
    handle_object_info_t *Temp_Info = NULL;
    handle_array_t *Plugin_List = NULL;
    int Error = DLIST_SUCCESS;
    unsigned int Index;
    task_action_t Current_Task;
    char *Object_Created = NULL;
    char *Units;
    double Size;
    boolean Topmost;
    boolean Heading_Printed;

    /* 
     * web message handle variables
     * added by wwliu.
     */

    my_operation_type type = MY_OP_NULL;
    my_object_info_t output_info;

    my_advance_info_t * advance = NULL;

    my_operation_t * op = NULL;
    my_operation_t * one_operation = NULL;

    my_task_t * tasks = NULL;

    my_feature_t * features = NULL;

    memset(&output_info, 0, sizeof(my_object_info_t));

    /* Sanity Check */
    if (ObjectTag != HANDLE_INFO) {
	Report_Standard_Error(DLIST_ITEM_TAG_WRONG);
	abort();
    }

    if (Object == NULL) {

	Report_Standard_Error(DLIST_CORRUPTED);
	abort();
    }

    /* Lets print. */

    if(0 != MY_GET_BASIC_INFO(&output_info, Object_Info))
    {
	    printf("Calling GET_BASIC_INFO() error!\n");
	    return -1;
    }

    switch (Object_Info->type) {
    case PLUGIN:
	    /*
	My_Printf("Plug-in ID: %d\n"
		  "Full Name: %s\n"
		  "Short Name: %s\n"
		  "OEM: %s\n"
		  "Version %d.%d.%d\n\n",
		  Object_Info->info.plugin.id,
		  Object_Info->info.plugin.long_name,
		  Object_Info->info.plugin.short_name,
		  Object_Info->info.plugin.oem_name,
		  Object_Info->info.plugin.version.major,
		  Object_Info->info.plugin.version.minor,
		  Object_Info->info.plugin.version.patchlevel);
	  */

	if (List_Options) {

		MY_CREATE_ADVANCE_INFO_T(&advance);

	    /* We will be ignoring errors here since not all plug-ins support options or the task
	       interface.  Plug-ins which don't support the task interface will cause the EVMS
	       Engine to return an error for either evms_create_task or evms_get_option_count.    */

	    /* We have a plugin.  We must create a task to get its options. */

	    /* Since options are unique to each task, we must repeat this process for each task type
	       that we can get options for, which currently is limited to Create, Create Container,
	       and Assign Plugin.  We are limited to these tasks because the other tasks in the task
	       interface require an object to create the task, and since we don't have an object in
	       this case, we can not create the task and then query the available options.             */

	    for (Current_Task = EVMS_Task_Create;
		 Current_Task != EVMS_Task_Message;) {

		Error =
		    Setup_Task(Object_Info->info.plugin.handle,
			       Current_Task, FALSE);

		switch (Current_Task) {
		case EVMS_Task_Create:

		    /* Set up the next task */
		    Current_Task = EVMS_Task_Create_Container;
		    if (Error)
			break;

		    switch (GetPluginType(Object_Info->info.plugin.id)) {
		    case EVMS_DEVICE_MANAGER:
			Object_Created = "Logical Disks";
			break;
		    case EVMS_SEGMENT_MANAGER:
			Object_Created = "Segments";
			break;
		    case EVMS_REGION_MANAGER:
			Object_Created = "Regions";
			break;
		    case EVMS_FEATURE:
			Object_Created = "EVMS Objects";
			break;
		    case EVMS_ASSOCIATIVE_FEATURE:
			Object_Created = "EVMS Objects";
			break;
		    default:
			Object_Created = NULL;
		    }

		    if(Object_Created != NULL)
		    {
			    type = MY_OP_TASK_CREATE;

			    /*
		    	if (Option_Count > 0) {
			My_Printf("This plug-in is used to create %s.\n",
				  Object_Created);
			My_Printf
			    ("This plug-in supports the following options when creating %s:\n",
			     Object_Created);
		    } else if (Object_Created != NULL) {
			My_Printf
			    ("This plug-in can be used to create %s.\n",
			     Object_Created);
		    }
		    */
		    }

		    break;
		case EVMS_Task_Create_Container:

		    /* Set up the next task */
		    Current_Task = EVMS_Task_Assign_Plugin;
		    if (Error)
			break;

		    type = MY_OP_TASK_CREATE_CONTAINER;

		    /*
		    if (Option_Count > 0) {
			My_Printf
			    ("This plug-in can be used to create containers.\n");
			My_Printf
			    ("This plug-in, when used to create containers, supports the following options:\n");
		    } else {
			My_Printf
			    ("This plug-in can be used to create containers.\n");
		    }
		    */
		    break;

		case EVMS_Task_Assign_Plugin:

		    /* Set up the next task */
		    Current_Task = EVMS_Task_mkfs;
		    if (Error)
			break;

		    type = MY_OP_TASK_ASSIGN_PLUGIN;

		    /*
		    if (Option_Count > 0) {
			My_Printf
			    ("This plug-in implements one or more partitioning schemes.  "
			     "It can be used to subdivide the storage provided by a disk or segment.  "
			     "The assign command is used to associate this plug-in with a disk or segment.  "
			     "When being assigned to a disk or segment, this plug-in supports the following options:\n");
		    } else {
			My_Printf
			    ("This plugin implements one or more partitioning schemes.  "
			     "It can be used to subdivide a disk or segment.\n");
		    }
		    */
		    break;

		case EVMS_Task_mkfs:

		    /* Set up the next task */
		    Current_Task = EVMS_Task_fsck;
		    if (Error)
			break;

		    type = MY_OP_TASK_MKFS;

		    /*
		    if (Option_Count > 0) {
			My_Printf
			    ("This plug-in can be used to format a volume for use with the %s filesystem.  "
			     "When used to format a volume, this plug-in supports the following options:\n",
			     Object_Info->info.plugin.short_name);
		    } else {
			My_Printf
			    ("This plug-in can be used to format a volume for use with the %s filesystem.\n",
			     Object_Info->info.plugin.short_name);
		    }
		    */

		    break;

		case EVMS_Task_fsck:

		    /* Set up the next task */
		    Current_Task = EVMS_Task_Message;
		    if (Error)
			break;

		    type = MY_OP_TASK_FSCK;

		    /*
		    if (Option_Count > 0) {
			My_Printf
			    ("This plug-in can be used to check the integrity of the %s filesystem.  "
			     "When used in this fashion, this plug-in supports the following options:\n",
			     Object_Info->info.plugin.short_name);
		    } else {
			My_Printf
			    ("This plug-in can be used to check the integrity of the %s filesystem.\n",
			     Object_Info->info.plugin.short_name);
		    }
		    */

		    break;

		default:
		    My_Printf
			("Internal error: Possible memory corruption.\n");
		    abort();
		    break;
		}

		int op_pos = -1;
		if(MY_OP_NULL != type)
		{
			op_pos = MY_ADD_OPERATION(advance, type, Option_Count);

			// clear 'found flag' after using it.
			type = MY_OP_NULL;

			if(op_pos < 0)
			{
				abort();
			}
		}

		if ((!Error) && (Option_Count > 0)) {
		    /*
		    Display_Option_Descriptors(FALSE);
		    */

		    MY_ADD_OPTION_DESCRIPTORS(advance->supported_operations[op_pos]->option, Option_Descriptors, Option_Count);

		}

		Error = Cleanup_Task();
	    }

	    if (!Error)
	    {
		Display_Plugin_Tasks(Object_Info->info.plugin.handle, 
				    advance);
	    }

	}
	break;
    case VOLUME:
	/*
	Units =
	    Determine_Units(Object_Info->info.volume.vol_size * 512,
			    &Size);

	My_Printf("Volume Name: %s\n" "Major: %d\n" "Minor: %d\n"
		  "Active: %s\n" "Volume Size: %.2f %s\n"
		  "Minor Number: %d\n", Object_Info->info.volume.name,
		  Object_Info->info.volume.dev_major,
		  Object_Info->info.volume.dev_minor,
		  Object_Info->info.volume.
		  flags & VOLFLAG_ACTIVE ? "TRUE" : "FALSE", Size, Units,
		  Object_Info->info.volume.dev_minor);

	if (Object_Info->info.volume.mount_point != NULL) {
	    My_Printf("Mount Point: %s\n",
		      Object_Info->info.volume.mount_point);
	} else {
	    My_Printf("This volume is not mounted.\n");
	}
		  */


	if (List_Options) {

		MY_CREATE_ADVANCE_INFO_T(&advance);

		/*
	    My_Printf
		("The following commands may be used with this volume:\n");
		*/

	    if (Object_Info->info.volume.file_system_manager != 0) {
		    MY_ADD_OPERATION(advance, MY_OP_CHECK, 0);
		// My_Printf("     Check\n");
	    }

	    if (!evms_can_convert_to_evms_volume
		(Object_Info->info.volume.handle)) {
		    MY_ADD_OPERATION(advance, MY_OP_CONVERT, 0);
		// My_Printf("     Convert \n");
	    }

	    if (evms_can_delete(Object_Info->info.volume.handle) == 0) {
		    MY_ADD_OPERATION(advance, MY_OP_DELETE, 0);
		// My_Printf("     Delete\n");
	    }

	    if (evms_can_expand(Object_Info->info.volume.handle) == 0) {
		    MY_ADD_OPERATION(advance, MY_OP_EXPAND, 0);
		// My_Printf("     Expand\n");
	    }

	    // My_Printf("     Format\n");
	    MY_ADD_OPERATION(advance, MY_OP_FORMAT, 0);

	    if (!(Object_Info->info.volume.flags & VOLFLAG_COMPATIBILITY)) {
		    MY_ADD_OPERATION(advance, MY_OP_RENAME, 0);
		// My_Printf("     Rename\n");
	    }

	    if (evms_can_delete(Object_Info->info.volume.handle) == 0) {
		    MY_ADD_OPERATION(advance, MY_OP_REVERT, 0);
		// My_Printf("     Revert\n");
	    }

	    if (evms_can_shrink(Object_Info->info.volume.handle) == 0) {
		    MY_ADD_OPERATION(advance, MY_OP_SHRINK, 0);
		// My_Printf("     Shrink\n");
	    }

	    if (Object_Info->info.volume.file_system_manager != 0) {
		    MY_ADD_OPERATION(advance, MY_OP_UNFORMAT, 0);
		// My_Printf("     Unformat\n");
	    }

	    /* We need to see if any features can be added to this volume. */

	    /* Get a list of the available features. */
	    Error = evms_get_plugin_list(EVMS_FEATURE, 0, &Plugin_List);
	    if (!Error && (Plugin_List != NULL)) {
		Heading_Printed = FALSE;

		for (Index = 0; Index < Plugin_List->count; Index++) {
		    Error =
			evms_can_add_feature_to_volume(Object_Info->info.
						       volume.handle,
						       Plugin_List->
						       handle[Index]);

		    if (!Error) {
			if (!Heading_Printed) {
			    Heading_Printed = TRUE;
			}

			Error =
			    evms_get_info(Plugin_List->handle[Index],
					  &Temp_Info);
			if (!Error && (Temp_Info != NULL)) {
				MY_ADD_FEATURE(advance, Temp_Info->info.plugin.long_name);
				/*
			    My_Printf("     %s\n",
				      Temp_Info->info.plugin.long_name);
				      */
			    evms_free(Temp_Info);
			}

		    }

		}

		evms_free(Plugin_List);
	    }

	    // My_Printf("\n");

	}
	break;
    case EVMS_OBJECT:
	/*
	Units =
	    Determine_Units(Object_Info->info.object.size * 512, &Size);

	My_Printf("Object Name: %s\n" "Major: %d\n" "Minor: %d\n"
		  "Active: %s\n" "Object Size: %.2f %s\n\n",
		  Object_Info->info.object.name,
		  Object_Info->info.object.dev_major,
		  Object_Info->info.object.dev_minor,
		  Object_Info->info.object.
		  flags & SOFLAG_ACTIVE ? "TRUE" : "FALSE", Size, Units);
		  */

	if (List_Options) {

		MY_CREATE_ADVANCE_INFO_T(&advance);

	    Topmost = FALSE;
	    if (((Object_Info->info.object.parent_objects == NULL) ||
		 (Object_Info->info.object.parent_objects->count == 0)) &&
		(Object_Info->info.object.volume == 0))
		Topmost = TRUE;

	    if (Topmost) {

		/* Create Volume */
		MY_ADD_OPERATION(advance, MY_OP_TASK_CREATE_VOLUME, 0);

		/* Create Storage Object */
		MY_ADD_OPERATION(advance, MY_OP_TASK_CREATE_SO, 0);

		/* Delete */
		if (evms_can_delete(Object_Info->info.object.handle) == 0) {
		    MY_ADD_OPERATION(advance, MY_OP_DELETE, 0);
		}

		/* Revert */
		if (evms_can_delete(Object_Info->info.object.handle) == 0) {
		    MY_ADD_OPERATION(advance, MY_OP_REVERT, 0);
		}
	    }

	    /* Expand  Move Slide Shrink */
	    Print_Expand_Shrink_Move_Slide_Info(EVMS_Task_Set_Info,
						EVMS_Task_Move,
						"EVMS object",
						Object_Info->info.object.
						handle,
						advance);

	    if (evms_can_replace(Object_Info->info.object.handle, 0) == 0) {
		MY_ADD_OPERATION(advance, MY_OP_REPLACE, 0);
	    }

	    /* Check for plug-in defined tasks. */
	    Display_Plugin_Tasks(Object_Info->info.object.handle,
				advance);

	}
	break;
    case REGION:
	/*
	Units =
	    Determine_Units(Object_Info->info.region.size * 512, &Size);
	My_Printf("Region Name: %s\n" "Major: %d\n" "Minor: %d\n"
		  "Active: %s\n" "Region Size: %.2f %s\n" "Region Type: ",
		  Object_Info->info.region.name,
		  Object_Info->info.region.dev_major,
		  Object_Info->info.region.dev_minor,
		  Object_Info->info.region.
		  flags & SOFLAG_ACTIVE ? "TRUE" : "FALSE", Size, Units);
		  */

	switch (Object_Info->info.region.data_type) {
	case META_DATA_TYPE:
	    My_Printf("Metadata\n\n");
	    break;
	case DATA_TYPE:
	    My_Printf("Data\n\n");
	    break;
	default:
	    My_Printf("Freespace\n\n");
	    break;
	}

	if (List_Options) {

	    MY_CREATE_ADVANCE_INFO_T(&advance);

	    Topmost = FALSE;
	    if (((Object_Info->info.region.parent_objects == NULL) ||
		 (Object_Info->info.region.parent_objects->count == 0)) &&
		(Object_Info->info.region.volume == 0))
		Topmost = TRUE;

	    if (Topmost) {

		if (Object_Info->info.region.data_type == DATA_TYPE) {

		    /* Create Volume */
		    MY_ADD_OPERATION(advance, MY_OP_TASK_CREATE_VOLUME, 0);

		    /* Delete */
		    if (evms_can_delete(Object_Info->info.region.handle) ==0) 
		    {
		    	MY_ADD_OPERATION(advance, MY_OP_DELETE, 0);
		    }

		    /* Create Storage Object */
		    MY_ADD_OPERATION(advance, MY_OP_TASK_CREATE_SO, 0);

		    /* Create Container */
		    MY_ADD_OPERATION(advance, MY_OP_TASK_CREATE_CONTAINER, 0);

		}

		/* Create Region */
		MY_ADD_OPERATION(advance, MY_OP_TASK_CREATE_REGION, 0);

	    }

	    /* Expand Move Slide Shrink */
	    if (Object_Info->info.region.data_type == DATA_TYPE)
		Print_Expand_Shrink_Move_Slide_Info(EVMS_Task_Set_Info,
						    EVMS_Task_Move,
						    "region",
						    Object_Info->info.
						    region.handle,
						    advance);

	    if (evms_can_replace(Object_Info->info.object.handle, 0) == 0) 
	    {
		    MY_ADD_OPERATION(advance, MY_OP_REPLACE, 0);
	    }

	    /* Check for plug-in defined tasks. */
	    Display_Plugin_Tasks(Object_Info->info.region.handle,
				    advance);

	}
	break;
    case CONTAINER:
	/*
	Units =
	    Determine_Units(Object_Info->info.container.size * 512, &Size);
	if (Freespace_Not_Size) {	
		// Hack for Freespace filter.
	    My_Printf("Container Name: %s\n"
		      "Container freespace: %.2f %s\n\n",
		      Object_Info->info.container.name, Size, Units);
	} else {
	    My_Printf("Container Name: %s\n"
		      "Container Size: %.2f %s\n\n",
		      Object_Info->info.container.name, Size, Units);
	}
	*/

	if (List_Options) {
			
	    MY_CREATE_ADVANCE_INFO_T(&advance);

	    /* Delete */
	    if (evms_can_delete(Object_Info->info.container.handle) == 0) 
	    {
		    MY_ADD_OPERATION(advance, MY_OP_DELETE, 0);
	    }

	    /* Transfer */
	    Print_Expand_Shrink_Move_Slide_Info(EVMS_Task_Expand,
						EVMS_Task_Expand,
						"container",
						Object_Info->info.
						container.handle,
						advance);

	    /* Check for plug-in defined tasks. */
	    Display_Plugin_Tasks(Object_Info->info.container.handle,
				    advance);

	}
	break;
    case SEGMENT:
	/*
	Units =
	    Determine_Units(Object_Info->info.segment.size * 512, &Size);
	My_Printf("Segment Name: %s\n" "Major: %d\n" "Minor: %d\n"
		  "Active: %s\n" "Segment Size: %.2f %s\n"
		  "Starting LBA: %" PRIu64 "\n" "Segment Type: ",
		  Object_Info->info.segment.name,
		  Object_Info->info.segment.dev_major,
		  Object_Info->info.segment.dev_minor,
		  Object_Info->info.segment.
		  flags & SOFLAG_ACTIVE ? "TRUE" : "FALSE", Size, Units,
		  Object_Info->info.segment.start);
	switch (Object_Info->info.segment.data_type) {
	case META_DATA_TYPE:
	    My_Printf("Metadata\n\n");
	    break;
	case DATA_TYPE:
	    My_Printf("Data\n\n");
	    break;
	default:
	    My_Printf("Freespace\n\n");
	    break;
	}
		  */

	if (List_Options) {

	    MY_CREATE_ADVANCE_INFO_T(&advance);

	    Topmost = FALSE;
	    if (((Object_Info->info.segment.parent_objects == NULL) ||
		 (Object_Info->info.segment.parent_objects->count == 0)) &&
		(Object_Info->info.segment.volume == 0))
		Topmost = TRUE;

	    if (Topmost) {
		/* Create Volume */
		/* Create Region */
		/* Create Object */
		/* Create Container */
		if (Object_Info->info.segment.data_type == DATA_TYPE) 
		{
		    MY_ADD_OPERATION(advance, MY_OP_TASK_CREATE_VOLUME, 0);
		    MY_ADD_OPERATION(advance, MY_OP_TASK_CREATE_REGION, 0);
		    MY_ADD_OPERATION(advance, MY_OP_TASK_CREATE_CONTAINER, 0);
		    MY_ADD_OPERATION(advance, MY_OP_TASK_CREATE_SO, 0);

		    /* Delete */
		    if (evms_can_delete(Object_Info->info.segment.handle)
			== 0) 
		    {
		    	MY_ADD_OPERATION(advance, MY_OP_DELETE, 0);
		    }
		}

	    }

	    /* Create Segment */
	    /* Allocate */
	    if (Object_Info->info.segment.data_type == FREE_SPACE_TYPE) 
	    {
		MY_ADD_OPERATION(advance, MY_OP_ALLOCATE, 0);
		MY_ADD_OPERATION(advance, MY_OP_TASK_CREATE_SEGMENT, 0);
	    }

	    /* Remove Command ? */

	    if (Object_Info->info.segment.data_type == DATA_TYPE)
		Print_Expand_Shrink_Move_Slide_Info(EVMS_Task_Set_Info,
						    EVMS_Task_Move,
						    "segment",
						    Object_Info->info.
						    segment.handle,
						    advance);

	    if (evms_can_replace(Object_Info->info.object.handle, 0) == 0) 
	    {
		MY_ADD_OPERATION(advance, MY_OP_REPLACE, 0);
	    }

	    /* Check for plug-in defined tasks. */
	    Display_Plugin_Tasks(Object_Info->info.segment.handle,
				    advance);

	}
	break;
    case DISK:
	/*
	Units = Determine_Units(Object_Info->info.disk.size * 512, &Size);
	My_Printf("Logical Disk Name: %s\n"
		  "Major: %d\n"
		  "Minor: %d\n"
		  "Active: %s\n"
		  "Logical Disk Size: %.2f %s\n"
		  "Logical Disk Geometry: %" PRIu64
		  " Cylinders, %d Heads, %d Sectors per Track, %d Bytes per Sector\n"
		  "LBA of the 1024 Cylinder Limit: %" PRIu64 "\n\n",
		  Object_Info->info.disk.name,
		  Object_Info->info.disk.dev_major,
		  Object_Info->info.disk.dev_minor,
		  Object_Info->info.disk.
		  flags & SOFLAG_ACTIVE ? "TRUE" : "FALSE", Size, Units,
		  Object_Info->info.disk.geometry.cylinders,
		  Object_Info->info.disk.geometry.heads,
		  Object_Info->info.disk.geometry.sectors_per_track,
		  Object_Info->info.disk.geometry.bytes_per_sector,
		  Object_Info->info.disk.geometry.boot_cylinder_limit);
		  */

	if (List_Options) {

	    MY_CREATE_ADVANCE_INFO_T(&advance);

	    Topmost = FALSE;
	    if (((Object_Info->info.disk.parent_objects == NULL) ||
		 (Object_Info->info.disk.parent_objects->count == 0)) &&
		(Object_Info->info.disk.volume == 0))
		Topmost = TRUE;

	    if (Topmost) {
		/* Assign */
		/* Create Volume */
		/* Create Container */
		/* Create Region */
		/* Create Object */
		/* Assign segment manager */
		MY_ADD_OPERATION(advance, MY_OP_TASK_CREATE_VOLUME, 0);
		MY_ADD_OPERATION(advance, MY_OP_TASK_CREATE_CONTAINER, 0);
		MY_ADD_OPERATION(advance, MY_OP_TASK_CREATE_REGION, 0);
		MY_ADD_OPERATION(advance, MY_OP_TASK_CREATE_SO, 0);
		MY_ADD_OPERATION(advance, MY_OP_ASSIGN_SEGMENT_MANAGER, 0);
	    }

	    /* Remove Command ? */

	    Print_Expand_Shrink_Move_Slide_Info(EVMS_Task_Set_Info,
						EVMS_Task_Shrink,
						"logical disk",
						Object_Info->info.disk.
						handle,
						advance);

	    if (evms_can_replace(Object_Info->info.object.handle, 0) == 0) {
		MY_ADD_OPERATION(advance, MY_OP_REPLACE, 0);
	    }

	    /* Check for plug-in defined tasks. */
	    Display_Plugin_Tasks(Object_Info->info.disk.handle,
				    advance);

	}
	break;
    default:
	return DLIST_CORRUPTED;
    }


    output_info.advance = advance;

    if(0 != SendObjectData(&output_info))
    {
	    printf("Send my_output_info_t failed !\n");
    }

    /* Free the memory used by the Object_Info. */
    evms_free(Object_Info);

    return DLIST_SUCCESS;

}


static void Display_Plugin_Tasks(object_handle_t Handle, my_advance_info_t * advance)
{
    unsigned int Error;
    function_info_array_t *Available_Tasks = NULL;
    unsigned int Index;

    if(advance == NULL)
    {
	    return;
    }

    my_task_t * one_task = NULL;

    /* Check for plug-in defined tasks. */
    Error = evms_get_plugin_functions(Handle, &Available_Tasks);
    if ((!Error) && (Available_Tasks->count > 0)) {
	    /*
	My_Printf
	    ("The following plug-in defined tasks are available for this disk:\n");
	    */
	for (Index = 0; Index < Available_Tasks->count; Index++) {
		/*
	    My_Printf("Task Name:  %s\n",
		      Available_Tasks->info[Index].name);
	    My_Printf("Task Title: %s\n",
		      Available_Tasks->info[Index].title);
	    My_Printf("Task Description: %s\n\n",
		      Available_Tasks->info[Index].help);
		      */

	    Error =
		Setup_Task(Handle, Available_Tasks->info[Index].function,
			   FALSE);

	    one_task = NULL;
	    MY_CREATE_TASK_T(&one_task, Option_Count);
	    MY_COPY_FUNCTION_INFO(&(one_task->function), &(Available_Tasks->info[Index]));

	    if ((!Error) && (Option_Count > 0)) {
		    /*
		My_Printf
		    ("The following options are available for this task:\n");
		Display_Option_Descriptors(TRUE);
		*/

		MY_ADD_OPTION_DESCRIPTORS(&(one_task->option), Option_Descriptors, Option_Count);

		Error = Cleanup_Task();
	    }

	    MY_ADD_TASK(advance, one_task);
	}
    }

}


static void Display_Option_Descriptors(boolean Print_Default_Value)
{
    u_int32_t i;

    for (i = 0; i < Option_Count; i++) {
	My_Printf("Option Name: %s\n", Option_Descriptors[i]->name);
	My_Printf("Description: %s\n", Option_Descriptors[i]->tip);
	My_Printf("Option Type: ");
	switch (Option_Descriptors[i]->type) {
	case EVMS_Type_String:	/* char*     */
	    My_Printf("Character String\n");
	    break;
	case EVMS_Type_Boolean:	/* boolean   */
	    My_Printf("TRUE or FALSE\n");
	    break;
	case EVMS_Type_Char:	/* char      */
	    My_Printf("A single character\n");
	    break;
	case EVMS_Type_Unsigned_Char:	/* unsigned char */
	    My_Printf("A single character\n");
	    break;
	case EVMS_Type_Real32:	/* float     */
	    My_Printf("A 32 bit floating point number.\n");
	    break;
	case EVMS_Type_Real64:	/* double    */
	    My_Printf("A 64 bit floating point number.\n");
	    break;
	case EVMS_Type_Int:	/* int       */
	    My_Printf("An integer number.\n");
	    break;
	case EVMS_Type_Int8:	/* int8_t    */
	    My_Printf("An integer number.\n");
	    break;
	case EVMS_Type_Int16:	/* int16_t   */
	    My_Printf("An integer number.\n");
	    break;
	case EVMS_Type_Int32:	/* int32_t   */
	    My_Printf("An integer number.\n");
	    break;
	case EVMS_Type_Int64:	/* int64_t   */
	    My_Printf("An integer number.\n");
	    break;
	case EVMS_Type_Unsigned_Int:	/* uint      */
	    My_Printf("A positive integer number.\n");
	    break;
	case EVMS_Type_Unsigned_Int8:	/* u_int8_t  */
	    My_Printf("An 8 bit positive integer number.\n");
	    break;
	case EVMS_Type_Unsigned_Int16:	/* u_int16_t */
	    My_Printf("A 16 bit positive integer number.\n");
	    break;
	case EVMS_Type_Unsigned_Int32:	/* u_int32_t */
	    My_Printf("A 32 bit positive integer number.\n");
	    break;
	case EVMS_Type_Unsigned_Int64:	/* u_int64_t */
	    My_Printf("A 64 bit positive integer number.\n");
	    break;
	default:
	    My_Printf("Unknown.\n");
	    break;
	}

	if (Print_Default_Value) {

	    if (Option_Descriptors[i]->
		flags & EVMS_OPTION_FLAGS_NO_INITIAL_VALUE) {
		My_Printf
		    ("The current value for this option is not available.\n");
	    } else {
		My_Printf("The current value is: ");

		switch (Option_Descriptors[i]->type) {
		case EVMS_Type_String:	/* char*     */
		    My_Printf("%s\n", Option_Descriptors[i]->value.s);
		    break;
		case EVMS_Type_Boolean:	/* boolean   */
		    if (Option_Descriptors[i]->value.b) {
			My_Printf("TRUE\n");
		    } else {
			My_Printf("FALSE\n");
		    }
		    break;
		case EVMS_Type_Char:	/* char      */
		    My_Printf("%c\n", Option_Descriptors[i]->value.c);
		    break;
		case EVMS_Type_Unsigned_Char:	/* unsigned char */
		    My_Printf("%c\n", Option_Descriptors[i]->value.uc);
		    break;
		case EVMS_Type_Real32:	/* float     */
		    My_Printf("%gn", Option_Descriptors[i]->value.r32);
		    break;
		case EVMS_Type_Real64:	/* double    */
		    My_Printf("%gn", Option_Descriptors[i]->value.r64);
		    break;
		case EVMS_Type_Int:	/* int       */
		    My_Printf("%d\n", Option_Descriptors[i]->value.i);
		    break;
		case EVMS_Type_Int8:	/* int8_t    */
		    My_Printf("%d\n", Option_Descriptors[i]->value.i8);
		    break;
		case EVMS_Type_Int16:	/* int16_t   */
		    My_Printf("%d\n", Option_Descriptors[i]->value.i16);
		    break;
		case EVMS_Type_Int32:	/* int32_t   */
		    My_Printf("%d\n", Option_Descriptors[i]->value.i32);
		    break;
		case EVMS_Type_Int64:	/* int64_t   */
		    My_Printf("%" PRId64 "\n",
			      Option_Descriptors[i]->value.i64);
		    break;
		case EVMS_Type_Unsigned_Int:	/* uint      */
		    My_Printf("%u\n", Option_Descriptors[i]->value.ui);
		    break;
		case EVMS_Type_Unsigned_Int8:	/* u_int8_t  */
		    My_Printf("%u\n", Option_Descriptors[i]->value.ui8);
		    break;
		case EVMS_Type_Unsigned_Int16:	/* u_int16_t */
		    My_Printf("%u\n", Option_Descriptors[i]->value.ui16);
		    break;
		case EVMS_Type_Unsigned_Int32:	/* u_int32_t */
		    My_Printf("%u\n", Option_Descriptors[i]->value.ui32);
		    break;
		case EVMS_Type_Unsigned_Int64:	/* u_int64_t */
		    My_Printf("%" PRIu64 "\n",
			      Option_Descriptors[i]->value.ui64);
		    break;
		default:
		    My_Printf("Unknown!\n");
		    break;
		}

	    }

	}

    }

    if (Option_Count != 0) {
	My_Printf("\n");
    }

    return;
}


static void Print_Extended_Info(extended_info_t * Info)
{
    value_t *Current_Value = NULL;
    u_int Index;
    u_int Limit;
    u_int Indent = 0;
    boolean No_Conversion;


    My_Printf("\nField Name: %s\n"
	      "Title: %s\n"
	      "Description: %s\n", Info->name, Info->title, Info->desc);

    No_Conversion =
	((Info->flags & EVMS_EINFO_FLAGS_NO_UNIT_CONVERSION) ==
	 EVMS_EINFO_FLAGS_NO_UNIT_CONVERSION);

    if (Info->collection_type == EVMS_Collection_None) {
	Current_Value = &(Info->value);
	Limit = 1;
	Indent = 0;
	My_Printf("The value of this field is: ");
    } else if (Info->collection_type == EVMS_Collection_List) {
	if (Info->collection.list->count > 0) {
	    Current_Value = &(Info->collection.list->value[0]);
	    Limit = Info->collection.list->count;
	    Indent = 5;
	    My_Printf
		("This field represents a collection.  The items in the collection are:\n");
	} else {
	    Limit = 0;
	    My_Printf("There is no value for this field.\n");
	}

    } else {
	Limit = 0;
	My_Printf("The value for this field is a range.\n");
	My_Printf("The range starts at ");
	Print_Value(0,
		    No_Conversion,
		    Info->type,
		    Info->unit, Info->format,
		    &(Info->collection.range->min));
	My_Printf("\nThe range ends at ");
	Print_Value(0,
		    No_Conversion,
		    Info->type,
		    Info->unit, Info->format,
		    &(Info->collection.range->max));
	My_Printf("\nIn increments of ");
	Print_Value(0,
		    No_Conversion,
		    Info->type,
		    Info->unit,
		    Info->format, &(Info->collection.range->increment));
	My_Printf("\n");
    }

    for (Index = 0; Index < Limit; Index++) {
	Print_Value(Indent,
		    No_Conversion,
		    Info->type, Info->unit, Info->format, Current_Value);
	if ((Index + 1) < Limit) {
	    My_Printf(",\n");
	    Current_Value++;
	}
    }

    if ((Info->flags & EVMS_EINFO_FLAGS_MORE_INFO_AVAILABLE) ==
	EVMS_EINFO_FLAGS_MORE_INFO_AVAILABLE) {
	My_Printf("\nThis field has additional information available.  "
		  "To access this additional information, specify the name of this field as part of the extended info query command.\n");
    }

    My_Printf("\n");

    return;

}


static void Print_Units(value_unit_t Units)
{
    switch (Units) {
    case EVMS_Unit_None:
	break;
    case EVMS_Unit_Disks:
	My_Printf(" disks ");
	break;
    case EVMS_Unit_Sectors:
	My_Printf(" sectors ");
	break;
    case EVMS_Unit_Segments:
	My_Printf(" segments ");
	break;
    case EVMS_Unit_Regions:
	My_Printf(" regions ");
	break;
    case EVMS_Unit_Percent:
	My_Printf(" percent ");
	break;
    case EVMS_Unit_Milliseconds:
	My_Printf(" milliseconds ");
	break;
    case EVMS_Unit_Microseconds:
	My_Printf(" microseconds ");
	break;
    case EVMS_Unit_Bytes:
	My_Printf(" bytes ");
	break;
    case EVMS_Unit_Kilobytes:
	My_Printf(" KB ");
	break;
    case EVMS_Unit_Megabytes:
	My_Printf(" MB ");
	break;
    case EVMS_Unit_Gigabytes:
	My_Printf(" GB ");
	break;
    case EVMS_Unit_Terabytes:
	My_Printf(" TB ");
	break;
    case EVMS_Unit_Petabytes:
	My_Printf(" PB ");
	break;
    default:
	break;
    }

    return;
}


static void
Print_Value(u_int Indent,
	    boolean No_Conversions,
	    value_type_t Value_Type,
	    value_unit_t Units,
	    value_format_t Suggested_Format, value_t * Value)
{

    switch (Value_Type) {
    case EVMS_Type_String:
	My_Printf("%s", Value->s);
	Print_Units(Units);
	break;
    case EVMS_Type_Boolean:
	if (Value->b) {
	    My_Printf("True");
	} else {
	    My_Printf("False");
	}
	Print_Units(Units);
	break;
    case EVMS_Type_Char:
	My_Printf("%c", Value->c);
	Print_Units(Units);
	break;
    case EVMS_Type_Unsigned_Char:
	My_Printf("%uc", Value->uc);
	Print_Units(Units);
	break;
    case EVMS_Type_Real32:
	My_Printf("%e", Value->r32);
	Print_Units(Units);
	break;
    case EVMS_Type_Real64:
	My_Printf("%e", Value->r64);
	Print_Units(Units);
	break;
    case EVMS_Type_Int:
	if (Suggested_Format == EVMS_Format_Hex) {
	    My_Printf("%x", Value->i);
	} else {
	    My_Printf("%d", Value->i);
	}
	Print_Units(Units);
	break;
    case EVMS_Type_Int8:
	if (Suggested_Format == EVMS_Format_Hex) {
	    My_Printf("%x", Value->i8);
	} else {
	    My_Printf("%d", Value->i8);
	}
	Print_Units(Units);
	break;
    case EVMS_Type_Int16:
	if (Suggested_Format == EVMS_Format_Hex) {
	    My_Printf("%x", Value->i16);
	} else {
	    My_Printf("%d", Value->i16);
	}
	Print_Units(Units);
	break;
    case EVMS_Type_Int32:
	if (Suggested_Format == EVMS_Format_Hex) {
	    My_Printf("%x", Value->i32);
	} else {
	    My_Printf("%d", Value->i32);
	}
	Print_Units(Units);
	break;
    case EVMS_Type_Int64:
	if (Suggested_Format == EVMS_Format_Hex) {
	    My_Printf("%" PRIx64, Value->i64);
	} else {
	    My_Printf("%" PRId64, Value->i64);
	}
	Print_Units(Units);
	break;
    case EVMS_Type_Unsigned_Int:
	if (Suggested_Format == EVMS_Format_Hex) {
	    My_Printf("%x", Value->ui);
	} else {
	    My_Printf("%u", Value->ui);
	}
	Print_Units(Units);
	break;
    case EVMS_Type_Unsigned_Int8:
	if (Suggested_Format == EVMS_Format_Hex) {
	    My_Printf("%x", Value->ui8);
	} else {
	    My_Printf("%u", Value->ui8);
	}
	Print_Units(Units);
	break;
    case EVMS_Type_Unsigned_Int16:
	if (Suggested_Format == EVMS_Format_Hex) {
	    My_Printf("%x", Value->ui16);
	} else {
	    My_Printf("%u", Value->ui16);
	}
	Print_Units(Units);
	break;
    case EVMS_Type_Unsigned_Int32:
	if (Suggested_Format == EVMS_Format_Hex) {
	    My_Printf("%x", Value->ui32);
	} else {
	    My_Printf("%u", Value->ui32);
	}
	Print_Units(Units);
	break;
    case EVMS_Type_Unsigned_Int64:
	if (Suggested_Format == EVMS_Format_Hex) {
	    My_Printf("%" PRIx64, Value->ui64);
	} else {
	    My_Printf("%" PRIu64, Value->ui64);
	}
	Print_Units(Units);
	break;
    default:
	My_Printf
	    ("The data type for the value of this field in unknown.\n");
	break;			/* Keep the compiler happy. */
    }

    return;

}


static int Do_Volume_Filter(Executable_Node * Current_Node)
{
    dlist_t List_To_Process;
    object_handle_t Volume_Handle;
    int Error = 0;

    /* Remove the list object from the stack. */
    Pop(LIST, sizeof(dlist_t), &List_To_Process);

    /* Process Children */
    Error = ForEachItem(Current_Node->Children, Process_Node, NULL, TRUE);
    if (!Error) {
	/* There should be one value on the stack for use by this function.  That should be the
	   handle of the volume we are interested in.                                             */
	Pop(HANDLE, sizeof(object_handle_t), &Volume_Handle);

	Error =
	    PruneList(List_To_Process, Scan_For_Volumes, &Volume_Handle);
	if (!Error) {
	    /* Push List_To_Process onto the stack so that other filters may process it. */
	    Push(LIST, sizeof(dlist_t), &List_To_Process);
	}

    }

    return Error;
}

static boolean
Scan_For_Volumes(ADDRESS Object,
		 TAG ObjectTag,
		 ADDRESS ObjectHandle,
		 ADDRESS Parameters, boolean * FreeMemory, uint * Error)
{
    object_handle_t *Volume_Handle = (object_handle_t *) Parameters;
    handle_object_info_t *Object_Info = (handle_object_info_t *) Object;
    boolean Return_Value = FALSE;
    handle_array_t *Plugin_Array;
    unsigned int I;

    /* Assume success */
    *Error = DLIST_SUCCESS;

    /* When we delete an item from the list, we don't want its memory freed. */
    *FreeMemory = FALSE;

    /* Sanity Check */
    if (ObjectTag != HANDLE_INFO) {
	Report_Standard_Error(DLIST_ITEM_TAG_WRONG);
	abort();
    }

    if ((Volume_Handle == NULL) || (Object == NULL)) {

	Report_Standard_Error(DLIST_CORRUPTED);
	abort();
    }

    /* Is the current object acceptable?  */
    switch (Object_Info->type) {
    case PLUGIN:
	/* Is this plugin used in the construction of volume? */

	/* Get a list of the plugins used to create the volume. */
	*Error = evms_get_feature_list(*Volume_Handle, &Plugin_Array);
	if (!*Error) {
	    /* Now search the list to see if the plugin of interest is there. */
	    Return_Value = TRUE;	/* Assume it is not. */
	    for (I = 0; I < Plugin_Array->count; I++) {
		if (Plugin_Array->handle[I] ==
		    Object_Info->info.plugin.handle) {
		    Return_Value = FALSE;
		    break;
		}

	    }

	}
	break;
    case VOLUME:
	if (Object_Info->info.volume.handle == *Volume_Handle)
	    Return_Value = FALSE;
	else
	    Return_Value = TRUE;
	break;
    case EVMS_OBJECT:
	if (Object_Info->info.object.volume == *Volume_Handle)
	    Return_Value = FALSE;
	else
	    Return_Value = TRUE;
	break;
    case REGION:
	if (Object_Info->info.region.volume == *Volume_Handle)
	    Return_Value = FALSE;
	else
	    Return_Value = TRUE;
	break;
    default:
	Return_Value = TRUE;
	break;
    }

    if (Return_Value) {
	evms_free(Object_Info);
    }

    return Return_Value;

}


static int Do_Convertible_Filter(Executable_Node * Current_Node)
{

    dlist_t List_To_Process;
    int Error = 0;

    /* Remove the list object from the stack. */
    Pop(LIST, sizeof(dlist_t), &List_To_Process);

    Error = PruneList(List_To_Process, Scan_For_Convertible, NULL);
    if (!Error) {
	/* Push List_To_Process onto the stack so that other filters may process it. */
	Push(LIST, sizeof(dlist_t), &List_To_Process);
    }

    return Error;

}


static boolean
Scan_For_Convertible(ADDRESS Object,
		     TAG ObjectTag,
		     ADDRESS ObjectHandle,
		     ADDRESS Parameters, boolean * FreeMemory,
		     uint * Error)
{
    handle_object_info_t *Object_Info = (handle_object_info_t *) Object;

    /* Assume success */
    *Error = DLIST_SUCCESS;

    /* When we delete an item from the list, we don't want its memory freed. */
    *FreeMemory = FALSE;

    /* Sanity Check */
    if (ObjectTag != HANDLE_INFO) {
	Report_Standard_Error(DLIST_ITEM_TAG_WRONG);
	abort();
    }

    if (Object == NULL) {

	Report_Standard_Error(DLIST_CORRUPTED);
	abort();

    }

    /* Is the current object acceptable?  If so, is it convertible to an EVMS native volume?  */
    if (Object_Info->type != VOLUME)
	return FALSE;

    return evms_can_convert_to_evms_volume(Object_Info->info.volume.
					   handle);

}


static int Do_List_Options_Filter(Executable_Node * Current_Node)
{

    /* Turn on the List_Options flag. */
    List_Options = TRUE;

    return 0;

}


static int Do_Plugin_Filter(Executable_Node * Current_Node)
{
    dlist_t List_To_Process;
    object_handle_t Handle;
    int Error = 0;

    /* Remove the list object from the stack. */
    Pop(LIST, sizeof(dlist_t), &List_To_Process);

    /* Process Children */
    Error = ForEachItem(Current_Node->Children, Process_Node, NULL, TRUE);
    if (!Error) {
	/* There should be one value on the stack for use by this function.  That should be the
	   handle of the plugin we are interested in.                                             */
	Pop(HANDLE, sizeof(object_handle_t), &Handle);

	Error = PruneList(List_To_Process, Scan_For_Plugin, &Handle);
	if (!Error) {
	    /* Push List_To_Process onto the stack so that other filters may process it. */
	    Push(LIST, sizeof(dlist_t), &List_To_Process);
	}

    }

    return Error;
}


static boolean
Scan_For_Plugin(ADDRESS Object,
		TAG ObjectTag,
		ADDRESS ObjectHandle,
		ADDRESS Parameters, boolean * FreeMemory, uint * Error)
{
    object_handle_t *Plugin_Handle = (object_handle_t *) Parameters;
    handle_object_info_t *Object_Info = (handle_object_info_t *) Object;
    boolean Return_Value = FALSE;
    handle_array_t *Plugin_Array;
    unsigned int I;

    /* Assume success */
    *Error = DLIST_SUCCESS;

    /* When we delete an item from the list, we don't want its memory freed. */
    *FreeMemory = FALSE;

    /* Sanity Check */
    if (ObjectTag != HANDLE_INFO) {
	Report_Standard_Error(DLIST_ITEM_TAG_WRONG);
	abort();
    }

    if ((Plugin_Handle == NULL) || (Object == NULL)) {

	Report_Standard_Error(DLIST_CORRUPTED);
	abort();
    }

    /* Is the current object acceptable?  */
    switch (Object_Info->type) {
    case PLUGIN:
	/* Is this plugin a match for the one specified? */
	if (Object_Info->info.plugin.handle == *Plugin_Handle)
	    return FALSE;
	else
	    return TRUE;
	break;
    case VOLUME:
	/* Is the specified plugin used in the construction of this volume? */

	/* Get a list of the plugins used to create the volume. */
	*Error =
	    evms_get_feature_list(Object_Info->info.volume.handle,
				  &Plugin_Array);
	if (!*Error) {
	    /* Now search the list to see if the plugin of interest is there. */
	    Return_Value = TRUE;	/* Assume it is not. */
	    for (I = 0; I < Plugin_Array->count; I++) {
		if (Plugin_Array->handle[I] == *Plugin_Handle) {
		    Return_Value = FALSE;
		    break;
		}

	    }

	}
	break;
    case EVMS_OBJECT:
	if (Object_Info->info.object.plugin == *Plugin_Handle)
	    Return_Value = FALSE;
	else
	    Return_Value = TRUE;
	break;
    case REGION:
	if (Object_Info->info.region.plugin == *Plugin_Handle)
	    Return_Value = FALSE;
	else
	    Return_Value = TRUE;
	break;
    case SEGMENT:
	if (Object_Info->info.segment.plugin == *Plugin_Handle)
	    Return_Value = FALSE;
	else
	    Return_Value = TRUE;
	break;
    case DISK:
	if (Object_Info->info.disk.plugin == *Plugin_Handle)
	    Return_Value = FALSE;
	else
	    Return_Value = TRUE;
	break;
    case CONTAINER:
	if (Object_Info->info.container.plugin == *Plugin_Handle)
	    Return_Value = FALSE;
	else
	    Return_Value = TRUE;
	break;
    default:
	Return_Value = FALSE;
	break;
    }

    if (Return_Value) {
	evms_free(Object_Info);
    }

    return Return_Value;

}


static int Do_Object_Filter(Executable_Node * Current_Node)
{
    dlist_t List_To_Process;
    object_handle_t Handle;
    handle_object_info_t *Object_Info;
    int Error = 0;

    /* Remove the list object from the stack. */
    Pop(LIST, sizeof(dlist_t), &List_To_Process);

    /* Process Children */
    Error = ForEachItem(Current_Node->Children, Process_Node, NULL, TRUE);
    if (!Error) {
	/* There should be one value on the stack for use by this function.  That should be the
	   handle of the storage object we are interested in.                                             */
	Pop(HANDLE, sizeof(object_handle_t), &Handle);

	/* Now we must get the information related to the object whose handle we have. */
	Error = evms_get_info(Handle, &Object_Info);
	if (!Error) {
	    Error =
		PruneList(List_To_Process, Scan_For_Objects, Object_Info);
	    if (!Error) {
		/* Push List_To_Process onto the stack so that other filters may process it. */
		Push(LIST, sizeof(dlist_t), &List_To_Process);
	    }

	} else
	    Report_Standard_Error(Error);

    }

    return Error;
}

static boolean
Scan_For_Objects(ADDRESS Object,
		 TAG ObjectTag,
		 ADDRESS ObjectHandle,
		 ADDRESS Parameters, boolean * FreeMemory, uint * Error)
{
    handle_object_info_t *Parameter_Info =
	(handle_object_info_t *) Parameters;
    handle_object_info_t *Object_Info = (handle_object_info_t *) Object;
    boolean Return_Value = FALSE;
    handle_array_t *Plugin_Array;
    unsigned int I;

    /* Assume success */
    *Error = DLIST_SUCCESS;

    /* When we delete an item from the list, we don't want its memory freed. */
    *FreeMemory = FALSE;

    /* Sanity Check */
    if (ObjectTag != HANDLE_INFO) {
	Report_Standard_Error(DLIST_ITEM_TAG_WRONG);
	abort();
    }

    if ((Parameters == NULL) || (Object == NULL)) {

	Report_Standard_Error(DLIST_CORRUPTED);
	abort();
    }

    /* Is the current object acceptable?  */
    switch (Object_Info->type) {
    case PLUGIN:
	/* Does some component of the object use this plugin? */

	/* Get a list of the plugins used to create the object. */
	*Error =
	    evms_get_feature_list(Parameter_Info->info.object.handle,
				  &Plugin_Array);
	if (!*Error) {
	    /* Now search the list to see if the plugin of interest is there. */
	    Return_Value = TRUE;	/* Assume it is not. */
	    for (I = 0; I < Plugin_Array->count; I++) {
		if (Plugin_Array->handle[I] ==
		    Object_Info->info.plugin.handle) {
		    Return_Value = FALSE;
		    break;
		}

	    }

	}
	break;
    case VOLUME:
	/* Does this volume contain this object */
	if (Parameter_Info->info.object.volume ==
	    Object_Info->info.volume.handle)
	    Return_Value = FALSE;
	else
	    Return_Value = TRUE;
	break;
    case EVMS_OBJECT:
	if (Object_Info->info.object.handle ==
	    Parameter_Info->info.object.handle)
	    Return_Value = FALSE;
	else
	    Return_Value = TRUE;
	break;
    case REGION:
	/* Is the storage region part of the object */
	if (Object_Info->info.region.handle ==
	    Parameter_Info->info.object.handle)
	    Return_Value = FALSE;
	else
	    Return_Value =
		Check_Child_For_Region(Parameter_Info->info.object.handle,
				       Object_Info->info.region.handle,
				       Error);
	break;
    case SEGMENT:
	if (Object_Info->info.segment.handle ==
	    Parameter_Info->info.object.handle)
	    Return_Value = FALSE;
	else
	    Return_Value =
		Check_Child_For_Segment(Parameter_Info->info.object.handle,
					Object_Info->info.region.handle,
					Error);
	break;
    case DISK:
	if (Parameter_Info->info.object.handle ==
	    Object_Info->info.disk.handle)
	    Return_Value = FALSE;
	else
	    Return_Value = TRUE;
	break;
    default:
	Return_Value = FALSE;
	break;
    }

    if (Return_Value) {
	evms_free(Object_Info);
    }

    return Return_Value;

}


static int Do_Container_Filter(Executable_Node * Current_Node)
{

    dlist_t List_To_Process;
    object_handle_t Handle;
    handle_object_info_t *Container_Info;
    int Error = 0;

    /* Remove the list object from the stack. */
    Pop(LIST, sizeof(dlist_t), &List_To_Process);

    /* Process Children */
    Error = ForEachItem(Current_Node->Children, Process_Node, NULL, TRUE);
    if (!Error) {
	/* There should be one value on the stack for use by this function.  That should be the
	   handle of the container we are interested in.                                             */
	Pop(HANDLE, sizeof(object_handle_t), &Handle);

	/* Now we must get the information related to the container whose handle we have. */
	Error = evms_get_info(Handle, &Container_Info);
	if (!Error) {
	    Error =
		PruneList(List_To_Process, Scan_For_Containers,
			  Container_Info);
	    if (!Error) {
		/* Push List_To_Process onto the stack so that other filters may process it. */
		Push(LIST, sizeof(dlist_t), &List_To_Process);
	    }

	} else
	    Report_Standard_Error(Error);

    }

    return Error;
}

static boolean
Scan_For_Containers(ADDRESS Object,
		    TAG ObjectTag,
		    ADDRESS ObjectHandle,
		    ADDRESS Parameters, boolean * FreeMemory, uint * Error)
{
    handle_object_info_t *Container_Info =
	(handle_object_info_t *) Parameters;
    handle_object_info_t *Object_Info = (handle_object_info_t *) Object;
    boolean Return_Value = FALSE;

    /* Assume success */
    *Error = DLIST_SUCCESS;

    /* When we delete an item from the list, we don't want its memory freed. */
    *FreeMemory = FALSE;

    /* Sanity Check */
    if (ObjectTag != HANDLE_INFO) {
	Report_Standard_Error(DLIST_ITEM_TAG_WRONG);
	abort();
    }

    if ((Container_Info == NULL) || (Object == NULL)) {

	Report_Standard_Error(DLIST_CORRUPTED);
	abort();
    }

    /* Is the current object acceptable?  */
    switch (Object_Info->type) {
    case PLUGIN:
	/* We must check to see if the current plugin is used for the container. */
	if (Object_Info->info.plugin.handle ==
	    Container_Info->info.container.plugin)
	    Return_Value = FALSE;
	else
	    Return_Value = TRUE;
	break;
    case VOLUME:
	Return_Value =
	    Check_Child_For_Container(Object_Info->info.volume.object,
				      Container_Info->info.container.
				      handle, Error);
	break;
    case EVMS_OBJECT:
	if (Object_Info->info.object.producing_container ==
	    Container_Info->info.container.handle)
	    Return_Value = FALSE;
	else
	    Return_Value = TRUE;
	break;
    case REGION:
	if (Object_Info->info.region.producing_container ==
	    Container_Info->info.container.handle)
	    Return_Value = FALSE;
	else
	    Return_Value = TRUE;
	break;
    case CONTAINER:
	if (Object_Info->info.container.handle ==
	    Container_Info->info.container.handle)
	    Return_Value = FALSE;
	else
	    Return_Value = TRUE;
	break;
    case SEGMENT:
	if (Object_Info->info.segment.consuming_container ==
	    Container_Info->info.container.handle)
	    Return_Value = FALSE;
	else
	    Return_Value = TRUE;
	break;
    case DISK:
	if (Object_Info->info.disk.consuming_container ==
	    Container_Info->info.container.handle)
	    Return_Value = FALSE;
	else
	    Return_Value = TRUE;
	break;
    default:
	Return_Value = FALSE;
	break;
    }

    if (Return_Value) {
	evms_free(Object_Info);
    }

    return Return_Value;

}


static int Do_Region_Filter(Executable_Node * Current_Node)
{

    dlist_t List_To_Process;
    object_handle_t Handle;
    handle_object_info_t *Region_Info;
    int Error = 0;

    /* Remove the list object from the stack. */
    Pop(LIST, sizeof(dlist_t), &List_To_Process);

    /* Process Children */
    Error = ForEachItem(Current_Node->Children, Process_Node, NULL, TRUE);
    if (!Error) {
	/* There should be one value on the stack for use by this function.  That should be the
	   handle of the storage region we are interested in.                                             */
	Pop(HANDLE, sizeof(object_handle_t), &Handle);
	if (!Error) {
	    /* Now we must get the information related to the region whose handle we have. */
	    Error = evms_get_info(Handle, &Region_Info);
	    if (!Error) {
		Error =
		    PruneList(List_To_Process, Scan_For_Regions,
			      Region_Info);
		if (!Error) {
		    /* Push List_To_Process onto the stack so that other filters may process it. */
		    Push(LIST, sizeof(dlist_t), &List_To_Process);
		}

	    } else
		Report_Standard_Error(Error);

	}

    }

    return Error;
}

static boolean
Scan_For_Regions(ADDRESS Object,
		 TAG ObjectTag,
		 ADDRESS ObjectHandle,
		 ADDRESS Parameters, boolean * FreeMemory, uint * Error)
{
    handle_object_info_t *Region_Info =
	(handle_object_info_t *) Parameters;
    handle_object_info_t *Object_Info = (handle_object_info_t *) Object;
    unsigned int I;
    boolean Return_Value = FALSE;

    /* Assume success */
    *Error = DLIST_SUCCESS;

    /* When we delete an item from the list, we don't want its memory freed. */
    *FreeMemory = FALSE;

    /* Sanity Check */
    if (ObjectTag != HANDLE_INFO) {
	Report_Standard_Error(DLIST_ITEM_TAG_WRONG);
	abort();
    }

    if ((Region_Info == NULL) || (Object == NULL)) {

	Report_Standard_Error(DLIST_CORRUPTED);
	abort();

    }

    /* Is the current object acceptable?  */
    switch (Object_Info->type) {
    case VOLUME:
	/* We must check each region in the Logical Volume to see if any match. */
	if (Object_Info->info.volume.handle ==
	    Region_Info->info.region.volume)
	    Return_Value = FALSE;
	else {
	    Return_Value =
		Check_Child_For_Region(Object_Info->info.volume.object,
				       Region_Info->info.region.handle,
				       Error);
	}
	break;
    case EVMS_OBJECT:
	if (Object_Info->info.object.handle ==
	    Region_Info->info.region.handle)
	    Return_Value = FALSE;
	else {
	    if ((Object_Info->info.object.child_objects == NULL) ||
		(Object_Info->info.object.child_objects->count == 0))
		Return_Value = TRUE;
	    else
		for (I = 0, Return_Value = TRUE;
		     (I < Object_Info->info.object.child_objects->count)
		     && Return_Value; I++) {
		    Return_Value =
			Check_Child_For_Region(Object_Info->info.object.
					       child_objects->handle[I],
					       Region_Info->info.region.
					       handle, Error);
		}

	}
	break;
    case REGION:
	if (Object_Info->info.region.handle ==
	    Region_Info->info.region.handle)
	    Return_Value = FALSE;
	else
	    Return_Value = TRUE;
	break;
    case CONTAINER:
	if (Region_Info->info.region.producing_container ==
	    Object_Info->info.container.handle)
	    Return_Value = FALSE;
	else
	    Return_Value = TRUE;
	break;
    default:
	Return_Value = FALSE;
	break;
    }

    if (Return_Value) {
	evms_free(Object_Info);
    }

    return Return_Value;

}

static int Do_Plugin_Mask(Executable_Node * Current_Node)
{
    plugin_id_t Mask = EVMS_NO_PLUGIN;

    /* Build the plugin id mask from the node value. */
    if (Current_Node->NodeValue == DeviceStr) {
	Mask = (plugin_id_t) EVMS_DEVICE_MANAGER;
    } else if (Current_Node->NodeValue == RegionStr) {
	Mask = (plugin_id_t) EVMS_REGION_MANAGER;
    } else if (Current_Node->NodeValue == SegmentStr) {
	Mask = (plugin_id_t) EVMS_SEGMENT_MANAGER;
    } else if (Current_Node->NodeValue == FeatureStr) {
	Mask = (plugin_id_t) EVMS_FEATURE;
    } else if (Current_Node->NodeValue == FSIMStr) {
	Mask = (plugin_id_t) EVMS_FILESYSTEM_INTERFACE_MODULE;
    }

    Push(PLUGIN_ID_MASK, sizeof(plugin_id_t), &Mask);

    return 0;

}

static int Do_Plugin_Type_Filter(Executable_Node * Current_Node)
{

    dlist_t List_To_Process;
    plugin_id_t Mask = 0;
    int Error = 0;

    /* Remove the list object from the stack. */
    Pop(LIST, sizeof(dlist_t), &List_To_Process);

    /* Process Children */
    Error = ForEachItem(Current_Node->Children, Process_Node, NULL, TRUE);
    if (!Error) {
	/* There should be one value on the stack for use by this function.  That should be the
	   Plug-in mask to use when deciding which plug-in we should return info on.              */
	Pop(PLUGIN_ID_MASK, sizeof(plugin_id_t), &Mask);
	if (!Error) {

	    Error = PruneList(List_To_Process, Scan_For_Plugins, &Mask);
	    if (!Error) {
		/* Push List_To_Process onto the stack so that other filters may process it. */
		Push(LIST, sizeof(dlist_t), &List_To_Process);
	    }

	}

    }

    return Error;

}

static boolean
Scan_For_Plugins(ADDRESS Object,
		 TAG ObjectTag,
		 ADDRESS ObjectHandle,
		 ADDRESS Parameters, boolean * FreeMemory, uint * Error)
{
    plugin_id_t *Mask = (plugin_id_t *) Parameters;
    handle_object_info_t *Object_Info = (handle_object_info_t *) Object;
    boolean Return_Value = FALSE;

    /* Assume success */
    *Error = DLIST_SUCCESS;

    /* When we delete an item from the list, we don't want its memory freed. */
    *FreeMemory = FALSE;

    /* Sanity Check */
    if (ObjectTag != HANDLE_INFO) {
	Report_Standard_Error(DLIST_ITEM_TAG_WRONG);
	abort();

    }

    if ((Parameters == NULL) || (Object == NULL)) {

	Report_Standard_Error(DLIST_CORRUPTED);
	abort();
    }

    /* Is the current object acceptable?  */
    switch (Object_Info->type) {
    case PLUGIN:
	if (((*Mask == EVMS_FEATURE) &&
	     ((GetPluginType(Object_Info->info.plugin.id) == EVMS_FEATURE)
	      || (GetPluginType(Object_Info->info.plugin.id) ==
		  EVMS_ASSOCIATIVE_FEATURE)))
	    || (GetPluginType(Object_Info->info.plugin.id) == *Mask))
	    Return_Value = FALSE;
	else
	    Return_Value = TRUE;
	break;
    default:
	Return_Value = FALSE;
	break;
    }

    if (Return_Value) {
	evms_free(Object_Info);
    }

    return Return_Value;

}


static int Do_Disk_Filter(Executable_Node * Current_Node)
{

    dlist_t List_To_Process;
    object_handle_t Disk_Handle;
    int Error = 0;

    /* Remove the list object from the stack. */
    Pop(LIST, sizeof(dlist_t), &List_To_Process);

    /* Process Children */
    Error = ForEachItem(Current_Node->Children, Process_Node, NULL, TRUE);
    if (!Error) {
	/* There should be one value on the stack for use by this function.  That should be
	   the handle of the disk we are interested in.  We will only output items which
	   reside on this disk.                                                              */
	Pop(HANDLE, sizeof(object_handle_t), &Disk_Handle);

	Error = PruneList(List_To_Process, Scan_For_Disks, &Disk_Handle);
	if (!Error) {
	    /* Push List_To_Process onto the stack so that other filters may process it. */
	    Push(LIST, sizeof(dlist_t), &List_To_Process);
	}

    }

    return Error;

}

static boolean
Scan_For_Disks(ADDRESS Object,
	       TAG ObjectTag,
	       ADDRESS ObjectHandle,
	       ADDRESS Parameters, boolean * FreeMemory, uint * Error)
{
    object_handle_t *Disk_Handle = (object_handle_t *) Parameters;
    handle_object_info_t *Object_Info = (handle_object_info_t *) Object;
    unsigned int I;
    boolean Return_Value = FALSE;

    /* Assume success */
    *Error = DLIST_SUCCESS;

    /* When we delete an item from the list, we don't want its memory freed. */
    *FreeMemory = FALSE;

    /* Sanity Check */
    if (ObjectTag != HANDLE_INFO) {
	Report_Standard_Error(DLIST_ITEM_TAG_WRONG);
	abort();
    }

    if (Object == NULL) {
	Report_Standard_Error(DLIST_CORRUPTED);
	abort();

    }

    /* Is the current object acceptable?  */
    switch (Object_Info->type) {
    case SEGMENT:
	if (Object_Info->info.segment.handle == *Disk_Handle)
	    Return_Value = FALSE;
	else {
	    /* We could have a segment nested inside of another segment.  Recurse through its children to find out. */
	    if ((Object_Info->info.segment.child_objects == NULL) ||
		(Object_Info->info.segment.child_objects->count == 0))
		Return_Value = TRUE;
	    else {

		for (I = 0, Return_Value = TRUE;
		     (I < Object_Info->info.segment.child_objects->count)
		     && Return_Value; I++) {
		    if (Object_Info->info.segment.child_objects->
			handle[I] == *Disk_Handle)
			Return_Value = FALSE;
		    else
			Return_Value =
			    Check_Child_For_Disk(Object_Info->info.segment.
						 child_objects->handle[I],
						 *Disk_Handle, Error);
		}

	    }

	}
	break;
    case DISK:
	if (Object_Info->info.disk.handle == *Disk_Handle)
	    Return_Value = FALSE;
	else
	    Return_Value = TRUE;
	break;
    default:
	Return_Value = FALSE;
	break;
    }

    if (Return_Value) {
	evms_free(Object_Info);
    }

    return Return_Value;

}


static int Do_Segment_Filter(Executable_Node * Current_Node)
{

    dlist_t List_To_Process;
    object_handle_t Segment_Handle;
    int Error = 0;

    /* Remove the list object from the stack. */
    Pop(LIST, sizeof(dlist_t), &List_To_Process);

    /* Process Children */
    Error = ForEachItem(Current_Node->Children, Process_Node, NULL, TRUE);
    if (!Error) {
	/* There should be one value on the stack for use by this function.  That should be
	   the handle of the segment we are interested in.  We will only output items which
	   use this segment.                                                                 */
	Pop(HANDLE, sizeof(object_handle_t), &Segment_Handle);

	Error =
	    PruneList(List_To_Process, Scan_For_Segments, &Segment_Handle);
	if (!Error) {
	    /* Push List_To_Process onto the stack so that other filters may process it. */
	    Push(LIST, sizeof(dlist_t), &List_To_Process);
	}

    }

    return Error;

}


static boolean
Scan_For_Segments(ADDRESS Object,
		  TAG ObjectTag,
		  ADDRESS ObjectHandle,
		  ADDRESS Parameters, boolean * FreeMemory, uint * Error)
{
    object_handle_t *Segment_Handle = (object_handle_t *) Parameters;
    handle_object_info_t *Object_Info = (handle_object_info_t *) Object;
    unsigned int I;
    boolean Return_Value = FALSE;

    /* Assume success */
    *Error = DLIST_SUCCESS;

    /* When we delete an item from the list, we don't want its memory freed. */
    *FreeMemory = FALSE;

    /* Sanity Check */
    if (ObjectTag != HANDLE_INFO) {
	Report_Standard_Error(DLIST_ITEM_TAG_WRONG);
	abort();
    }

    if (Object == NULL) {
	Report_Standard_Error(DLIST_CORRUPTED);
	abort();

    }

    /* Is the current object acceptable?  */
    switch (Object_Info->type) {
    case SEGMENT:
	if (Object_Info->info.segment.handle == *Segment_Handle)
	    Return_Value = FALSE;
	else {
	    /* We could have a segment nested inside of another segment.  Recurse through its children to find out. */
	    if ((Object_Info->info.segment.child_objects == NULL) ||
		(Object_Info->info.segment.child_objects->count == 0))
		Return_Value = TRUE;
	    else {

		for (I = 0, Return_Value = TRUE;
		     (I < Object_Info->info.segment.child_objects->count)
		     && Return_Value; I++) {
		    if (Object_Info->info.segment.child_objects->
			handle[I] == *Segment_Handle)
			Return_Value = FALSE;
		    else
			Return_Value =
			    Check_Child_For_Disk(Object_Info->info.segment.
						 child_objects->handle[I],
						 *Segment_Handle, Error);
		}

	    }

	}
	break;
    case DISK:
	if (Object_Info->info.disk.parent_objects != NULL) {
	    /* See if this disk produced the segment we are interested in. */
	    for (I = 0, Return_Value = TRUE;
		 (I < Object_Info->info.disk.parent_objects->count)
		 && Return_Value; I++) {
		if (Object_Info->info.disk.parent_objects->handle[I] ==
		    *Segment_Handle)
		    Return_Value = FALSE;
	    }
	} else
	    Return_Value = TRUE;
	break;
    case CONTAINER:
	/* See if this container is using the segment we are interested in. */
	if (Object_Info->info.container.objects_consumed != NULL) {
	    for (I = 0, Return_Value = TRUE;
		 (I < Object_Info->info.container.objects_consumed->count)
		 && Return_Value; I++) {
		if (Object_Info->info.container.objects_consumed->
		    handle[I] == *Segment_Handle)
		    Return_Value = FALSE;
	    }
	} else
	    Return_Value = TRUE;
	break;
    default:
	Return_Value = FALSE;
	break;
    }

    if (Return_Value) {
	evms_free(Object_Info);
    }

    return Return_Value;

}


static int
Generic_Size_Filter(Executable_Node * Current_Node,
		    boolean Filter_On_Freespace,
		    Size_Filter_Types Filter_Type)
{

    dlist_t List_To_Process;
    Size_Filter_Structure Filter_Data;
    int Error = 0;
    uint Child_Count = 0;
    u_int64_t Tolerance = 0;
    u_int64_t Size;
    boolean Tolerance_Is_Percent;

    /* Remove the list object from the stack. */
    Pop(LIST, sizeof(dlist_t), &List_To_Process);

    /* Clear the current units field.  If it still contains this value after processing our children, then
       the user did not specify any units for the value they entered, in which case we will use our default
       unit, which is BYTES.  If they specified a unit KB, MB, GB, or TB, then their entry will be converted
       to bytes and Current_Units will reflect this.  If they specified a time unit, then we must put out
       an error.                                                                                              */
    Current_Units = EVMS_Unit_None;

    /* Process Children */
    Error = ForEachItem(Current_Node->Children, Process_Node, NULL, TRUE);
    if (!Error) {

	Filter_Data.Use_Freespace = Filter_On_Freespace;

	if (Filter_Type == EQ) {

	    /* The number of values on the stack for us depends on the number of children we have.
	       We may have two children.  The first would put on the stack the size, in bytes, of
	       the item we are interested in.  The second, if present, would put two values on the
	       stack.  The first would be the tolerance.  The second would be a boolean value to
	       indicate whether the tolerance in bytes or percent.  If in percent, then we
	       would need to calculate the tolerance based upon the size, in bytes, of the item
	       we are interested in.                                                               */
	    Error = GetListSize(Current_Node->Children, &Child_Count);
	    Check_For_Error(Error);

	    switch (Child_Count) {
	    case 2:		/* Two children - precision specified. */
		/* Get the boolean value to tell us whether or not the tolerance is a percentage. */
		Pop(BOOLEAN_VALUE, sizeof(boolean), &Tolerance_Is_Percent);
		/* Now get the tolerance. */
		Pop(INTEGER_VALUE, sizeof(u_int64_t), &Tolerance);
	    case 1:		/* Only one child - no precision specified. */
		/* Get the size of the item, in bytes, that we are interested in. */
		Pop(INTEGER_VALUE, sizeof(u_int64_t), &Size);
		break;
	    default:		/* Bad things are happening!  Abort! */
		My_Printf
		    ("Internal error: Unexpected node structure for command.\n");
		abort();
	    }

	    if (Child_Count == 2) {
		if (Tolerance_Is_Percent) {
		    /* Calculate the actual tolerance in bytes. */
		    Tolerance = (Size * Tolerance) / 100;
		}

		Filter_Data.Lower_Limit = Size - Tolerance;
		Filter_Data.Upper_Limit = Size + Tolerance;

	    } else {
		Filter_Data.Lower_Limit = Size;
		Filter_Data.Upper_Limit = Size;
	    }

	} else {

	    /* There should be one value on the stack for us.  That should be the
	       size, in bytes, of the item we are interested in.                     */
	    Pop(INTEGER_VALUE, sizeof(u_int64_t), &Size);

	    Filter_Data.Lower_Limit = Size;
	    Filter_Data.Upper_Limit = Size;
	}

	if (!Error) {

	    Filter_Data.Filter_Type = Filter_Type;
	    Error =
		PruneList(List_To_Process, Scan_For_Size, &Filter_Data);
	    if (!Error) {
		/* Push List_To_Process onto the stack so that other filters may process it. */
		Push(LIST, sizeof(dlist_t), &List_To_Process);
	    }

	}

    }

    return Error;

}


static boolean
Scan_For_Size(ADDRESS Object,
	      TAG ObjectTag,
	      ADDRESS ObjectHandle,
	      ADDRESS Parameters, boolean * FreeMemory, uint * Error)
{
    Size_Filter_Structure *Filter_Data =
	(Size_Filter_Structure *) Parameters;
    handle_object_info_t *Object_Info = (handle_object_info_t *) Object;
    handle_object_info_t *Region_Info = NULL;
    u_int Index;
    u_int64_t Size_To_Compare = 0;
    boolean Return_Value = FALSE;

    /* Assume success */
    *Error = DLIST_SUCCESS;

    /* When we delete an item from the list, we don't want its memory freed. */
    *FreeMemory = FALSE;

    /* Sanity Check */
    if (ObjectTag != HANDLE_INFO) {
	Report_Standard_Error(DLIST_ITEM_TAG_WRONG);
	abort();
    }

    if ((Filter_Data == NULL) || (Object == NULL)) {

	Report_Standard_Error(DLIST_CORRUPTED);
	abort();

    }

    /* Is the current object acceptable?  If so, what is its size?  */
    switch (Object_Info->type) {
    case VOLUME:
	Size_To_Compare =
	    Object_Info->info.volume.vol_size * EVMS_VSECTOR_SIZE;
	break;
    case EVMS_OBJECT:
	Size_To_Compare =
	    Object_Info->info.object.size * EVMS_VSECTOR_SIZE;
	break;
    case REGION:
	Size_To_Compare =
	    Object_Info->info.region.size * EVMS_VSECTOR_SIZE;
	break;
    case CONTAINER:
	if ((!Filter_Data->Use_Freespace) ||
	    (Object_Info->info.container.objects_produced == NULL) ||
	    (Object_Info->info.container.objects_produced->count == 0))
	    Size_To_Compare =
		Object_Info->info.container.size * EVMS_VSECTOR_SIZE;
	else {

	    for (Index = 0;
		 Index <
		 Object_Info->info.container.objects_produced->count;
		 Index++) {
		*Error =
		    evms_get_info(Object_Info->info.container.
				  objects_produced->handle[Index],
				  &Region_Info);
		if ((*Error == 0)
		    && (Region_Info->info.region.data_type ==
			FREE_SPACE_TYPE))
		    Size_To_Compare = Region_Info->info.region.size;

		if (Region_Info != NULL)
		    evms_free(Region_Info);
	    }

	}
	break;
    case SEGMENT:
	Size_To_Compare =
	    Object_Info->info.segment.size * EVMS_VSECTOR_SIZE;
	break;
    case DISK:
	Size_To_Compare = Object_Info->info.disk.size * EVMS_VSECTOR_SIZE;
	break;
    default:
	return FALSE;
	break;
    }

    switch (Filter_Data->Filter_Type) {
    case GT:
	if (Size_To_Compare > Filter_Data->Lower_Limit)
	    Return_Value = FALSE;
	else
	    Return_Value = TRUE;
	break;
    case LT:
	if (Size_To_Compare < Filter_Data->Upper_Limit)
	    Return_Value = FALSE;
	else
	    Return_Value = TRUE;
	break;
    default:
	if ((Size_To_Compare >= Filter_Data->Lower_Limit) &&
	    (Size_To_Compare <= Filter_Data->Upper_Limit))
	    Return_Value = FALSE;
	else
	    Return_Value = TRUE;
	break;
    }

    if (Return_Value) {
	evms_free(Object_Info);
    }

    return Return_Value;

}


static int Do_Freespace_Size_Filter(Executable_Node * Current_Node)
{
    if (Current_Node->NodeValue == GTStr)
	return Generic_Size_Filter(Current_Node, TRUE, GT);
    else if (Current_Node->NodeValue == LTStr)
	return Generic_Size_Filter(Current_Node, TRUE, LT);
    else
	return Generic_Size_Filter(Current_Node, TRUE, EQ);

}


static int Do_GT_Filter(Executable_Node * Current_Node)
{
    return Generic_Size_Filter(Current_Node, FALSE, GT);
}


static int Do_LT_Filter(Executable_Node * Current_Node)
{

    return Generic_Size_Filter(Current_Node, FALSE, LT);

}


static int Do_EQ_Filter(Executable_Node * Current_Node)
{

    return Generic_Size_Filter(Current_Node, FALSE, EQ);

}


static int Do_Topmost_Objects(Executable_Node * Current_Node)
{
    dlist_t Return_Values;
    int Error = 0;
    unsigned int index;
    handle_array_t *Object_List;
    handle_object_info_t *Object_Info;
    ADDRESS NotNeeded;

    /* Remove the list object from the stack. */
    Pop(LIST, sizeof(dlist_t), &Return_Values);

    /* Get the list of topmost objects which are eligible to become volumes. */
    Error =
	evms_get_object_list(EVMS_OBJECT, 0, 0, 0, TOPMOST, &Object_List);

    if (!Error) {
	/* We need to convert each handle in the Object_List to a handle_object_info_t entry
	   so that any children of this node can process it.                                 */
	for (index = 0; index < Object_List->count; index++) {
	    Error =
		evms_get_info(Object_List->handle[index], &Object_Info);
	    if (Error) {
		Report_Standard_Error(Error);
		break;
	    }

	    /* Add the info structure to the Return_Values list. */
	    Error =
		InsertObject(Return_Values, Object_Info, HANDLE_INFO, NULL,
			     AppendToList, TRUE, &NotNeeded);
	    Check_For_Error(Error);
	}

	evms_free(Object_List);

    } else
	Report_Standard_Error(Error);

    if (!Error) {
	/* Push List_To_Process onto the stack so that other filters may process it. */
	Push(LIST, sizeof(dlist_t), &Return_Values);

	/* There are no selected objects for this task, so the selected objects filter is not needed. */
	Use_Selected_Objects_Filter = FALSE;

    }

    return Error;

}


static int Do_Unclaimed_Filter(Executable_Node * Current_Node)
{
    dlist_t List_To_Process;
    int Error = 0;

    /* Remove the list object from the stack. */
    Pop(LIST, sizeof(dlist_t), &List_To_Process);

    Error = PruneList(List_To_Process, Scan_For_Unclaimed, NULL);
    if (!Error) {
	/* Push List_To_Process onto the stack so that other filters may process it. */
	Push(LIST, sizeof(dlist_t), &List_To_Process);
    }

    return Error;

}


static boolean
Scan_For_Unclaimed(ADDRESS Object,
		   TAG ObjectTag,
		   ADDRESS ObjectHandle,
		   ADDRESS Parameters, boolean * FreeMemory, uint * Error)
{
    handle_object_info_t *Object_Info = (handle_object_info_t *) Object;
    boolean Return_Value = FALSE;

    /* Assume success */
    *Error = DLIST_SUCCESS;

    /* When we delete an item from the list, we don't want its memory freed. */
    *FreeMemory = FALSE;

    /* Sanity Check */
    if (ObjectTag != HANDLE_INFO) {
	Report_Standard_Error(DLIST_ITEM_TAG_WRONG);
	abort();
    }

    if (Object == NULL) {

	Report_Standard_Error(DLIST_CORRUPTED);
	abort();

    }

    /* Is the current object acceptable?  */
    switch (Object_Info->type) {
    case REGION:
	if ((Object_Info->info.region.volume == 0) &&
	    ((Object_Info->info.region.parent_objects == NULL) ||
	     (Object_Info->info.region.parent_objects->count == 0)))
	    Return_Value = FALSE;
	else
	    Return_Value = TRUE;
	break;
    case SEGMENT:
	if ((Object_Info->info.segment.volume == 0) &&
	    ((Object_Info->info.segment.parent_objects == NULL) ||
	     (Object_Info->info.segment.parent_objects->count == 0)) &&
	    (Object_Info->info.segment.data_type != META_DATA_TYPE))
	    Return_Value = FALSE;
	else
	    Return_Value = TRUE;
	break;
    case DISK:
	if ((Object_Info->info.disk.volume == 0) &&
	    ((Object_Info->info.disk.parent_objects == NULL) ||
	     (Object_Info->info.disk.parent_objects->count == 0)))
	    Return_Value = FALSE;
	else
	    Return_Value = TRUE;
	break;
    case EVMS_OBJECT:
	if ((Object_Info->info.object.volume == 0) &&
	    ((Object_Info->info.object.parent_objects == NULL) ||
	     (Object_Info->info.object.parent_objects->count == 0)))
	    Return_Value = FALSE;
	else
	    Return_Value = TRUE;
	break;
    default:
	Return_Value = FALSE;
	break;
    }

    if (Return_Value) {
	evms_free(Object_Info);
    }

    return Return_Value;

}


static int Do_Freespace_Filter(Executable_Node * Current_Node)
{

    dlist_t List_To_Process;
    int Error = 0;

    /* Remove the list object from the stack. */
    Pop(LIST, sizeof(dlist_t), &List_To_Process);

    Error = PruneList(List_To_Process, Scan_For_Freespace, NULL);
    if (!Error) {
	/* Push List_To_Process onto the stack so that other filters may process it. */
	Push(LIST, sizeof(dlist_t), &List_To_Process);
    }

    return Error;

}

static boolean
Scan_For_Freespace(ADDRESS Object,
		   TAG ObjectTag,
		   ADDRESS ObjectHandle,
		   ADDRESS Parameters, boolean * FreeMemory, uint * Error)
{
    handle_object_info_t *Object_Info = (handle_object_info_t *) Object;
    handle_object_info_t *Child_Info;
    unsigned int I;
    int Engine_Error = 0;
    boolean Return_Value = FALSE;

    /* Assume success */
    *Error = DLIST_SUCCESS;

    /* When we delete an item from the list, we don't want its memory freed. */
    *FreeMemory = FALSE;

    /* Sanity Check */
    if (ObjectTag != HANDLE_INFO) {
	Report_Standard_Error(DLIST_ITEM_TAG_WRONG);
	abort();
    }

    if (Object == NULL) {

	Report_Standard_Error(DLIST_CORRUPTED);
	abort();

    }

    /* Is the current object acceptable?  */
    switch (Object_Info->type) {
    case CONTAINER:
	Return_Value = TRUE;	/* Assume no freespace. */

	if ((Object_Info->info.container.objects_produced != NULL) &&
	    (Object_Info->info.container.objects_produced->count > 0)) {

	    /* Search for freespace region. */
	    for (I = 0;
		 I < Object_Info->info.container.objects_produced->count;
		 I++) {

		Engine_Error =
		    evms_get_info(Object_Info->info.container.
				  objects_produced->handle[I],
				  &Child_Info);
		if (Engine_Error) {
		    *Error = DLIST_CORRUPTED;
		    return FALSE;
		}

		if (Child_Info->info.region.data_type == FREE_SPACE_TYPE) {
		    Return_Value = FALSE;
		    Freespace_Not_Size = TRUE;
		    Object_Info->info.container.size =
			Child_Info->info.region.size;
		    evms_free(Child_Info);
		}

	    }

	}

	break;
    case SEGMENT:
	Return_Value = TRUE;
	if (Object_Info->info.segment.data_type == FREE_SPACE_TYPE) {
	    Return_Value = FALSE;
	}
	break;
    default:
	Return_Value = TRUE;
	break;
    }

    if (Return_Value) {
	evms_free(Object_Info);
    }

    return Return_Value;

}

static boolean
Check_Child_For_Disk(object_handle_t Handle, object_handle_t Disk_Handle,
		     uint * Error)
{
    handle_object_info_t *Object_Info;
    unsigned int I;
    boolean Return_Value = FALSE;


    *Error = evms_get_info(Handle, &Object_Info);
    if (*Error) {
	*Error = DLIST_CORRUPTED;
	Report_Standard_Error(*Error);
	return FALSE;
    }

    if ((Object_Info->info.segment.child_objects == NULL) ||
	(Object_Info->info.segment.child_objects->count == 0))
	Return_Value = TRUE;
    else {

	for (I = 0, Return_Value = TRUE;
	     (I < Object_Info->info.segment.child_objects->count)
	     && Return_Value; I++) {
	    if (Object_Info->info.segment.child_objects->handle[I] ==
		Disk_Handle)
		Return_Value = FALSE;
	    else
		Return_Value =
		    Check_Child_For_Disk(Object_Info->info.segment.
					 child_objects->handle[I],
					 Disk_Handle, Error);
	}

    }

    evms_free(Object_Info);

    return Return_Value;
}


static boolean
Check_Child_For_Region(object_handle_t Handle,
		       object_handle_t Region_Handle, uint * Error)
{
    handle_object_info_t *Object_Info = NULL;
    unsigned int I;
    boolean Return_Value = FALSE;


    *Error = evms_get_info(Handle, &Object_Info);
    if (*Error) {
	*Error = DLIST_CORRUPTED;
	Report_Standard_Error(*Error);
	return FALSE;
    }

    switch (Object_Info->type) {
    case CONTAINER:
	Return_Value = TRUE;
	break;
    case EVMS_OBJECT:
	for (I = 0, Return_Value = TRUE;
	     (I < Object_Info->info.object.child_objects->count)
	     && Return_Value; I++) {
	    if (Object_Info->info.object.child_objects->handle[I] ==
		Region_Handle)
		Return_Value = FALSE;
	    else
		Return_Value =
		    Check_Child_For_Region(Object_Info->info.object.
					   child_objects->handle[I],
					   Region_Handle, Error);
	}
	break;
    case REGION:
	for (I = 0, Return_Value = TRUE;
	     (I < Object_Info->info.region.child_objects->count)
	     && Return_Value; I++) {
	    if (Object_Info->info.region.child_objects->handle[I] ==
		Region_Handle)
		Return_Value = FALSE;
	    else
		Return_Value =
		    Check_Child_For_Region(Object_Info->info.region.
					   child_objects->handle[I],
					   Region_Handle, Error);
	}
	break;
    default:
	Return_Value = TRUE;
	break;
    }

    evms_free(Object_Info);

    return Return_Value;
}


static boolean
Check_Child_For_Segment(object_handle_t Handle,
			object_handle_t Segment_Handle, uint * Error)
{
    handle_object_info_t *Object_Info = NULL;
    unsigned int I;
    boolean Return_Value = FALSE;


    *Error = evms_get_info(Handle, &Object_Info);
    if (*Error) {
	*Error = DLIST_CORRUPTED;
	Report_Standard_Error(*Error);
	return FALSE;
    }

    switch (Object_Info->type) {
    case CONTAINER:
	for (I = 0, Return_Value = TRUE;
	     (I < Object_Info->info.container.objects_consumed->count)
	     && Return_Value; I++) {
	    if (Object_Info->info.container.objects_consumed->handle[I] ==
		Segment_Handle)
		return FALSE;
	    else
		Return_Value =
		    Check_Child_For_Segment(Object_Info->info.container.
					    objects_consumed->handle[I],
					    Segment_Handle, Error);
	}
	break;
    case EVMS_OBJECT:
	for (I = 0, Return_Value = TRUE;
	     (I < Object_Info->info.object.child_objects->count)
	     && Return_Value; I++) {
	    if (Object_Info->info.object.child_objects->handle[I] ==
		Segment_Handle)
		Return_Value = FALSE;
	    else
		Return_Value =
		    Check_Child_For_Segment(Object_Info->info.object.
					    child_objects->handle[I],
					    Segment_Handle, Error);
	}
	break;
    case REGION:
	for (I = 0, Return_Value = TRUE;
	     (I < Object_Info->info.region.child_objects->count)
	     && Return_Value; I++) {
	    if (Object_Info->info.region.child_objects->handle[I] ==
		Segment_Handle)
		Return_Value = FALSE;
	    else
		Return_Value =
		    Check_Child_For_Region(Object_Info->info.region.
					   child_objects->handle[I],
					   Segment_Handle, Error);
	}
	break;
    case SEGMENT:
	if (Object_Info->info.segment.handle == Segment_Handle)
	    Return_Value = FALSE;
	else
	    Return_Value = TRUE;
    default:
	Return_Value = TRUE;
	break;
    }

    evms_free(Object_Info);

    return Return_Value;
}


static boolean
Check_Child_For_Container(object_handle_t Handle,
			  object_handle_t Container_Handle, uint * Error)
{

    handle_object_info_t *Object_Info = NULL;
    unsigned int I;
    boolean Return_Value = FALSE;


    *Error = evms_get_info(Handle, &Object_Info);
    if (*Error) {
	*Error = DLIST_CORRUPTED;
	Report_Standard_Error(*Error);
	return FALSE;
    }

    switch (Object_Info->type) {
    case CONTAINER:
	if (Object_Info->info.container.handle == Container_Handle)
	    Return_Value = FALSE;
	else
	    Return_Value = TRUE;
	break;
    case EVMS_OBJECT:
    case SEGMENT:
    case REGION:
    case DISK:
	if (Object_Info->info.object.producing_container ==
	    Container_Handle)
	    Return_Value = FALSE;
	else
	    for (I = 0, Return_Value = TRUE;
		 (I < Object_Info->info.object.child_objects->count)
		 && Return_Value; I++) {
		if (Object_Info->info.object.child_objects->handle[I] ==
		    Container_Handle)
		    Return_Value = FALSE;
		else
		    Return_Value =
			Check_Child_For_Container(Object_Info->info.object.
						  child_objects->handle[I],
						  Container_Handle, Error);
	    }
	break;
    default:
	Return_Value = TRUE;
	break;
    }

    evms_free(Object_Info);

    return Return_Value;

}


static int Do_Expand_Filter(Executable_Node * Current_Node)
{
    dlist_t List_To_Process;
    int Error = 0;

    /* Remove the list object from the stack. */
    Pop(LIST, sizeof(dlist_t), &List_To_Process);

    Error = PruneList(List_To_Process, Scan_For_Expandable, NULL);
    if (!Error) {
	/* Push List_To_Process onto the stack so that other filters may process it. */
	Push(LIST, sizeof(dlist_t), &List_To_Process);
    }

    return Error;
}


static boolean
Scan_For_Expandable(ADDRESS Object,
		    TAG ObjectTag,
		    ADDRESS ObjectHandle,
		    ADDRESS Parameters, boolean * FreeMemory, uint * Error)
{
    handle_object_info_t *Object_Info = (handle_object_info_t *) Object;
    object_handle_t Handle = 0;
    int Task_Error = 0;
    boolean Return_Value = FALSE;
    boolean Use_Task_Method;

    /* Assume success */
    *Error = DLIST_SUCCESS;

    /* When we delete an item from the list, we don't want its memory freed. */
    *FreeMemory = FALSE;

    /* Sanity Check */
    if (ObjectTag != HANDLE_INFO) {
	Report_Standard_Error(DLIST_ITEM_TAG_WRONG);
	abort();
    }

    if (Object == NULL) {

	Report_Standard_Error(DLIST_CORRUPTED);
	abort();

    }

    /* Is the current object acceptable?  If so, is it expandable?  */
    switch (Object_Info->type) {
    case VOLUME:
	Return_Value = evms_can_expand(Object_Info->info.volume.handle);
	Use_Task_Method = FALSE;
	break;
    case EVMS_OBJECT:
	/* If the object is topmost, then use evms_can_expand.  Otherwise, use the task method. */
	if (((Object_Info->info.object.parent_objects == NULL) ||
	     (Object_Info->info.object.parent_objects->count == 0)) &&
	    (Object_Info->info.object.volume == 0)) {
	    Return_Value =
		evms_can_expand(Object_Info->info.object.handle);
	    Use_Task_Method = FALSE;
	} else {
	    Handle = Object_Info->info.object.handle;
	    Use_Task_Method = TRUE;
	}
	break;
    case REGION:
	Handle = Object_Info->info.region.handle;
	Use_Task_Method = TRUE;
	break;
    case SEGMENT:
	Handle = Object_Info->info.segment.handle;
	Use_Task_Method = TRUE;
	break;
    case DISK:
	Handle = Object_Info->info.disk.handle;
	Use_Task_Method = TRUE;
	break;
    case CONTAINER:
	Handle = Object_Info->info.container.handle;
	Use_Task_Method = TRUE;
	break;
    default:
	return FALSE;
	break;
    }

    if (Use_Task_Method) {
	Task_Error = Setup_Task(Handle, EVMS_Task_Expand, FALSE);

	if (!Task_Error)
	    Return_Value = FALSE;
	else
	    Return_Value = TRUE;

	Task_Error = Cleanup_Task();
    }

    if (Return_Value != FALSE)
	evms_free(Object_Info);

    if (Return_Value != FALSE)
	return TRUE;
    else
	return FALSE;

}


static int Do_Shrink_Filter(Executable_Node * Current_Node)
{
    dlist_t List_To_Process;
    int Error = 0;

    /* Remove the list object from the stack. */
    Pop(LIST, sizeof(dlist_t), &List_To_Process);

    Error = PruneList(List_To_Process, Scan_For_Shrinkable, NULL);
    if (!Error) {
	/* Push List_To_Process onto the stack so that other filters may process it. */
	Push(LIST, sizeof(dlist_t), &List_To_Process);
    }

    return Error;
}


static boolean
Scan_For_Shrinkable(ADDRESS Object,
		    TAG ObjectTag,
		    ADDRESS ObjectHandle,
		    ADDRESS Parameters, boolean * FreeMemory, uint * Error)
{
    handle_object_info_t *Object_Info = (handle_object_info_t *) Object;
    object_handle_t Handle = 0;
    int Task_Error = 0;
    boolean Return_Value = FALSE;
    boolean Use_Task_Method = FALSE;

    /* Assume success */
    *Error = DLIST_SUCCESS;

    /* When we delete an item from the list, we don't want its memory freed. */
    *FreeMemory = FALSE;

    /* Sanity Check */
    if (ObjectTag != HANDLE_INFO) {
	Report_Standard_Error(DLIST_ITEM_TAG_WRONG);
	abort();
    }

    if (Object == NULL) {

	Report_Standard_Error(DLIST_CORRUPTED);
	abort();

    }

    /* Is the current object acceptable?  If so, is it shrinkable?  */
    switch (Object_Info->type) {
    case VOLUME:
	Return_Value = evms_can_shrink(Object_Info->info.volume.handle);
	break;
    case EVMS_OBJECT:
    case REGION:
    case SEGMENT:
    case DISK:
	/* If the object is topmost, then use evms_can_shrink.  Otherwise, use the task method. */
	if (((Object_Info->info.object.parent_objects == NULL) ||
	     (Object_Info->info.object.parent_objects->count == 0)) &&
	    (Object_Info->info.object.volume == 0)) {
	    Return_Value =
		evms_can_shrink(Object_Info->info.object.handle);
	} else {
	    Handle = Object_Info->info.object.handle;
	    Use_Task_Method = TRUE;
	}
	break;
    case CONTAINER:
	Return_Value = evms_can_shrink(Object_Info->info.container.handle);
	break;
    default:
	Return_Value = FALSE;
	break;
    }

    if (Use_Task_Method) {
	Task_Error = Setup_Task(Handle, EVMS_Task_Shrink, FALSE);
	if (!Task_Error)
	    Return_Value = FALSE;
	else
	    Return_Value = TRUE;

	Task_Error = Cleanup_Task();
    }

    if (Return_Value) {
	evms_free(Object_Info);
	return TRUE;
    } else
	return FALSE;

}


static int Do_Remove(Executable_Node * Current_Node)
{
    object_handle_t Handle;
    int Error;

    /* Process our children first! */
    Error = ForEachItem(Current_Node->Children, Process_Node, NULL, TRUE);
    if (!Error) {
	/* Get the handle of the disk we are to remove the segment manager from. */
	Pop(HANDLE, sizeof(object_handle_t), &Handle);

	/* Now remove the segment manager from the disk/segment. */
	Error = evms_unassign(Handle);
	if (Error)
	    Report_Standard_Error(Error);
    }

    return Error;
}


static int
Do_Query_Acceptable_Expand_Filter(Executable_Node * Current_Node)
{
    int Error = 0;
    int Cleanup_Error = 0;
    handle_array_t *Selected_Objects = NULL;
    dlist_t Return_Values;

    /* Activate the Get_Acceptable hack. */
    Get_Acceptable = TRUE;
    Use_Selected_Objects_Filter = TRUE;

    Error = Do_Expand(Current_Node);

    /* Deactivate the Get_Acceptable hack. */
    Get_Acceptable = FALSE;

    if (!Error) {

	if (Use_Selected_Objects_Filter) {
	    /* Get the list of acceptable objects produced by our children. */
	    Pop(LIST, sizeof(dlist_t), &Return_Values);

	    /* Get the list of selected items. */
	    Error = evms_get_selected_objects(Task, &Selected_Objects);
	    if (!Error) {
		/* Remove the selected items from the list of acceptable objects. */
		Error =
		    PruneList(Return_Values, Scan_For_Selected_Objects,
			      Selected_Objects);
		if (!Error) {
		    /* Push List_To_Process onto the stack so that other filters may process it. */
		    Push(LIST, sizeof(dlist_t), &Return_Values);
		}

		evms_free(Selected_Objects);
	    } else
		Report_Standard_Error(Error);

	}

	/* Eliminate the expand task created by Do_Expand since we don't need it anymore. */
	Cleanup_Error = Cleanup_Task();
	if (!Error && Cleanup_Error)
	    Error = Cleanup_Error;

    }

    return Error;

}


static int
Do_Query_Acceptable_Create_Filter(Executable_Node * Current_Node)
{
    int Error = 0;
    int Cleanup_Error = 0;
    handle_array_t *Selected_Objects = NULL;
    dlist_t Return_Values;


    /* Set the task type. */
    Task_Action = EVMS_Task_Create;

    /* Activate the Get_Acceptable hack. */
    Get_Acceptable = TRUE;
    Use_Selected_Objects_Filter = TRUE;

    /* Process our children. */
    Error = ForEachItem(Current_Node->Children, Process_Node, NULL, TRUE);

    /* Deactivate the Get_Acceptable hack. */
    Get_Acceptable = FALSE;

    if (!Error) {

	if (Use_Selected_Objects_Filter) {
	    /* Get the list of acceptable objects produced by our children. */
	    Pop(LIST, sizeof(dlist_t), &Return_Values);

	    /* Get the list of selected items. */
	    Error = evms_get_selected_objects(Task, &Selected_Objects);
	    if (!Error) {
		/* Remove the selected items from the list of acceptable objects. */
		Error =
		    PruneList(Return_Values, Scan_For_Selected_Objects,
			      Selected_Objects);
		if (!Error) {
		    /* Push List_To_Process onto the stack so that other filters may process it. */
		    Push(LIST, sizeof(dlist_t), &Return_Values);
		}

		evms_free(Selected_Objects);
	    } else
		Report_Standard_Error(Error);

	    /* We don't need the create task that was created by our children, so get rid of it. */
	    Cleanup_Error = Cleanup_Task();
	    if (!Error && Cleanup_Error)
		Error = Cleanup_Error;

	}

    }

    return Error;
}


static boolean
Scan_For_Selected_Objects(ADDRESS Object,
			  TAG ObjectTag,
			  ADDRESS ObjectHandle,
			  ADDRESS Parameters,
			  boolean * FreeMemory, uint * Error)
{
    handle_object_info_t *Object_Info = (handle_object_info_t *) Object;
    handle_array_t *Handle_Array = (handle_array_t *) Parameters;
    object_handle_t Current_Handle;
    unsigned int Index;
    boolean Return_Value = FALSE;

    /* Assume success */
    *Error = DLIST_SUCCESS;

    /* When we delete an item from the list, we don't want its memory freed. */
    *FreeMemory = FALSE;

    /* Sanity Check */
    if (ObjectTag != HANDLE_INFO) {
	Report_Standard_Error(DLIST_ITEM_TAG_WRONG);
	abort();
    }

    if (Object == NULL) {

	Report_Standard_Error(DLIST_CORRUPTED);
	abort();

    }

    /* Is the current object acceptable?  If so, does its handle appear in the Handle_Array?  */
    switch (Object_Info->type) {
    case VOLUME:
	Current_Handle = Object_Info->info.volume.handle;
	break;
    case EVMS_OBJECT:
	Current_Handle = Object_Info->info.object.handle;
	break;
    case REGION:
	Current_Handle = Object_Info->info.region.handle;
	break;
    case CONTAINER:
	Current_Handle = Object_Info->info.container.handle;
	break;
    case SEGMENT:
	Current_Handle = Object_Info->info.segment.handle;
	break;
    case DISK:
	Current_Handle = Object_Info->info.disk.handle;
	break;
    default:
	return FALSE;
	break;
    }

    /* Does Current_Handle appear in Handle_Array? */
    Return_Value = FALSE;
    for (Index = 0; Index < Handle_Array->count; Index++) {
	if (Current_Handle == Handle_Array->handle[Index]) {
	    Return_Value = TRUE;
	    evms_free(Object_Info);
	    break;
	}

    }

    return Return_Value;
}


static int Do_Task_Adjust(Executable_Node * Current_Node)
{
    int Error = DLIST_SUCCESS;

    /* Set the task type. */
    Task_Action = EVMS_Task_Create_Container;

    /* Process our children. */
    Error = ForEachItem(Current_Node->Children, Process_Node, NULL, TRUE);

    return Error;
}

#include "../RecvOneFrame.h"

void Report_Progress(char *Message)
{

    int i;

    switch (Verbose_Mode) {

    case 1:
	My_Printf("%s", Message);
	break;
    case 2:
	/* Wipe out the last message printed on the screen. */
	for (i = 0; i < Last_Message_Length; i++) {
	    My_Printf(" ");
	}

	My_Printf("\r");

	/* Save the length of the new message to display. */
	Last_Message_Length = strlen(Message);

	/* The EVMS Engine always ends its status messages with a linefeed.  We need to convert
	   that to a carriage return in order to get the display we want.                         */
	if (Last_Message_Length > 0) {
	    if (Message[Last_Message_Length - 1] == '\n') {
		Message[Last_Message_Length - 1] = '\r';
	    }
	}

	/* Display the new message. */
	My_Printf("%s", Message);

	break;
    }

    return;
}


#endif
