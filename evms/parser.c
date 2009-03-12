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
 * Module: parser.c
 *
 * Authors:
 *  Original Author : B. Rafanello
 */

/*
 *
 */

/* Identify this file. */
#define PARSER_C

/*--------------------------------------------------
 * Necessary include files
 --------------------------------------------------*/

#include <ctype.h>		/* toupper */
#include <stdlib.h>		/* malloc */
#include <string.h>		/* malloc */
#include "token.h"		/* TokenType, Token, MaxIdentifierLength */
#include "scanner.h"
#include "screener.h"		/* GetScreenedToken , and all of the predefined strings for keywords. */
#include "dlist.h"
#include "error.h"
/* include "evms_appAPI.h" */
#include "parser.h"


/*--------------------------------------------------
 * Private constants
 --------------------------------------------------*/
#define HANDLE_TAG  39392

/*--------------------------------------------------
 * Private types
 --------------------------------------------------*/
typedef enum {
    Try_Again,
    Error,
    Token_Accepted
} Parse_Results;

typedef unsigned long Available_Generic_Filters;	/* This is a bitmap with one bit for each of the available generic filters. The Bit defintions follow: */
#define  Available_Filter      (1<<0)
#define  Container_Filter      (1<<1)
#define  Convertible_Filter    (1<<2)
#define  Expandable_Filter     (1<<3)
#define  EQ_Filter             (1<<4)
#define  GT_Filter             (1<<5)
#define  LT_Filter             (1<<6)
#define  LO_Filter             (1<<7)
#define  Object_Filter         (1<<8)
#define  Plugin_Filter         (1<<9)
#define  Region_Filter         (1<<10)
#define  Segment_Filter        (1<<11)
#define  Shrinkable_Filter     (1<<12)
#define  Unclaimed_Filter      (1<<13)
#define  Volume_Filter         (1<<14)
#define  Plugin_Type_Filter    (1<<15)
#define  Disk_Filter           (1<<16)
#define  Freespace_Filter      (1<<17)


/*--------------------------------------------------
 * Private variables
 --------------------------------------------------*/
static TokenType *Current_Token = NULL;	/* Token being parsed. */
static boolean Use_Current_Token = FALSE;	/* Used to have the current token processed again. */
static TokenType Null_Token = { EofToken, 0, 0, 0, NULL, TRUE };	/* Used to represent an empty token. */
static char *Command_Filename = NULL;	/* Name of command file, if any, being parsed. */
static dlist_t Tokens = NULL;	/* Tokens being stored during when lookahead is in operation. */
static uint LookAhead_Active = 0;	/* Used to indicate whether or not Look Ahead is active for parsing. */
static dlist_t Lookahead_Handles = NULL;	/* Used to hold the handles of tokens to return to in the Tokens list. */

/*--------------------------------------------------
 * Private functions
 --------------------------------------------------*/

/* Set up token handling functions so that if we need to go to a lookahead scheme, it won't
   affect any existing code.                                                                 */
static void Consume_Token(void);	/* Frees the current token and sets CurrentToken to NULL. */
static void Next_Token(void);	/* Gets the next token to be parsed and sets CurrentToken to point to it. */
static Executable_Node *Activate_Lookahead(void);	/* Start Lookahead mode. */
static boolean Restore_Current_Token(Executable_Node * Temp_Node);	/* Restores Current_Token to the value it was at before any Lookahead calls were made. */
static boolean Terminate_Lookahead(Executable_Node * Current_Node, Executable_Node * Temp_Node, boolean Keep_Children);	/* End lookahead mode. */
static void Assign_Text(Executable_Node * Node, TokenType * Token);	/* Moves Token->Token_Text to Node->Node_Value taking into account whether or not
									   the Token is in the Lookahead buffer (i.e. the Tokens list. )                   */

/* Private Utility Functions */
static void Free_Command_List(dlist_t Command_List);	/* Deletes all entries in the Command_List and frees their memory. */
static boolean Kill_Command_List_Entries(ADDRESS Object, TAG ObjectTag, ADDRESS ObjectHandle, ADDRESS Parameters, boolean * FreeMemory, uint * Error);	/* Used with the PruneList function to delete entries in the Command_List. */

static boolean Kill_Tokens_List_Entries(ADDRESS Object,
					TAG ObjectTag,
					ADDRESS ObjectHandle,
					ADDRESS Parameters,
					boolean * FreeMemory,
					uint * Error);


/* Private functions corresponding to non-terminals in the grammar. */
static Parse_Results Command(Executable_Node * Current_Node,
			     boolean Optional);
static Parse_Results Allocation_Parameters(Executable_Node * Current_Node,
					   boolean Optional);
static Parse_Results Assign_Parameters(Executable_Node * Current_Node,
				       boolean Optional);
static Parse_Results Add_Feature_Parameters(Executable_Node * Current_Node,
					    boolean Optional);
static Parse_Results Creation_Parameters(Executable_Node * Current_Node,
					 boolean Optional);
static Parse_Results Check_For_Compatibility_Keyword(Executable_Node *
						     Current_Node,
						     boolean Optional);
static Parse_Results Expand_Shrink_Task_Parameters(Executable_Node *
						   Current_Node,
						   boolean Optional);
static Parse_Results Format_Parameters(Executable_Node * Current_Node,
				       boolean Optional);
static Parse_Results Set_Parameters(Executable_Node * Current_Node,
				    NodeTypes Node_To_Create,
				    boolean Optional);
static Parse_Results Check_Parameters(Executable_Node * Current_Node,
				      boolean Optional);
static Parse_Results Mount_Parameters(Executable_Node * Current_Node,
				      boolean Optional);
static Parse_Results Query_Type(Executable_Node * Current_Node,
				boolean Optional);
static Parse_Results Plugin_Types(Executable_Node * Current_Node,
				  boolean Optional);
static Parse_Results Help_Type(Executable_Node * Current_Node,
			       boolean Optional);
static Parse_Results Commit_Type(Executable_Node * Current_Node,
				 boolean Optional);
static Parse_Results Feature_Identifier(Executable_Node * Current_Node,
					boolean Optional,
					boolean Parameters_Follow,
					boolean Objects_Are_Optional);
static Parse_Results Plugin_Identifier(Executable_Node * Current_Node,
				       boolean Parameters_Follow,
				       boolean Get_Object_List,
				       boolean Optional);
static Parse_Results Specify_Object(Executable_Node * Current_Node,
				    NodeTypes Node_To_Create,
				    boolean Optional);
static Parse_Results Size(Executable_Node * Current_Node,
			  boolean Optional);
static Parse_Results Precision(Executable_Node * Current_Node,
			       boolean Optional);
static Parse_Results Accept_Name(Executable_Node * Current_Node, boolean Name_Optional);	/* Used for Name = "string" */
static Parse_Results Name(Executable_Node * Current_Node, boolean Field_Name, boolean Optional);	/* Used for Name = Value pairs */
static Parse_Results Get_String(Executable_Node * Current_Node,
				boolean AcceptNumberAsString,
				boolean Optional);
static Parse_Results Parse_Name_Value_Pair(Executable_Node * Current_Node,
					   boolean Optional);
static Parse_Results Value(Executable_Node * Current_Node,
			   boolean Optional);
static Parse_Results Parse_Object_List(Executable_Node * Current_Node,
				       boolean Optional);
static Parse_Results Parse_Query_Acceptable_Parameters(Executable_Node *
						       Current_Node,
						       boolean Optional);
static Parse_Results Parse_Query_Acceptable_Create(Executable_Node *
						   Current_Node,
						   boolean Optional);
static Parse_Results Parse_Query_Extended_Info_Parameters(Executable_Node *
							  Current_Node,
							  boolean
							  Optional);
static Parse_Results Parse_Units(Executable_Node * Current_Node,
				 boolean Allow_Time_Units,
				 boolean Optional);
static Parse_Results Parse_Filters(Executable_Node * Current_Node,
				   Available_Generic_Filters
				   Filters_Allowed,
				   Available_Generic_Filters *
				   Filters_In_Use, boolean Optional);
static Parse_Results Parse_Optional_Query_Filters(Executable_Node *
						  Current_Node,
						  Available_Generic_Filters
						  Allowed_Filters,
						  boolean Optional);
static Parse_Results Check_For_Colon(boolean Optional);
static Parse_Results Check_For_Comma(boolean Comma_Is_Optional);
static Parse_Results Check_For_EqualSign(boolean EqualSign_Is_Optional);
static Parse_Results Check_For_Open_Brace(boolean Optional);
static Parse_Results Check_For_Closing_Brace(boolean Optional);
static Parse_Results Check_For_Open_Paren(boolean Optional);
static Parse_Results Check_For_Closing_Paren(boolean Optional);
static Executable_Node *Make_Child_Node(Executable_Node * Current_Node,
					boolean Add_To_Current_Node);



/*--------------------------------------------------
 * Public Functions
 --------------------------------------------------*/

dlist_t Parse_Command_Line(void)
{
    Parse_Results Command_Found = FALSE;	/* Used to hold the return value from lower level parsing functions. */
    dlist_t Command_List = NULL;	/* List of commands parsed thus far. */
    Executable_Node *Current_Node = NULL;	/* The node used to represent the current command being parsed. */
    ADDRESS Unneeded;		/* Used as a parameter for parameters we don't care about. */

    /* Clear out the lookahead system in case a previous command failed while lookahead was active. */
    LookAhead_Active = 0;
    if (Tokens != NULL) {

	/* Remove any left over tokens from the Tokens list. */
	if (PruneList(Tokens, &Kill_Tokens_List_Entries, NULL) !=
	    DLIST_SUCCESS) {
	    Report_Parser_Error(Internal_Error_Encountered, NULL, NULL);
	    return NULL;
	}

	/* Now destroy the Tokens list. */
	if (DestroyList(&Tokens, FALSE) != DLIST_SUCCESS) {
	    Report_Parser_Error(Internal_Error_Encountered, NULL, NULL);
	    return NULL;
	}


    }

    if (Lookahead_Handles != NULL) {

	/* Clear any handles from the handles list. */
	if (DestroyList(&Lookahead_Handles, TRUE)) {
	    Report_Parser_Error(Internal_Error_Encountered, NULL, NULL);
	    return NULL;
	}

    }

    /* Create the dlist_t we are to return. */
    Command_List = CreateList();
    if (Command_List == NULL) {

	Report_Parser_Error(Out_Of_Memory, NULL, NULL);

	return NULL;

    }

    /* Begin parsing! */

    /* Get the first token. */
    Next_Token();

    /* We will loop until we are out of tokens. */
    while ((Current_Token != NULL) &&
	   (Current_Token->Characterization != EofToken) &&
	   (Current_Token->Characterization != InvalidCharacter)) {

	/* We must create a command node. */
	Current_Node = Make_Child_Node(NULL, FALSE);
	if (Current_Node == NULL) {
	    Report_Parser_Error(Out_Of_Memory, NULL, NULL);

	    Free_Command_List(Command_List);
	    Command_List = NULL;

	    return NULL;

	}

	/* Check for EOF or InvalidCharacter */
	if ((Current_Token == NULL)
	    || (Current_Token->Characterization == EofToken)
	    || (Current_Token->Characterization == InvalidCharacter)) {
	    Report_Parser_Error(Expecting_Command_Start, Command_Filename,
				Current_Token);

	    /* Free the Current_Node. */
	    DestroyList(&Current_Node->Children, TRUE);
	    free(Current_Node);
	    Current_Node = NULL;

	    /* We must free all of the nodes in the Command_List. */
	    Free_Command_List(Command_List);
	    Command_List = NULL;

	    break;
	}

	Command_Found = Command(Current_Node, FALSE);	/* Attempt to parse a command. */

	/* We should have a complete command.  Put the command into the Command_List.
	   If the command is not complete, when the Command_List is destroyed using the Free_Command_List
	   function, then the node we just added to the command list will be disposed of also.             */
	if (InsertObject
	    (Command_List, Current_Node, STACK_NODE, NULL, AppendToList,
	     TRUE, &Unneeded) != DLIST_SUCCESS) {
	    /* We must be out of memory! */
	    Report_Parser_Error(Out_Of_Memory, NULL, NULL);

	    /* Free the Current_Node. */
	    DestroyList(&Current_Node->Children, TRUE);
	    free(Current_Node);
	    Current_Node = NULL;

	    Free_Command_List(Command_List);
	    Command_List = NULL;

	    break;

	}

	/* Did we successfully parse a command? */
	if (Command_Found != Token_Accepted) {
	    /* No commands were recognized or an error occurred. */
	    if (Command_Found == Try_Again)
		Report_Parser_Error(Expecting_Command_Name, NULL,
				    Current_Token);

	    /* Any error should have been reported by whoever found it. */

	    /* Free any commands in the Command_List. */
	    Free_Command_List(Command_List);
	    Command_List = NULL;

	    break;

	}

	/* Since we completed this command, lets look for another. */

	/* Get the next token. */
	Next_Token();

	/* Are we done? */
	if ((Current_Token == NULL)
	    || (Current_Token->Characterization == EofToken)) {
	    Consume_Token();
	    break;
	}

	/* Check for InvalidCharacter */
	if (Current_Token->Characterization == InvalidCharacter) {

	    Report_Parser_Error(Expecting_Command_Separator,
				Command_Filename, Current_Token);

	    /* We must free all of the nodes in the Command_List. */
	    Free_Command_List(Command_List);
	    Command_List = NULL;

	    break;

	}

	/* Check for the command separator - the ':' */
	if ((Current_Token->Characterization != Separator) ||
	    (Current_Token->TokenText[0] != ':')) {

	    Report_Parser_Error(Expecting_Command_Separator,
				Command_Filename, Current_Token);

	    /* We must free all of the nodes in the Command_List. */
	    Free_Command_List(Command_List);
	    Command_List = NULL;

	    break;

	}

	/* We have a command separator.  Consume the token and start the while loop again. */
	Consume_Token();
	Next_Token();

    }				/* while */

    return Command_List;

}


/*******************************************************************
 *  Private Functions
 *******************************************************************/

static Parse_Results
Command(Executable_Node * Current_Node, boolean Optional)
{

    Executable_Node *Temp_Node = NULL;	/* Used for lookahead operations. */
    Parse_Results Return_Value;	/* Used to track the return value from lower level parsing functions. */
    boolean ASM_Found = FALSE;	/* Used when parsing Add Segment Manager. */
    boolean AF_Found = FALSE;	/* Used when parsing Add Feature. */

    /* At this point, we know that '-' or '--' has been encountered.
       Also, we know that Current_Token has been loaded for us and
       has already been checked for NULL, EOF, and InvalidCharacter
       tokens.  We also know that Current_Node has been allocated
       but its fields have not yet been initialized.                 */

    /* A command is always a KeyWord. */
    if (Current_Token->Characterization != KeyWord) {
	if (Optional)
	    return Try_Again;
	else {
	    Report_Parser_Error(Expecting_Command_Name, NULL,
				Current_Token);
	    return Error;
	}

    }

    /* Which command do we have? */
    switch (Current_Token->TokenText[0]) {
    case 'A':			/* Activate, Add Feature, Allocate, Assign, Add Segment Manager */

	/* Do we have an activate command? */
	if ((Current_Token->TokenText == ActivateStr)
	    || (Current_Token->TokenText == ActStr)) {

	    /* Mark Current_Node as being an Activate command. */
	    Current_Node->NodeType = Activate_Node;

	    /* We have an activate command. Let's consume the token, check for the
	       colon which should follow the activate keyword, and then call
	       the Specify_Object function.                                */
	    Consume_Token();

	    Return_Value = Check_For_Colon(Optional);
	    if (Return_Value != Token_Accepted)
		return Return_Value;

	    return Specify_Object(Current_Node, Translate_Name_To_Handle,
				  Optional);

	}

	ASM_Found = FALSE;
	AF_Found = FALSE;

	/* Do we have Add Feature or Add Segment Manager? */
	if (Current_Token->TokenText == AddStr) {

	    /* Check for the Feature or Segment keywords. */
	    Consume_Token();
	    Next_Token();

	    if ((Current_Token != NULL) &&
		(Current_Token->Characterization == KeyWord) &&
		(Current_Token->TokenText == FeatureStr))
		AF_Found = TRUE;
	    else if ((Current_Token != NULL) &&
		     (Current_Token->Characterization == KeyWord) &&
		     (Current_Token->TokenText == SegmentStr)) {

		/* Check for the Manager keyword. */
		Consume_Token();
		Next_Token();

		if ((Current_Token != NULL) &&
		    (Current_Token->Characterization == KeyWord) &&
		    (Current_Token->TokenText == ManagerStr))
		    ASM_Found = TRUE;
		else {

		    if (Optional)
			return Try_Again;

		    Report_Parser_Error(Expecting_Manager_Keyword,
					Command_Filename, Current_Token);

		    return Error;

		}

	    } else {

		if (Optional)
		    return Try_Again;

		Report_Parser_Error(Expecting_Feature_Or_Segment_Keyword,
				    Command_Filename, Current_Token);

		return Error;

	    }

	}

	/* Do we have an Add Feature command? */
	if (AF_Found || (Current_Token->TokenText == AFStr)) {

	    /* Mark Current_Node as being an Add Feature command. */
	    Current_Node->NodeType = Add_Feature_Node;

	    /* We have an Add Feature command. Lets consume the token, check for the
	       colon which should follow the Feature keyword. */
	    Consume_Token();

	    Return_Value = Check_For_Colon(Optional);
	    if (Return_Value != Token_Accepted)
		return Return_Value;

	    return Add_Feature_Parameters(Current_Node, Optional);

	}

	/* Do we have an allocate command? */
	if ((Current_Token->TokenText == AllocateStr) ||
	    (Current_Token->TokenLength == 1)) {

	    /* Mark Current_Node as being an Allocate command. */
	    Current_Node->NodeType = Allocate_Node;

	    /* We have an allocate command. Lets consume the token, check for the
	       colon which should follow the allocate keyword, and then call
	       the Allocation_Parameters function.                                */
	    Consume_Token();

	    Return_Value = Check_For_Colon(Optional);
	    if (Return_Value != Token_Accepted)
		return Return_Value;

	    return Allocation_Parameters(Current_Node, Optional);

	}

	/* Do we have an assign command? */
	if ((Current_Token->TokenText == AssignStr)
	    || (Current_Token->TokenText == AsmStr) || ASM_Found) {

	    /* Mark Current_Node as being an Assign command. */
	    Current_Node->NodeType = Assign_Node;

	    /* We have an assign command. Lets consume the token, check for the
	       colon which should follow the assign keyword, and then call
	       the Assign_Parameters function.                                */
	    Consume_Token();

	    Return_Value = Check_For_Colon(Optional);
	    if (Return_Value != Token_Accepted)
		return Return_Value;

	    return Assign_Parameters(Current_Node, Optional);

	}

	break;

    case 'C':			/* Check, Commit, Convert, Create */
	/* Do we have a check command? */
	if (Current_Token->TokenText == CheckStr) {

	    /* Mark Current_Node as being a Check command. */
	    Current_Node->NodeType = Check_Node;

	    Consume_Token();

	    Return_Value = Check_For_Colon(Optional);
	    if (Return_Value != Token_Accepted)
		return Return_Value;

	    return Check_Parameters(Current_Node, Optional);
	}

	/* Do we have a commit command? */
	if (Current_Token->TokenText == CommitStr) {

	    /* Mark Current_Node as being a Commit command. */
	    Current_Node->NodeType = Commit_Node;

	    Consume_Token();

	    /* Since this is optional, activate Lookahead mode. */
	    Temp_Node = Activate_Lookahead();
	    if (Temp_Node == NULL)
		return Error;

	    Return_Value = Check_For_Colon(TRUE);
	    if (Return_Value == Token_Accepted)
		Return_Value = Commit_Type(Current_Node, TRUE);

	    /* Restore the current token so that parsing can continue. */
	    if ((Return_Value != Token_Accepted) &&
		(!Restore_Current_Token(Temp_Node)))
		return Error;

	    /* Leave lookahead mode. */
	    if (!Terminate_Lookahead
		(Current_Node, Temp_Node,
		 (Return_Value == Token_Accepted)))
		Return_Value = Error;

	    /* Since these parameters were optional, indicate success unless Error was returned. */
	    if (Return_Value == Try_Again)
		Return_Value = Token_Accepted;

	    return Token_Accepted;
	}

	/* Do we have a convert command? */
	if (Current_Token->TokenText == ConvertStr) {

	    /* Mark Current_Node as being a Convert command. */
	    Current_Node->NodeType = Convert_Node;
	    Current_Node->NodeValue = (void *) EVMSStr;

	    Consume_Token();

	    Return_Value = Check_For_Colon(Optional);
	    if (Return_Value != Token_Accepted)
		return Return_Value;

	    Return_Value = Specify_Object(Current_Node, Translate_Name_To_Handle, Optional);	/* Get the storage object which is to become a volume. */

	    if (Return_Value == Token_Accepted) {

		Return_Value = Check_For_Comma(Optional);
		if (Return_Value == Token_Accepted) {

		    /* We have two possible choices here - we need lookahead to decide which we have. */
		    Temp_Node = Activate_Lookahead();
		    if (Temp_Node == NULL)
			return Error;

		    Return_Value = Accept_Name(Temp_Node, TRUE);	/* Get the name for the volume. */
		    if (Return_Value == Try_Again) {

			/* Restore the current token so that parsing can continue. */
			if (!Restore_Current_Token(Temp_Node))
			    return Error;

			Return_Value =
			    Check_For_Compatibility_Keyword(Temp_Node,
							    TRUE);
			if (Return_Value == Token_Accepted)
			    Current_Node->NodeValue =
				(void *) CompatibilityStr;
			else if (Return_Value != Error)
			    Report_Parser_Error
				(Expecting_Name_Or_Compatibility,
				 Command_Filename, Current_Token);

		    }

		    /* Exit Lookahead mode. */
		    if (!Terminate_Lookahead
			(Current_Node, Temp_Node,
			 (Return_Value == Token_Accepted)))
			return Error;

		}

	    }

	    return Return_Value;

	}

	/* Do we have a create command? */
	if ((Current_Token->TokenText == CreateStr) ||
	    (Current_Token->TokenLength == 1)) {

	    /* Mark Current_Node as being a Create command. */
	    Current_Node->NodeType = Create_Node;

	    /* We have a create command. Lets consume the token, check for the
	       colon which should follow the create keyword, and then call
	       the Creation_Parameters function.                                */
	    Consume_Token();

	    Return_Value = Check_For_Colon(Optional);
	    if (Return_Value != Token_Accepted)
		return Return_Value;

	    return Creation_Parameters(Current_Node, Optional);

	}

	break;

    case 'D':			/* Deactivate, Delete */

	/* Do we have a deactivate command? */
	if ((Current_Token->TokenText == DeactivateStr)
	    || (Current_Token->TokenText == DeactStr)) {

	    /* Mark Current_Node as being a Deactivate command. */
	    Current_Node->NodeType = Deactivate_Node;

	    /* We have a deactivate command. Let's consume the token, check for the
	       colon which should follow the deactivate keyword, and then call
	       the Specify_Object function.                                */
	    Consume_Token();

	    Return_Value = Check_For_Colon(Optional);
	    if (Return_Value != Token_Accepted)
		return Return_Value;

	    return Specify_Object(Current_Node, Translate_Name_To_Handle,
				  Optional);

	}

	/* Do we have a delete command? */
	if ((Current_Token->TokenText == DeleteStr) ||
	    (Current_Token->TokenText == DrStr) ||
	    (Current_Token->TokenLength == 1)) {

	    /* Mark Current_Node as being a Revert command, unless Delete Recursive was specified. */
	    if (Current_Token->TokenText == DrStr)
		Current_Node->NodeType = Delete_Node;
	    else
		Current_Node->NodeType = Revert_Node;

	    if (Current_Token->TokenText == DeleteStr) {
		/* We must check to see if the "Recursive" keyword was specified. */

		/* Since this is optional, activate Lookahead mode. */
		Temp_Node = Activate_Lookahead();
		if (Temp_Node == NULL)
		    return Error;

		Consume_Token();
		Next_Token();

		/* Check for the Recursive keyword. */
		if ((Current_Token != NULL) &&
		    (Current_Token->Characterization == KeyWord) &&
		    (Current_Token->TokenText == RecursiveStr)) {
		    /* Since we found the Recursive keyword, correct the node type of Current_Node. */
		    Current_Node->NodeType = Delete_Node;
		} else {
		    /* Restore the current token so that parsing can continue. */
		    if (!Restore_Current_Token(Temp_Node))
			return Error;
		}

		/* Leave lookahead mode. */
		if (!Terminate_Lookahead(Current_Node, Temp_Node, FALSE))
		    return Error;

	    } else
		/* We don't need the current token anymore, so consume it. */
		Consume_Token();

	    Return_Value = Check_For_Colon(Optional);
	    if (Return_Value != Token_Accepted)
		return Return_Value;

	    return Specify_Object(Current_Node, Translate_Name_To_Handle,
				  Optional);

	}

	break;

    case 'E':			/* Echo, Exit, Expand */
	/* Do we have an expand command? */
	if ((Current_Token->TokenText == ExpandStr) ||
	    (Current_Token->TokenLength == 1)) {
	    /* Mark Current_Node as being a Expand command. */
	    Current_Node->NodeType = Expand_Node;

	    /* We have an expand command! */
	    Consume_Token();

	    Return_Value = Check_For_Colon(Optional);
	    if (Return_Value != Token_Accepted)
		return Return_Value;

	    return Expand_Shrink_Task_Parameters(Current_Node, Optional);

	}

	/* Do we have an Echo command? */
	if (Current_Token->TokenText == EchoStr) {
	    /* Mark Current_Node as being a Echo command. */
	    Current_Node->NodeType = Echo_Node;

	    /* We have an Echo command! */
	    Consume_Token();

	    Return_Value = Check_For_Colon(Optional);
	    if (Return_Value != Token_Accepted)
		return Return_Value;

	    /* Since this is optional, activate Lookahead mode. */
	    Temp_Node = Activate_Lookahead();
	    if (Temp_Node == NULL)
		return Error;

	    Return_Value = Get_String(Temp_Node, TRUE, TRUE);

	    /* Restore the current token so that parsing can continue. */
	    if ((Return_Value != Token_Accepted) &&
		(!Restore_Current_Token(Temp_Node)))
		return Error;

	    /* Leave lookahead mode. */
	    if (!Terminate_Lookahead
		(Current_Node, Temp_Node,
		 (Return_Value == Token_Accepted)))
		Return_Value = Error;

	    if (Return_Value != Error)
		Return_Value = Token_Accepted;

	    return Return_Value;

	}

	/* Do we have an Exit command? */
	if (Current_Token->TokenText == ExitStr) {
	    /* Mark Current_Node as being an Exit command.  NOTE:  The Exit and Quit commands are synonyms. */
	    Current_Node->NodeType = Quit_Node;

	    /* We have an Exit command. Lets consume the token.  This command is complete. */
	    Consume_Token();

	    return Token_Accepted;

	}

	break;

    case 'F':			/* Format */
	/* Do we have a format command? */
	if ((Current_Token->TokenText == FormatStr) ||
	    (Current_Token->TokenLength == 1)) {

	    /* Mark Current_Node as being a Format command. */
	    Current_Node->NodeType = Format_Node;

	    /* We have an format command! */
	    Consume_Token();

	    Return_Value = Check_For_Colon(Optional);
	    if (Return_Value != Token_Accepted)
		return Return_Value;

	    return Format_Parameters(Current_Node, Optional);

	}

	break;

    case 'H':			/* Help */
	/* Do we have a help command? */
	if ((Current_Token->TokenText == HelpStr) ||
	    (Current_Token->TokenLength == 1)) {
	    /* Mark Current_Node as being a Help command. */
	    Current_Node->NodeType = Help_Node;

	    /* We have a help command. Lets consume the token, check for the
	       colon which should follow the help keyword, and then call
	       the Help_Type function.                                        */
	    Consume_Token();

	    /* Since this is optional, activate Lookahead mode. */
	    Temp_Node = Activate_Lookahead();
	    if (Temp_Node == NULL)
		return Error;

	    Return_Value = Check_For_Colon(TRUE);
	    if (Return_Value == Token_Accepted)
		Return_Value = Help_Type(Current_Node, TRUE);

	    /* Restore the current token so that parsing can continue. */
	    if ((Return_Value != Token_Accepted) &&
		(!Restore_Current_Token(Temp_Node)))
		return Error;

	    /* Leave lookahead mode. */
	    if (!Terminate_Lookahead
		(Current_Node, Temp_Node,
		 (Return_Value == Token_Accepted)))
		Return_Value = Error;

	    /* Since these parameters were optional, indicate success unless Error was returned. */
	    if (Return_Value == Try_Again)
		Return_Value = Token_Accepted;

	    return Return_Value;

	}

	break;

    case 'M':			/* Mkfs, Modify, Mount */
	if (Current_Token->TokenText == MkfsStr) {
	    /* The Mkfs and Format commands are the same command.  Mkfs & Format are synonyms
	       in the EVMS CLI.                                                               */

	    /* Mark Current_Node as being a Format command. */
	    Current_Node->NodeType = Format_Node;

	    /* We have an format command! */
	    Consume_Token();

	    Return_Value = Check_For_Colon(Optional);
	    if (Return_Value != Token_Accepted)
		return Return_Value;

	    return Format_Parameters(Current_Node, Optional);

	}

	/* Do we have a Modify command? */
	if (Current_Token->TokenText == ModifyStr) {
	    /* The Modify and Set commands are the same command.  Modify & Set are synonyms in
	       the EVMS CLI.                                                                   */

	    /* Mark Current_Node as being a Set command. */
	    Current_Node->NodeType = Set_Node;

	    Consume_Token();

	    Return_Value = Check_For_Colon(Optional);
	    if (Return_Value != Token_Accepted)
		return Return_Value;

	    return Set_Parameters(Current_Node, Translate_Name_To_Handle,
				  Optional);

	}

	/* Do we have a Mount command? */
	if (Current_Token->TokenText == MountStr) {

	    /* Mark Current_Node as being a Mount command. */
	    Current_Node->NodeType = Mount_Node;

	    Consume_Token();

	    Return_Value = Check_For_Colon(Optional);
	    if (Return_Value != Token_Accepted)
		return Return_Value;

	    return Mount_Parameters(Current_Node, Optional);

	}

	break;

    case 'P':			/* Probe */
	/* Do we have a Probe command? */
	if ((Current_Token->TokenText == ProbeStr) ||
	    (Current_Token->TokenLength == 1)) {
	    /* Mark Current_Node as being a Probe command. */
	    Current_Node->NodeType = Probe_Node;

	    /* We have a Probe command. Lets consume the token.  This command is complete. */
	    Consume_Token();

	    return Token_Accepted;

	}

	break;

    case 'Q':			/* Query, Quit */
	/* Do we have a query command? */
	if ((Current_Token->TokenText == QueryStr) ||
	    (Current_Token->TokenLength == 1)) {

	    /* Mark Current_Node as being a Query command. */
	    Current_Node->NodeType = Query_Node;

	    /* We have a query command. Lets consume the token, check for the
	       colon which should follow the query keyword, and then call
	       the Query_Type function.                                        */
	    Consume_Token();

	    Return_Value = Check_For_Colon(Optional);
	    if (Return_Value != Token_Accepted)
		return Return_Value;

	    return Query_Type(Current_Node, Optional);

	}

	/* Do we have a Quit command? */
	if (Current_Token->TokenText == QuitStr) {
	    /* Mark Current_Node as being a Quit command.  NOTE:  The Exit and Quit commands are synonyms. */
	    Current_Node->NodeType = Quit_Node;

	    /* We have a Quit command. Lets consume the token.  This command is complete. */
	    Consume_Token();

	    return Token_Accepted;

	}

	break;

    case 'R':			/* Rename, Remove, Replace or Revert */

	/* Do we have revert? */
	if (Current_Token->TokenText == RevertStr) {

	    /* We have a revert command. Lets consume the token, check for the
	       colon which should follow the revert keyword, and then call
	       the Specify_Object function.                                */
	    Consume_Token();

	    /* Mark Current_Node as being a Revert command. */
	    Current_Node->NodeType = Revert_Node;

	    Return_Value = Check_For_Colon(Optional);
	    if (Return_Value != Token_Accepted)
		return Return_Value;

	    return Specify_Object(Current_Node, Translate_Name_To_Handle,
				  Optional);

	}

	if (Current_Token->TokenText == ReplaceStr) {
	    /* Mark Current_Node as being a Replace command. */
	    Current_Node->NodeType = Replace_Node;

	    Consume_Token();

	    Return_Value = Check_For_Colon(Optional);
	    if (Return_Value != Token_Accepted)
		return Return_Value;

	    Return_Value =
		Specify_Object(Current_Node, Translate_Name_To_Handle,
			       Optional);
	    if (Return_Value != Token_Accepted)
		return Return_Value;

	    Return_Value = Check_For_Comma(Optional);
	    if (Return_Value != Token_Accepted)
		return Return_Value;

	    return Specify_Object(Current_Node, Translate_Name_To_Handle,
				  Optional);

	}

	if (Current_Token->TokenText == RenameStr) {
	    /* Mark Current_Node as being a Rename command. */
	    Current_Node->NodeType = Rename_Node;

	    Consume_Token();

	    Return_Value = Check_For_Colon(Optional);
	    if (Return_Value != Token_Accepted)
		return Return_Value;

	    Return_Value =
		Specify_Object(Current_Node, Translate_Name_To_Handle,
			       Optional);
	    if (Return_Value != Token_Accepted)
		return Return_Value;

	    Return_Value = Check_For_Comma(Optional);
	    if (Return_Value != Token_Accepted)
		return Return_Value;

	    return Accept_Name(Current_Node, Optional);

	}

	if (Current_Token->TokenText == RemoveStr) {
	    /* Mark Current_Node as being a Remove command. */
	    Current_Node->NodeType = Remove_Node;

	    Consume_Token();

	    Return_Value = Check_For_Colon(Optional);
	    if (Return_Value != Token_Accepted)
		return Return_Value;

	    return Specify_Object(Current_Node, Translate_Name_To_Handle,
				  Optional);

	}

	break;

    case 'S':			/* Save, Set, Shrink */
	/* Do we have a Save Command? */
	if (Current_Token->TokenText == SaveStr) {
	    /* The save and commit commands are the same command.  Save & Commit are synonyms in
	       the EVMS CLI.                                                                      */

	    /* Mark Current_Node as being a Commit command. */
	    Current_Node->NodeType = Commit_Node;

	    Consume_Token();

	    /* Since this is optional, activate Lookahead mode. */
	    Temp_Node = Activate_Lookahead();
	    if (Temp_Node == NULL)
		return Error;

	    Return_Value = Check_For_Colon(TRUE);
	    if (Return_Value == Token_Accepted)
		Return_Value = Commit_Type(Current_Node, TRUE);

	    /* Restore the current token so that parsing can continue. */
	    if ((Return_Value != Token_Accepted) &&
		(!Restore_Current_Token(Temp_Node)))
		return Error;

	    /* Leave lookahead mode. */
	    if (!Terminate_Lookahead
		(Current_Node, Temp_Node,
		 (Return_Value == Token_Accepted)))
		Return_Value = Error;

	    /* Since these parameters were optional, indicate success unless Error was returned. */
	    if (Return_Value == Try_Again)
		Return_Value = Token_Accepted;

	    return Token_Accepted;
	}

	/* Do we have a shrink command? */
	if ((Current_Token->TokenText == ShrinkStr) ||
	    (Current_Token->TokenLength == 1)) {
	    /* Mark Current_Node as being a Shrink command. */
	    Current_Node->NodeType = Shrink_Node;

	    Consume_Token();

	    Return_Value = Check_For_Colon(Optional);
	    if (Return_Value != Token_Accepted)
		return Return_Value;

	    return Expand_Shrink_Task_Parameters(Current_Node, Optional);

	}

	/* Do we have a Set command? */
	if (Current_Token->TokenText == SetStr) {
	    /* Mark Current_Node as being a Set command. */
	    Current_Node->NodeType = Set_Node;

	    Consume_Token();

	    Return_Value = Check_For_Colon(Optional);
	    if (Return_Value != Token_Accepted)
		return Return_Value;

	    return Set_Parameters(Current_Node, Translate_Name_To_Handle,
				  Optional);

	}

	break;
    case 'T':			/* Task */
	if ((Current_Token->TokenText == TaskStr) ||
	    (Current_Token->TokenLength == 1)) {
	    /* Mark Current_Node as being a Task command. */
	    Current_Node->NodeType = Task_Node;

	    Consume_Token();

	    Return_Value = Check_For_Colon(Optional);
	    if (Return_Value != Token_Accepted)
		return Return_Value;

	    /* Get the Task Name. */
	    Return_Value = Specify_Object(Current_Node, StringData, FALSE);
	    if (Return_Value != Token_Accepted)
		return Return_Value;

	    Return_Value = Check_For_Comma(FALSE);
	    if (Return_Value != Token_Accepted)
		return Return_Value;

	    return Expand_Shrink_Task_Parameters(Current_Node, FALSE);

	}

	break;
    case 'U':			/* Unformat, Unmkfs, Unmount */
	/* Do we have an Unformat command? */
	if ((Current_Token->TokenLength == 1) ||
	    ((Current_Token->TokenText == UnformatStr) ||
	     (Current_Token->TokenText == UnMkfsStr))) {
	    /* Mark Current_Node as being an Unformat command. */
	    Current_Node->NodeType = Unformat_Node;

	    Consume_Token();

	    Return_Value = Check_For_Colon(Optional);
	    if (Return_Value != Token_Accepted)
		return Return_Value;

	    return Specify_Object(Current_Node, Name_To_Volume_Handle,
				  Optional);

	}

	if (Current_Token->TokenText == UnmountStr) {
	    /* Mark Current_Node as being an Unmount command. */
	    Current_Node->NodeType = Unmount_Node;

	    Consume_Token();

	    Return_Value = Check_For_Colon(Optional);
	    if (Return_Value != Token_Accepted)
		return Return_Value;

	    return Specify_Object(Current_Node, Translate_Name_To_Handle,
				  Optional);

	}

	break;
    default:			/* Error! Unrecognized command! */
	break;

    }

    /* No command found. */
    if (Optional)
	return Try_Again;

    Report_Parser_Error(Expecting_Command_Name, NULL, Current_Token);

    return Error;

}

static Parse_Results
Assign_Parameters(Executable_Node * Current_Node, boolean Optional)
{
    Parse_Results Return_Value = Error;	/* Used to track the return value from other functions called by this function. */

    /* Get the segment manager to assign, along with its options. */
    Return_Value = Plugin_Identifier(Current_Node, TRUE, TRUE, Optional);

    return Return_Value;

}


static Parse_Results
Allocation_Parameters(Executable_Node * Current_Node, boolean Optional)
{
    Executable_Node *Temp_Node = NULL;	/* Used for lookahead operations. */
    Executable_Node *Child = NULL;	/* Used for the "dummy" object list node.  This object list node will have just one child. */
    Parse_Results Return_Value = Error;	/* Used to track the return value from other functions called by this function. */

    /* Create the child node and add it to the children list of Current_Node.  This child node will be an object list node. */
    Child = Make_Child_Node(Current_Node, TRUE);
    if (Child == NULL)
	return Error;

    /* To make the Interpreter code simpler, we will create an object list node to place the single object used by this command in. */
    Child->NodeType = ObjectList;

    /* Get the container/region we are to allocate from. */
    Return_Value =
	Specify_Object(Child, Translate_Name_To_Handle, Optional);
    if (Return_Value == Token_Accepted) {
	Return_Value = Check_For_Comma(Optional);
	if (Return_Value == Token_Accepted) {
	    /* Get the parameters which tell us how to allocate from the container. */

	    Temp_Node = Activate_Lookahead();
	    if (Temp_Node == NULL)
		return Error;

	    do {
		Return_Value = Parse_Name_Value_Pair(Temp_Node, TRUE);

		if (Return_Value == Try_Again)
		    if (!Restore_Current_Token(Temp_Node))
			return Error;

		if (!Terminate_Lookahead
		    (Current_Node, Temp_Node,
		     (Return_Value == Token_Accepted)))
		    return Error;

		if (Return_Value == Token_Accepted) {
		    /* We have just parsed a name-value pair.  Look for another one. */

		    Temp_Node = Activate_Lookahead();
		    if (Temp_Node == NULL)
			return Error;

		    Return_Value = Check_For_Comma(TRUE);
		    if (Return_Value == Try_Again) {
			if (!Restore_Current_Token(Temp_Node))
			    return Error;

		    }

		    if ((Return_Value != Token_Accepted)
			&&
			(!Terminate_Lookahead
			 (Current_Node, Temp_Node, FALSE)))
			return Error;

		}

	    }
	    while (Return_Value == Token_Accepted);

	    if (Return_Value != Error)
		Return_Value = Token_Accepted;

	}

    }

    return Return_Value;

}


static Parse_Results
Creation_Parameters(Executable_Node * Current_Node, boolean Optional)
{
    Executable_Node *Temp_Node = NULL;	/* Used for lookahead operations. */
    Executable_Node *Child = NULL;	/* Used for the "dummy" object list node.  This object list node will have just one child. */
    Parse_Results Return_Value = Error;	/* Used to track the return value from other functions called by this function. */

    /* Get the token to parse. */
    Next_Token();

    /* Check for EOF or InvalidCharacter */
    if ((Current_Token == NULL)
	|| (Current_Token->Characterization == EofToken)
	|| (Current_Token->Characterization == InvalidCharacter)) {

	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Creation_Parameters,
			    Command_Filename, Current_Token);

	return Error;

    }

    /* The parameter we are looking for is characterized as a KeyWord.  */
    if (Current_Token->Characterization != KeyWord) {

	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Creation_Parameters,
			    Command_Filename, Current_Token);

	return Error;

    }

    /* Lets see which creation parameter we have. */
    switch (Current_Token->TokenText[0]) {
    case 'V':			/* Volume */
	if ((Current_Token->TokenText == VolumeStr) ||
	    (Current_Token->TokenLength == 1)) {
	    /* We are going to create a volume.  Mark Current_Node. */
	    Current_Node->NodeValue = (void *) VolumeStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    /* The volume keyword should be followed by a comma. */
	    Return_Value = Check_For_Comma(Optional);
	    if (Return_Value == Token_Accepted) {
		Return_Value = Specify_Object(Current_Node, Translate_Name_To_Handle, Optional);	/* Get the storage object which is to become a volume. */

		if (Return_Value == Token_Accepted) {
		    Return_Value = Check_For_Comma(Optional);
		    if (Return_Value == Token_Accepted) {

			/* We have two possible choices here - we need lookahead to decide which we have. */
			Temp_Node = Activate_Lookahead();
			if (Temp_Node == NULL)
			    return Error;

			Return_Value = Accept_Name(Temp_Node, TRUE);	/* Get the name for the volume. */
			if (Return_Value == Try_Again) {

			    /* Restore the current token so that parsing can continue. */
			    if (!Restore_Current_Token(Temp_Node))
				return Error;

			    Return_Value =
				Check_For_Compatibility_Keyword(Temp_Node,
								TRUE);
			    if (Return_Value == Token_Accepted)
				Current_Node->NodeValue =
				    (void *) CompatibilityStr;
			    else if (Return_Value != Error)
				Report_Parser_Error
				    (Expecting_Name_Or_Compatibility,
				     Command_Filename, Current_Token);

			}

			/* Exit Lookahead mode. */
			if (!Terminate_Lookahead
			    (Current_Node, Temp_Node,
			     (Return_Value == Token_Accepted)))
			    return Error;

		    }

		}

	    }

	} else {
	    if (Optional)
		return Try_Again;

	    Report_Parser_Error(Expecting_Volume_Parameter,
				Command_Filename, Current_Token);
	    Return_Value = Error;
	}

	break;
    case 'O':			/* Object */
	if ((Current_Token->TokenText == ObjectStr) ||
	    (Current_Token->TokenLength == 1)) {
	    /* We are going to create an EVMS object.  Mark Current_Node. */
	    Current_Node->NodeValue = (void *) ObjectStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    /* Check for the comma which follows the Object keyword. */
	    Return_Value = Check_For_Comma(Optional);
	    if (Return_Value == Token_Accepted) {

		Return_Value =
		    Feature_Identifier(Current_Node, Optional, TRUE,
				       FALSE);

	    }

	} else {
	    if (Optional)
		return Try_Again;

	    Report_Parser_Error(Expecting_Object_Parameter,
				Command_Filename, Current_Token);
	    Return_Value = Error;
	}

	break;

    case 'C':
	if ((Current_Token->TokenText == ContainerStr) ||
	    (Current_Token->TokenLength == 1)) {
	    /* We are going to create a container.  Mark Current_Node. */
	    Current_Node->NodeValue = (void *) ContainerStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    Return_Value = Check_For_Comma(Optional);
	    if (Return_Value == Token_Accepted) {

		Return_Value =
		    Plugin_Identifier(Current_Node, TRUE, TRUE, Optional);

	    }

	} else {
	    if (Optional)
		return Try_Again;

	    Report_Parser_Error(Expecting_Container_Parameter,
				Command_Filename, Current_Token);
	    Return_Value = Error;
	}

	break;

    case 'R':			/* Region */
	if ((Current_Token->TokenText == RegionStr) ||
	    (Current_Token->TokenLength == 1)) {
	    /* We are going to create a region.  Mark Current_Node. */
	    Current_Node->NodeValue = (void *) RegionStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    Return_Value = Check_For_Comma(Optional);
	    if (Return_Value == Token_Accepted) {

		Return_Value =
		    Plugin_Identifier(Current_Node, TRUE, TRUE, Optional);

	    }

	} else {
	    if (Optional)
		return Try_Again;

	    Report_Parser_Error(Expecting_Volume_Parameter,
				Command_Filename, Current_Token);
	    Return_Value = Error;
	}

	break;

    case 'S':			/* Segment */
	if ((Current_Token->TokenText == SegmentStr) ||
	    (Current_Token->TokenLength == 1)) {
	    /* We are going to allocate a segment from a block of freespace.  Mark Current_Node. */
	    Current_Node->NodeType = Allocate_Node;
	    Current_Node->NodeValue = (void *) SegmentStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    Return_Value = Check_For_Comma(Optional);
	    if (Return_Value == Token_Accepted) {
		/* Create the child node and add it to the children list of Current_Node.  This child node will be an object list node. */
		Child = Make_Child_Node(Current_Node, TRUE);
		if (Child == NULL)
		    return Error;

		/* To make the Interpreter code simpler, we will create an object list node to place the single object used by this command in. */
		Child->NodeType = ObjectList;

		/* Get the block of freespace to allocate from. */
		Return_Value =
		    Specify_Object(Child, Name_To_Segment_Handle,
				   Optional);
		if (Return_Value == Token_Accepted) {
		    Return_Value = Check_For_Comma(Optional);
		    if (Return_Value == Token_Accepted) {
			/* Get the parameters which tell us how to allocate from the block of freespace. */

			Temp_Node = Activate_Lookahead();
			if (Temp_Node == NULL)
			    return Error;

			do {
			    Return_Value =
				Parse_Name_Value_Pair(Temp_Node, TRUE);

			    if (Return_Value == Try_Again)
				if (!Restore_Current_Token(Temp_Node))
				    return Error;

			    if (!Terminate_Lookahead
				(Current_Node, Temp_Node,
				 (Return_Value == Token_Accepted)))
				return Error;

			    if (Return_Value == Token_Accepted) {
				/* We have just parsed a name-value pair.  Look for another one. */

				Temp_Node = Activate_Lookahead();
				if (Temp_Node == NULL)
				    return Error;

				Return_Value = Check_For_Comma(TRUE);
				if (Return_Value == Try_Again) {
				    if (!Restore_Current_Token(Temp_Node))
					return Error;

				}

				if ((Return_Value != Token_Accepted)
				    &&
				    (!Terminate_Lookahead
				     (Current_Node, Temp_Node, FALSE)))
				    return Error;

			    }

			}
			while (Return_Value == Token_Accepted);

			if (Return_Value != Error)
			    Return_Value = Token_Accepted;

		    }

		}

	    }

	} else {
	    if (Optional)
		return Try_Again;

	    Report_Parser_Error(Expecting_Volume_Parameter,
				Command_Filename, Current_Token);
	    Return_Value = Error;
	}

	break;
    default:
	Report_Parser_Error(Expecting_Creation_Parameters,
			    Command_Filename, Current_Token);

	return Error;

	break;

    }

    return Return_Value;

}


static Parse_Results
Check_For_Compatibility_Keyword(Executable_Node * Current_Node,
				boolean Optional)
{
    /* Get the token to parse. */
    Next_Token();

    /* Check for EOF or InvalidCharacter */
    if ((Current_Token == NULL) ||
	(Current_Token->Characterization == EofToken) ||
	(Current_Token->Characterization == InvalidCharacter) ||
	(Current_Token->Characterization != KeyWord)) {

	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Compatibility_Keyword,
			    Command_Filename, Current_Token);

	return Error;

    }

    if ((Current_Token->TokenText == CompatibilityStr) ||
	((Current_Token->TokenLength == 1) &&
	 (Current_Token->TokenText[0] == 'C'))) {

	/* We are through with this token, so consume it. */
	Consume_Token();

	return Token_Accepted;

    } else {
	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Compatibility_Keyword,
			    Command_Filename, Current_Token);
    }

    return Error;

}


static Parse_Results
Check_Parameters(Executable_Node * Current_Node, boolean Optional)
{
    return Set_Parameters(Current_Node, Name_To_Volume_Handle, Optional);
}


static Parse_Results
Add_Feature_Parameters(Executable_Node * Current_Node, boolean Optional)
{
    Parse_Results Return_Value = Error;	/* Used to track the return value from other functions called by this function. */

    /* Get the feature, along with its options. */
    Return_Value = Plugin_Identifier(Current_Node, TRUE, TRUE, Optional);

    return Return_Value;

}


static Parse_Results
Format_Parameters(Executable_Node * Current_Node, boolean Optional)
{
    Parse_Results Return_Value = Error;	/* Used to track the return value from other functions called by this function. */

    /* Get the FSIM, along with its options. */
    Return_Value = Plugin_Identifier(Current_Node, TRUE, TRUE, Optional);

    return Return_Value;

}


static Parse_Results
Set_Parameters(Executable_Node * Current_Node, NodeTypes Node_To_Create,
	       boolean Optional)
{
    Executable_Node *Temp_Node = NULL;	/* Used for lookahead operations. */
    Parse_Results Return_Value = Error;	/* Used to track the return value from other functions called by this function. */

    Return_Value = Specify_Object(Current_Node, Node_To_Create, Optional);
    if (Return_Value != Token_Accepted)
	return Return_Value;

    /* Get however many name-value pairs there may be. */
    do {
	/* The following name-value pairs are optional. */
	Temp_Node = Activate_Lookahead();
	if (Temp_Node == NULL)
	    return Error;

	/* Check for a comma - if found, a name-value pair may follow. */
	Return_Value = Check_For_Comma(TRUE);
	if (Return_Value == Token_Accepted)
	    Return_Value = Parse_Name_Value_Pair(Temp_Node, TRUE);

	if (Return_Value == Try_Again) {
	    /* Restore the current token so that parsing can continue. */
	    if (!Restore_Current_Token(Temp_Node))
		return Error;
	}

	/* Exit Lookahead mode. */
	if (!Terminate_Lookahead
	    (Current_Node, Temp_Node, (Return_Value == Token_Accepted)))
	    return Error;

    }
    while (Return_Value == Token_Accepted);

    if (Return_Value == Error)
	return Error;
    else
	return Token_Accepted;

}

static Parse_Results
Expand_Shrink_Task_Parameters(Executable_Node * Current_Node,
			      boolean Optional)
{
    Executable_Node *Temp_Node = NULL;	/* Used for lookahead operations. */
    Parse_Results Return_Value = Error;	/* Used to track the return value from other functions called by this function. */

    Return_Value =
	Specify_Object(Current_Node, Translate_Name_To_Handle, Optional);
    if (Return_Value != Token_Accepted)
	return Return_Value;

    /* The expansion/shrink point may be followed by a series of Name-Value pairs. */

    /* Get however many name-value pairs there may be. */
    do {
	/* The following name-value pairs are optional. */
	Temp_Node = Activate_Lookahead();
	if (Temp_Node == NULL)
	    return Error;

	/* Check for a comma - if found, a name-value pair may follow. */
	Return_Value = Check_For_Comma(TRUE);
	if (Return_Value == Token_Accepted)
	    Return_Value = Parse_Name_Value_Pair(Temp_Node, TRUE);

	if (Return_Value == Try_Again) {
	    /* Restore the current token so that parsing can continue. */
	    if (!Restore_Current_Token(Temp_Node))
		return Error;
	}

	/* Exit Lookahead mode. */
	if (!Terminate_Lookahead
	    (Current_Node, Temp_Node, (Return_Value == Token_Accepted)))
	    return Error;

    }
    while (Return_Value == Token_Accepted);

    if (Return_Value == Error)
	return Return_Value;

    /* Now we must get any objects to be used for the expansion. */
    Temp_Node = Activate_Lookahead();
    if (Temp_Node == NULL)
	return Error;

    Return_Value = Parse_Object_List(Temp_Node, TRUE);

    if (Return_Value == Try_Again) {
	/* Restore the current token so that parsing can continue. */
	if (!Restore_Current_Token(Temp_Node))
	    return Error;
    }

    /* Exit Lookahead mode. */
    if (!Terminate_Lookahead
	(Current_Node, Temp_Node, (Return_Value == Token_Accepted)))
	return Error;

    /* Since the specification of storage objects or name value pairs is optional in some circumstances,
       a Return_Value of Try_Again is acceptable and we will thus translate it to Token_Accepted.         */
    if (Return_Value != Error)
	Return_Value = Token_Accepted;

    return Return_Value;

}


static Parse_Results
Mount_Parameters(Executable_Node * Current_Node, boolean Optional)
{
    Executable_Node *Temp_Node = NULL;	/* Used for lookahead operations. */
    Parse_Results Return_Value = Error;	/* Used to track the return value from other functions called by this function. */

    /* Get the handle for the volume name. */
    Return_Value =
	Specify_Object(Current_Node, Translate_Name_To_Handle, Optional);
    if (Return_Value != Token_Accepted)
	return Return_Value;

    Return_Value = Check_For_Comma(Optional);
    if (Return_Value != Token_Accepted)
	return Return_Value;

    /* Get the mount point. */
    Return_Value = Specify_Object(Current_Node, StringData, Optional);
    if (Return_Value != Token_Accepted)
	return Return_Value;

    /* The following mount options string is optional. */
    Temp_Node = Activate_Lookahead();
    if (Temp_Node == NULL)
	return Error;

    /* Check for a comma - if found, an options string may follow. */
    Return_Value = Check_For_Comma(TRUE);
    if (Return_Value == Token_Accepted)
	Return_Value = Specify_Object(Temp_Node, StringData, Optional);

    if (Return_Value == Try_Again) {
	/* Restore the current token so that parsing can continue. */
	if (!Restore_Current_Token(Temp_Node))
	    return Error;
    }

    /* Exit Lookahead mode. */
    if (!Terminate_Lookahead
	(Current_Node, Temp_Node, (Return_Value == Token_Accepted)))
	return Error;

    /* Since the specification of the mount options is optional, a Return_Value
       of Try_Again is acceptable and we will thus translate it to
       Token_Accepted. */
    if (Return_Value != Error)
	Return_Value = Token_Accepted;

    return Return_Value;

}


static Parse_Results
Query_Type(Executable_Node * Current_Node, boolean Optional)
{
    Parse_Results Return_Value;	/* Used to track the return value from other functions called by this function. */
    Available_Generic_Filters Allowed_Filters = 0;	/* Used to specify which filters are allowed for a particular Query. */

    /* Get the token to parse. */
    Next_Token();

    /* Check for EOF or InvalidCharacter */
    if ((Current_Token == NULL)
	|| (Current_Token->Characterization == EofToken)
	|| (Current_Token->Characterization == InvalidCharacter)) {

	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Query_Parameters, Command_Filename,
			    Current_Token);

	return Error;

    }

    /* The parameter we are looking for is characterized as a KeyWord.  */
    if (Current_Token->Characterization != KeyWord) {

	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Query_Parameters, Command_Filename,
			    Current_Token);

	return Error;

    }

    /* Lets see which query parameter we have. */
    switch (Current_Token->TokenText[0]) {
    case 'P':			/* Plugins or Parent */
	if ((Current_Token->TokenText == PluginsStr) ||
	    (Current_Token->TokenLength == 1)) {
	    /* We are going to produce a query about plugins.  Mark Current_Node. */
	    Current_Node->NodeValue = (void *) PluginsStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    /* The remaining parameters are optional.  Lets see if any were specified. */
	    Allowed_Filters |= Available_Filter;
	    Allowed_Filters |= Container_Filter;
	    Allowed_Filters |= Plugin_Filter;
//        Allowed_Filters |= LT_Filter;
//        Allowed_Filters |= GT_Filter;
//        Allowed_Filters |= EQ_Filter;
	    Allowed_Filters |= Volume_Filter;
//        Allowed_Filters |= Expandable_Filter;
//        Allowed_Filters |= Shrinkable_Filter;
//        Allowed_Filters |= Unclaimed_Filter;
	    Allowed_Filters |= Region_Filter;
	    Allowed_Filters |= LO_Filter;
	    Allowed_Filters |= Object_Filter;
	    Allowed_Filters |= Plugin_Type_Filter;
	    Return_Value =
		Parse_Optional_Query_Filters(Current_Node, Allowed_Filters,
					     TRUE);

	    if (Return_Value == Error)
		break;

	    Return_Value = Token_Accepted;	/* The command is complete. */

	} else {

	    if (Current_Token->TokenText == ParentStr) {
		/* We are going to produce a query about an objects parent.  Mark Current_Node. */
		Current_Node->NodeValue = (void *) ParentStr;
		Current_Node->NodeType = Query_Node_Type_2;

		/* We are through with this token, so consume it. */
		Consume_Token();

		/* Check for a comma. */
		Return_Value = Check_For_Comma(Optional);

		if (Return_Value == Error)
		    break;

		if (Return_Value == Try_Again) {

		    if (Optional)
			return Try_Again;

		    Report_Parser_Error(Expecting_Region_Or_Object,
					Command_Filename, Current_Token);
		    Return_Value = Error;

		} else
		    Return_Value =
			Specify_Object(Current_Node,
				       Translate_Name_To_Handle, Optional);

	    } else {

		if (Optional)
		    return Try_Again;

		Report_Parser_Error(Expecting_Plugins_Or_Parent_Parameter,
				    Command_Filename, Current_Token);
		Return_Value = Error;

	    }

	}

	break;

    case 'V':			/* Volume */
	if ((Current_Token->TokenText == VolumesStr) ||
	    (Current_Token->TokenLength == 1)) {
	    /* We are going to produce a query about volumes.  Mark Current_Node. */
	    Current_Node->NodeValue = (void *) VolumesStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    /* The remaining parameters are optional.  Lets see if any were specified. */
//        Allowed_Filters |= Available_Filter;
	    Allowed_Filters |= Container_Filter;
	    Allowed_Filters |= Plugin_Filter;
	    Allowed_Filters |= LT_Filter;
	    Allowed_Filters |= GT_Filter;
	    Allowed_Filters |= EQ_Filter;
	    Allowed_Filters |= Volume_Filter;
	    Allowed_Filters |= Expandable_Filter;
	    Allowed_Filters |= Shrinkable_Filter;
//        Allowed_Filters |= Unclaimed_Filter;
	    Allowed_Filters |= Region_Filter;
	    Allowed_Filters |= LO_Filter;
	    Allowed_Filters |= Object_Filter;
//        Allowed_Filters |= Plugin_Type_Filter;
	    Allowed_Filters |= Convertible_Filter;
	    Return_Value =
		Parse_Optional_Query_Filters(Current_Node, Allowed_Filters,
					     TRUE);

	    if (Return_Value == Error)
		break;

	    Return_Value = Token_Accepted;	/* The command is complete. */

	} else {

	    if (Optional)
		return Try_Again;

	    Report_Parser_Error(Expecting_Volumes_Parameter,
				Command_Filename, Current_Token);
	    Return_Value = Error;

	}

	break;
    case 'R':			/* Region */
	if ((Current_Token->TokenText == RegionsStr) ||
	    (Current_Token->TokenLength == 1)) {
	    /* We are going to produce a query about regions.  Mark Current_Node. */
	    Current_Node->NodeValue = (void *) RegionsStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    /* The remaining parameters are optional.  Lets see if any were specified. */
	    Allowed_Filters |= Available_Filter;
	    Allowed_Filters |= Container_Filter;
	    Allowed_Filters |= Plugin_Filter;
	    Allowed_Filters |= LT_Filter;
	    Allowed_Filters |= GT_Filter;
	    Allowed_Filters |= EQ_Filter;
	    Allowed_Filters |= Volume_Filter;
	    Allowed_Filters |= Expandable_Filter;
	    Allowed_Filters |= Shrinkable_Filter;
	    Allowed_Filters |= Unclaimed_Filter;
	    Allowed_Filters |= Region_Filter;
	    Allowed_Filters |= LO_Filter;
	    Allowed_Filters |= Object_Filter;
//        Allowed_Filters |= Plugin_Type_Filter;
//        Allowed_Filters |= Convertible_Filter;
	    Return_Value =
		Parse_Optional_Query_Filters(Current_Node, Allowed_Filters,
					     TRUE);

	    if (Return_Value == Error)
		break;

	    Return_Value = Token_Accepted;	/* The command is complete. */

	} else {

	    if (Optional)
		return Try_Again;

	    Report_Parser_Error(Expecting_Regions_Parameter,
				Command_Filename, Current_Token);
	    Return_Value = Error;

	}

	break;
    case 'D':			/* Disks */
	if ((Current_Token->TokenText == DisksStr) ||
	    (Current_Token->TokenLength == 1)) {
	    /* We are going to produce a query about the disks in the system.  Mark Current_Node. */
	    Current_Node->NodeValue = (void *) DisksStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    /* The remaining parameters are optional.  Lets see if any were specified. */
	    Allowed_Filters |= Available_Filter;
//        Allowed_Filters |= Container_Filter;
	    Allowed_Filters |= Plugin_Filter;
	    Allowed_Filters |= LT_Filter;
	    Allowed_Filters |= GT_Filter;
	    Allowed_Filters |= EQ_Filter;
//        Allowed_Filters |= Volume_Filter;
//        Allowed_Filters |= Expandable_Filter;
//        Allowed_Filters |= Shrinkable_Filter;
	    Allowed_Filters |= Unclaimed_Filter;
//        Allowed_Filters |= Region_Filter;
	    Allowed_Filters |= LO_Filter;
//        Allowed_Filters |= Object_Filter;
//        Allowed_Filters |= Plugin_Type_Filter;
//        Allowed_Filters |= Convertible_Filter;
	    Allowed_Filters |= Segment_Filter;
	    Allowed_Filters |= Disk_Filter;
	    Return_Value =
		Parse_Optional_Query_Filters(Current_Node, Allowed_Filters,
					     TRUE);

	    if (Return_Value == Error)
		break;

	    Return_Value = Token_Accepted;	/* The command is complete. */

	} else {

	    if (Optional)
		return Try_Again;

	    Report_Parser_Error(Expecting_Disks_Parameter,
				Command_Filename, Current_Token);
	    Return_Value = Error;

	}

	break;
    case 'S':			/* Segments, Shrink Points */
	if ((Current_Token->TokenText == SegmentsStr) ||
	    (Current_Token->TokenLength == 1)) {
	    /* We are going to produce a query about the disk segments in the system.  Mark Current_Node. */
	    Current_Node->NodeValue = (void *) SegmentsStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    /* The remaining parameters are optional.  Lets see if any were specified. */
	    Allowed_Filters |= Available_Filter;
	    Allowed_Filters |= Container_Filter;
	    Allowed_Filters |= Plugin_Filter;
	    Allowed_Filters |= LT_Filter;
	    Allowed_Filters |= GT_Filter;
	    Allowed_Filters |= EQ_Filter;
//        Allowed_Filters |= Volume_Filter;
	    Allowed_Filters |= Expandable_Filter;
	    Allowed_Filters |= Shrinkable_Filter;
	    Allowed_Filters |= Unclaimed_Filter;
//        Allowed_Filters |= Region_Filter;
	    Allowed_Filters |= LO_Filter;
//        Allowed_Filters |= Object_Filter;
//        Allowed_Filters |= Plugin_Type_Filter;
//        Allowed_Filters |= Convertible_Filter;
	    Allowed_Filters |= Disk_Filter;
	    Allowed_Filters |= Segment_Filter;
	    Allowed_Filters |= Freespace_Filter;
	    Return_Value =
		Parse_Optional_Query_Filters(Current_Node, Allowed_Filters,
					     TRUE);

	    if (Return_Value == Error)
		break;

	    Return_Value = Token_Accepted;	/* The command is complete. */

	} else if (Current_Token->TokenText == SPStr) {
	    /* We are going to query the shrink points of a volume or object. */
	    Current_Node->NodeValue = (void *) SPStr;
	    Current_Node->NodeType = Query_Node_Type_2;
	    Consume_Token();

	    /* Check for a comma. */
	    Return_Value = Check_For_Comma(Optional);

	    if (Return_Value == Error)
		break;

	    if (Return_Value == Try_Again) {

		if (Optional)
		    return Try_Again;

		Report_Parser_Error(Expecting_Volume_Or_Object,
				    Command_Filename, Current_Token);
		Return_Value = Error;

	    } else
		Return_Value =
		    Specify_Object(Current_Node, Translate_Name_To_Handle,
				   Optional);

	} else if (Current_Token->TokenText == ShrinkStr) {

	    Consume_Token();
	    Next_Token();

	    if ((Current_Token != NULL) &&
		(Current_Token->Characterization == KeyWord) &&
		(Current_Token->TokenText == PointsStr)) {
		/* We are going to query the shrink points of a volume or object. */
		Current_Node->NodeValue = (void *) SPStr;
		Current_Node->NodeType = Query_Node_Type_2;
		Consume_Token();

		/* Check for a comma. */
		Return_Value = Check_For_Comma(Optional);

		if (Return_Value == Error)
		    break;

		if (Return_Value == Try_Again) {

		    if (Optional)
			return Try_Again;

		    Report_Parser_Error(Expecting_Volume_Or_Object,
					Command_Filename, Current_Token);
		    Return_Value = Error;

		} else
		    Return_Value =
			Specify_Object(Current_Node,
				       Translate_Name_To_Handle, Optional);

	    } else {
		if (Optional)
		    return Try_Again;

		Report_Parser_Error(Expecting_Query_Shrink_Points_Keywords,
				    Command_Filename, Current_Token);

		return Error;

	    }

	} else {

	    if (Optional)
		return Try_Again;

	    Report_Parser_Error(Expecting_Query_Parameters,
				Command_Filename, Current_Token);

	    return Error;

	}


	break;

    case 'A':			/* Acceptable */
	if ((Current_Token->TokenText == AcceptableStr) ||
	    (Current_Token->TokenLength == 1)) {
	    /* We are going to produce a query about what is acceptable to a plugin.  Mark Current_Node. */
	    Current_Node->NodeValue = (void *) AcceptableStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    /* Check for a comma. */
	    Return_Value = Check_For_Comma(Optional);

	    if (Return_Value == Error)
		break;

	    if (Return_Value == Try_Again) {

		if (Optional)
		    return Try_Again;

		Report_Parser_Error(Expecting_Acceptable_Parameter,
				    Command_Filename, Current_Token);
		Return_Value = Error;

	    }

	    /* Now we know that a parameter should follow. */
	    Return_Value =
		Parse_Query_Acceptable_Parameters(Current_Node, Optional);

	} else {

	    if (Optional)
		return Try_Again;

	    Report_Parser_Error(Expecting_Acceptable_Parameter,
				Command_Filename, Current_Token);
	    Return_Value = Error;

	}

	break;
    case 'E':			/* Expand Points, Extended Info */
	if (Current_Token->TokenText == EPStr) {
	    /* We are going to query the expansion points of a volume or object. */
	    Current_Node->NodeValue = (void *) EPStr;
	    Current_Node->NodeType = Query_Node_Type_2;
	    Consume_Token();

	    /* Check for a comma. */
	    Return_Value = Check_For_Comma(Optional);

	    if (Return_Value == Error)
		break;

	    if (Return_Value == Try_Again) {

		if (Optional)
		    return Try_Again;

		Report_Parser_Error(Expecting_Volume_Or_Object,
				    Command_Filename, Current_Token);
		Return_Value = Error;

	    } else
		Return_Value =
		    Specify_Object(Current_Node, Translate_Name_To_Handle,
				   Optional);

	} else if (Current_Token->TokenText == ExpandStr) {

	    Consume_Token();
	    Next_Token();

	    if ((Current_Token != NULL) &&
		(Current_Token->Characterization == KeyWord) &&
		(Current_Token->TokenText == PointsStr)) {
		/* We are going to query the expansion points of a volume or object. */
		Current_Node->NodeValue = (void *) EPStr;
		Current_Node->NodeType = Query_Node_Type_2;
		Consume_Token();

		/* Check for a comma. */
		Return_Value = Check_For_Comma(Optional);

		if (Return_Value == Error)
		    break;

		if (Return_Value == Try_Again) {

		    if (Optional)
			return Try_Again;

		    Report_Parser_Error(Expecting_Volume_Or_Object,
					Command_Filename, Current_Token);
		    Return_Value = Error;

		} else
		    Return_Value =
			Specify_Object(Current_Node,
				       Translate_Name_To_Handle, Optional);

	    } else {
		if (Optional)
		    return Try_Again;

		Report_Parser_Error(Expecting_Query_Expand_Points_Keywords,
				    Command_Filename, Current_Token);

		return Error;

	    }

	} else if (Current_Token->TokenText == EIStr) {
	    /* We are going to query the extended info of something. */
	    Current_Node->NodeValue = (void *) NULL;
	    Current_Node->NodeType = Query_Extended_Info;
	    Consume_Token();

	    Return_Value =
		Parse_Query_Extended_Info_Parameters(Current_Node,
						     Optional);

	} else if (Current_Token->TokenText == ExtendedStr) {

	    Consume_Token();
	    Next_Token();

	    if ((Current_Token != NULL) &&
		(Current_Token->Characterization == KeyWord) &&
		(Current_Token->TokenText == InfoStr)) {
		/* We are going to query the extended info of something. */
		Current_Node->NodeValue = (void *) NULL;
		Current_Node->NodeType = Query_Extended_Info;
		Consume_Token();

		Return_Value =
		    Parse_Query_Extended_Info_Parameters(Current_Node,
							 Optional);

	    } else {
		if (Optional)
		    return Try_Again;

		Report_Parser_Error(Expecting_Query_Extended_Info_Keywords,
				    Command_Filename, Current_Token);

		return Error;

	    }

	} else {

	    if (Optional)
		return Try_Again;

	    Report_Parser_Error(Expecting_Query_Parameters,
				Command_Filename, Current_Token);

	    return Error;

	}

	break;
    case 'C':			/* Containers or Children. */
	if ((Current_Token->TokenText == ContainersStr) ||
	    (Current_Token->TokenLength == 1)) {
	    /* We are going to produce a query about containers.  Mark Current_Node. */
	    Current_Node->NodeValue = (void *) ContainersStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    /* The remaining parameters are optional.  Lets see if any were specified. */
//        Allowed_Filters |= Available_Filter;
	    Allowed_Filters |= Container_Filter;
	    Allowed_Filters |= Plugin_Filter;
	    Allowed_Filters |= LT_Filter;
	    Allowed_Filters |= GT_Filter;
	    Allowed_Filters |= EQ_Filter;
//        Allowed_Filters |= Volume_Filter;
//        Allowed_Filters |= Expandable_Filter;
//        Allowed_Filters |= Shrinkable_Filter;
//        Allowed_Filters |= Unclaimed_Filter;
	    Allowed_Filters |= Region_Filter;
	    Allowed_Filters |= LO_Filter;
//        Allowed_Filters |= Object_Filter;
//        Allowed_Filters |= Plugin_Type_Filter;
//        Allowed_Filters |= Convertible_Filter;
//        Allowed_Filters |= Disk_Filter;
	    Allowed_Filters |= Segment_Filter;
	    Allowed_Filters |= Freespace_Filter;
	    Return_Value =
		Parse_Optional_Query_Filters(Current_Node, Allowed_Filters,
					     TRUE);

	    if (Return_Value == Error)
		break;

	    Return_Value = Token_Accepted;	/* The command is complete. */

	} else {

	    if (Current_Token->TokenText == ChildrenStr) {
		/* We are going to produce a query about the Children of an object.  Mark Current_Node. */
		Current_Node->NodeValue = (void *) ChildrenStr;
		Current_Node->NodeType = Query_Node_Type_2;

		/* We are through with this token, so consume it. */
		Consume_Token();

		/* Check for a comma. */
		Return_Value = Check_For_Comma(Optional);

		if (Return_Value == Error)
		    break;

		if (Return_Value == Try_Again) {

		    if (Optional)
			return Try_Again;

		    Report_Parser_Error(Expecting_Volume_Or_Object,
					Command_Filename, Current_Token);
		    Return_Value = Error;

		} else
		    Return_Value =
			Specify_Object(Current_Node,
				       Translate_Name_To_Handle, Optional);

	    } else {

		if (Optional)
		    return Try_Again;

		Report_Parser_Error
		    (Expecting_Containers_Or_Children_Parameter,
		     Command_Filename, Current_Token);
		Return_Value = Error;

	    }

	}

	break;
    case 'O':			/* Objects */
	if ((Current_Token->TokenText == ObjectsStr) ||
	    (Current_Token->TokenLength == 1)) {
	    /* We are going to produce a query about the EVMS objects in the system.  Mark Current_Node. */
	    Current_Node->NodeValue = (void *) ObjectsStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    /* The remaining parameters are optional.  Lets see if any were specified. */
	    Allowed_Filters |= Available_Filter;
	    Allowed_Filters |= Container_Filter;
	    Allowed_Filters |= Plugin_Filter;
	    Allowed_Filters |= LT_Filter;
	    Allowed_Filters |= GT_Filter;
	    Allowed_Filters |= EQ_Filter;
	    Allowed_Filters |= Volume_Filter;
	    Allowed_Filters |= Expandable_Filter;
	    Allowed_Filters |= Shrinkable_Filter;
	    Allowed_Filters |= Unclaimed_Filter;
	    Allowed_Filters |= Region_Filter;
	    Allowed_Filters |= LO_Filter;
	    Allowed_Filters |= Object_Filter;
//        Allowed_Filters |= Plugin_Type_Filter;
//        Allowed_Filters |= Convertible_Filter;
//        Allowed_Filters |= Disk_Filter;
//        Allowed_Filters |= Freespace_Filter;
	    Return_Value =
		Parse_Optional_Query_Filters(Current_Node, Allowed_Filters,
					     TRUE);

	    if (Return_Value == Error)
		break;

	    Return_Value = Token_Accepted;	/* The command is complete. */

	} else {

	    if (Optional)
		return Try_Again;

	    Report_Parser_Error(Expecting_Objects_Parameter,
				Command_Filename, Current_Token);
	    Return_Value = Error;

	}

	break;

    default:
	if (Optional)
	    Return_Value = Try_Again;
	else {
	    Return_Value = Error;
	    Report_Parser_Error(Expecting_Query_Parameters,
				Command_Filename, Current_Token);
	}
	break;

    }

    return Return_Value;

}


static Parse_Results
Commit_Type(Executable_Node * Current_Node, boolean Optional)
{
    Parse_Results Return_Value = Error;	/* Used to track the return value from other functions called by this function. */

    /* Get the token to parse. */
    Next_Token();

    /* Check for EOF or InvalidCharacter */
    if ((Current_Token == NULL)
	|| (Current_Token->Characterization == EofToken)
	|| (Current_Token->Characterization == InvalidCharacter)
	|| (Current_Token->Characterization != KeyWord)) {

	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Commit_Parameters, Command_Filename,
			    Current_Token);

	return Error;

    }

    /* Lets see if we are to hold commits or release commits. */
    switch (Current_Token->TokenText[0]) {
    case 'H':			/* Hold */
	if (Current_Token->TokenText == HoldStr) {
	    Current_Node->NodeValue = HoldStr;
	    Consume_Token();
	    Return_Value = Token_Accepted;
	    break;
	}
	Return_Value = Try_Again;
	break;
    case 'R':			/* Release */
	if (Current_Token->TokenText == ReleaseStr) {
	    Current_Node->NodeValue = ReleaseStr;
	    Consume_Token();
	    Return_Value = Token_Accepted;
	    break;
	}
	Return_Value = Try_Again;
	break;
    default:
	Return_Value = Try_Again;
	break;
    }

    if (Return_Value == Try_Again && (!Optional)) {
	Report_Parser_Error(Expecting_Help_Parameters, Command_Filename,
			    Current_Token);
	Return_Value = Error;
    }

    return Return_Value;
}


static Parse_Results
Help_Type(Executable_Node * Current_Node, boolean Optional)
{
    Executable_Node *Temp_Node = NULL;	/* Used for lookahead operations. */
    Parse_Results Return_Value = Error;	/* Used to track the return value from other functions called by this function. */
    boolean Help_Topic_Found = FALSE;
    boolean ASM_Found = FALSE;	/* Used when parsing Add Segment Manager. */
    boolean AF_Found = FALSE;	/* Used when parsing Add Feature. */

    /* Get the token to parse. */
    Next_Token();

    /* Check for EOF or InvalidCharacter */
    if ((Current_Token == NULL)
	|| (Current_Token->Characterization == EofToken)
	|| (Current_Token->Characterization == InvalidCharacter)) {

	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Help_Parameters, Command_Filename,
			    Current_Token);

	return Error;

    }

    /* The parameter we are looking for is characterized as a KeyWord.  */
    if (Current_Token->Characterization != KeyWord) {

	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Help_Parameters, Command_Filename,
			    Current_Token);

	return Error;

    }

    /* Lets see which command help is being requested for. */
    switch (Current_Token->TokenText[0]) {
    case 'A':
	ASM_Found = FALSE;
	AF_Found = FALSE;

	/* Do we have Add Feature or Add Segment Manager? */
	if (Current_Token->TokenText == AddStr) {

	    /* Check for the Feature or Segment keywords. */
	    Consume_Token();
	    Next_Token();

	    if ((Current_Token != NULL) &&
		(Current_Token->Characterization == KeyWord) &&
		(Current_Token->TokenText == FeatureStr))
		AF_Found = TRUE;
	    else if ((Current_Token != NULL) &&
		     (Current_Token->Characterization == KeyWord) &&
		     (Current_Token->TokenText == SegmentStr)) {

		/* Check for the Manager keyword. */
		Consume_Token();
		Next_Token();

		if ((Current_Token != NULL) &&
		    (Current_Token->Characterization == KeyWord) &&
		    (Current_Token->TokenText == ManagerStr))
		    ASM_Found = TRUE;
		else {

		    if (Optional)
			return Try_Again;

		    Report_Parser_Error(Expecting_Manager_Keyword,
					Command_Filename, Current_Token);

		    return Error;

		}

	    } else {

		if (Optional)
		    return Try_Again;

		Report_Parser_Error(Expecting_Feature_Or_Segment_Keyword,
				    Command_Filename, Current_Token);

		return Error;

	    }

	}

	/* Do we have an Add Feature command? */
	if (AF_Found || (Current_Token->TokenText == AFStr)) {

	    Help_Topic_Found = TRUE;

	    /* We need help for the Add Feature command. */
	    Current_Node->NodeValue = (void *) AFStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    Return_Value = Token_Accepted;

	} else if ((Current_Token->TokenText == ActivateStr)
		   || (Current_Token->TokenText == ActStr)) {

	    Help_Topic_Found = TRUE;

	    /* We need help for the Activate command. */
	    Current_Node->NodeValue = (void *) ActivateStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    Return_Value = Token_Accepted;

	} else if ((Current_Token->TokenText == AllocateStr)
		   || (Current_Token->TokenLength == 1)) {

	    Help_Topic_Found = TRUE;

	    /* We need help for the allocate command. */
	    Current_Node->NodeValue = (void *) AllocateStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    Return_Value = Token_Accepted;

	} else if (Current_Token->TokenText == AssignStr) {

	    Help_Topic_Found = TRUE;

	    /* We need help for the assign command. */
	    Current_Node->NodeValue = (void *) AssignStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    Return_Value = Token_Accepted;

	} else if ((Current_Token->TokenText == AsmStr) || ASM_Found) {

	    Help_Topic_Found = TRUE;

	    /* We need help for the Add Segment Manager command. */
	    Current_Node->NodeValue = (void *) AsmStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    Return_Value = Token_Accepted;

	}

	break;
    case 'C':			/* Check, Commit, Convert, Create */
	if ((Current_Token->TokenText == CreateStr) ||
	    (Current_Token->TokenLength == 1)) {

	    Help_Topic_Found = TRUE;

	    /* We need help for the create command. */
	    Current_Node->NodeValue = (void *) CreateStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    Return_Value = Token_Accepted;

	} else if (Current_Token->TokenText == CheckStr) {

	    Help_Topic_Found = TRUE;

	    /* We need help for the check command. */
	    Current_Node->NodeValue = (void *) CheckStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    Return_Value = Token_Accepted;

	} else if (Current_Token->TokenText == ConvertStr) {

	    Help_Topic_Found = TRUE;

	    /* We need help for the convert command. */
	    Current_Node->NodeValue = (void *) ConvertStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    Return_Value = Token_Accepted;

	} else if (Current_Token->TokenText == CommitStr) {

	    Help_Topic_Found = TRUE;

	    /* We need help for the commit command. */
	    Current_Node->NodeValue = (void *) CommitStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    Return_Value = Token_Accepted;

	}
	break;

    case 'D':			/* Deactivate, Delete */
	if ((Current_Token->TokenText == DeactivateStr) ||
	    (Current_Token->TokenText == DeactStr)) {

	    Help_Topic_Found = TRUE;

	    /* We need help for the Deactivate command. */
	    Current_Node->NodeValue = (void *) DeactivateStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    Return_Value = Token_Accepted;

	} else if ((Current_Token->TokenText == DeleteStr) ||
		   (Current_Token->TokenText == DrStr) ||
		   (Current_Token->TokenLength == 1)) {

	    /* Are we doing a delete or delete recursive? */
	    if (Current_Token->TokenText == DeleteStr) {
		/* We must check to see if the "Recursive" keyword was specified. */

		/* Since this is optional, activate Lookahead mode. */
		Temp_Node = Activate_Lookahead();
		if (Temp_Node == NULL)
		    return Error;

		Consume_Token();
		Next_Token();

		/* Check for the Recursive keyword. */
		if ((Current_Token != NULL) &&
		    (Current_Token->Characterization == KeyWord) &&
		    (Current_Token->TokenText != RecursiveStr)) {
		    /* Restore the current token so that parsing can continue. */
		    if (!Restore_Current_Token(Temp_Node))
			return Error;
		}

		/* Leave lookahead mode. */
		if (!Terminate_Lookahead(Current_Node, Temp_Node, FALSE))
		    return Error;

	    } else
		Consume_Token();

	    Help_Topic_Found = TRUE;

	    /* We need help for the delete command. */
	    Current_Node->NodeValue = (void *) DeleteStr;

	    Return_Value = Token_Accepted;

	}

	break;
    case 'E':			/* Echo, Exit, Expand */
	if ((Current_Token->TokenText == ExpandStr) ||
	    (Current_Token->TokenLength == 1)) {

	    Help_Topic_Found = TRUE;

	    /* We need help for the expand command. */
	    Current_Node->NodeValue = (void *) ExpandStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    Return_Value = Token_Accepted;

	} else if (Current_Token->TokenText == EchoStr) {

	    Help_Topic_Found = TRUE;

	    /* We need help for the Echo command. */
	    Current_Node->NodeValue = (void *) EchoStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    Return_Value = Token_Accepted;

	} else if (Current_Token->TokenText == ExitStr) {

	    Help_Topic_Found = TRUE;

	    /* We need help for the exit command. */
	    Current_Node->NodeValue = (void *) ExitStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    Return_Value = Token_Accepted;

	}


	break;
    case 'F':			/* Format */
	if ((Current_Token->TokenText == FormatStr) ||
	    (Current_Token->TokenLength == 1)) {

	    Help_Topic_Found = TRUE;

	    /* We need help for the format command. */
	    Current_Node->NodeValue = (void *) FormatStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    Return_Value = Token_Accepted;

	}

	break;
    case 'M':			/* Mkfs, Modify, Mount */
	if ((Current_Token->TokenText == MkfsStr) ||
	    (Current_Token->TokenLength == 1)) {

	    Help_Topic_Found = TRUE;

	    /* We need help for the Mkfs command. */
	    Current_Node->NodeValue = (void *) MkfsStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    Return_Value = Token_Accepted;

	} else if (Current_Token->TokenText == ModifyStr) {

	    Help_Topic_Found = TRUE;

	    /* We need help for the modify command. */
	    Current_Node->NodeValue = (void *) ModifyStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    Return_Value = Token_Accepted;

	} else if (Current_Token->TokenText == MountStr) {

	    Help_Topic_Found = TRUE;

	    /* We need help for the mount command. */
	    Current_Node->NodeValue = (void *) MountStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    Return_Value = Token_Accepted;

	}

	break;
    case 'S':			/* Save, Set, Shrink */
	if ((Current_Token->TokenText == ShrinkStr) ||
	    (Current_Token->TokenLength == 1)) {

	    Help_Topic_Found = TRUE;

	    /* We need help for the create command. */
	    Current_Node->NodeValue = (void *) ShrinkStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    Return_Value = Token_Accepted;

	} else if (Current_Token->TokenText == SetStr) {

	    Help_Topic_Found = TRUE;

	    /* We need help for the set command. */
	    Current_Node->NodeValue = (void *) SetStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    Return_Value = Token_Accepted;

	} else if (Current_Token->TokenText == SaveStr) {

	    Help_Topic_Found = TRUE;

	    /* We need help for the save command. */
	    Current_Node->NodeValue = (void *) SaveStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    Return_Value = Token_Accepted;

	}

	break;
    case 'Q':			/* Query, Quit */
	if ((Current_Token->TokenText == QueryStr) ||
	    (Current_Token->TokenLength == 1)) {

	    Help_Topic_Found = TRUE;

	    /* We need help for the create command. */
	    Current_Node->NodeValue = (void *) QueryStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    Return_Value = Token_Accepted;

	} else if (Current_Token->TokenText == QuitStr) {

	    Help_Topic_Found = TRUE;

	    /* We need help for the quit command. */
	    Current_Node->NodeValue = (void *) QuitStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    Return_Value = Token_Accepted;

	}

	break;
    case 'P':
	if ((Current_Token->TokenText == ProbeStr) ||
	    (Current_Token->TokenLength == 1)) {

	    Help_Topic_Found = TRUE;

	    /* We need help for the create command. */
	    Current_Node->NodeValue = (void *) ProbeStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    Return_Value = Token_Accepted;

	}

	break;
    case 'R':			/* Remove, Rename or Revert */
	if (Current_Token->TokenText == RenameStr) {

	    Help_Topic_Found = TRUE;

	    /* We need help for the rename command. */
	    Current_Node->NodeValue = (void *) RenameStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    Return_Value = Token_Accepted;

	} else {

	    if (Current_Token->TokenText == RevertStr) {

		Help_Topic_Found = TRUE;

		/* We need help for the revert command. */
		Current_Node->NodeValue = (void *) RevertStr;

		/* We are through with this token, so consume it. */
		Consume_Token();

		Return_Value = Token_Accepted;

	    } else {

		if (Current_Token->TokenText == RemoveStr) {

		    Help_Topic_Found = TRUE;

		    /* We need help for the remove command. */
		    Current_Node->NodeValue = (void *) RemoveStr;

		    /* We are through with this token, so consume it. */
		    Consume_Token();

		    Return_Value = Token_Accepted;

		} else {
		    if (Current_Token->TokenText == ReplaceStr) {

			Help_Topic_Found = TRUE;

			/* We need help for the replace command. */
			Current_Node->NodeValue = (void *) ReplaceStr;

			/* We are through with this token, so consume it. */
			Consume_Token();

			Return_Value = Token_Accepted;

		    }

		}

	    }

	}
	break;
    case 'T':			/* Task */
	if ((Current_Token->TokenText == TaskStr) ||
	    (Current_Token->TokenLength == 1)) {

	    Help_Topic_Found = TRUE;

	    /* We need help for the Task command. */
	    Current_Node->NodeValue = (void *) TaskStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    Return_Value = Token_Accepted;

	}

	break;
    case 'U':			/* Unformat, Unmkfs, Unmount */
	if ((Current_Token->TokenText == UnformatStr) ||
	    (Current_Token->TokenLength == 1)) {

	    Help_Topic_Found = TRUE;

	    /* We need help for the unformat command. */
	    Current_Node->NodeValue = (void *) UnformatStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    Return_Value = Token_Accepted;

	} else if (Current_Token->TokenText == UnMkfsStr) {

	    Help_Topic_Found = TRUE;

	    /* We need help for the Unmkfs command. */
	    Current_Node->NodeValue = (void *) UnMkfsStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    Return_Value = Token_Accepted;

	} else if (Current_Token->TokenText == UnmountStr) {

	    Help_Topic_Found = TRUE;

	    /* We need help for the Unmount command. */
	    Current_Node->NodeValue = (void *) UnmountStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    Return_Value = Token_Accepted;

	}

	break;
    default:
	break;

    }

    if (!Help_Topic_Found) {

	/* Use standard error handling */
	if (Optional)
	    Return_Value = Try_Again;
	else {

	    Report_Parser_Error(Expecting_Help_Parameters,
				Command_Filename, Current_Token);
	    Return_Value = Error;

	}

    }

    return Return_Value;

}

static Parse_Results
Plugin_Types(Executable_Node * Current_Node, boolean Optional)
{
    Executable_Node *Child = NULL;	/* The node used to represent the current command being parsed. */
    Parse_Results Return_Value = Error;	/* Used to track the return value from other functions called by this function. */

    /* Create the child node and add it to the children list of Current_Node.  We will fill in the contents of Child later. */
    Child = Make_Child_Node(Current_Node, TRUE);
    if (Child == NULL)
	return Error;

    Child->NodeType = Plugin_Mask;

    /* Get the token to parse. */
    Next_Token();

    /* Check for EOF, InvalidCharacter, or bad token characterization. */
    if ((Current_Token == NULL) ||
	(Current_Token->Characterization == EofToken) ||
	(Current_Token->Characterization == InvalidCharacter) ||
	(Current_Token->Characterization != KeyWord)) {

	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Plugin_Type_Parameter,
			    Command_Filename, Current_Token);

	return Error;

    }

    /* Lets see which what kind of plugin we are after. */
    switch (Current_Token->TokenText[0]) {
    case 'C':			/* Cluster Management, CM */
	if (Current_Token->TokenText == CMStr) {
	    /* We are after a Cluster Management plugin. */
	    Child->NodeValue = (void *) CMStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    Return_Value = Token_Accepted;

	    break;

	}

	if (Current_Token->TokenText == ClusterStr) {

	    /* We have the start of the Cluster Manager plugin type. */
	    Consume_Token();

	    /* Get the next token, which we hope will be the Management keyword. */
	    Next_Token();

	    /* Check for EOF or InvalidCharacter */
	    if ((Current_Token == NULL) ||
		(Current_Token->Characterization == EofToken) ||
		(Current_Token->Characterization == InvalidCharacter)) {

		if (Optional)
		    return Try_Again;

		Report_Parser_Error(Expecting_Plugin_Type_Parameter,
				    Command_Filename, Current_Token);

		return Error;

	    }

	    if ((Current_Token->Characterization == KeyWord) &&
		(Current_Token->TokenText == ManagementStr)) {

		/* We are after a Cluster Management plugin. */
		Child->NodeValue = (void *) CMStr;

		/* We are through with this token, so consume it. */
		Consume_Token();

		Return_Value = Token_Accepted;

		break;

	    }

	}

	/* No matching keyword. */

	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Plugin_Type_Parameter,
			    Command_Filename, Current_Token);
	Return_Value = Error;

	break;

    case 'D':			/* Device Manager, DLM, Distributed Lock Management */
	if (Current_Token->TokenLength == 1) {
	    /* We are after a Device Manager plugin. */
	    Child->NodeValue = (void *) DeviceStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    Return_Value = Token_Accepted;

	    break;


	}

	if (Current_Token->TokenText == DLMStr) {
	    /* We are after a Distributed Lock Management plugin. */
	    Child->NodeValue = (void *) DLMStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    Return_Value = Token_Accepted;

	    break;

	}

	if (Current_Token->TokenText == DeviceStr) {

	    /* We have the start of the Device Manager plugin type. */
	    Consume_Token();

	    /* Get the next token, which we hope will be the Manager keyword. */
	    Next_Token();

	    if ((Current_Token->Characterization == KeyWord) &&
		(Current_Token->TokenText == ManagerStr)) {

		/* We are after a Device Manager plugin. */
		Child->NodeValue = (void *) DeviceStr;

		/* We are through with this token, so consume it. */
		Consume_Token();

		Return_Value = Token_Accepted;

		break;

	    }

	} else {

	    if (Current_Token->TokenText == DistributedStr) {

		/* We have the start of the Distributed Lock Management plugin type. */
		Consume_Token();

		/* Get the next token, which we hope will be the Lock keyword. */
		Next_Token();

		/* Check for EOF or InvalidCharacter */
		if ((Current_Token == NULL)
		    || (Current_Token->Characterization == EofToken)
		    || (Current_Token->Characterization ==
			InvalidCharacter)) {

		    if (Optional)
			return Try_Again;

		    Report_Parser_Error(Expecting_Plugin_Type_Parameter,
					Command_Filename, Current_Token);

		    return Error;

		}

		/* The parameter we are looking for is characterized as a KeyWord.  */
		if (Current_Token->Characterization != KeyWord) {

		    if (Optional)
			return Try_Again;

		    Report_Parser_Error(Expecting_Plugin_Type_Parameter,
					Command_Filename, Current_Token);

		    return Error;

		}

		if (Current_Token->TokenText == LockStr) {

		    /* We have two parts of the Distributed Lock Management plugin type. */
		    Consume_Token();

		    /* Get the next token, which we hope will be the Management keyword. */
		    Next_Token();

		    /* Check for EOF or InvalidCharacter */
		    if ((Current_Token == NULL)
			|| (Current_Token->Characterization == EofToken)
			|| (Current_Token->Characterization ==
			    InvalidCharacter)) {

			if (Optional)
			    return Try_Again;

			Report_Parser_Error
			    (Expecting_Plugin_Type_Parameter,
			     Command_Filename, Current_Token);

			return Error;

		    }

		    /* The parameter we are looking for is characterized as a KeyWord.  */
		    if (Current_Token->Characterization != KeyWord) {

			if (Optional)
			    return Try_Again;

			Report_Parser_Error
			    (Expecting_Plugin_Type_Parameter,
			     Command_Filename, Current_Token);

			return Error;

		    }

		    if (Current_Token->TokenText == ManagerStr) {

			/* We are after a DLM plugin. */
			Child->NodeValue = (void *) DLMStr;

			/* We are through with this token, so consume it. */
			Consume_Token();

			Return_Value = Token_Accepted;

			break;

		    }

		}

	    }

	}

	/* No matching keyword. */

	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Plugin_Type_Parameter,
			    Command_Filename, Current_Token);
	Return_Value = Error;

	break;

    case 'R':			/* Region Manager */
	if (Current_Token->TokenLength == 1) {

	    /* We are after a Segment Manager plugin. */
	    Child->NodeValue = (void *) RegionStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    Return_Value = Token_Accepted;

	    break;

	}

	if (Current_Token->TokenText == RegionStr) {

	    /* We have the start of the Region Manager plugin type. */
	    Consume_Token();

	    /* Get the next token, which we hope will be the Manager keyword. */
	    Next_Token();

	    if ((Current_Token->Characterization == KeyWord) &&
		(Current_Token->TokenText == ManagerStr)) {

		/* We are after a Region Manager plugin. */
		Child->NodeValue = (void *) RegionStr;

		/* We are through with this token, so consume it. */
		Consume_Token();

		Return_Value = Token_Accepted;

		break;

	    }

	}
	/* No matching keyword. */

	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Plugin_Type_Parameter,
			    Command_Filename, Current_Token);
	Return_Value = Error;
	break;
    case 'S':			/* Segment Manager */
	if (Current_Token->TokenLength == 1) {

	    /* We are after a Segment Manager plugin. */
	    Child->NodeValue = (void *) SegmentStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    Return_Value = Token_Accepted;

	    break;

	}

	if (Current_Token->TokenText == SegmentStr) {

	    /* We have the start of the Segment Manager plugin type. */
	    Consume_Token();

	    /* Get the next token, which we hope will be the Manager keyword. */
	    Next_Token();

	    if ((Current_Token->Characterization == KeyWord) &&
		(Current_Token->TokenText == ManagerStr)) {

		/* We are after a Segment Manager plugin. */
		Child->NodeValue = (void *) SegmentStr;

		/* We are through with this token, so consume it. */
		Consume_Token();

		Return_Value = Token_Accepted;

		break;

	    }

	}
	/* No matching keyword. */

	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Plugin_Type_Parameter,
			    Command_Filename, Current_Token);
	Return_Value = Error;
	break;
    case 'F':			/* Feature, File System Interface Module, FSIM */
	if ((Current_Token->TokenLength == 1)
	    || (Current_Token->TokenText == FeatureStr)) {
	    /* We are after a Feature plugin. */
	    Child->NodeValue = (void *) FeatureStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    Return_Value = Token_Accepted;

	    break;


	}

	if (Current_Token->TokenText == FSIMStr) {
	    /* We are after a Filesystem Interface Module plugin. */
	    Child->NodeValue = (void *) FSIMStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    Return_Value = Token_Accepted;

	    break;

	}

	if (Current_Token->TokenText == FilesystemStr) {

	    /* We have the start of the Filesystem Interface Module plugin type. */
	    Consume_Token();

	    /* Get the next token, which we hope will be the Interface keyword. */
	    Next_Token();

	    /* Check for error conditions. */
	    if ((Current_Token == NULL) ||
		(Current_Token->Characterization == EofToken) ||
		(Current_Token->Characterization == InvalidCharacter) ||
		(Current_Token->Characterization != KeyWord)) {

		if (Optional)
		    return Try_Again;

		Report_Parser_Error(Expecting_Plugin_Type_Parameter,
				    Command_Filename, Current_Token);

		return Error;

	    }

	    if (Current_Token->TokenText == InterfaceStr) {

		/* We have two parts of the Filesystem Interface Module plugin type. */
		Consume_Token();

		/* Get the next token, which we hope will be the Management keyword. */
		Next_Token();

		/* Check for error conditions. */
		if ((Current_Token == NULL) ||
		    (Current_Token->Characterization == EofToken) ||
		    (Current_Token->Characterization == InvalidCharacter)
		    || (Current_Token->Characterization != KeyWord)) {

		    if (Optional)
			return Try_Again;

		    Report_Parser_Error(Expecting_Plugin_Type_Parameter,
					Command_Filename, Current_Token);

		    return Error;

		}

		if (Current_Token->TokenText == ModuleStr) {

		    /* We are after a FSIM plugin. */
		    Child->NodeValue = (void *) FSIMStr;

		    /* We are through with this token, so consume it. */
		    Consume_Token();

		    Return_Value = Token_Accepted;

		    break;

		}

	    }

	}

	/* No matching keyword. */

	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Plugin_Type_Parameter,
			    Command_Filename, Current_Token);
	Return_Value = Error;
	break;
    default:
	if (Optional)
	    Return_Value = Try_Again;
	else {

	    Report_Parser_Error(Expecting_Plugin_Type_Parameter,
				Command_Filename, Current_Token);
	    Return_Value = Error;

	}

	break;

    }

    return Return_Value;

}

static Parse_Results
Feature_Identifier(Executable_Node * Current_Node, boolean Optional,
		   boolean Parameters_Follow, boolean Objects_Are_Optional)
{
    Executable_Node *Child = NULL;	/* The node used to represent the current command being parsed. */
    Executable_Node *Temp_Node = NULL;	/* Used for lookahead operations. */
    Parse_Results Return_Value = Error;	/* Used to track the return value from other functions called by this function. */

    /* Create the child node and add it to the children list of Current_Node.  We will fill in the contents of Child later. */
    Child = Make_Child_Node(Current_Node, TRUE);
    if (Child == NULL)
	return Error;

    /* Get the token to parse. */
    Next_Token();

    /* Check for EOF or InvalidCharacter */
    if ((Current_Token == NULL)
	|| (Current_Token->Characterization == EofToken)
	|| (Current_Token->Characterization == InvalidCharacter)) {

	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Feature_Identifier, Command_Filename,
			    Current_Token);

	return Error;

    }

    switch (Current_Token->Characterization) {

    case Number:		/* We should have a Feature ID which we will need to turn into a handle later. */

	if (Parameters_Follow)
	    Child->NodeType = Feature_ID;
	else
	    Child->NodeType = Plugin_ID;
	Assign_Text(Child, Current_Token);

	/* We are through with this token, so consume it. */
	Consume_Token();

	Return_Value = Token_Accepted;

	break;
    case KeyWord:
    case String:
	if (Parameters_Follow)
	    Child->NodeType = Feature_Name;
	else
	    Child->NodeType = Name_To_Plugin_Handle;

	Assign_Text(Child, Current_Token);

	/* We are through with this token, so consume it. */
	Consume_Token();

	Return_Value = Token_Accepted;

	break;
    default:
	if (Optional)
	    Return_Value = Try_Again;
	else {

	    Report_Parser_Error(Expecting_Feature_Identifier,
				Command_Filename, Current_Token);
	    Return_Value = Error;

	}

	break;
    }

    if (Parameters_Follow && (Return_Value == Token_Accepted)) {

	/* If we accepted the token, then we must look for the parameters that go with the specified feature. */

	/* An equal sign must follow the feature identifier. */
	Return_Value = Check_For_EqualSign(Optional);
	if (Return_Value == Token_Accepted) {
	    Return_Value = Check_For_Open_Brace(Optional);
	    if (Return_Value == Token_Accepted) {
		/* All of the name value pairs between the braces are optional! */

		Temp_Node = Activate_Lookahead();
		if (Temp_Node == NULL)
		    return Error;

		do {
		    Return_Value = Parse_Name_Value_Pair(Temp_Node, TRUE);

		    if (Return_Value == Try_Again)
			if (!Restore_Current_Token(Temp_Node))
			    return Error;

		    if (!Terminate_Lookahead
			(Child, Temp_Node,
			 (Return_Value == Token_Accepted)))
			return Error;

		    if (Return_Value == Token_Accepted) {
			/* We have just parsed a name-value pair.  Look for another one. */

			Temp_Node = Activate_Lookahead();
			if (Temp_Node == NULL)
			    return Error;

			Return_Value = Check_For_Comma(TRUE);
			if (Return_Value == Try_Again) {
			    if (!Restore_Current_Token(Temp_Node))
				return Error;

			    if (!Terminate_Lookahead
				(Child, Temp_Node, FALSE))
				return Error;
			}

		    }

		}
		while (Return_Value == Token_Accepted);


		if (Return_Value != Error) {
		    Return_Value = Check_For_Closing_Brace(Optional);
		    if (Return_Value == Token_Accepted) {
			/* Activate lookahead since the objects could be optional. */
			Temp_Node = Activate_Lookahead();
			if (Temp_Node == NULL)
			    return Error;

			/* Now we must check for the objects and regions to be used. */
			Return_Value =
			    Parse_Object_List(Temp_Node,
					      (Optional
					       || Objects_Are_Optional));

			if ((Return_Value == Try_Again)
			    && (!Restore_Current_Token(Temp_Node)))
			    return Error;

			if (!Terminate_Lookahead
			    (Child, Temp_Node,
			     (Return_Value == Token_Accepted)))
			    return Error;

			if (Optional || Objects_Are_Optional)
			    Return_Value = Token_Accepted;

		    }

		}

	    }

	}

    }

    return Return_Value;

}

static Parse_Results
Plugin_Identifier(Executable_Node * Current_Node,
		  boolean Parameters_Follow, boolean Get_Object_List,
		  boolean Optional)
{
    Executable_Node *Child = NULL;	/* The node used to represent the current command being parsed. */
    Executable_Node *Temp_Node = NULL;	/* Used for lookahead operations. */
    Parse_Results Return_Value = Error;	/* Used to track the return value from other functions called by this function. */

    /* Create the child node and add it to the children list of Current_Node.  We will fill in the contents of Child later. */
    Child = Make_Child_Node(Current_Node, TRUE);
    if (Child == NULL)
	return Error;

    /* Get the token to parse. */
    Next_Token();

    /* Check for EOF or InvalidCharacter */
    if ((Current_Token == NULL)
	|| (Current_Token->Characterization == EofToken)
	|| (Current_Token->Characterization == InvalidCharacter)) {

	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Plugin_Identifier, Command_Filename,
			    Current_Token);

	return Error;

    }

    switch (Current_Token->Characterization) {

    case Number:		/* We should have a Plugin ID which we will need to turn into a handle later. */

	Child->NodeType = Plugin_ID;
	Assign_Text(Child, Current_Token);

	/* We are through with this token, so consume it. */
	Consume_Token();

	Return_Value = Token_Accepted;

	break;
    case KeyWord:
    case String:
	if (Parameters_Follow)
	    Child->NodeType = Plugin_Name;
	else
	    Child->NodeType = Name_To_Plugin_Handle;

	Assign_Text(Child, Current_Token);

	/* We are through with this token, so consume it. */
	Consume_Token();

	Return_Value = Token_Accepted;

	break;
    default:
	if (Optional)
	    Return_Value = Try_Again;
	else {

	    Report_Parser_Error(Expecting_Plugin_Identifier,
				Command_Filename, Current_Token);
	    Return_Value = Error;

	}

	break;
    }

    /* If we accepted the token, then we must look for the parameters that go with the specified plug-in. */
    if (Parameters_Follow && (Return_Value == Token_Accepted)) {

	/* An equal sign must follow the plug-in identifier. */
	Return_Value = Check_For_EqualSign(Optional);
	if (Return_Value == Token_Accepted) {
	    Return_Value = Check_For_Open_Brace(Optional);
	    if (Return_Value == Token_Accepted) {
		/* All of the name value pairs between the braces are optional! */

		Temp_Node = Activate_Lookahead();
		if (Temp_Node == NULL)
		    return Error;

		do {
		    Return_Value = Parse_Name_Value_Pair(Temp_Node, TRUE);

		    if (Return_Value == Try_Again)
			if (!Restore_Current_Token(Temp_Node))
			    return Error;

		    if (!Terminate_Lookahead
			(Child, Temp_Node,
			 (Return_Value == Token_Accepted)))
			return Error;

		    if (Return_Value == Token_Accepted) {
			/* We have just parsed a name-value pair.  Look for another one. */

			Temp_Node = Activate_Lookahead();
			if (Temp_Node == NULL)
			    return Error;

			Return_Value = Check_For_Comma(TRUE);
			if (Return_Value == Try_Again) {
			    if (!Restore_Current_Token(Temp_Node))
				return Error;

			    if (!Terminate_Lookahead
				(Child, Temp_Node, FALSE))
				return Error;
			}

		    }

		}
		while (Return_Value == Token_Accepted);


		if (Return_Value != Error) {
		    Return_Value = Check_For_Closing_Brace(Optional);

		    if ((Return_Value == Token_Accepted)
			&& Get_Object_List) {

			/* Now we must check for the segments/regions/disks to be used. */
			Return_Value = Parse_Object_List(Child, Optional);

		    }

		}

	    }

	}

    }

    return Return_Value;

}

static Parse_Results
Specify_Object(Executable_Node * Current_Node, NodeTypes Node_To_Create,
	       boolean Optional)
{
    Executable_Node *Child = NULL;	/* The node used to represent the current command being parsed. */
    Parse_Results Return_Value = Error;	/* Used to track the return value from other functions called by this function. */

    /* Get the token to parse. */
    Next_Token();

    /* Check for EOF or InvalidCharacter */
    if ((Current_Token == NULL)
	|| (Current_Token->Characterization == EofToken)
	|| (Current_Token->Characterization == InvalidCharacter)) {

	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Object_Identifier, Command_Filename,
			    Current_Token);

	return Error;

    }

    if (Current_Token->Characterization == String) {

	/* Create the child node and add it to the children list of Current_Node. */
	Child = Make_Child_Node(Current_Node, TRUE);
	if (Child == NULL)
	    return Error;

	Child->NodeType = Node_To_Create;
	Assign_Text(Child, Current_Token);

	/* We are through with this token, so consume it. */
	Consume_Token();

	Return_Value = Token_Accepted;

    } else {
	if (Optional)
	    Return_Value = Try_Again;
	else {

	    Report_Parser_Error(Expecting_Object_Identifier,
				Command_Filename, Current_Token);
	    Return_Value = Error;

	}

    }

    return Return_Value;

}

static Parse_Results Size(Executable_Node * Current_Node, boolean Optional)
{
    Executable_Node *Child = NULL;	/* The node used to represent the current command being parsed. */

    /* Create the child node and add it to the children list of Current_Node.  We will fill in the contents of Child later. */
    Child = Make_Child_Node(Current_Node, TRUE);
    if (Child == NULL)
	return Error;

    /* Get the token to parse. */
    Next_Token();

    /* Check for EOF or InvalidCharacter */
    if ((Current_Token == NULL)
	|| (Current_Token->Characterization == EofToken)
	|| (Current_Token->Characterization == InvalidCharacter)) {

	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Size_Parameters, Command_Filename,
			    Current_Token);

	return Error;

    }

    /* Check the characterization of the token. */
    if ((Current_Token->Characterization != Number) &&
	(Current_Token->Characterization != RealNumber)) {
	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Size_Value, Command_Filename,
			    Current_Token);

	return Error;

    }

    /* Fill in the appropriate fields in Child. */
    if (Current_Token->Characterization == RealNumber)
	Child->NodeType = RealNumberData;
    else
	Child->NodeType = IntegerData;

    Assign_Text(Child, Current_Token);

    /* We are through with this token, so consume it. */
    Consume_Token();

    /* Now we must find out the unit associated with the number. */
    return Parse_Units(Child, FALSE, Optional);

}


static Parse_Results
Precision(Executable_Node * Current_Node, boolean Optional)
{
    Executable_Node *Child = NULL;	/* The node used to represent the current command being parsed. */
    Executable_Node *Temp_Node = NULL;	/* Used for lookahead operations. */
    Parse_Results Return_Value = Error;	/* Used to track the return value from other functions called by this function. */

    /* Create the child node and add it to the children list of Current_Node.  We will fill in the contents of Child later. */
    Child = Make_Child_Node(Current_Node, TRUE);
    if (Child == NULL)
	return Error;

    /* Get the token to parse. */
    Next_Token();

    /* Check for EOF or InvalidCharacter */
    if ((Current_Token == NULL)
	|| (Current_Token->Characterization == EofToken)
	|| (Current_Token->Characterization == InvalidCharacter)) {

	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Size_Value, Command_Filename,
			    Current_Token);

	return Error;

    }

    /* Check the characterization of the token. */
    if (Current_Token->Characterization != Number) {
	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Size_Value, Command_Filename,
			    Current_Token);

	return Error;

    }

    /* Fill in the appropriate fields in Child. */
    Child->NodeType = Precision_Node;
    Assign_Text(Child, Current_Token);

    /* We are through with this token, so consume it. */
    Consume_Token();

    /* There may be an optional unit (KB, MB, GB, TB) associated with this value.  Check for it. */
    Temp_Node = Activate_Lookahead();
    if (Temp_Node == NULL)
	return Error;

    Return_Value = Parse_Units(Temp_Node, FALSE, TRUE);
    if (Return_Value == Try_Again) {

	/* Restore the current token so that parsing can continue. */
	if (!Restore_Current_Token(Temp_Node))
	    return Error;

    }

    /* Exit Lookahead mode. */
    if (!Terminate_Lookahead
	(Child, Temp_Node, (Return_Value == Token_Accepted)))
	return Error;

    /* Since the unit was optional, the command is complete without it. Set Return_Value accordingly. */
    if (Return_Value != Error)
	Return_Value = Token_Accepted;

    return Return_Value;
}


static Parse_Results
Parse_Units(Executable_Node * Current_Node, boolean Allow_Time_Units,
	    boolean Optional)
{
    Executable_Node *Child = NULL;	/* The node used to represent the current command being parsed. */
    Parser_Errors Error_Code;

    /* Set the error message to use. */
    if (Allow_Time_Units)
	Error_Code = Expecting_Units_Parameter;
    else
	Error_Code = Expecting_Disk_Units_Parameter;

    /* Create the child node and add it to the children list of Current_Node.  We will fill in the contents of Child later. */
    Child = Make_Child_Node(Current_Node, TRUE);
    if (Child == NULL)
	return Error;

    /* Get the token to parse. */
    Next_Token();

    /* Check for EOF or InvalidCharacter */
    if ((Current_Token == NULL)
	|| (Current_Token->Characterization == EofToken)
	|| (Current_Token->Characterization == InvalidCharacter)) {

	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Error_Code, Command_Filename, Current_Token);

	return Error;

    }

    /* We are expecting a key word. */
    if (Current_Token->Characterization != KeyWord) {
	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Error_Code, Command_Filename, Current_Token);

	return Error;

    }

    switch (Current_Token->TokenText[0]) {

    case 'M':			/* MB, Minutes, Milliseconds, Microseconds */
	if (Current_Token->TokenText == MegabyteStr) {
	    /* Accept the token. */
	    Child->NodeType = Units_Node;
	    Child->NodeValue = (void *) MegabyteStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();
	    break;
	}

	if ((Current_Token->TokenText == MinutesStr) && Allow_Time_Units) {
	    /* Accept the token. */
	    Child->NodeType = Units_Node;
	    Child->NodeValue = (void *) MinutesStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();
	    break;
	}

	if ((Current_Token->TokenText == MillisecondsStr)
	    && Allow_Time_Units) {
	    /* Accept the token. */
	    Child->NodeType = Units_Node;
	    Child->NodeValue = (void *) MillisecondsStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();
	    break;
	}

	if ((Current_Token->TokenText == MicrosecondsStr)
	    && Allow_Time_Units) {
	    /* Accept the token. */
	    Child->NodeType = Units_Node;
	    Child->NodeValue = (void *) MicrosecondsStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();
	    break;
	}

	Report_Parser_Error(Error_Code, Command_Filename, Current_Token);

	return Error;

	break;			/* Keep the compiler happy. */
    case 'G':			/* GB */
	if (Current_Token->TokenText != GigabyteStr) {
	    Report_Parser_Error(Error_Code, Command_Filename,
				Current_Token);

	    return Error;
	}

	/* Accept the token. */
	Child->NodeType = Units_Node;
	Child->NodeValue = (void *) GigabyteStr;

	/* We are through with this token, so consume it. */
	Consume_Token();

	break;
    case 'H':			/* Hours */
	if (((Current_Token->TokenText == HoursStr) ||
	     (Current_Token->TokenLength == 1)) && Allow_Time_Units) {
	    /* Accept the token. */
	    Child->NodeType = Units_Node;
	    Child->NodeValue = (void *) HoursStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();
	    break;
	}

	Report_Parser_Error(Error_Code, Command_Filename, Current_Token);

	return Error;

	break;			/* Keep the compiler happy. */
    case 'K':			/* KB */
	if (Current_Token->TokenText != KilobyteStr) {
	    Report_Parser_Error(Error_Code, Command_Filename,
				Current_Token);

	    return Error;
	}

	/* Accept the token. */
	Child->NodeType = Units_Node;
	Child->NodeValue = (void *) KilobyteStr;

	/* We are through with this token, so consume it. */
	Consume_Token();

	break;
    case 'S':			/* Seconds, Sectors */
	if (((Current_Token->TokenText == SecondsStr) ||
	     (Current_Token->TokenLength == 1)) && Allow_Time_Units) {
	    /* Accept the token. */
	    Child->NodeType = Units_Node;
	    Child->NodeValue = (void *) SecondsStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();
	    break;
	}

	if (Current_Token->TokenText == SectorsStr) {
	    /* Accept the token. */
	    Child->NodeType = Units_Node;
	    Child->NodeValue = (void *) SectorsStr;

	    /* We are through with this token, so consume it. */
	    Consume_Token();
	    break;
	}

	Report_Parser_Error(Error_Code, Command_Filename, Current_Token);

	return Error;

	break;			/* Keep the compiler happy. */
    case 'T':			/* TB */
	if (Current_Token->TokenText != TerrabyteStr) {
	    Report_Parser_Error(Error_Code, Command_Filename,
				Current_Token);

	    return Error;
	}

	/* Accept the token. */
	Child->NodeType = Units_Node;
	Child->NodeValue = (void *) TerrabyteStr;

	/* We are through with this token, so consume it. */
	Consume_Token();

	break;
    default:

	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Error_Code, Command_Filename, Current_Token);

	return Error;

	break;
    }

    return Token_Accepted;

}


static Parse_Results
Value(Executable_Node * Current_Node, boolean Optional)
{
    Executable_Node *Child = NULL;	/* The node used to represent the current command being parsed. */
    Executable_Node *Temp_Node = NULL;	/* Used for lookahead operations. */
    Parse_Results Return_Value = Error;	/* Used to track the return value from other functions called by this function. */

    /* Create the child node and add it to the children list of Current_Node.  We will fill in the contents of Child later. */
    Child = Make_Child_Node(Current_Node, TRUE);
    if (Child == NULL)
	return Error;

    /* Get the token to parse. */
    Next_Token();

    /* Check for EOF or InvalidCharacter */
    if ((Current_Token == NULL)
	|| (Current_Token->Characterization == EofToken)
	|| (Current_Token->Characterization == InvalidCharacter)) {

	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Number_Or_String, Command_Filename,
			    Current_Token);

	return Error;

    }

    /* We are expecting either a numeric or string type here. */
    switch (Current_Token->Characterization) {

    case Number:
	Child->NodeType = IntegerData;
	Assign_Text(Child, Current_Token);

	Consume_Token();

	/* There may be an optional unit (KB, MB, GB, TB) associated with this value.  Check for it. */
	Temp_Node = Activate_Lookahead();
	if (Temp_Node == NULL)
	    return Error;

	Return_Value = Parse_Units(Temp_Node, TRUE, TRUE);
	if (Return_Value == Try_Again) {

	    /* Restore the current token so that parsing can continue. */
	    if (!Restore_Current_Token(Temp_Node))
		return Error;

	}

	/* Exit Lookahead mode. */
	if (!Terminate_Lookahead
	    (Child, Temp_Node, (Return_Value == Token_Accepted)))
	    return Error;

	/* Since the unit was optional, the command is complete without it. Set Return_Value accordingly. */
	if (Return_Value != Error)
	    Return_Value = Token_Accepted;

	break;
    case RealNumber:
	Child->NodeType = RealNumberData;
	Assign_Text(Child, Current_Token);

	Consume_Token();

	/* There may be an optional unit (KB, MB, GB, TB) associated with this value.  Check for it. */
	Temp_Node = Activate_Lookahead();
	if (Temp_Node == NULL)
	    return Error;

	Return_Value = Parse_Units(Temp_Node, TRUE, TRUE);
	if (Return_Value == Try_Again) {

	    /* Restore the current token so that parsing can continue. */
	    if (!Restore_Current_Token(Temp_Node))
		return Error;

	}

	/* Exit Lookahead mode. */
	if (!Terminate_Lookahead
	    (Child, Temp_Node, (Return_Value == Token_Accepted)))
	    return Error;

	/* Since the unit was optional, the command is complete without it. Set Return_Value accordingly. */
	if (Return_Value != Error)
	    Return_Value = Token_Accepted;

	break;
    case String:
	Child->NodeType = StringData;
	Assign_Text(Child, Current_Token);
	Return_Value = Token_Accepted;
	break;
    case KeyWord:
	Child->NodeType = StringData;
	Child->NodeValue = strdup(Current_Token->TokenText);
	if (Child->NodeValue == NULL) {
	    Report_Parser_Error(Out_Of_Memory, Command_Filename,
				Current_Token);
	    return Error;
	}
	Return_Value = Token_Accepted;
	break;
    default:
	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Number_Or_String, Command_Filename,
			    Current_Token);

	return Error;

	break;
    }

    return Return_Value;

}


static Parse_Results
Name(Executable_Node * Current_Node, boolean Field_Name, boolean Optional)
{

    /* Wrapper for <String> to use for <Name> in grammar. */
    if (Get_String(Current_Node, FALSE, Optional) != Token_Accepted) {

	if (Optional)
	    return Try_Again;

	if (Field_Name)
	    Report_Parser_Error(Expecting_Field_Name, Command_Filename,
				Current_Token);
	else
	    Report_Parser_Error(Expecting_Name_Value, Command_Filename,
				Current_Token);

	return Error;

    }

    return Token_Accepted;

}


static Parse_Results
Get_String(Executable_Node * Current_Node, boolean AcceptNumberAsString,
	   boolean Optional)
{
    Executable_Node *Child = NULL;	/* The node used to represent the current command being parsed. */
    boolean Keep_Token;

    /* Get the token to parse. */
    Next_Token();

    /* Check to see if the token characterization is correct. */
    switch (Current_Token->Characterization) {
    case KeyWord:
    case String:
	Keep_Token = TRUE;
	break;
    case Number:
    case RealNumber:
	if (AcceptNumberAsString) {
	    Keep_Token = TRUE;
	    break;
	}
    default:
	Keep_Token = FALSE;
    }

    if (!Keep_Token) {

	if (Optional)
	    return Try_Again;

	return Error;

    }

    /* Create the child node and add it to the children list of Current_Node. */
    Child = Make_Child_Node(Current_Node, TRUE);
    if (Child == NULL)
	return Error;

    Child->NodeType = StringData;

    if (Current_Token->Characterization == KeyWord) {
	Child->NodeValue = strdup(Current_Token->TokenText);
	if (Child->NodeValue == NULL) {
	    Report_Parser_Error(Out_Of_Memory, Command_Filename,
				Current_Token);
	    return Error;
	}
    } else
	Assign_Text(Child, Current_Token);

    Consume_Token();

    return Token_Accepted;

}


static Parse_Results
Accept_Name(Executable_Node * Current_Node, boolean Name_Optional)
{
    Executable_Node *Child = NULL;	/* The node used to represent the current command being parsed. */
    Parse_Results Return_Value = Error;	/* Used to track the return value from other functions called by this function. */

    Next_Token();

    /* Check for EOF, InvalidCharacter, or incorrect characterization */
    if ((Current_Token == NULL) ||
	(Current_Token->Characterization == EofToken) ||
	(Current_Token->Characterization == InvalidCharacter) ||
	(Current_Token->Characterization != KeyWord)) {

	if (Name_Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Name, Command_Filename,
			    Current_Token);

	return Error;

    }

    /* Is the token the Name keyword? */
    if ((Current_Token->TokenText == NameStr) ||
	((Current_Token->TokenLength == 1) &&
	 (Current_Token->TokenText[0] == 'N'))) {
	/* We have the Name keyword. */

	/* Create the child node and add it to the children list of Current_Node. */
	Child = Make_Child_Node(Current_Node, TRUE);
	if (Child == NULL)
	    return Error;

	Child->NodeType = StringData;

	Consume_Token();

	Return_Value = Check_For_EqualSign(Name_Optional);
	if (Return_Value == Token_Accepted) {
	    Next_Token();

	    /* Check for EOF, InvalidCharacter, or incorrect characterization */
	    if ((Current_Token == NULL) ||
		(Current_Token->Characterization == EofToken) ||
		(Current_Token->Characterization == InvalidCharacter) ||
		(Current_Token->Characterization != String)) {

		if (Name_Optional)
		    Return_Value = Try_Again;
		else {
		    Report_Parser_Error(Expecting_Name, Command_Filename,
					Current_Token);

		    Return_Value = Error;
		}

	    } else {
		Assign_Text(Child, Current_Token);
		Return_Value = Token_Accepted;

		Consume_Token();
	    }

	}

    } else {
	if (Name_Optional)
	    Return_Value = Try_Again;
	else
	    Report_Parser_Error(Expecting_Object_Identifier,
				Command_Filename, Current_Token);

    }

    return Return_Value;

}


static Parse_Results
Parse_Name_Value_Pair(Executable_Node * Current_Node, boolean Optional)
{
    Executable_Node *Child = NULL;	/* The node used to represent the current command being parsed. */
    Executable_Node *Temp_Node = NULL;	/* Used for Lookahead operations. */
    Parse_Results Return_Value;	/* Used to track the return value from other functions called by this function. */
    boolean List_Mode = FALSE;	/* Set to TRUE if we have Name = (value,value,...)  to parse. */

    /* Create the child node and add it to the children list of Current_Node.  We will fill in the contents of Child later. */
    Child = Make_Child_Node(Current_Node, TRUE);
    if (Child == NULL)
	return Error;

    Child->NodeType = Name_Value_Pair;

    /* Get the name. */
    Return_Value = Name(Child, FALSE, Optional);
    if (Return_Value == Token_Accepted) {
	/* We should have an equal sign next, followed by a value. */
	Return_Value = Check_For_EqualSign(Optional);
	if (Return_Value == Token_Accepted) {

	    /* Do we have a list of values? */
	    Temp_Node = Activate_Lookahead();
	    if (Temp_Node == NULL)
		return Error;

	    /* Check for open parenthesis */
	    Return_Value = Check_For_Open_Paren(TRUE);

	    if (Return_Value == Try_Again) {

		/* Restore the current token so that parsing can continue. */
		if (!Restore_Current_Token(Temp_Node))
		    return Error;

	    } else if (Return_Value == Token_Accepted)
		List_Mode = TRUE;
	    else
		return Error;

	    /* Exit Lookahead mode. */
	    if (!Terminate_Lookahead
		(Child, Temp_Node, (Return_Value == Token_Accepted)))
		return Error;

	    do {

		/* Now lets get the value to go with the name. */
		Return_Value = Value(Child, Optional);

		if (List_Mode) {

		    /* Check for the end of the list. */
		    Temp_Node = Activate_Lookahead();
		    if (Temp_Node == NULL)
			return Error;

		    /* Check for closing parenthesis */
		    Return_Value = Check_For_Closing_Paren(TRUE);

		    if (Return_Value == Try_Again) {

			/* Restore the current token so that parsing can continue. */
			if (!Restore_Current_Token(Temp_Node))
			    return Error;

			/* Since this was not the end of the list, a comma must follow. */
			Return_Value = Check_For_Comma(FALSE);
		    } else if (Return_Value == Token_Accepted)
			List_Mode = FALSE;

		    /* Exit Lookahead mode. */
		    if (!Terminate_Lookahead
			(Child, Temp_Node,
			 (Return_Value == Token_Accepted)))
			return Error;

		}

	    }
	    while ((Return_Value == Token_Accepted) && List_Mode);

	}

    }

    return Return_Value;

}


static Parse_Results
Parse_Optional_Query_Filters(Executable_Node * Current_Node,
			     Available_Generic_Filters Allowed_Filters,
			     boolean Optional)
{
    Executable_Node *Temp_Node = NULL;	/* Used for lookahead operations. */
    Parse_Results Return_Value;	/* Used to track the return value from other functions called by this function. */
    Available_Generic_Filters Filters_Found = 0;

    /* Loop looking for each of the possible optional parameters.  Abort when a parameter other than one of the ones we
       are looking for is found.                                                                                        */
    for (;;) {

	/* Since these parameters are optional, activate look ahead. */
	Temp_Node = Activate_Lookahead();
	if (Temp_Node == NULL)
	    return Error;

	/* Check for a comma which would indicate possible additional parameters. */
	Return_Value = Check_For_Comma(TRUE);
	if (Return_Value == Token_Accepted) {
	    /* Check for parameters.  Since we found the comma, these parameters are not optional anymore! */
	    Return_Value =
		Parse_Filters(Temp_Node, Allowed_Filters, &Filters_Found,
			      FALSE);
	}

	if (Return_Value == Try_Again)
	    if (!Restore_Current_Token(Temp_Node))
		return Error;

	/* Save any parameters we may have found. */
	if (!Terminate_Lookahead
	    (Current_Node, Temp_Node, (Return_Value == Token_Accepted)))
	    return Error;

	if (Return_Value != Token_Accepted)
	    break;

    }

    /* Since all of the parameters we are looking for here are optional, it doesn't matter whether we found any or not. */
    return Return_Value;
}


static Parse_Results
Parse_Query_Extended_Info_Parameters(Executable_Node * Current_Node,
				     boolean Optional)
{
    Executable_Node *Temp_Node = NULL;	/* Used for lookahead operations. */
    Parse_Results Return_Value;	/* Used to track the return value from other functions called by this function. */

    /* Check for a comma. */
    Return_Value = Check_For_Comma(Optional);

    if (Return_Value == Error)
	return Return_Value;

    if (Return_Value == Try_Again) {

	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_EVMS_Identifier, Command_Filename,
			    Current_Token);
	Return_Value = Error;

    } else {
	/* Get the object we are to get the extended info for. */
	Return_Value =
	    Specify_Object(Current_Node, Translate_Name_To_Handle,
			   Optional);
	if (Return_Value == Token_Accepted) {
	    /* Now get the optional field name. */

	    /* Activate lookahead. */
	    Temp_Node = Activate_Lookahead();
	    if (Temp_Node == NULL)
		return Error;

	    /* Check for a comma. */
	    Return_Value = Check_For_Comma(TRUE);
	    if (Return_Value == Token_Accepted)
		Return_Value = Name(Temp_Node, TRUE, TRUE);

	    if ((Return_Value != Token_Accepted) &&
		(!Restore_Current_Token(Temp_Node)))
		return Error;


	    /* Exit Lookahead mode. */
	    if (!Terminate_Lookahead
		(Current_Node, Temp_Node,
		 (Return_Value == Token_Accepted)))
		return Error;

	    if (Return_Value != Error)
		Return_Value = Token_Accepted;	/* Name was optional, so the command is complete with or without it! */

	}

    }

    return Return_Value;
}


static Parse_Results
Parse_Object_List(Executable_Node * Current_Node, boolean Optional)
{
    Executable_Node *Child = NULL;	/* The node used to represent the current command being parsed. */
    Executable_Node *Temp_Node = NULL;	/* Used for lookahead operations. */
    Parse_Results Return_Value;	/* Used to track the return value from other functions called by this function. */
    uint Count = 0;		/* Used to track how many items are in the list. */

    /* Create the child node and add it to the children list of Current_Node.  We will fill in the contents of Child later. */
    Child = Make_Child_Node(Current_Node, TRUE);
    if (Child == NULL)
	return Error;

    Child->NodeType = ObjectList;

    /* We have a list of objects separated by commas.  A comma precedes the list as well. */

    for (;;) {

	/* Activate lookahead. */
	Temp_Node = Activate_Lookahead();
	if (Temp_Node == NULL)
	    return Error;

	/* Do we have a comma? */
	Return_Value = Check_For_Comma(TRUE);
	if (Return_Value != Token_Accepted)
	    break;

	Return_Value =
	    Specify_Object(Temp_Node, Translate_Name_To_Handle, TRUE);
	if (Return_Value == Token_Accepted)
	    Count++;
	else
	    break;

	/* Exit Lookahead mode. */
	if (!Terminate_Lookahead(Child, Temp_Node, TRUE))
	    return Error;

    }

    /* Restore the current token so that whoever we return to can parse it. */
    if (!Restore_Current_Token(Temp_Node))
	return Error;

    /* Exit Lookahead mode. */
    if (!Terminate_Lookahead(Child, Temp_Node, FALSE))
	return Error;

    /* Do we have any objects in the list? */
    if (Count == 0) {
	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Object_List, Command_Filename, NULL);

	return Error;
    }

    return Token_Accepted;

}


static Parse_Results
Parse_Query_Acceptable_Parameters(Executable_Node * Current_Node,
				  boolean Optional)
{
    Executable_Node *Child = NULL;	/* The node used to represent the current command being parsed. */
    Parse_Results Return_Value = Error;	/* Used to track the return value from other functions called by this function. */

    /* Create the child node and add it to the children list of Current_Node.  We will fill in the contents of Child later. */
    Child = Make_Child_Node(Current_Node, TRUE);
    if (Child == NULL)
	return Error;

    /* Get the token to parse. */
    Next_Token();

    /* Check for EOF or InvalidCharacter */
    if ((Current_Token == NULL)
	|| (Current_Token->Characterization == EofToken)
	|| (Current_Token->Characterization == InvalidCharacter)) {

	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Query_Acceptable_Parameters,
			    Command_Filename, Current_Token);

	return Error;

    }

    /* The parameter we are looking for is characterized as a KeyWord.  */
    if (Current_Token->Characterization != KeyWord) {

	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Query_Acceptable_Parameters,
			    Command_Filename, Current_Token);

	return Error;

    }

    switch (Current_Token->TokenText[0]) {
    case 'C':			/* Create */
	if ((Current_Token->TokenText == CreateStr) ||
	    (Current_Token->TokenLength == 1)) {
	    Child->NodeType = Filter;
	    Child->NodeValue = CreateStr;

	    Consume_Token();

	    Return_Value = Check_For_Comma(Optional);
	    if (Return_Value == Token_Accepted)
		Return_Value =
		    Parse_Query_Acceptable_Create(Child, Optional);
	} else if (Optional)
	    Return_Value = Try_Again;
	else {
	    Return_Value = Error;
	    Report_Parser_Error(Expecting_Query_Acceptable_Parameters,
				Command_Filename, Current_Token);
	}
	break;
    case 'E':			/* Expand */
	if ((Current_Token->TokenText == ExpandStr) ||
	    (Current_Token->TokenLength == 1)) {
	    Child->NodeType = Filter;
	    Child->NodeValue = ExpandStr;

	    Consume_Token();

	    Return_Value = Check_For_Comma(Optional);
	    if (Return_Value == Token_Accepted) {
		Return_Value =
		    Specify_Object(Child, Translate_Name_To_Handle,
				   Optional);
		if (Return_Value == Token_Accepted) {
		    Return_Value = Parse_Object_List(Child, TRUE);
		    if (Return_Value == Try_Again)
			Return_Value = Token_Accepted;	/* The object list is optional, so if we don't find one we don't care. */
		}

	    }

	} else if (Optional)
	    Return_Value = Try_Again;
	else {
	    Return_Value = Error;
	    Report_Parser_Error(Expecting_Query_Acceptable_Parameters,
				Command_Filename, Current_Token);
	}
	break;
    default:
	if (Optional)
	    Return_Value = Try_Again;
	else {
	    Return_Value = Error;
	    Report_Parser_Error(Expecting_Query_Acceptable_Parameters,
				Command_Filename, Current_Token);
	}
	break;
    }

    return Return_Value;

}


static Parse_Results
Parse_Query_Acceptable_Create(Executable_Node * Current_Node,
			      boolean Optional)
{
    Executable_Node *Child = NULL;	/* The node used to represent the current command being parsed. */
    Parse_Results Return_Value = Error;	/* Used to track the return value from other functions called by this function. */
    boolean Create_Volume = FALSE;
    boolean Create_Container = FALSE;

    /* Get the token to parse. */
    Next_Token();

    /* Check for EOF or InvalidCharacter */
    if ((Current_Token == NULL)
	|| (Current_Token->Characterization == EofToken)
	|| (Current_Token->Characterization == InvalidCharacter)) {

	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Query_Acceptable_Create,
			    Command_Filename, Current_Token);

	return Error;

    }

    /* The parameter we are looking for is characterized as a KeyWord.  */
    if (Current_Token->Characterization != KeyWord) {

	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Query_Acceptable_Create,
			    Command_Filename, Current_Token);

	return Error;

    }

    /* Assume failure. */
    if (Optional)
	Return_Value = Try_Again;
    else
	Return_Value = Error;

    /* See what we have. */
    switch (Current_Token->TokenText[0]) {
    case 'V':			/* Volume */
	if ((Current_Token->TokenText == VolumeStr) ||
	    (Current_Token->TokenLength == 1)) {

	    /* Create the child node and add it to the children list of Current_Node. */
	    Child = Make_Child_Node(Current_Node, TRUE);
	    if (Child == NULL)
		return Error;

	    /* Fill in the fields of Child */
	    Child->NodeType = Topmost_Objects;
	    Child->NodeValue = VolumeStr;

	    Return_Value = Token_Accepted;
	    Create_Volume = TRUE;
	}
	break;
    case 'O':			/* Object */
	if ((Current_Token->TokenText == ObjectStr) ||
	    (Current_Token->TokenLength == 1)) {
	    /* We don't need a new node for queries of this type. */
	    Return_Value = Token_Accepted;
	}
	break;
    case 'R':			/* Region */
	if ((Current_Token->TokenText == RegionStr) ||
	    (Current_Token->TokenLength == 1)) {
	    /* We don't need a new node for queries of this type. */
	    Return_Value = Token_Accepted;
	}
	break;
    case 'S':			/* Segment */
	if ((Current_Token->TokenText == SegmentStr) ||
	    (Current_Token->TokenLength == 1)) {
	    /* We don't need a new node for queries of this type. */
	    Return_Value = Token_Accepted;
	}
	break;
    case 'C':			/* Container */
	if ((Current_Token->TokenText == ContainerStr) ||
	    (Current_Token->TokenLength == 1)) {
	    /* Create the child node and add it to the children list of Current_Node. */
	    Child = Make_Child_Node(Current_Node, TRUE);
	    if (Child == NULL)
		return Error;

	    /* Fill in the fields of Child */
	    Child->NodeType = Task_Adjust;
	    Child->NodeValue = ContainerStr;

	    Create_Container = TRUE;
	    Return_Value = Token_Accepted;

	}
	break;
    default:
	break;
    }

    if (Return_Value == Error)
	Report_Parser_Error(Expecting_Query_Acceptable_Create,
			    Command_Filename, Current_Token);

    if (Return_Value == Token_Accepted) {

	Consume_Token();

	if (!Create_Volume) {

	    Return_Value = Check_For_Comma(Optional);
	    if (Return_Value == Token_Accepted) {
		if (Create_Container)
		    Return_Value =
			Feature_Identifier(Child, Optional, TRUE, TRUE);
		else
		    Return_Value =
			Feature_Identifier(Current_Node, Optional, TRUE,
					   TRUE);
	    }

	}

    }

    return Return_Value;

}


/*******************************************
 * Utility Functions                       *
 *******************************************/

static void Assign_Text(Executable_Node * Node, TokenType * Token)
{
    if (Token->In_Lookahead_Queue) {

	Node->NodeValue = strdup(Current_Token->TokenText);
	if (Node->NodeValue == NULL) {
	    Report_Parser_Error(Out_Of_Memory, Command_Filename,
				Current_Token);
	    exit(ENOMEM);
	}

    } else {
	Node->NodeValue = Token->TokenText;
	Token->TokenText = NULL;
    }
    return;
}

static void Consume_Token(void)
{				/* Frees the current token and sets CurrentToken to NULL. */

    if ((Current_Token != NULL) && (!Current_Token->In_Lookahead_Queue)) {

	if ((Current_Token->TokenText != NULL) &&
	    (Current_Token->Characterization != KeyWord))
	    free(Current_Token->TokenText);

	free(Current_Token);

    }

    Current_Token = NULL;
    Use_Current_Token = FALSE;

}


static void Next_Token(void)
{				/* Gets the next token to be parsed and sets CurrentToken to point to it. */

    uint Return_Value;
    ADDRESS Unneeded;

    if (Use_Current_Token) {
	Use_Current_Token = FALSE;
	return;
    }

    if (Tokens == NULL)
	Current_Token = GetScreenedToken();
    else {
	do {
	    Return_Value =
		GetNextObject(Tokens, TOKEN_CODE, (void *) &Current_Token);
	}
	while ((Return_Value == DLIST_SUCCESS) &&
	       ((Current_Token == NULL)
		|| (Current_Token == &Null_Token)));

	if (Return_Value != DLIST_SUCCESS) {

	    if (Return_Value == DLIST_END_OF_LIST) {

		/* There were no tokens remaining in the list, so get one. */
		Current_Token = GetScreenedToken();

		/* If lookahead is active, then we must add the new token to the Tokens list. */
		if (LookAhead_Active > 0) {

		    /* Add Current_Token to the Tokens list. */
		    if (InsertObject
			(Tokens, Current_Token, TOKEN_CODE, NULL,
			 AppendToList, TRUE, &Unneeded) != DLIST_SUCCESS) {
			/* We must be out of memory! */
			Report_Parser_Error(Out_Of_Memory, NULL, NULL);

			Current_Token = NULL;
			return;

		    }

		    Current_Token->In_Lookahead_Queue = TRUE;

		} else {

		    /* Get rid of the Tokens list since Lookahead is not active and all of the
		       tokens in it have been used -- it is not needed anymore.                 */
		    PruneList(Tokens, &Kill_Tokens_List_Entries, NULL);

		    DestroyList(&Tokens, TRUE);

		}

	    } else {
		/* This error condition should only happen if there is an internal error of some sort! */
		Report_Parser_Error(Internal_Error_Encountered, NULL,
				    NULL);

		exit(0xFF);

	    }

	}

    }

    return;

}

static boolean Restore_Current_Token(Executable_Node * Temp_Node)
{				/* Restores Current_Token to the value it was at before any Lookahead calls were made. */

    ADDRESS Token_Handle;
    ADDRESS Token_Handle_ptr;
    int Error;

    /* Free Temp_Node */

    /* Free the NodeValue (if any ) */
    if (Temp_Node->NodeValue != NULL)
	free(Temp_Node->NodeValue);

    /* If the Temp_Node has children, we must kill them also. */
    if (Temp_Node->Children != NULL) {

	/* Kill the children. */
	Error =
	    PruneList(Temp_Node->Children, Kill_Command_List_Entries,
		      NULL);

	if (Error != DLIST_SUCCESS) {
	    Report_Parser_Error(Internal_Error_Encountered, NULL, NULL);
	    return FALSE;
	}

    }

    /* Now we must deal with resetting the current token. */
    if (Tokens == NULL) {
	Report_Parser_Error(Internal_Error_Encountered, NULL, NULL);
	return FALSE;
    }

    /* Get the handle of the Token to reset to. */
    if (GoToEndOfList(Lookahead_Handles) != DLIST_SUCCESS) {
	Report_Parser_Error(Internal_Error_Encountered, NULL, NULL);
	return FALSE;
    }

    if (GetObject
	(Lookahead_Handles, HANDLE_TAG, NULL, FALSE,
	 &Token_Handle_ptr) != DLIST_SUCCESS) {
	Report_Parser_Error(Internal_Error_Encountered, NULL, NULL);
	return FALSE;
    }

    memcpy(&Token_Handle, Token_Handle_ptr, sizeof(ADDRESS));

    /* Reset the Tokens list based upon the handle from the Lookahead_Handles list. */
    if (GoToSpecifiedItem(Tokens, Token_Handle) != DLIST_SUCCESS) {
	Report_Parser_Error(Internal_Error_Encountered, NULL, NULL);
	return FALSE;
    }

    /* Get the token from the tokens list. */
    if (GetObject(Tokens, TOKEN_CODE, NULL, TRUE, (void *) &Current_Token)
	!= DLIST_SUCCESS) {
	Report_Parser_Error(Internal_Error_Encountered, NULL, NULL);
	return FALSE;
    }

    if (Current_Token == &Null_Token)
	Current_Token = NULL;

    return TRUE;

}

static Executable_Node *Activate_Lookahead(void)
{

    ADDRESS Lookahead_Handle;
    ADDRESS *Lookahead_Handle_ptr;
    ADDRESS NotNeeded;

    if (Tokens == NULL) {
	/* Create the Tokens list. */
	Tokens = CreateList();
	if (Tokens == NULL) {

	    /* Since we can't create the Tokens list, we can't do lookahead! */
	    Report_Parser_Error(Out_Of_Memory, NULL, NULL);

	    return FALSE;

	}

	if (Current_Token == NULL)
	    Current_Token = &Null_Token;

	/* Add the Current_Token to the Tokens list. */
	if (InsertObject
	    (Tokens, Current_Token, TOKEN_CODE, NULL, AppendToList, FALSE,
	     (void *) &Lookahead_Handle) != DLIST_SUCCESS) {
	    /* We must be out of memory! */
	    Report_Parser_Error(Out_Of_Memory, NULL, NULL);

	    return FALSE;

	}

	Current_Token->In_Lookahead_Queue = TRUE;

    }

    if (Lookahead_Handles == NULL) {
	/* Create the Lookahead_Handles list. */
	Lookahead_Handles = CreateList();
	if (Lookahead_Handles == NULL) {

	    /* Since we can't create the Lookahead_Handles list, we can't do lookahead! */
	    Report_Parser_Error(Out_Of_Memory, NULL, NULL);

	    return FALSE;

	}

    }

    Lookahead_Handle_ptr = malloc(sizeof(ADDRESS));
    if (Lookahead_Handle_ptr == NULL) {
	Report_Parser_Error(Out_Of_Memory, NULL, NULL);
	return FALSE;
    }

    /* Get the handle of the Current_Token in the Tokens List. */
    if (GetHandle(Tokens, &Lookahead_Handle) != DLIST_SUCCESS) {
	Report_Parser_Error(Internal_Error_Encountered, NULL, NULL);
	return FALSE;
    }

    memcpy(Lookahead_Handle_ptr, &Lookahead_Handle, sizeof(ADDRESS));

    /* Add the handle of the Current_Token in the Tokens list to the Lookahead_Handles list. */
    if (InsertObject
	(Lookahead_Handles, Lookahead_Handle_ptr, HANDLE_TAG, NULL,
	 AppendToList, FALSE, &NotNeeded)) {
	/* We must be out of memory! */
	Report_Parser_Error(Out_Of_Memory, NULL, NULL);

	return FALSE;
    }

    /* Enable Lookahead. */
    LookAhead_Active++;

    /* We need to return a temporary executable node to our caller for use while lookahead is active. */
    return Make_Child_Node(NULL, FALSE);

}


static boolean
Terminate_Lookahead(Executable_Node * Current_Node,
		    Executable_Node * Temp_Node, boolean Keep_Children)
{
    ADDRESS Current_Handle = NULL;	/* The handle of the current item in the Tokens list. */
    ADDRESS Lookahead_Handle_ptr;
    TAG tag;

    if (LookAhead_Active > 0)
	LookAhead_Active--;
    else
	return FALSE;

    if ((Tokens != NULL) && (LookAhead_Active == 0)) {

	/* Delete all of the items in the Tokens list prior to the current one. */

	/* Get the handle of the current token in the Tokens list. */
	if (GetHandle(Tokens, &Current_Handle) != DLIST_SUCCESS) {
	    Report_Parser_Error(Internal_Error_Encountered, NULL, NULL);
	    return FALSE;
	}

	if (PruneList(Tokens, &Kill_Tokens_List_Entries, Current_Handle) !=
	    DLIST_SUCCESS) {
	    Report_Parser_Error(Internal_Error_Encountered, NULL, NULL);
	    return FALSE;
	}

    }

    /* Remove the last handle from the Lookahead_Handles list. */
    if ((GoToEndOfList(Lookahead_Handles) != DLIST_SUCCESS) ||
	(BlindExtractObject
	 (Lookahead_Handles, &tag, NULL,
	  &Lookahead_Handle_ptr) != DLIST_SUCCESS))
	return FALSE;

    free(Lookahead_Handle_ptr);

    if (Keep_Children) {

	/* Now we must append the children list of Temp_Node to the children list of Current_Node.
	   Then we can free Temp_Node.                                                              */
	if (AppendList(Current_Node->Children, Temp_Node->Children) !=
	    DLIST_SUCCESS) {
	    Report_Parser_Error(Internal_Error_Encountered, NULL, NULL);
	    return FALSE;
	}

    } else {

	/* We don't need the children, so delete them. */
	if (PruneList
	    (Temp_Node->Children, &Kill_Command_List_Entries,
	     NULL) != DLIST_SUCCESS) {
	    Report_Parser_Error(Internal_Error_Encountered, NULL, NULL);
	    return FALSE;
	}

    }

    if (DestroyList(&Temp_Node->Children, TRUE) != DLIST_SUCCESS) {
	Report_Parser_Error(Internal_Error_Encountered, NULL, NULL);
	return FALSE;
    }

    if (Temp_Node->NodeValue != NULL)
	free(Temp_Node->NodeValue);

    free(Temp_Node);

    return TRUE;

}

static boolean
Kill_Tokens_List_Entries(ADDRESS Object,
			 TAG ObjectTag,
			 ADDRESS ObjectHandle,
			 ADDRESS Parameters,
			 boolean * FreeMemory, uint * Error)
{

    /* Establish easy access to the current token. */
    TokenType *Token_To_Kill = (TokenType *) Object;
    ADDRESS Token_To_Match = Parameters;

    /* Assume success. */
    *Error = DLIST_SUCCESS;

    /* Sanity check!  Is the node of the expected type? */
    if (ObjectTag != TOKEN_CODE) {

	/* We have an illegal node in the list!  Abort! */
	*Error = DLIST_ITEM_TAG_WRONG;
	return FALSE;

    }

    /* Should we kill the current token? */
    if (ObjectHandle != Token_To_Match) {

	if ((Token_To_Kill != NULL) &&
	    (Token_To_Kill != &Null_Token) &&
	    (Token_To_Kill->TokenText != NULL) &&
	    (Token_To_Kill->Characterization != KeyWord))
	    free(Token_To_Kill->TokenText);

	if ((Token_To_Kill == NULL) || (Token_To_Kill == &Null_Token))
	    *FreeMemory = FALSE;
	else
	    *FreeMemory = TRUE;

	return TRUE;

    }

    /* We have found the current token.  We can stop deleting tokens. */
    *FreeMemory = FALSE;
    *Error = DLIST_SEARCH_COMPLETE;
    return FALSE;

}



static void Free_Command_List(dlist_t Command_List)
{				/* Deletes all entries in the Command_List and frees their memory. */

    if (Command_List != NULL) {

	/* Free all of the elements within the command list. */
	PruneList(Command_List, &Kill_Command_List_Entries, NULL);

	/* Now free the command list itself. */
	DestroyList(&Command_List, TRUE);

    }

    return;

}

static boolean
Kill_Command_List_Entries(ADDRESS Object,
			  TAG ObjectTag,
			  ADDRESS ObjectHandle,
			  ADDRESS Parameters,
			  boolean * FreeMemory, uint * Error)
{

    /* Establish easy access to the current node. */
    Executable_Node *Node_To_Kill = (Executable_Node *) Object;

    /* Assume success. */
    *Error = DLIST_SUCCESS;

    /* Sanity check!  Is the node of the expected type? */
    if (ObjectTag != STACK_NODE) {

	/* We have an illegal node in the list!  Abort! */
	*Error = DLIST_ITEM_TAG_WRONG;
	return FALSE;

    }

    /* Free the NodeValue (if any ) */
    /* if ( Node_To_Kill->NodeValue != NULL )
       free(Node_To_Kill->NodeValue); */

    /* If the node has children, we must kill them also. */
    if (Node_To_Kill->Children != NULL) {

	/* Kill the children. */
	*Error =
	    PruneList(Node_To_Kill->Children, Kill_Command_List_Entries,
		      NULL);

	if (*Error != DLIST_SUCCESS)
	    return FALSE;

	*Error = DestroyList(&Node_To_Kill->Children, TRUE);
	if (*Error != DLIST_SUCCESS)
	    return FALSE;

    }

    /* Now kill the node itself. */
    *FreeMemory = TRUE;

    return TRUE;

}

static Parse_Results Check_For_Colon(boolean Optional)
{

    /* Get the next token. */
    Next_Token();

    /* Check for EOF or InvalidCharacter */
    if ((Current_Token == NULL)
	|| (Current_Token->Characterization == EofToken)
	|| (Current_Token->Characterization == InvalidCharacter)) {

	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Colon_Separator, Command_Filename,
			    Current_Token);

	return Error;

    }

    /* Check for the ':' separator. */
    if ((Current_Token->Characterization != Separator) ||
	(Current_Token->TokenText[0] != ':')) {

	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Colon_Separator, Command_Filename,
			    Current_Token);

	return Error;

    }

    /* Since we found the ':', consume the token. */
    Consume_Token();

    return Token_Accepted;

}

static Parse_Results Check_For_Comma(boolean Comma_Is_Optional)
{

    /* Get the next token. */
    Next_Token();

    /* Check for EOF or InvalidCharacter */
    if ((Current_Token == NULL)
	|| (Current_Token->Characterization == EofToken)
	|| (Current_Token->Characterization == InvalidCharacter)) {
	if (Comma_Is_Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Comma_Separator, Command_Filename,
			    Current_Token);

	return Error;

    }

    /* Check for the ',' separator. */
    if ((Current_Token->Characterization != Separator) ||
	(Current_Token->TokenText[0] != ',')) {

	if (Comma_Is_Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Comma_Separator, Command_Filename,
			    Current_Token);

	return Error;

    }

    /* Since we found the ',', consume the token. */
    Consume_Token();

    return Token_Accepted;

}

static Parse_Results Check_For_EqualSign(boolean EqualSign_Is_Optional)
{

    /* Get the next token. */
    Next_Token();

    /* Check for EOF or InvalidCharacter */
    if ((Current_Token == NULL)
	|| (Current_Token->Characterization == EofToken)
	|| (Current_Token->Characterization == InvalidCharacter)) {

	if (EqualSign_Is_Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Equal_Sign_Separator,
			    Command_Filename, Current_Token);

	return Error;

    }

    /* Check for the '=' separator. */
    if ((Current_Token->Characterization != Separator) ||
	(Current_Token->TokenText[0] != '=')) {

	if (EqualSign_Is_Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Equal_Sign_Separator,
			    Command_Filename, Current_Token);

	return Error;

    }

    /* Since we found the '=', consume the token. */
    Consume_Token();

    return Token_Accepted;

}

static Parse_Results Check_For_Open_Brace(boolean Optional)
{

    /* Get the next token. */
    Next_Token();

    /* Check for EOF or InvalidCharacter */
    if ((Current_Token == NULL)
	|| (Current_Token->Characterization == EofToken)
	|| (Current_Token->Characterization == InvalidCharacter)) {

	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Open_Brace_Separator,
			    Command_Filename, Current_Token);

	return Error;

    }

    /* Check for the '{' separator. */
    if ((Current_Token->Characterization != Separator) ||
	(Current_Token->TokenText[0] != '{')) {

	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Open_Brace_Separator,
			    Command_Filename, Current_Token);

	return Error;

    }

    /* Since we found the '{', consume the token. */
    Consume_Token();

    return Token_Accepted;

}


static Parse_Results Check_For_Closing_Brace(boolean Optional)
{

    /* Get the next token. */
    Next_Token();

    /* Check for EOF or InvalidCharacter */
    if ((Current_Token == NULL)
	|| (Current_Token->Characterization == EofToken)
	|| (Current_Token->Characterization == InvalidCharacter)) {

	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Closing_Brace_Separator,
			    Command_Filename, Current_Token);

	return Error;

    }

    /* Check for the '}' separator. */
    if ((Current_Token->Characterization != Separator) ||
	(Current_Token->TokenText[0] != '}')) {

	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Closing_Brace_Separator,
			    Command_Filename, Current_Token);

	return Error;

    }

    /* Since we found the '}', consume the token. */
    Consume_Token();

    return Token_Accepted;

}


static Parse_Results Check_For_Open_Paren(boolean Optional)
{

    /* Get the next token. */
    Next_Token();

    /* Check for EOF or InvalidCharacter */
    if ((Current_Token == NULL)
	|| (Current_Token->Characterization == EofToken)
	|| (Current_Token->Characterization == InvalidCharacter)) {

	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Open_Paren_Separator,
			    Command_Filename, Current_Token);

	return Error;

    }

    /* Check for the '{' separator. */
    if ((Current_Token->Characterization != Separator) ||
	(Current_Token->TokenText[0] != '(')) {

	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Open_Paren_Separator,
			    Command_Filename, Current_Token);

	return Error;

    }

    /* Since we found the '(', consume the token. */
    Consume_Token();

    return Token_Accepted;

}


static Parse_Results Check_For_Closing_Paren(boolean Optional)
{

    /* Get the next token. */
    Next_Token();

    /* Check for EOF or InvalidCharacter */
    if ((Current_Token == NULL)
	|| (Current_Token->Characterization == EofToken)
	|| (Current_Token->Characterization == InvalidCharacter)) {

	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Closing_Paren_Separator,
			    Command_Filename, Current_Token);

	return Error;

    }

    /* Check for the '}' separator. */
    if ((Current_Token->Characterization != Separator) ||
	(Current_Token->TokenText[0] != ')')) {

	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Closing_Paren_Separator,
			    Command_Filename, Current_Token);

	return Error;

    }

    /* Since we found the ')', consume the token. */
    Consume_Token();

    return Token_Accepted;

}


static Executable_Node *Make_Child_Node(Executable_Node * Current_Node,
					boolean Add_To_Current_Node)
{

    Executable_Node *Child = NULL;
    void *Unneeded;

    /* Allocate the child node. */
    Child = (Executable_Node *) malloc(sizeof(Executable_Node));
    if (Child != NULL) {
	Child->NodeValue = NULL;
	Child->Children = CreateList();
	if (Child->Children == NULL) {
	    free(Child);
	    Child = NULL;
	}

    }

    /* Did we succeed in creating the child node? */
    if (Child == NULL) {

	Report_Parser_Error(Out_Of_Memory, NULL, NULL);

	return NULL;

    }

    if (Add_To_Current_Node) {

	/* Add the child node to the Current_Node. */
	if (InsertObject
	    (Current_Node->Children, Child, STACK_NODE, NULL, AppendToList,
	     TRUE, &Unneeded) != DLIST_SUCCESS) {
	    /* We must be out of memory! */
	    Report_Parser_Error(Out_Of_Memory, NULL, NULL);

	    /* Free the Child node. */
	    DestroyList(&Child->Children, TRUE);
	    free(Child);
	    Child = NULL;

	}

    }

    Child->NodeValue = NULL;
    return Child;

}

static Parse_Results
Parse_Filters(Executable_Node * Current_Node,
	      Available_Generic_Filters Filters_Allowed,
	      Available_Generic_Filters * Filters_In_Use, boolean Optional)
{
    Executable_Node *Child = NULL;	/* The node used to represent the current command being parsed. */
    Executable_Node *Temp_Node = NULL;	/* Used for lookahead purposes. */
    Parse_Results Return_Value = Try_Again;	/* Used to track the return value from other functions called by this function. */
    boolean Bad_Filter = FALSE;
    boolean Double_Filter = FALSE;
    void *Node_Value = NULL;

    Next_Token();

    /* Check for EOF, InvalidCharacter, or incorrect characterization */
    if ((Current_Token == NULL) ||
	(Current_Token->Characterization == EofToken) ||
	(Current_Token->Characterization == InvalidCharacter) ||
	(Current_Token->TokenLength == 0) ||
	(Current_Token->Characterization != KeyWord)) {

	if (Optional)
	    return Try_Again;

	Report_Parser_Error(Expecting_Filter_Identifier, Command_Filename,
			    Current_Token);

	return Error;

    }

    switch (Current_Token->TokenText[0]) {
    case 'A':
	/* Is the token the Available keyword? */
	if ((Current_Token->TokenText == AvailableStr) ||
	    (Current_Token->TokenLength == 1)) {

	    if (Filters_Allowed & Available_Filter) {

		if (!(*Filters_In_Use & Available_Filter)) {

		    Node_Value = AvailableStr;
		    Return_Value = Token_Accepted;
		    *Filters_In_Use |= Available_Filter;

		} else
		    Double_Filter = TRUE;

	    } else
		Bad_Filter = TRUE;
	}
	break;
    case 'C':
	/* Is the token the Convertible keyword? */
	if (Current_Token->TokenText == ConvertibleStr) {

	    if (Filters_Allowed & Convertible_Filter) {

		if (!(*Filters_In_Use & Convertible_Filter)) {

		    Node_Value = ConvertibleStr;
		    Return_Value = Token_Accepted;
		    *Filters_In_Use |= Convertible_Filter;

		} else
		    Double_Filter = TRUE;

	    } else
		Bad_Filter = TRUE;
	} else if ((Current_Token->TokenText == ContainerStr) ||
		   (Current_Token->TokenLength == 1)) {
	    /* We have the Container keyword. */

	    if (Filters_Allowed & Container_Filter) {

		if (!(*Filters_In_Use & Container_Filter)) {

		    /* Create the child node and add it to the children list of Current_Node. */
		    Child = Make_Child_Node(Current_Node, TRUE);
		    if (Child == NULL)
			return Error;

		    Child->NodeType = Filter;
		    Child->NodeValue = ContainerStr;

		    Consume_Token();

		    Return_Value = Check_For_EqualSign(Optional);
		    if (Return_Value == Token_Accepted)
			Return_Value =
			    Specify_Object(Child, Name_To_Container_Handle,
					   Optional);

		    if (Return_Value == Token_Accepted)
			*Filters_In_Use |= Container_Filter;
		} else
		    Double_Filter = TRUE;

	    } else
		Bad_Filter = TRUE;

	}
	break;
    case 'D':
	/* Is the token of the Disk keyword? */
	if ((Current_Token->TokenText == DiskStr) ||
	    (Current_Token->TokenLength == 1)) {
	    /* We have the Disk keyword. */
	    if (Filters_Allowed & Disk_Filter) {

		if (!(*Filters_In_Use & Disk_Filter)) {

		    /* Create the child node and add it to the children list of Current_Node. */
		    Child = Make_Child_Node(Current_Node, TRUE);
		    if (Child == NULL)
			return Error;

		    Child->NodeType = Filter;
		    Child->NodeValue = DiskStr;

		    Consume_Token();

		    Return_Value = Check_For_EqualSign(Optional);
		    if (Return_Value == Token_Accepted)
			Return_Value =
			    Specify_Object(Child, Name_To_Disk_Handle,
					   Optional);

		    if (Return_Value == Token_Accepted)
			*Filters_In_Use |= Disk_Filter;

		} else
		    Double_Filter = TRUE;

	    } else
		Bad_Filter = TRUE;

	}
	break;
    case 'E':
	/* Is the token the Expandable keyword? */
	if ((Current_Token->TokenText == ExpandableStr) ||
	    (Current_Token->TokenLength == 1)) {

	    if (Filters_Allowed & Expandable_Filter) {

		if (!(*Filters_In_Use & Expandable_Filter)) {

		    Node_Value = ExpandableStr;
		    Return_Value = Token_Accepted;
		    *Filters_In_Use |= Expandable_Filter;

		} else
		    Double_Filter = TRUE;

	    } else
		Bad_Filter = TRUE;

	} else if (Current_Token->TokenText == EQStr) {
	    /* We have the EQ keyword. */

	    if (Filters_Allowed & EQ_Filter) {

		if (!(*Filters_In_Use & EQ_Filter)) {

		    /* Create the child node and add it to the children list of Current_Node. */
		    Child = Make_Child_Node(Current_Node, TRUE);
		    if (Child == NULL)
			return Error;

		    Child->NodeType = Filter;
		    Child->NodeValue = EQStr;

		    Consume_Token();

		    /* Now get the size. */
		    Return_Value = Size(Child, Optional);
		    if (Return_Value == Token_Accepted) {
			/* Now get the precision.  The precision is a number representing how much of a "fudge" factor we should allow
			   when determining if two values are equal.                                                                   */

			/* The precision is optional. */

			Temp_Node = Activate_Lookahead();
			if (Temp_Node == NULL)
			    return Error;

			Return_Value = Check_For_Comma(TRUE);
			if (Return_Value == Token_Accepted)
			    Return_Value = Precision(Temp_Node, TRUE);	/* Get the name for the volume. */

			if (Return_Value == Try_Again) {

			    /* Restore the current token so that parsing can continue. */
			    if (!Restore_Current_Token(Temp_Node))
				return Error;

			}

			/* Exit Lookahead mode. */
			if (!Terminate_Lookahead
			    (Child, Temp_Node,
			     (Return_Value == Token_Accepted)))
			    return Error;

			/* Precision is optional, so we return Token_Accepted regardless of whether or not we found a Precision specified.
			   The only exception is if an error occurred.                                                                      */
			if (Return_Value != Error)
			    Return_Value = Token_Accepted;

			if (Return_Value == Token_Accepted)
			    *Filters_In_Use |= EQ_Filter;

		    }

		} else
		    Double_Filter = TRUE;

	    } else
		Bad_Filter = TRUE;

	}
	break;
    case 'F':
	/* Is the token the freespace keyword? */
	if ((Current_Token->TokenText == FreespaceStr) ||
	    (Current_Token->TokenLength == 1)) {
	    /* We have the Freespace keyword. */

	    if (Filters_Allowed & Freespace_Filter) {

		if (!(*Filters_In_Use & Freespace_Filter)) {
		    Node_Value = FreespaceStr;
		    Return_Value = Token_Accepted;
		    *Filters_In_Use |= Freespace_Filter;
		} else
		    Double_Filter = TRUE;
	    } else
		Bad_Filter = TRUE;

	}
	break;
    case 'G':
	if (Current_Token->TokenText == GTStr) {
	    /* We have the GT keyword. */

	    if (Filters_Allowed & GT_Filter) {

		if (!(*Filters_In_Use & GT_Filter)) {

		    /* Create the child node and add it to the children list of Current_Node. */
		    Child = Make_Child_Node(Current_Node, TRUE);
		    if (Child == NULL)
			return Error;

		    Child->NodeType = Filter;
		    Child->NodeValue = GTStr;

		    Consume_Token();

		    /* Now get the size. */
		    Return_Value = Size(Child, Optional);
		    if (Return_Value == Token_Accepted)
			*Filters_In_Use |= GT_Filter;

		} else
		    Double_Filter = TRUE;
	    } else
		Bad_Filter = TRUE;
	}
	break;
    case 'L':
	if (Current_Token->TokenText == LTStr) {
	    /* We have the LT keyword. */

	    if (Filters_Allowed & LT_Filter) {

		if (!(*Filters_In_Use & LT_Filter)) {

		    /* Create the child node and add it to the children list of Current_Node. */
		    Child = Make_Child_Node(Current_Node, TRUE);
		    if (Child == NULL)
			return Error;

		    Child->NodeType = Filter;
		    Child->NodeValue = LTStr;

		    Consume_Token();

		    /* Now get the size. */
		    Return_Value = Size(Child, Optional);
		    if (Return_Value == Token_Accepted)
			*Filters_In_Use |= LT_Filter;

		} else
		    Double_Filter = TRUE;

	    } else
		Bad_Filter = TRUE;
	} else if (Current_Token->TokenText == LOStr) {

	    /* List Options */

	    if (Filters_Allowed & LO_Filter) {

		if (!(*Filters_In_Use & LO_Filter)) {

		    /* Create the child node and add it to the children list of Current_Node. */
		    Child = Make_Child_Node(Current_Node, TRUE);
		    if (Child == NULL)
			return Error;

		    Child->NodeType = Filter;
		    Child->NodeValue = (void *) LOStr;

		    /* We are through with this token, so consume it. */
		    Consume_Token();

		    Return_Value = Token_Accepted;
		    *Filters_In_Use |= LO_Filter;

		} else
		    Double_Filter = TRUE;
	    } else
		Bad_Filter = TRUE;

	} else if (Current_Token->TokenText == ListStr) {
	    /* We are through with this token, so consume it. */
	    Consume_Token();

	    /* Get the next token. */
	    Next_Token();

	    /* Check for EOF or InvalidCharacter or wrong token characterization. */
	    if ((Current_Token == NULL)
		|| (Current_Token->Characterization == EofToken)
		|| (Current_Token->Characterization == InvalidCharacter)
		|| (Current_Token->Characterization != KeyWord))
		Return_Value = Try_Again;

	    if (Current_Token->TokenText == OptionsStr) {

		if (Filters_Allowed & LO_Filter) {

		    if (!(*Filters_In_Use & LO_Filter)) {

			/* Create the child node and add it to the children list of Current_Node. */
			Child = Make_Child_Node(Current_Node, TRUE);
			if (Child == NULL)
			    return Error;

			Child->NodeType = Filter;
			Child->NodeValue = (void *) LOStr;

			/* We are through with this token, so consume it. */
			Consume_Token();

			Return_Value = Token_Accepted;
			*Filters_In_Use |= LO_Filter;

		    } else
			Double_Filter = TRUE;
		} else
		    Bad_Filter = TRUE;

	    }

	}
	break;
    case 'O':
	if ((Current_Token->TokenText == ObjectStr) ||
	    (Current_Token->TokenLength == 1)) {
	    /* We have the Object keyword. */

	    if (Filters_Allowed & Object_Filter) {

		if (!(*Filters_In_Use & Object_Filter)) {

		    /* Create the child node and add it to the children list of Current_Node. */
		    Child = Make_Child_Node(Current_Node, TRUE);
		    if (Child == NULL)
			return Error;

		    Child->NodeType = Filter;
		    Child->NodeValue = ObjectStr;

		    Consume_Token();

		    Return_Value = Check_For_EqualSign(Optional);
		    if (Return_Value == Token_Accepted)
			Return_Value =
			    Specify_Object(Child, Name_To_Object_Handle,
					   Optional);

		    if (Return_Value == Token_Accepted)
			*Filters_In_Use |= Object_Filter;

		} else
		    Double_Filter = TRUE;
	    } else
		Bad_Filter = TRUE;

	}
	break;
    case 'P':
	/* Is the token the Plugin keyword? */
	if ((Current_Token->TokenText == PluginStr) ||
	    (Current_Token->TokenLength == 1)) {
	    /* We have the Plugin keyword. */

	    if (Filters_Allowed & Plugin_Filter) {

		if (!(*Filters_In_Use & Plugin_Filter)) {

		    /* Create the child node and add it to the children list of Current_Node.  */
		    Child = Make_Child_Node(Current_Node, TRUE);
		    if (Child == NULL)
			return Error;

		    Child->NodeType = Filter;
		    Child->NodeValue = PluginStr;

		    Consume_Token();

		    Return_Value = Check_For_EqualSign(Optional);
		    if (Return_Value == Token_Accepted)
			Return_Value =
			    Plugin_Identifier(Child, FALSE, FALSE,
					      Optional);

		    if (Return_Value == Token_Accepted)
			*Filters_In_Use |= Plugin_Filter;

		} else
		    Double_Filter = TRUE;
	    } else
		Bad_Filter = TRUE;

	}
	break;
    case 'R':
	if ((Current_Token->TokenText == RegionStr) ||
	    (Current_Token->TokenLength == 1)) {
	    /* We have the Region keyword. */

	    if (Filters_Allowed & Region_Filter) {

		if (!(*Filters_In_Use & Region_Filter)) {

		    /* Create the child node and add it to the children list of Current_Node. */
		    Child = Make_Child_Node(Current_Node, TRUE);
		    if (Child == NULL)
			return Error;

		    Child->NodeType = Filter;
		    Child->NodeValue = RegionStr;

		    Consume_Token();

		    Return_Value = Check_For_EqualSign(Optional);
		    if (Return_Value == Token_Accepted)
			Return_Value =
			    Specify_Object(Child, Name_To_Region_Handle,
					   Optional);

		    if (Return_Value == Token_Accepted)
			*Filters_In_Use |= Region_Filter;

		} else
		    Double_Filter = TRUE;
	    } else
		Bad_Filter = TRUE;

	}
	break;
    case 'S':
	/* Is the token the Shrinkable keyword? */
	if (Current_Token->TokenText == ShrinkableStr) {

	    if (Filters_Allowed & Shrinkable_Filter) {

		if (!(*Filters_In_Use & Shrinkable_Filter)) {

		    Node_Value = ShrinkStr;
		    Return_Value = Token_Accepted;

		    *Filters_In_Use |= Shrinkable_Filter;

		} else
		    Double_Filter = TRUE;

	    } else
		Bad_Filter = TRUE;

	} else if ((Current_Token->TokenText == SegmentStr) ||
		   (Current_Token->TokenLength == 1)) {
	    /* We have the Segment keyword. */

	    if (Filters_Allowed & Segment_Filter) {

		if (!(*Filters_In_Use & Segment_Filter)) {

		    /* Create the child node and add it to the children list of Current_Node. */
		    Child = Make_Child_Node(Current_Node, TRUE);
		    if (Child == NULL)
			return Error;

		    Child->NodeType = Filter;
		    Child->NodeValue = SegmentStr;

		    Consume_Token();

		    Return_Value = Check_For_EqualSign(Optional);
		    if (Return_Value == Token_Accepted)
			Return_Value =
			    Specify_Object(Child, Name_To_Segment_Handle,
					   Optional);

		    if (Return_Value == Token_Accepted)
			*Filters_In_Use |= Segment_Filter;

		} else
		    Double_Filter = TRUE;
	    } else
		Bad_Filter = TRUE;

	}
	break;
    case 'T':
	if ((Current_Token->TokenText == TypeStr) ||
	    (Current_Token->TokenLength == 1)) {

	    if (Filters_Allowed & Plugin_Type_Filter) {

		if (!(*Filters_In_Use & Plugin_Type_Filter)) {

		    /* Create the child node and add it to the children list of Current_Node. */
		    Child = Make_Child_Node(Current_Node, TRUE);
		    if (Child == NULL)
			return Error;

		    Child->NodeType = Filter;
		    Child->NodeValue = (void *) TypeStr;

		    /* We are through with this token, so consume it. */
		    Consume_Token();

		    /* Get the = */
		    Return_Value = Check_For_EqualSign(Optional);

		    if (Return_Value == Token_Accepted)
			Return_Value = Plugin_Types(Child, Optional);

		    if (Return_Value == Token_Accepted)
			*Filters_In_Use |= Plugin_Type_Filter;
		} else
		    Double_Filter = TRUE;
	    } else
		Bad_Filter = TRUE;

	}
	break;
    case 'U':
	/* Is the token the Unclaimed keyword? */
	if ((Current_Token->TokenText == UnclaimedStr) ||
	    (Current_Token->TokenLength == 1)) {

	    if (Filters_Allowed & Unclaimed_Filter) {

		if (!(*Filters_In_Use & Unclaimed_Filter)) {

		    Node_Value = UnclaimedStr;
		    Return_Value = Token_Accepted;

		    *Filters_In_Use |= Unclaimed_Filter;

		} else
		    Double_Filter = TRUE;

	    } else
		Bad_Filter = TRUE;

	}
	break;
    case 'V':
	if ((Current_Token->TokenText == VolumeStr) ||
	    (Current_Token->TokenLength == 1)) {
	    /* We have the Volume keyword. */

	    if (Filters_Allowed & Volume_Filter) {

		if (!(*Filters_In_Use & Volume_Filter)) {

		    /* Create the child node and add it to the children list of Current_Node. */
		    Child = Make_Child_Node(Current_Node, TRUE);
		    if (Child == NULL)
			return Error;

		    Child->NodeType = Filter;
		    Child->NodeValue = VolumeStr;

		    Consume_Token();

		    Return_Value = Check_For_EqualSign(Optional);
		    if (Return_Value == Token_Accepted)
			Return_Value =
			    Specify_Object(Child, Name_To_Volume_Handle,
					   Optional);

		    if (Return_Value == Token_Accepted)
			*Filters_In_Use |= Volume_Filter;

		} else
		    Double_Filter = TRUE;

	    } else
		Bad_Filter = TRUE;

	}
	break;
    default:
	break;			/* Keep the compiler happy. */
    }

    if ((Node_Value != NULL) && (Return_Value == Token_Accepted)) {

	/* Create the child node and add it to the children list of Current_Node. */
	Child = Make_Child_Node(Current_Node, TRUE);
	if (Child == NULL)
	    return Error;

	Child->NodeType = Filter;
	Child->NodeValue = Node_Value;

	Consume_Token();

    }

    if (Return_Value == Try_Again) {
	if (Optional)
	    return Try_Again;

	if (Bad_Filter)
	    Report_Parser_Error(Wrong_Filter, Command_Filename,
				Current_Token);
	else if (Double_Filter)
	    Report_Parser_Error(Duplicate_Filter, Command_Filename,
				Current_Token);
	else
	    Report_Parser_Error(Expecting_Filter_Identifier,
				Command_Filename, Current_Token);

	Return_Value = Error;
    }

    return Return_Value;

}
