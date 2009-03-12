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
 * Module: scanner.c
 */

/*
 * Change History:
 *
 */

/*
 *
 */

/* This module implements the scanner, which is the front end of the syntactic
   analyzer used by the LVM command.  The scanner is an FSA whose states and
   actions are as indicated in the table below:
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
          ',',':','}','(',')'               -> Start                    => "Separator"
          ' '                               -> SingleSpace
          '\t'                              -> SingleTab
          EOF                               -> EndState
                                            -> Error;

   PositiveNumberCheck --
          '0' .. '9'                        -> IsNumber
                                            -> EndState                 => "Invalid Character";

   IsPathName --
          '/' , 'A' .. 'Z' , 'a' .. 'z'     -> IsPathName
          '0' .. '9' , '.'                       -> IsPathName
          '_' , '-'                         -> IsPathName
                                            -> SeparatorOrError         => "String";

   IsKeyWord --
          'A' .. 'Z' , 'a' .. 'z'           -> IsKeyWord
          '0' .. '9' , '.'                  -> IsKeyWord
          '_' , '-' , '#'                   -> IsKeyWord
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


/* Identify this file. */
#define SCANNER_C

/*--------------------------------------------------
 * Necessary include files
 --------------------------------------------------*/
#include <ctype.h>		/* toupper */
#include <stdlib.h>		/* malloc */
#include <stdio.h>
#include <string.h>
#include "token.h"		/* TokenType, TokenCharacterizations, MaxIdentifierLength */
#include "error.h"		/* Scanner_Errors, Report_Scanner_Error */
#include "scanner.h"		/* GetToken, SetInput prototypes */

/*--------------------------------------------------
 * Private constants
 --------------------------------------------------*/



/*--------------------------------------------------
 * Private Type definitions
 --------------------------------------------------*/

/* Private types for the GetToken function. */

/* The following enumeration has one entry for each state in the
   state table.  A variable of this type will be used to keep
   track of which state the FSA is in at any given time.          */
typedef enum {
    Start,
    OptionCheck,
    IsNegativeNumber,
    IsNumber,
    IsRealNumber,
    SeparatorOrError,
    PositiveNumberCheck,
    IsKeyWord,
    IsPathName,
    EndOfString,
    IsString,
    SingleSpace,
    IsMultiSpace,
    SingleTab,
    IsMultiTab,
    EndState,
    ErrorState
} State;

/* Private types for the GetCharacter function. */

/* The following enumeration has one entry for each state in the
   state table.  A variable of this type will be used to keep
   track of which state the FSA is in at any given time.          */
typedef enum {
    Normal,
    Normal2,
    End_State,
    Comment_Found,
    Comment_Found2,
    Not_Comment,
    Start_Comment_Check,
    Embedded_Comment_Check,
    Find_End_Of_Comment,
    Start_Substitution,
    Not_Substitution,
    Substitution_Found,
    Skip_Initial_Spaces,
    Get_Replacement_ID,
    Skip_End_Spaces,
    Do_Substitution
} Input_State;


typedef struct Input_Context_t {
    char *CommandLine;
    char *CurrentCommandLine;
    char *Error_Source;		/* Used for error reporting purposes.  It points to either CommandLine or the name of the input file. */
    uint CurrentRow;		/* If the command line is actually a command file, this will indicate which line of the command file is being processed. */
    uint CurrentColumn;		/* The current position (0 based) within the current line/command line expressed as an offset. */
    FILE *InputFile;
    struct Input_Context_t *Next_Context;
} Input_Context_Type;


/*--------------------------------------------------
  Private global variables.
--------------------------------------------------*/
static Input_Context_Type Anchor_Context = { NULL, NULL, NULL, 0, 0, NULL, &Anchor_Context };	/* Holds the initial input context. */
static Input_Context_Type *Input_Context = &Anchor_Context;	/* Holds the current input source and position within that input source. */
static Substitution_Table_Type *Replacement_Parameters = NULL;	/* Used to hold substitutable parameters. */
static uint CharactersConsumed;	/* Count of the number of characters we have used from the command line. */
static char Buffer[MaxIdentifierLength];	/* Our buffer for creating tokens. */
static uint PositionInBuffer;	/* Used to keep track of where to put characters in the Buffer. */
static boolean ExamineCurrentCharacter = FALSE;	/* Causes a state to not load a new character but to examine the existing one. */
static boolean CharacterInjected = FALSE;	/* Used for states where a character is just "stuffed" into CharacterToExamine. 
						   rather than using the character in NextCharacter.                            */
static char CharacterToExamine = ' ';	/* The current character to examine.  Examination of a character will
					   result in the FSA transitioning to a new state in accordance with
					   the state table at the beginning of this file.                              */
static char NextCharacter = ' ';	/* The next character that will be examined. */
static boolean Detect_Comments = TRUE;	/* Control comment detection and elimination.  If FALSE, then comment detection is disabled. */
static boolean Substitutions_Allowed = TRUE;	/* Controls whether $(x) is treated as text or a command to replace the $(x) with a parameter #x from the command line used to invoke the EVMS CLI. */
static State CurrentState = Start;	/* The current state within the FSA used by the GetToken function.  */
static Input_State Current_Input_State = Normal;	/* The current state of the FSA used for the GetCharacter function. */


/*--------------------------------------------------
 Local Function Prototypes
--------------------------------------------------*/

/* The following function creates a token using the the characters in the Buffer.  */
static TokenType *MakeToken(TokenCharacterizations Characterization);

/* The following function gets a character using the current context.  If the
   current context uses a file as its input source, then this function updates
   the CurrentColumn and CurrentRow variables.  If the current context uses
   a buffer as input and the next context is the anchor context, then this
   function updates the CurrentColumn and CurrentRow variables.  CharactersConsumed
   is updated at all times.
   NOTE:  If a NULL is encountered in a file or input buffer, it will be treated
          as EOF and NULL will be returned.
   NOTE:  If an I/O error is encountered while attempting to access an input file,
          this will be treated as EOF and NULL will be returned.       
   NOTE:  Whenever this function returnes NULL, whether due to an error or an EOF,
          all contexts other than the anchor context will be closed out -- i.e.
          contexts where the input is a file will have the file closed, contexts
          where input is from a buffer will have the buffer deallocated, the
          context records themselves will be deallocated.
   NOTE:  If the current context has no more characters available, but other contexts
          exist on the context stack, then the current context will be closed and
          the next available context on the context stack will become the current
          context and be used as the source for the next character.       
   NOTE:  The character returned by this function is returned in the NextCharacter
          global variable.                                                                */
static void Read_Character(void);

/* The following function gets a character to examine taking into account comments and
   substitutions -- i.e. comments are skipped and text substitutions are made when 
   a substitution command is encountered.                                                */
static void GetCharacter(void);

/* KeepCharacter - This puts the character passed to it into Buffer if Buffer is not yet full. */
static void KeepCharacter(void);

/*--------------------------------------------------
 There are no public global variables.
--------------------------------------------------*/



/*--------------------------------------------------
 * Public Functions Available
 --------------------------------------------------*/

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
boolean
SetInput(boolean IsFile, char *FilenameOrString,
	 Substitution_Table_Type * Replacement_Parameter_Table)
{

    uint Index;			/* Used to walk Buffer. */
    Input_Context_Type *Current_Context;	/* Used to delete the input context stack. */

    /* Eliminate any existing input context stack. */
    for (Current_Context = Input_Context; Input_Context != &Anchor_Context;
	 Current_Context = Input_Context) {
	Input_Context = Current_Context->Next_Context;

	/* Free the command line, if there is one. */
	if (Current_Context->CommandLine != NULL)
	    free(Current_Context->CommandLine);

	/* Close the input file, if there is one. */
	if (Current_Context->InputFile != NULL)
	    fclose(Current_Context->InputFile);

	/* Eliminate the empty context. */
	free(Current_Context);
    }

    /* Initialize our global variables. */
    CharactersConsumed = 0;
    CurrentState = Start;
    Current_Input_State = Normal;
    PositionInBuffer = 0;
    ExamineCurrentCharacter = FALSE;
    CharacterToExamine = ' ';
    NextCharacter = ' ';
    Replacement_Parameters = Replacement_Parameter_Table;


    Input_Context->CurrentColumn = 0;
    Input_Context->CurrentRow = 1;


    /* Null out Buffer. */
    for (Index = 0; Index < MaxIdentifierLength; Index++)
	Buffer[Index] = 0;

    if (!IsFile) {
	/* Since we have a command line, lets save it accordingly. */
	Input_Context->CommandLine = FilenameOrString;
	Input_Context->CurrentCommandLine = FilenameOrString;
	Input_Context->Error_Source = FilenameOrString;
	Input_Context->InputFile = NULL;

	/* Indicate success. */
	return TRUE;

    } else {
	/* Since we have a command file, we must open it and prepare it for use. */

	Input_Context->CommandLine = NULL;
	Input_Context->CurrentCommandLine = NULL;

	/* Open the file. */
	Input_Context->InputFile = fopen(FilenameOrString, "rt");

	/* Did we succeed? */
	if (Input_Context->InputFile) {
	    Input_Context->Error_Source = strdup(FilenameOrString);
	    if (Input_Context->Error_Source == NULL) {
		/* Report the error. */
		Report_Scanner_Error(Scanner_Out_Of_Memory, NULL, NULL, 0,
				     0);

		/* Indicate failure. */
		return FALSE;

	    }

	    return TRUE;	/* Indicate success */
	} else {

	    /* Report the error. */
	    Report_Scanner_Error(Bad_Command_File, FilenameOrString, NULL,
				 0, 0);

	    /* Indicate failure. */
	    return FALSE;

	}

    }

}


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
/*   Side Effects: May alter the following static global variables,  */
/*                 either directly or by calling other functions:    */
/*                   CurrentCommandLine                              */
/*                   CharactersConsumed                              */
/*                   CurrentRow                                      */
/*                   CurrentColumn                                   */
/*                   InputFile                                       */
/*                   Buffer                                          */
/*                   PositionInBuffer                                */
/*                   ExamineCurrentCharacter                         */
/*                   CharacterToExamine                              */
/*                   CurrentState                                    */
/*                                                                   */
/*   Notes:                                                          */
/*                                                                   */
/*********************************************************************/
TokenType *GetToken(void)
{
    /* The FSA depicted in the state table at the beginning of this file is
       simulated using a switch statement.  Each case in the switch statement
       corresponds to a state in the state table.  The CurrentState variable
       is used to indicate which state the FSA is in.                         */

    for (;;) {
	switch (CurrentState) {
	case Start:		/* The START state */

	    /* Initialize PositionInBuffer to 0 since we are beginning a new token. */
	    PositionInBuffer = 0;

	    /* Enable comment detection and elimination. */
	    Detect_Comments = TRUE;

	    /* Enable command line parameter substitution. */
	    Substitutions_Allowed = TRUE;

	    /* Get a character to examine. */
	    GetCharacter();

	    /* Are we at EOF? */
	    if (CharacterToExamine == 0) {

		/* We are out of input.  Go to the EndState. */
		CurrentState = EndState;
		break;

	    }

	    /* If the first character is a quote, then we will not be keeping it. */
	    if (CharacterToExamine == 0x22) {

		/* Comments can not appear in strings, so disable comment detection. */
		Detect_Comments = FALSE;

		/* Command line parameter substitution is not allowed in quoted strings. */
		Substitutions_Allowed = FALSE;

		/* Go to the IsString state. */
		CurrentState = IsString;
		break;

	    }

	    /* Since we will be keeping this character, put it in the buffer. */
	    KeepCharacter();

	    /* Process the current character according to the state table. */

	    /* Check for a space. */
	    if (CharacterToExamine == ' ') {
		CurrentState = SingleSpace;	/* Transition to the SingleSpace state. */
		break;
	    }

	    /* Check for a tab. */
	    if (CharacterToExamine == '\t') {
		CurrentState = SingleTab;	/* Transition to the SingleTab state. */
		break;
	    }

	    /* Check for a minus sign. */
	    if (CharacterToExamine == '-') {

		/* Transition to the OptionCheck state. */
		CurrentState = OptionCheck;
		break;

	    }

	    /* Check for a plus sign. */
	    if (CharacterToExamine == '+') {

		/* Transition to the PositiveNumberCheck state. */
		CurrentState = PositiveNumberCheck;
		break;

	    }

	    /* Check for a number. */
	    if ((CharacterToExamine >= '0') && (CharacterToExamine <= '9')) {

		/* Transition to the IsNumber state. */
		CurrentState = IsNumber;
		break;

	    }

	    /* Check for a path name. */
	    if (CharacterToExamine == '/') {

		/* Transition to the IsPathName state. */
		CurrentState = IsPathName;
		break;

	    }

	    /* Is the character a letter of the alphabet? */
	    if (((CharacterToExamine >= 'A') &&
		 (CharacterToExamine <= 'Z')) ||
		((CharacterToExamine >= 'a')
		 && (CharacterToExamine <= 'z'))) {

		/* Transition to the IsKeyword state. */
		CurrentState = IsKeyWord;
		break;

	    }

	    /* Check for a separator. */
	    if ((CharacterToExamine == ',')
		|| (CharacterToExamine == ':')
		|| (CharacterToExamine == '{')
		|| (CharacterToExamine == '}')
		|| (CharacterToExamine == '(')
		|| (CharacterToExamine == ')')
		|| (CharacterToExamine == '=')) {

		/* We must create and return a separator token. */
		return MakeToken(Separator);

	    }

	    /* Since we did not recognize the character, go to the error state! */
	    CurrentState = ErrorState;

	    /* Report the error. */
	    Report_Scanner_Error(Invalid_Character,
				 Input_Context->Error_Source,
				 &CharacterToExamine,
				 Input_Context->CurrentColumn,
				 Input_Context->CurrentRow);

	    break;

	case OptionCheck:

	    /* Get a character to examine. */
	    GetCharacter();

	    /* Is the character part of a number or something else? */
	    if (CharacterToExamine == '-') {

		/* Keep the character. */
		KeepCharacter();

		/* Set the state to resume the FSA at the next time this function is called. */
		CurrentState = Start;

		/* We will treat this character as a separator since it is not part of a number. */
		return MakeToken(Separator);

	    }

	    /* Since we did not recognize the character, we will give the IsNegativeNumber state a chance. */
	    ExamineCurrentCharacter = TRUE;
	    CurrentState = IsNegativeNumber;
	    break;

	case IsNegativeNumber:

	    /* Get the next character to examine. */
	    GetCharacter();

	    /* Is the current character being examined a number? */
	    if ((CharacterToExamine >= '0') && (CharacterToExamine <= '9')) {

		/* We have a negative number!  Keep the character and transition to the IsNumber state. */
		KeepCharacter();
		CurrentState = IsNumber;
		break;

	    }

	    /* Since we did not recognize this character, it can't be part of a number.  We should only
	       have a '-' in the buffer, so we will save the current character for the Start state
	       to work on while we output a Separator token for the '-' in the buffer.                     */
	    ExamineCurrentCharacter = TRUE;

	    CurrentState = Start;

	    return MakeToken(Separator);

	    break;		/* Keep the compiler happy. */

	case IsNumber:		/* The IsNumber state. */

	    /* Get the next character to examine. */
	    GetCharacter();

	    /* Is the character to examine part of a number? */
	    if ((CharacterToExamine >= '0') && (CharacterToExamine <= '9')) {

		/* Lets keep the character. */
		KeepCharacter();

		break;

	    } else {

		/* Is the character a period?  If so, we may have a real number. */
		if (CharacterToExamine == '.') {

		    /* We will keep the character and change state. */
		    KeepCharacter();

		    CurrentState = IsRealNumber;
		    break;

		} else {

		    /* Is the character a letter of the alphabet? */
		    if (((CharacterToExamine >= 'A') &&
			 (CharacterToExamine <= 'Z')) ||
			((CharacterToExamine >= 'a') &&
			 (CharacterToExamine <= 'z'))) {

			/* Transition to the IsKeyword state. */
			CurrentState = IsKeyWord;

		    }

		}

	    }

	    /* If CurrentState is still IsNumber, then we did not recognize the character.
	       Since we did not recognize the character, transition to the SeparatorOrError state. */
	    if (CurrentState == IsNumber)
		CurrentState = SeparatorOrError;

	    /* We want the next state to examine this token. */
	    ExamineCurrentCharacter = TRUE;

	    /* According to the FSA table at the beginning of this file, we must output a token. */
	    return MakeToken(Number);

	    break;

	case IsRealNumber:

	    /* Get the next character to examine. */
	    GetCharacter();

	    /* Is the character to examine part of a number? */
	    if ((CharacterToExamine >= '0') && (CharacterToExamine <= '9')) {

		/* Lets keep the character. */
		KeepCharacter();

		break;

	    }

	    /* Is the character a letter of the alphabet? */
	    if (((CharacterToExamine >= 'A') &&
		 (CharacterToExamine <= 'Z')) ||
		((CharacterToExamine >= 'a')
		 && (CharacterToExamine <= 'z'))) {

		/* Transition to the IsKeyword state. */
		CurrentState = IsKeyWord;

	    } else
		/* Since we did not recognize the character, transition to the SeparatorOrError state. */
		CurrentState = SeparatorOrError;

	    /* We want the next state to examine this token. */
	    ExamineCurrentCharacter = TRUE;

	    /* According to the FSA table at the beginning of this file, we must output a token. */
	    return MakeToken(RealNumber);

	    break;

	case SeparatorOrError:

	    /* Get the next character to examine. */
	    GetCharacter();

	    /* Check for a separator. */
	    if ((CharacterToExamine == ',')
		|| (CharacterToExamine == ':')
		|| (CharacterToExamine == '{')
		|| (CharacterToExamine == '}')
		|| (CharacterToExamine == '(')
		|| (CharacterToExamine == ')')
		|| (CharacterToExamine == '=')) {

		/* We will keep the character and return to the start state after creating a token. */
		KeepCharacter();

		CurrentState = Start;

		/* We must create and return a separator token. */
		return MakeToken(Separator);

	    }

	    /* Check for a space. */
	    if (CharacterToExamine == ' ') {

		/* Go to the SingleSpace state. */
		CurrentState = SingleSpace;

		/* Keep the current character. */
		KeepCharacter();

		break;

	    }

	    /* Check for a tab character. */
	    if (CharacterToExamine == '\t') {

		/* Go to the SingleTab state. */
		CurrentState = SingleTab;

		/* Keep the current character. */
		KeepCharacter();

		break;

	    }

	    /* Check for EOF */
	    if (CharacterToExamine == 0) {

		/* Go to the end state. */
		CurrentState = EndState;

		break;

	    }

	    /* Since we did not recognize the character, report the error and go to the error state! */
	    KeepCharacter();

	    Report_Scanner_Error(Invalid_Character,
				 Input_Context->Error_Source,
				 &CharacterToExamine,
				 Input_Context->CurrentColumn,
				 Input_Context->CurrentRow);

	    CurrentState = ErrorState;
	    break;

	case PositiveNumberCheck:

	    /* Get the next character to examine. */
	    GetCharacter();

	    /* Is the character to examine part of a number? */
	    if ((CharacterToExamine >= '0') && (CharacterToExamine <= '9')) {

		/* Lets keep the character. */
		KeepCharacter();

		/* Lets go to the IsNumber state. */
		CurrentState = IsNumber;

		break;

	    }

	    /* Since we did not recognize the character, report the error and go to the error state! */
	    KeepCharacter();

	    Report_Scanner_Error(Invalid_Character,
				 Input_Context->Error_Source,
				 &CharacterToExamine,
				 Input_Context->CurrentColumn,
				 Input_Context->CurrentRow);

	    CurrentState = ErrorState;
	    break;

	case IsPathName:

	    /* Get the next character to examine. */
	    GetCharacter();

	    /* Is the character a letter of the alphabet, a number, or '_' or '-'? */
	    if ((CharacterToExamine == '/') ||
		((CharacterToExamine >= 'A') &&
		 (CharacterToExamine <= 'Z')) ||
		((CharacterToExamine >= 'a') &&
		 (CharacterToExamine <= 'z')) ||
		((CharacterToExamine >= '0') &&
		 (CharacterToExamine <= '9')) ||
		(CharacterToExamine == '_') ||
		(CharacterToExamine == '-') || (CharacterToExamine == '.'))
	    {

		/* Lets keep it. */
		KeepCharacter();

		break;

	    }

	    /* Since we did not recognize the character, transition to the SeparatorOrError state. */
	    CurrentState = SeparatorOrError;

	    /* We want the SeparatorOrError state to examine this token. */
	    ExamineCurrentCharacter = TRUE;

	    /* According to the FSA table at the beginning of this file, we must output a token. */
	    return MakeToken(String);

	    break;

	case IsKeyWord:

	    /* Get the next character to examine. */
	    GetCharacter();

	    /* Is the character a letter of the alphabet, a number, or '_' or '-'? */
	    if (((CharacterToExamine >= 'A') &&
		 (CharacterToExamine <= 'Z')) ||
		((CharacterToExamine >= 'a') &&
		 (CharacterToExamine <= 'z')) ||
		((CharacterToExamine >= '0') &&
		 (CharacterToExamine <= '9')) ||
		(CharacterToExamine == '_') ||
		(CharacterToExamine == '-') ||
		(CharacterToExamine == '.') || (CharacterToExamine == '#'))
	    {

		/* Lets keep it. */
		KeepCharacter();

		break;

	    }

	    /* Is the character a '/'?  If so, transition to the IsPathName state. */
	    if (CharacterToExamine == '/') {

		CurrentState = IsPathName;
		KeepCharacter();

		break;
	    }

	    /* Since we did not recognize the character, transition to the SeparatorOrError state. */
	    CurrentState = SeparatorOrError;

	    /* We want the SeparatorOrError state to examine this token. */
	    ExamineCurrentCharacter = TRUE;

	    /* According to the FSA table at the beginning of this file, we must output a token. */
	    return MakeToken(KeyWord);

	    break;

	case EndOfString:


	    /* Get the next character to examine. */
	    GetCharacter();

	    /* Is the character the ending quote mark for the string?  If it is, then the Quote that
	       caused this state to be invoked is an embedded quote.  We will keep this quote and
	       return to the IsString state to continue looking for the end of the string.             */
	    if (CharacterToExamine == 0x22) {

		KeepCharacter();
		CurrentState = IsString;

	    } else {

		/* We have found the end of the string.  The next state is the start state. */
		CurrentState = Start;

		/* We want the current character to be returned on the next GetCharacter call so that it
		   can be examined by the Start state.                                                   */
		ExamineCurrentCharacter = TRUE;

		/* Output the token. */
		return MakeToken(String);

	    }

	    break;

	case IsString:

	    /* Get the next character to examine. */
	    GetCharacter();

	    /* Is the character the ending quote mark for the string?  Are we at EOF? */
	    if ((CharacterToExamine == 0x22) || (CharacterToExamine == 0)) {

		/* We may have found the end of the string. */
		CurrentState = EndOfString;

	    } else {

		/* Keep the character and return to this state. */
		KeepCharacter();

	    }

	    break;

	case SingleSpace:

	    /* Get the next character to examine. */
	    GetCharacter();

	    /* Is the character a space? */
	    if (CharacterToExamine == ' ') {

		/* Keep the character and go to the multi-space state. */
		KeepCharacter();

		CurrentState = IsMultiSpace;

		break;

	    }

	    /* Since the following character was not a space, set up to output a token and
	       resume the FSA at the start state with the current character.                */
	    ExamineCurrentCharacter = TRUE;
	    CurrentState = Start;

	    return MakeToken(Space);

	    break;		/* Keep the compiler happy. */

	case IsMultiSpace:

	    /* Get the next character to examine. */
	    GetCharacter();

	    /* Is the character a space? */
	    if (CharacterToExamine == ' ') {

		/* Keep the character and stay in this state. */
		KeepCharacter();

		break;

	    }

	    /* Since the following character was not a space, set up to output a token and
	       resume the FSA at the start state with the current character.                */
	    ExamineCurrentCharacter = TRUE;
	    CurrentState = Start;

	    return MakeToken(MultiSpace);

	    break;		/* Keep the compiler happy. */

	case SingleTab:

	    /* Get the next character to examine. */
	    GetCharacter();

	    /* Is the character a tab? */
	    if (CharacterToExamine == '\t') {

		/* Keep the character and go to the IsMultiTab state. */
		KeepCharacter();

		CurrentState = IsMultiTab;

		break;

	    }

	    /* Since the following character was not a tab, set up to output a token and
	       resume the FSA at the start state with the current character.                */
	    ExamineCurrentCharacter = TRUE;
	    CurrentState = Start;

	    return MakeToken(Tab);

	    break;		/* Keep the compiler happy. */

	case IsMultiTab:

	    /* Get the next character to examine. */
	    GetCharacter();

	    /* Is the character a tab? */
	    if (CharacterToExamine == '\t') {

		/* Keep the character and stay in this state. */
		KeepCharacter();

		break;

	    }

	    /* Since the following character was not a tab, set up to output a token and
	       resume the FSA at the start state with the current character.                */
	    ExamineCurrentCharacter = TRUE;
	    CurrentState = Start;

	    return MakeToken(MultiTab);

	    break;		/* Keep the compiler happy. */

	case EndState:		/* The "END" state. */

	    /* Cleanup */
	    if (Input_Context->InputFile != NULL) {

		/* Since we were getting our input from a command file, close it. */
		fclose(Input_Context->InputFile);
		Input_Context->InputFile = NULL;

	    }

	    /* Make an EOF token. */
	    return MakeToken(EofToken);

	case ErrorState:	/* The "ERROR" state. */

	    /* Set up to go to the end state. */
	    CurrentState = EndState;

	    /* Now lets make an error token. */
	    return MakeToken(InvalidCharacter);

	    break;

	}			/* switch */

    }				/* for */

}


/*--------------------------------------------------
 * Local Functions Available
 --------------------------------------------------*/


/*********************************************************************/
/*                                                                   */
/*   Function Name: MakeToken                                        */
/*                                                                   */
/*   Descriptive Name: Creates a token from the contents of the scan */
/*                     buffer and returns it as the function result. */
/*                                                                   */
/*   Input: TokenTypes Characterization - The characterization to    */
/*                                        assign to the token being  */
/*                                        made.                      */
/*                                                                   */
/*   Output: If Success : The function return value will be non-NULL,*/
/*                        and it will be a pointer to a new token.   */
/*                                                                   */
/*           If Failure : The function return value will be NULL.    */
/*                                                                   */
/*   Error Handling: If an error occurs, all memory allocated by this*/
/*                   function is freed, and NULL is returned as      */
/*                   the function return value.                      */
/*                                                                   */
/*   Side Effects:  Each position in buffer that contained a         */
/*                  character used in the token is set to NULL.      */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static TokenType *MakeToken(TokenCharacterizations Characterization)
{
    uint Index;			/* Used as an index when stepping through the Buffer. */
    TokenType *New_Token;	/* The return value. */


    /* Do we have the memory for our return value? */
    New_Token = (TokenType *) malloc(sizeof(TokenType));
    if (New_Token == NULL) {

	Report_Scanner_Error(Scanner_Out_Of_Memory,
			     Input_Context->Error_Source,
			     &CharacterToExamine,
			     Input_Context->CurrentColumn,
			     Input_Context->CurrentRow);

	return NULL;

    }

    /* To make a token, we must allocate memory for the contents of
       Buffer and then copy the contents of Buffer.  Once this has been
       done, we need to set the Characterization, TokenRow, TokenColumn,
       and TokenLength fields.                                           */

    if (PositionInBuffer > 0) {
	/* Allocate memory. */
	New_Token->TokenText = (char *) malloc(PositionInBuffer + 1);
	if (New_Token->TokenText == NULL) {
	    /* Malloc failed!  We must be out of memory. */
	    Report_Scanner_Error(Scanner_Out_Of_Memory,
				 Input_Context->Error_Source,
				 &CharacterToExamine,
				 Input_Context->CurrentColumn,
				 Input_Context->CurrentRow);
	    free(New_Token);
	    return NULL;
	}

	/* Copy the contents of Buffer. */
	for (Index = 0; Index < PositionInBuffer; Index++) {
	    New_Token->TokenText[Index] = Buffer[Index];
	    Buffer[Index] = 0;
	}

	/* Make sure that the string we copied from Buffer is NULL terminated. */
	New_Token->TokenText[PositionInBuffer] = 0;

    } else			/* Buffer is empty */
	New_Token->TokenText = NULL;

    /* Characterize the token. */
    New_Token->Characterization = Characterization;

    /* Save the token length and position. */
    New_Token->TokenRow = Input_Context->CurrentRow;
    New_Token->TokenColumn = Input_Context->CurrentColumn;
    New_Token->TokenLength = PositionInBuffer;
    New_Token->In_Lookahead_Queue = FALSE;

    /* Reset the position at which characters will be added to the buffer. */
    PositionInBuffer = 0;

    /* Indicate success! */
    return New_Token;

}


/* The following function gets a character using the current context.  If the
   current context uses a file as its input source, then this function updates
   the CurrentColumn and CurrentRow variables.  If the current context uses
   a buffer as input and the next context is the anchor context, then this
   function updates the CurrentColumn and CurrentRow variables.  CharactersConsumed
   is updated at all times.
   NOTE:  If a NULL is encountered in a file or input buffer, it will be treated
          as EOF and NULL will be returned.
   NOTE:  If an I/O error is encountered while attempting to access an input file,
          this will be treated as EOF and NULL will be returned.       
   NOTE:  Whenever this function returnes NULL, whether due to an error or an EOF,
          all contexts other than the anchor context will be closed out -- i.e.
          contexts where the input is a file will have the file closed, contexts
          where input is from a buffer will have the buffer deallocated, the
          context records themselves will be deallocated.
   NOTE:  If the current context has no more characters available, but other contexts
          exist on the context stack, then the current context will be closed and
          the next available context on the context stack will become the current
          context and be used as the source for the next character.       
   NOTE:  The character returned by this function is returned in the NextCharacter
          global variable.                                                                
   NOTE:  Carriage Returns are ignored and not returned.                                */
static void Read_Character(void)
{

    Input_Context_Type *New_Context = NULL;
    boolean Context_Change = FALSE;

    /* Get the next character taking into account any input context changes necessary. */
    do {

	Context_Change = FALSE;

	/* Are we reading from a file or from the command line? */
	if (Input_Context->CommandLine != NULL) {

	    /* We must get the next character from the command line. */
	    NextCharacter = *Input_Context->CurrentCommandLine;

	    if (NextCharacter != 0)
		Input_Context->CurrentCommandLine++;
	    else
		Context_Change = TRUE;

	} else {

	    /* Is there an open file for us to deal with? */
	    if (Input_Context->InputFile != NULL) {

		/* We are reading from a file.  Get the next character from the file. */
		if ((fread(&NextCharacter, 1, 1, Input_Context->InputFile)
		     != 1) || (NextCharacter == 0)) {

		    /* There was a problem!  We have reached the end of the input file or an I/O error perhaps. */
		    fclose(Input_Context->InputFile);

		    /* Clear out InputFile so that future calls to this function will return NULL. */
		    Input_Context->InputFile = NULL;

		    /* Set NextCharacter to EOF. */
		    NextCharacter = 0;

		    /* See if another input context is available. */
		    Context_Change = TRUE;

		}

	    } else
		Context_Change = TRUE;

	}

	/* Do we need to change contexts? */
	if (Context_Change) {

	    /* Is there another context available? */
	    if (Input_Context != &Anchor_Context) {

		New_Context = Input_Context->Next_Context;
		free(Input_Context);
		Input_Context = New_Context;

	    } else {
		NextCharacter = 0;
		Context_Change = FALSE;
	    }

	} else {
	    /* Before we return a character, we adjust the CurrentRow and CurrentColumn variables.  If CharacterToExamine is a line feed
	       or carriage return/line feed pair, then we must adjust CurrentRow and CurrentColumn accordingly.  If it is anything else,
	       then we just adjust CurrentColumn.                                                                                         */
	    if (NextCharacter == '\n') {

		Input_Context->CurrentColumn = 0;
		Input_Context->CurrentRow++;

		/* Fake a context change to get another character to return as we don't want to return this one. */
		Context_Change = TRUE;

	    } else
		Input_Context->CurrentColumn++;

	    CharactersConsumed++;

	}

    }
    while (Context_Change);

    return;

}




/* The following function gets a character from the command line or the input file taking into account comments and substitutions. */
static void GetCharacter(void)
/* State machine for GetCharacter:

  State                Action           Transition Condition            New State               Output
  ----------------------------------------------------------------------------------------------------------------
  Normal               Read Character                                   Normal2
                                        
  Normal2           
                                        Detect_Comments == TRUE  &&
                                        NextCharacter = '/'        =>   Start Comment Check
                                        
                                        Substitutions_Allowed == TRUE &&
                                        NextCharacter == '$'       =>   Start Substitution 
                                        
                                        EOF                        =>   End State
                                        
                                                                   =>   Normal                  CharacterToExamine = NextCharacter
                                        
                                        
  End State                                                        =>   End State               CharacterToExamine = 0                                        
                                        
  Start Comment Check  Read Character   NextCharacter      == '*'  =>   Comment Count
                                        NextCharacter      != '*'  =>   Not Comment 
                                        EOF                        =>   End State                        
                                        
  Not Comment                                                      =>   Normal2                 CharacterToExamine = '/'
  
  Comment Count        NestingLevel += 1                           =>   Comment Found                                                                                   
                                                                                     
  Comment Found        Read Character                              =>   Comment Found2
  
  Comment Found2                        NextCharacter == '*'       =>   Find End Of Comment
                                        NextCharacter == '/'       =>   Embedded Comment Check
                                        NextCharacter != '*' &&
                                          NextCharacter != '/'     =>   Comment Found
                                        EOF                        =>   End State
                                        
  Embedded Comment Check Read Character NextCharacter == '*'       =>   Comment Count
                                        NextCharacter != '*'       =>   Comment Found                                        
                                        
  Find End Of Comment Read Character    NextCharacter == '/'       =>   Exit Comment
                                        NextCharacter != '/"       =>   Comment Found2
                                        EOF                        =>   End State   
                                        
  Exit Comment        NestingLevel -= 1 NestingLevel == 0          =>   Normal
                                        NestingLevel > 0           =>   Comment Found                                          
                                        
  Start Substitution  Read Character    NextCharacter == '('       =>   Substitution Found
                                        NextCharacter != '('       =>   Not Substitution  
                                        EOF                        =>   Not Substitution        NextCharacter = 0
                                        
  Not Substitution                                                 =>   Normal2                 CharacterToExamine = '$'
  
  Substitution Found  Replacement ID = 0                           =>   Skip Initial Spaces
  
  Skip Initial Spaces Read Character    NextCharacter == ' '       =>   Skip Initial Spaces
                                        NextCharacter == '0' ..'9' =>   Get Replacement ID
                                        else                       =>   End State               CharacterToExamine = 0
                                                                                                Output error message
                                                                                                
  Get Replacement ID  Replacement ID *= 10
                      Replacement ID += NextCharacter - '0'
                      Read Character    
                                        NextCharacter == ' '       =>   Skip End Spaces
                                        NextCharacter == '0' ..'9' =>   Get Replacement ID
                                        NextCharacter == ')'       =>   Do Substitution
                                        else                       =>   End State               CharacterToExamine = 0
                                                                                                Output error message
                                        
  Skip End Spaces     Read Character    NextCharacter == ' '       =>   Skip End Spaces
                                        NextCharacter == ')'       =>   Do Substitution
                                        else                       =>   End State               CharacterToExamine = 0
                                                                                                Output error message
                                                                                
  Do Substitution     Find Replacement String
                      Create Context Record
                      make new context record the current context record
                                                                   =>   Normal
                                                                   
-------------------------------------------------------------------------------------------------------------------------------

Notes:  Read Character will check for additional contexts when it encounters an EOF condition.  If additional contexts exist,
        it will remove the current context from the context stack and make the next context on top of the context stack
        the current context.                                                                                                                                                    
*/
{

    uint Substitution_Number = 0;	/* Initialize this variable to keep the compiler happy. */
    Input_Context_Type *New_Context = NULL;	/* Initialize this variable to keep the compiler happy. */
    uint Nesting_Level = 0;	/* Used to track nested comments. */

    /* Should we return the currently loaded character or get a new one? */
    if (ExamineCurrentCharacter)
	Current_Input_State = Normal2;

    for (;;) {
	switch (Current_Input_State) {
	case Normal:
	    Read_Character();
	case Normal2:
	    /* Are we looking for comments?  If so, could NextCharacter be the start of a comment? */
	    if (Detect_Comments && (NextCharacter == '/')) {
		Current_Input_State = Start_Comment_Check;
		ExamineCurrentCharacter = FALSE;
		break;
	    }

	    /* Are we looking for a substitution command?  If so, could NextCharacter be the start of a substitution command? */
	    if (Substitutions_Allowed && (NextCharacter == '$')) {
		Current_Input_State = Start_Substitution;
		ExamineCurrentCharacter = FALSE;
		break;
	    }

	    if (NextCharacter == 0) {
		Current_Input_State = End_State;
		ExamineCurrentCharacter = FALSE;
		break;
	    }

	    if (ExamineCurrentCharacter) {
		ExamineCurrentCharacter = FALSE;
		if (CharacterInjected) {
		    CharacterInjected = FALSE;
		    Current_Input_State = Normal2;
		} else
		    Current_Input_State = Normal;

	    } else {
		CharacterToExamine = NextCharacter;
		Current_Input_State = Normal;
		CharacterInjected = FALSE;
	    }

	    return;
	case End_State:
	    CharacterToExamine = 0;
	    return;
	case Comment_Found:
	    Read_Character();
	case Comment_Found2:
	    Current_Input_State = Comment_Found;
	    if (NextCharacter == '/')
		Current_Input_State = Embedded_Comment_Check;
	    if (NextCharacter == '*')
		Current_Input_State = Find_End_Of_Comment;
	    if (NextCharacter == 0)
		Current_Input_State = End_State;
	    break;
	case Not_Comment:
	    CharacterToExamine = '/';
	    CharacterInjected = TRUE;
	    Current_Input_State = Normal2;
	    return;
	case Start_Comment_Check:
	    Read_Character();
	    /* Assume we don't have a comment. */
	    Current_Input_State = Not_Comment;

	    /* Change target state if we do have a comment. */
	    if (NextCharacter == '*') {
		Current_Input_State = Comment_Found;
		Nesting_Level++;
	    }

	    /* Do we have EOF? */
	    if (NextCharacter == 0)
		Current_Input_State = End_State;

	    break;
	case Embedded_Comment_Check:
	    Read_Character();
	    Current_Input_State = Comment_Found;

	    if (NextCharacter == '*')
		Nesting_Level++;

	    /* Do we have EOF? */
	    if (NextCharacter == 0)
		Current_Input_State = End_State;

	    break;
	case Find_End_Of_Comment:
	    Read_Character();

	    /* Assume that we do not have the end of a comment. */
	    Current_Input_State = Comment_Found2;

	    /* Test for end of comment. */
	    if (NextCharacter == '/') {
		Nesting_Level--;
		if (Nesting_Level == 0)
		    Current_Input_State = Normal;
	    }

	    /* Test for EOF */
	    if (NextCharacter == 0)
		Current_Input_State = End_State;

	    break;
	case Start_Substitution:
	    Read_Character();

	    Current_Input_State = Not_Substitution;

	    if (NextCharacter == '(')
		Current_Input_State = Substitution_Found;

	    if (NextCharacter == 0)
		Current_Input_State = End_State;

	    break;
	case Not_Substitution:
	    CharacterToExamine = '$';
	    CharacterInjected = TRUE;
	    Current_Input_State = Normal2;
	    return;
	case Substitution_Found:
	    Substitution_Number = 0;
	    Current_Input_State = Skip_Initial_Spaces;
	    break;
	case Skip_Initial_Spaces:
	    Read_Character();
	    if (NextCharacter == ' ')
		break;
	    if ((NextCharacter >= '0') && (NextCharacter <= '9')) {
		Current_Input_State = Get_Replacement_ID;
		break;
	    }
	    if (NextCharacter == 0) {
		Current_Input_State = End_State;
		break;
	    }
	    Report_Scanner_Error(Invalid_Character_In_Substitution,
				 &NextCharacter, NULL,
				 Input_Context->CurrentColumn,
				 Input_Context->CurrentRow);
	    Current_Input_State = End_State;
	    break;
	case Get_Replacement_ID:
	    Substitution_Number *= 10;
	    Substitution_Number += (NextCharacter - '0');
	    Read_Character();
	    if (NextCharacter == ' ') {
		Current_Input_State = Skip_End_Spaces;
		break;
	    }
	    if (NextCharacter == ')') {
		Current_Input_State = Do_Substitution;
		break;
	    }
	    if ((NextCharacter >= '0') && (NextCharacter <= '9'))
		break;
	    if (NextCharacter == 0) {
		Current_Input_State = End_State;
		break;
	    }
	    Report_Scanner_Error(Invalid_Character_In_Substitution,
				 &NextCharacter, NULL,
				 Input_Context->CurrentColumn,
				 Input_Context->CurrentRow);
	    Current_Input_State = End_State;
	    break;
	case Skip_End_Spaces:
	    Read_Character();
	    if (NextCharacter == ')') {
		Current_Input_State = Do_Substitution;
		break;
	    }
	    if (NextCharacter == ' ')
		break;

	    Report_Scanner_Error(Invalid_Character_In_Substitution,
				 &NextCharacter, NULL,
				 Input_Context->CurrentColumn,
				 Input_Context->CurrentRow);
	    Current_Input_State = End_State;
	    break;
	case Do_Substitution:
	    Current_Input_State = Normal;
	    if ((Substitution_Number > 0)
		&& (Replacement_Parameters != NULL)
		&& (Substitution_Number <= Replacement_Parameters->count)) {

		/* Is the substitution a file or a buffer? */
		if (Replacement_Parameters->
		    Replacement_Parameters[Substitution_Number -
					   1].Is_Filename) {

		    /* Allocate an Input_Context_Type record. */
		    New_Context = (Input_Context_Type *)
			malloc(sizeof(Input_Context_Type));
		    if (New_Context == NULL) {
			/* Out of memory */
			Report_Scanner_Error(Scanner_Out_Of_Memory, Buffer,
					     &CharacterToExamine, 0, 0);
			Current_Input_State = End_State;
			continue;
		    }

		    /* Since we have a command file, we must open it and prepare it for use. */

		    New_Context->CommandLine = NULL;
		    New_Context->CurrentCommandLine = NULL;

		    /* Open the file. */
		    New_Context->InputFile =
			fopen(Replacement_Parameters->
			      Replacement_Parameters[Substitution_Number -
						     1].Text, "rt");

		    /* Did we succeed? */
		    if (New_Context->InputFile) {
			New_Context->Error_Source =
			    strdup(Replacement_Parameters->
				   Replacement_Parameters
				   [Substitution_Number - 1].Text);
			if (New_Context->Error_Source == NULL) {
			    /* Report the error. */
			    Report_Scanner_Error(Scanner_Out_Of_Memory,
						 NULL, NULL, 0, 0);

			    /* Indicate failure. */
			    Current_Input_State = End_State;
			    continue;
			}

			New_Context->Next_Context = Input_Context;
			New_Context->CurrentColumn = 0;
			New_Context->CurrentRow = 1;
			Input_Context = New_Context;

		    }

		} else {

		    /* Is this an infinite recursion situation? */
		    if (Replacement_Parameters->
			Replacement_Parameters[Substitution_Number -
					       1].Text !=
			Input_Context->CommandLine) {

			New_Context = (Input_Context_Type *)
			    malloc(sizeof(Input_Context_Type));
			if (New_Context != NULL) {
			    New_Context->Next_Context = Input_Context;
			    Input_Context = New_Context;
			    New_Context->CommandLine =
				Replacement_Parameters->
				Replacement_Parameters[Substitution_Number
						       - 1].Text;
			    New_Context->CurrentCommandLine =
				New_Context->CommandLine;
			    New_Context->CurrentColumn = 0;
			    New_Context->CurrentRow = 1;
			    New_Context->Error_Source =
				New_Context->CommandLine;
			} else {
			    /* Out of memory */
			    Report_Scanner_Error(Scanner_Out_Of_Memory,
						 Buffer,
						 &CharacterToExamine, 0,
						 0);
			    Current_Input_State = End_State;
			}

		    } else {
			/* Illegal substitution command. */
			Report_Scanner_Error(Invalid_Substitution,
					     &NextCharacter, NULL,
					     Input_Context->CurrentColumn,
					     Input_Context->CurrentRow);
			Current_Input_State = End_State;
		    }

		}

	    } else {
		/* Illegal substitution command. */
		Report_Scanner_Error(Invalid_Substitution, &NextCharacter,
				     NULL, Input_Context->CurrentColumn,
				     Input_Context->CurrentRow);
		Current_Input_State = End_State;
	    }

	    break;
	default:		/* Internal Error! */
	    abort();
	    break;
	}

    }

}

/* KeepCharacter - This puts the character passed to it into Buffer if Buffer is not yet full. */
static void KeepCharacter(void)
{

    /* Do we have room in the Buffer? */
    if (PositionInBuffer < MaxIdentifierLength) {

	/* Place the character into the buffer. */
	Buffer[PositionInBuffer] = CharacterToExamine;
	PositionInBuffer++;

    }

    return;

}
