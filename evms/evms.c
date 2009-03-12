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
 * Module: evms.c
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
#define EVMS_C

/*--------------------------------------------------
 * Necessary include files
 --------------------------------------------------*/
#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/ioctl.h>
#include <string.h>		/* strcat, strcpy */
#include <ctype.h>		/* toupper */
#include <stdio.h>		/* printf */
#include <stdlib.h>		/* strtol */
#include <termios.h>
#include <stdarg.h>
#include <unistd.h>

#ifdef USE_READLINE
#include <readline/readline.h>	/* readline */
#include <readline/history.h>	/* add_history */
#endif


#include "dlist.h"		/* dlist_t */
#include "evms.h"
#include "error.h"		/* ReportError */
#include "token.h"
#include "scanner.h"		/* SetInput */
#include "parser.h"		/* Parse */
#include "interpreter.h"
#include "frontend.h"

#ifdef DEBUG_PARSER

#include "screener.h"

#endif

#include "../Protocol.h"
#include "SendOneFrame.h"


/*--------------------------------------------------
 * Global Variables
 --------------------------------------------------*/
boolean Batch = FALSE, Interactive = TRUE;	/* Controls whether or not EVMS will prompt the user for commands.
						   If it is FALSE, then input is expected from a file.             */
ui_callbacks_t Callback_Functions;	/* Used to pass call back functions to the EVMS Engine.  These
					   functions allow the EVMS Engine to provide status and error messages
					   for display to the user during complicated operations.               */

Substitution_Table_Entry Substitution_Value;
Substitution_Table_Type Substitution_Table;

boolean ParseOnly = FALSE;	/* Controls whether or not the output of the parser is set to the
				   interpreter for execution.                                      */

boolean Continuous = TRUE;	/* Flag to control how many commands will be prompted for.
				   If the flag is FALSE, then we will only prompt once for a
				   command.  If TRUE, then we will continuously prompt for a
				   command until the command line returned is NULL.            */

/*--------------------------------------------------
 * Private Constants
 --------------------------------------------------*/
#define VERSION_NUMBER			VERSION
#define REQUIRED_ENGINE_VERSION_MAJOR   10
#define REQUIRED_ENGINE_VERSION_MINOR   0
#define REQUIRED_ENGINE_VERSION_PATCH   0
#define REPLACEMENT_PARAMETER           73



/*--------------------------------------------------
 * There are No Private Type Definitions
 --------------------------------------------------*/




/*--------------------------------------------------
  Private Global Variables.
--------------------------------------------------*/
static char CommandLine[MAX_CONTENT_LEN];	/* Buffer to store the reconstructed command line in. */
#ifdef DEBUG_PARSER

char *IntegerDataStr = "INTEGER DATA";
char *RealNumberDataStr = "REAL NUMBER DATA";
char *StringDataStr = "STRING DATA";
char *FilterStr = "FILTER";
char *FeatureNameStr = "FEATURE NAME";
char *PluginNameStr = "PLUG-IN NAME";
#endif



/*--------------------------------------------------
 Local Function Prototypes
--------------------------------------------------*/
static void Display_Command_Line_Help(void);
#ifndef DEBUG_PARSER
static int Display_Engine_Message(char *message_text,
				  int *answer, char **choices);
#endif
#ifdef DEBUG_PARSER
static int Map_Command_List_Entries(ADDRESS Object,
				    TAG ObjectTag,
				    uint ObjectSize,
				    ADDRESS ObjectHandle,
				    ADDRESS Parameters);
#endif
static int Init_Substitution_Table(ADDRESS Object,
				   TAG ObjectTag,
				   ADDRESS ObjectHandle,
				   ADDRESS Parameters);


/*--------------------------------------------------
 * Public Functions Available
 --------------------------------------------------*/
int My_Printf(char *format, ...);
int Indent_Printf(int indent_level, boolean indent_first_line,
		  char *format, ...);
int Row_Delimiter(char ch, char *header);


int g_sock_fd = -1;

/* Controls the level of debugging information output to the user.*/
debug_level_t Debug_Level = -1;

/* The mode in which the Engine should be opened. */
engine_mode_t Engine_Mode = ENGINE_READWRITE;

/* Cluster nodename to administer or NULL for local */
char *Node_Name = NULL;

/* Has verbose mode been specified? */
boolean Verbose = FALSE;


/*********************************************************************/
/*                                                                   */
/*   Function Name: main                                             */
/*                                                                   */
/*   Descriptive Name:  The entry point for the EVMS.EXE program.    */
/*                                                                   */
/*   Input: int argc - The number of parameters on the command line. */
/*          char * argv - An array of pointers to the text of each   */
/*                        parameter found on the command line.       */
/*                                                                   */
/*   Output: If Success : The program's exit code will be 0.         */
/*                                                                   */
/*           If Failure : The program's exit code will be 1.         */
/*                                                                   */
/*   Error Handling: If an error occurs, all memory allocated by this*/
/*                   program is freed and an error message is output */
/*                   to the user.                                    */
/*                                                                   */
/*   Side Effects: The state of the disks, partitions, and/or volumes*/
/*                 in the system may be altered.  Nodes may be       */
/*                 created or removed from the /dev tree.            */
/*                                                                   */
/*   Notes:                                                          */
/*                                                                   */
/*********************************************************************/
int evms_init(int argc, char *argv[], int sock_fd)
{
#ifndef DEBUG_PARSER
    evms_version_t Engine_Version;	/* Used to check the version of the EVMS Engine to ensure we have a version we can work with. */
#endif
    uint ParameterIndex;	/* Used to access the item in argv being processed. */
    dlist_t Replacement_Parms;	/* This list holds all command line parameters which are not recognized by the interpreter.
				   These parameters can be referenced from EVMS CLI commands using $(x) notation, where x
				   is the number of the parameter (i.e. - first unrecognized parameter, second unrecognized
				   parameter, etc. etc. ).                                                                   */
    ADDRESS Not_Needed = NULL;
    int ReturnValue = 0;	/* The value to return.  A value is returned in case this
				   program was called from a batch file.  A return value
				   of 0 indicates successful program completion while
				   any non-zero value is a program defined error code.
				   We will assume success and initialize ReturnValue to 0. */
    int Index = 0;		/* Used when converting strings to uppercase. */
    int Length = 0;		/* Used when converting strings to uppercase. */
    char *Token;		/* Used for parameter processing. */
    boolean RL_Found = FALSE;	/* Set to TRUE if all remaining command line parameters are replacement parameters. */
    boolean R_Found = FALSE;	/* Set to TRUE if only the next command line parameter is a replacement parameter. */
    int InputIsFile = 0;	/* If > 0, then input is coming from a command file. */
#ifdef USE_READLINE
    char *line = NULL;		/* Line returned from readline */
#endif

#ifdef DEBUG_PARSER
    uint Indentation_Level = 0;	/* Used when displaying command nodes produced by the parser. */
    boolean No_Commits = FALSE;	/* If TRUE, then don't perform commits after each command. */

#endif

    // Store socket descripter used in this procedure, wwliu, 2008,8,7
    g_sock_fd = sock_fd;

    /* Process command line parameters */
    if (argc > 1) {

	/* make sure CLI output is unbuffered, e.g. '... choice: ' doesn't
	   get flushed if no controlling terminal exists, e.g. interacting
	   with the CLI over sockets 
	 */
	setvbuf(stdout, NULL, _IONBF, 0);

	My_Printf("\nEVMS Command Line Interpreter Version %s\n\n",
		  VERSION_NUMBER);

	/* Initialize the Replacement_Parms list. */
	Replacement_Parms = CreateList();

	if (Replacement_Parms == NULL) {
	    Report_Standard_Error(ENOMEM);
	    return (ENOMEM);
	}


	for (ParameterIndex = 1; ParameterIndex < argc; ParameterIndex++) {

	    Token = argv[ParameterIndex];

	    if (!RL_Found) {

		if ((strcmp("-c", Token) == 0) ||
		    (strcmp("-C", Token) == 0) ||
		    (strcmp("--c", Token) == 0) ||
		    (strcmp("--C", Token) == 0)) {
		    if (No_Commits) {
			My_Printf
			    ("The -c command line option has been specified more than once.\n");
		    }

		    No_Commits = TRUE;
		    continue;

		}

		if ((strcmp("-v", Token) == 0) ||
		    (strcmp("-V", Token) == 0) ||
		    (strcmp("--V", Token) == 0) ||
		    (strcmp("--v", Token) == 0)) {
		    if (Verbose) {
			My_Printf
			    ("More than one of the verbose options (-v, -v0, -v1,-v2) has been specified.\n");
		    }

		    Verbose = TRUE;
#ifndef DEBUG_PARSER
		    Verbose_Mode = 2;
#endif
		    continue;
		}

		if ((strcmp("-v0", Token) == 0) ||
		    (strcmp("-V0", Token) == 0) ||
		    (strcmp("--V0", Token) == 0) ||
		    (strcmp("--v0", Token) == 0)) {
		    if (Verbose) {
			My_Printf
			    ("More than one of the verbose options (-v, -v0, -v1,-v2) has been specified.\n");
		    }

		    Verbose = TRUE;
#ifndef DEBUG_PARSER
		    Verbose_Mode = 0;
#endif
		    continue;
		}

		if ((strcmp("-v1", Token) == 0) ||
		    (strcmp("-V1", Token) == 0) ||
		    (strcmp("--V1", Token) == 0) ||
		    (strcmp("--v1", Token) == 0)) {
		    if (Verbose) {
			My_Printf
			    ("More than one of the verbose options (-v, -v0, -v1,-v2) has been specified.\n");
		    }

		    Verbose = TRUE;
#ifndef DEBUG_PARSER
		    Verbose_Mode = 1;
#endif
		    continue;
		}

		if ((strcmp("-v2", Token) == 0) ||
		    (strcmp("-V2", Token) == 0) ||
		    (strcmp("--V2", Token) == 0) ||
		    (strcmp("--v2", Token) == 0)) {
		    if (Verbose) {
			My_Printf
			    ("More than one of the verbose options (-v, -v0, -v1,-v2) has been specified.\n");
		    }

		    Verbose = TRUE;
#ifndef DEBUG_PARSER
		    Verbose_Mode = 2;
#endif
		    continue;
		}

		if ((strcmp("-S", Token) == 0) ||
		    (strcmp("-s", Token) == 0) ||
		    (strcmp("--S", Token) == 0) ||
		    (strcmp("--s", Token) == 0)) {

		    /* Have we seen this command line option already? */
		    if (!Continuous) {
			My_Printf
			    ("The -s command line option has been specified more than once.\n");
		    }

		    Continuous = FALSE;
		    continue;

		}

		if ((strcmp("-H", Token) == 0) ||
		    (strcmp("-h", Token) == 0) ||
		    (strcmp("--H", Token) == 0) ||
		    (strcmp("--h", Token) == 0) ||
		    (strcasecmp("-HELP", Token) == 0) ||
		    (strcasecmp("--HELP", Token) == 0)) {

		    /* We have a help request.  This takes precedence over all other command line input.
		       Output a help message and abort.                                                   */
		    Display_Command_Line_Help();

		    return (0);

		}

		if ((strcmp("-N", Token) == 0) ||
		    (strcmp("-n", Token) == 0) ||
		    (strcmp("--n", Token) == 0) ||
		    (strcmp("--N", Token) == 0)) {
		    /* We should have a cluster nodename as the next argv. */

		    if ((ParameterIndex + 1) >= argc) {

			My_Printf("Bad nodename specified.\n");

			return (ENOENT);

		    }

		    ParameterIndex++;
		    Node_Name = argv[ParameterIndex];
		    continue;

		}


		if ((strcmp("-M", Token) == 0) ||
		    (strcmp("-m", Token) == 0) ||
		    (strcmp("--M", Token) == 0) ||
		    (strcmp("--m", Token) == 0)) {
		    /* We should have an open mode as the next argv. */

		    if ((ParameterIndex + 1) >= argc) {

			My_Printf("No Engine mode specified.\n");

			return (EINVAL);

		    }

		    ParameterIndex++;

		    /* Get the debug level to use. */
		    Token = argv[ParameterIndex];
		    Length = strlen(Token);

		    /* Convert to upper case. */
		    for (Index = 0; Index < Length; Index++)
			Token[Index] = toupper(Token[Index]);

		    /* Look for a valid mode. */
		    if (strcmp("READONLY", Token) == 0)
			Engine_Mode = ENGINE_READONLY;
		    else if (strcmp("READWRITE", Token) == 0)
			Engine_Mode = ENGINE_READWRITE;
		    else if (strcmp("READWRITE_CRITICAL", Token) == 0)
			Engine_Mode = ENGINE_READWRITE_CRITICAL;
		    else if (strcmp("DAEMON", Token) == 0)
			Engine_Mode = ENGINE_DAEMON;
		    else {

			My_Printf("Open mode specified is not valid.\n");

			return (EINVAL);

		    }

		    continue;

		}

		if ((strcmp("-D", Token) == 0) ||
		    (strcmp("-d", Token) == 0) ||
		    (strcmp("--d", Token) == 0) ||
		    (strcmp("--D", Token) == 0)) {
		    /* We should have a debug level as the next argv. */

		    if ((ParameterIndex + 1) >= argc) {

			My_Printf("No debug level specified.\n");

			return (EINVAL);

		    }

		    ParameterIndex++;

		    /* Get the debug level to use. */
		    Token = argv[ParameterIndex];
		    Length = strlen(Token);

		    /* Convert to upper case. */
		    for (Index = 0; Index < Length; Index++)
			Token[Index] = toupper(Token[Index]);

		    /* Look for a valid debug level. */
		    if (strcmp("DEFAULT", Token) == 0)
			Debug_Level = DEFAULT;
		    else if (strcmp("CRITICAL", Token) == 0)
			Debug_Level = CRITICAL;
		    else if (strcmp("SERIOUS", Token) == 0)
			Debug_Level = SERIOUS;
		    else if (strcmp("ERROR", Token) == 0)
			Debug_Level = ERROR;
		    else if (strcmp("WARNING", Token) == 0)
			Debug_Level = WARNING;
		    else if (strcmp("DETAILS", Token) == 0)
			Debug_Level = DETAILS;
		    else if (strcmp("EXTRA", Token) == 0)
			Debug_Level = EXTRA;
		    else if (strcmp("ENTRY_EXIT", Token) == 0)
			Debug_Level = ENTRY_EXIT;
		    else if (strcmp("EVERYTHING", Token) == 0)
			Debug_Level = EVERYTHING;
		    else if (strcmp("DEBUG", Token) == 0)
			Debug_Level = DEBUG;
		    else {

			My_Printf("Debug level specified is not valid.\n");

			return (EINVAL);

		    }

		    continue;

		}

		if ((strcmp("-r", Token) == 0) ||
		    (strcmp("-R", Token) == 0) ||
		    (strcmp("--r", Token) == 0) ||
		    (strcmp("--R", Token) == 0)) {

		    /* There should be another parameter on the command line.  The next
		       parameter on the command line is a replacement parameter.         */
		    if ((ParameterIndex + 1) >= argc) {

			My_Printf("Replacement parameter missing.\n");

			return (EINVAL);

		    }

		    R_Found = TRUE;
		    RL_Found = TRUE;
		    continue;

		}

		if ((strcmp("-rl", Token) == 0) ||
		    (strcmp("-RL", Token) == 0) ||
		    (strcmp("--rl", Token) == 0) ||
		    (strcmp("--RL", Token) == 0)) {
		    RL_Found = TRUE;
		    continue;
		}

		/* We have an unrecognized command line parameter! */
		My_Printf
		    ("%s is not a valid command line parameter.  Use -h to get a list of the valid command line parameters.\n",
		     Token);
		return (EINVAL);

	    } else {
		Substitution_Table_Entry *Substitution_Value_ptr;
		Substitution_Value_ptr =
		    malloc(sizeof(Substitution_Table_Entry));
		if (Substitution_Value_ptr == NULL) {
		    Report_Standard_Error(ENOMEM);
		    return (ENOMEM);
		}

		/* All remaining parameters go into the Replacement Parameters List. */
		Substitution_Value.Text = Token;
		Substitution_Value.Is_Filename = FALSE;
		memcpy(Substitution_Value_ptr, &Substitution_Value,
		       sizeof(Substitution_Table_Entry));

		ReturnValue =
		    InsertObject(Replacement_Parms, Substitution_Value_ptr,
				 REPLACEMENT_PARAMETER, NULL, AppendToList,
				 TRUE, &Not_Needed);
		if (ReturnValue != 0) {
		    Report_Standard_Error(ReturnValue);
		    return (ReturnValue);
		}

		if (R_Found) {
		    R_Found = FALSE;
		    RL_Found = FALSE;
		}

	    }

	}

	/* How many items are in the Replacement_Parms list? */
	ReturnValue =
	    GetListSize(Replacement_Parms, &Substitution_Table.count);
	if (ReturnValue != DLIST_SUCCESS) {
	    Report_Standard_Error(ReturnValue);
	    return ReturnValue;
	}

	if (Substitution_Table.count > 0) {
	    Substitution_Table.Replacement_Parameters =
		(Substitution_Table_Entry *)
		malloc(sizeof(Substitution_Table_Entry) *
		       Substitution_Table.count);
	    if (Substitution_Table.Replacement_Parameters == NULL) {
		Report_Standard_Error(ENOMEM);
		return ENOMEM;
	    }

	    Substitution_Table.count = 0;
	    ReturnValue =
		ForEachItem(Replacement_Parms, Init_Substitution_Table,
			    &Substitution_Table, TRUE);
	    if (ReturnValue != DLIST_SUCCESS) {
		Report_Standard_Error(ReturnValue);
		return ReturnValue;
	    }

	}
    }

#ifndef DEBUG_PARSER

	if (!ParseOnly) {

	    /* Get the version of the EVMS Engine. */
	    evms_get_api_version(&Engine_Version);

	    /* Is this a version of the EVMS Engine we can work with? */
	    if ((Engine_Version.major != REQUIRED_ENGINE_VERSION_MAJOR) ||
		(Engine_Version.minor < REQUIRED_ENGINE_VERSION_MINOR) ||
		((Engine_Version.minor == REQUIRED_ENGINE_VERSION_MINOR) &&
		 (Engine_Version.patchlevel <
		  REQUIRED_ENGINE_VERSION_PATCH))) {
		My_Printf("Incorrect version of the EVMS Engine.  "
			  "This version of the EVMS Command Line Interpreter requires a version of the EVMS Engine which supports "
			  "the EVMS Application Programming Interface version %d.%d.%d.\n",
			  REQUIRED_ENGINE_VERSION_MAJOR,
			  REQUIRED_ENGINE_VERSION_MINOR,
			  REQUIRED_ENGINE_VERSION_PATCH);

		My_Printf
		    ("The version of the EVMS Engine currently loaded supports "
		     "the EVMS Application Programming Interface version %d.%d.%d.\n",
		     Engine_Version.major, Engine_Version.minor,
		     Engine_Version.patchlevel);

		return EPERM;
	    }

	    /* Set up the messaging functions for the EVMS Engine. */
	    Callback_Functions.user_message = Display_Engine_Message;
	    Callback_Functions.user_communication = NULL;
	    Callback_Functions.progress = NULL;
	    Callback_Functions.status = Report_Progress;

	    /* Open the EVMS Engine. */
	    ReturnValue =
		evms_open_engine(Node_Name, Engine_Mode,
				 &Callback_Functions, Debug_Level,
				 DEFAULT_LOG_FILE);

	    if (ReturnValue != 0) {	/* What should the return value be!!!!  Get the engine guys to clearly document the return codes! */
		My_Printf
		    ("Unable to open the EVMS Engine.  Engine returns code %i.\n",
		     ReturnValue);
		Report_Standard_Error(ReturnValue);
		return (ReturnValue);

	    }
	}
#endif




    return ReturnValue;
}

int evms_deinit()
{
#ifndef DEBUG_PARSER
    if (!ParseOnly)
	evms_close_engine();
#endif

    return 0;
}


int evms_func(char *cmd, int len)
{

    int ReturnValue = 0;

    /* 
     * Used to hold the list of command nodes returned by the
     * parser.  This list will be given to the Stack Machine,
     * which will then act as an interpreter for the command
     * nodes.                                                 
     */

    /* Set the input source. */
    dlist_t Command_List;

    if (!SetInput(FALSE, CommandLine, &Substitution_Table)) {

	My_Printf("Internal error.\n");

	return -1;
    }

    /* Null out the buffer so we have a clean buffer to work with. */
    memset(CommandLine, 0, sizeof(CommandLine));

    if (Verbose_Mode >= 2) {
	/* Clear status message from the screen. */
	Report_Progress("");
    }

    /* Put out our command prompt. */
    // FIXME My_Printf("EVMS: ");

    /* Get a command line from the user. */
    // FIXME fgets(CommandLine, sizeof(CommandLine), stdin);
    //
    memcpy(CommandLine, cmd, len > 4096 ? 4096 : len);

    if ((strlen(CommandLine) > 0) &&
	(CommandLine[0] != 0) && (CommandLine[0] != '\n')) {

	/* Now lets parse the command line. */
	Command_List = Parse_Command_Line();

	/* Was there a parsing error? */
	if (Command_List != NULL) {

	    if (!ParseOnly) {
		ReturnValue =
		    Interpret_Commands(Command_List, Interactive,
				       Engine_Mode, Debug_Level,
				       Node_Name);

		if (ReturnValue == TIME_TO_QUIT) {
		    ReturnValue = 0;
		}

	    }
	} else {
	    ReturnValue = -1;
	}

    } else {
	ReturnValue = -1;
    }

    return ReturnValue;
}


/*--------------------------------------------------
 * Private Functions Available
 --------------------------------------------------*/

static void Display_Command_Line_Help(void)
{

    My_Printf
	("The EVMS Command Line Interpreter is invoked using the following syntax:\n\n");

    Indent_Printf(1, FALSE,
		  "evms [-b] [-c] [-d <debug-level>] [-f <filename>] [-h] [-m <engine-mode>] "
		  "[-n <node>] [-p] [-r <parameter>] [-s] "
		  "[-v[0-2]] [-rl <parameter> ... <parameter>]\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "-b      tells the EVMS CLI to run in batch mode.  "
		  "Any time there is a prompt for input from the user, the default value should be "
		  "accepted automatically. "
		  "This is the default behavior with the -f option.\n");
    Indent_Printf(1, FALSE,
		  "-c      indicates that changes are not to be committed to disk until the "
		  "user exits the EVMS Command Line Interpreter.  "
		  "Normally changes are committed to disk after each individual command is completed, "
		  "even when there are multiple commands specified on the same command line.\n");
    Indent_Printf(1, FALSE,
		  "-d      <debug-level> sets the debug level, which controls how much information "
		  "is logged in the evmsEngine.log file located in /var/log directory.  "
		  "<debug-level> is either Critical, Serious, Error, Warning, Default, "
		  "Details, Entry_Exit, Debug, Extra, or Everything.\n");
    Indent_Printf(1, FALSE,
		  "-f      <filename> tells the EVMS Command Line Interpreter to accept input "
		  "from the file specified by <filename>.  "
		  "When all of the commands in <filename> have been processed, "
		  "the EVMS Command Line Interpreter will exit.\n");
    Indent_Printf(1, FALSE,
		  "-h      displays this help text.  "
		  "To get help on commands accepted by the EVMS Command Line Interpreter, invoke the interpreter, "
		  "enter the word 'help' at the prompt, and press enter.\n");
    Indent_Printf(1, FALSE,
		  "-m      <engine-mode> tells the EVMS Command Line Interpreter the mode in which "
		  "it should open the EVMS Engine.  "
		  "<engine-mode> is either ReadOnly, ReadWrite, or Daemon.  "
		  "Daemon mode should only be specified when using the Command Line Interpreter "
		  "to start the EVMS Engine daemon on nodes in a cluster.\n");
    Indent_Printf(1, FALSE,
		  "-n      <node> tells the EVMS Command Line Interpreter that all "
		  "commands are intended for the given node name in a cluster.\n");
    Indent_Printf(1, FALSE,
		  "-p      tells the EVMS Command Line Interpreter to parse commands only.  "
		  "Errors found during parsing will be reported.  "
		  "Commands will NOT be executed.\n");
    Indent_Printf(1, FALSE,
		  "-r      tells the EVMS Command Line Interpreter that the next command line "
		  "argument is a replacement parameter.  "
		  "See examples below.\n");
    Indent_Printf(1, FALSE,
		  "-s      tells the EVMS Command Line Interpreter to prompt for a single command "
		  "line, execute the command line, and then exit.  "
		  "The EVMS Command Line Interpreter normally prompts for command lines until it finds an "
		  "empty command line, at which point it exits.\n");
    Indent_Printf(1, FALSE,
		  "-v      Enable verbose mode.  "
		  "Currently, this causes EVMS Engine status messages to be displayed whenever changes are committed to disk.  "
		  "The messages are displayed using a single line on the screen, i.e., "
		  "the current message is erased and the next message is displayed in its place.\n");
    Indent_Printf(1, FALSE,
		  "-v0     Disable verbose mode.  "
		  "Currently, this causes EVMS Engine status messages to be discarded whenever changes are committed to disk.\n");
    Indent_Printf(1, FALSE,
		  "-v1     Enable verbose mode 1.  "
		  "Currently, this causes EVMS Engine status messages to be displayed whenever changes are committed to disk.  "
		  "The messages are displayed one per line on the screen.\n");
    Indent_Printf(1, FALSE,
		  "-v2     Enable verbose mode 2.  "
		  "Currently, this causes EVMS Engine status messages to be displayed whenever changes are committed to disk.  "
		  "The messages are displayed using a single line on the screen, i.e., "
		  "the current message is erased and the next message is displayed in its place.\n");
    Indent_Printf(1, FALSE,
		  "-rl     tells the EVMS Command Line Interpreter that all of the remaining "
		  "command line arguments are replacement parameters.  "
		  "Replacement parameters are accessed in EVMS commands using the $(x) notation, "
		  "where x is a number identifying which replacement parameter to use.  "
		  "Replacement parameters are assigned numbers (starting with 1) as they are encountered on the command line.\n\n");

    Indent_Printf(1, FALSE,
		  "Example:\n"
		  "evms -c -f testcase -r sda -r sdb\n\n"
		  "sda is replacement parameter 1\n"
		  "sdb is replacement parameter 2\n\n");

    Indent_Printf(1, FALSE,
		  "Example:\n"
		  "evms -r sda -c -f testcase -rl sdb sdc\n\n"
		  "sda is replacement parameter 1\n"
		  "sdb is replacement parameter 2\n"
		  "sdc is replacement parameter 3\n\n");

    Indent_Printf(1, FALSE,
		  "Example:\n"
		  "evms -c -f testcase -rl sda 100\n\n"
		  "sda is replacement parameter 1\n"
		  "100 is replacement parameter 2\n\n");

    Indent_Printf(2, FALSE,
		  "if testcase contains the command:\n\n"
		  "allocate:$(1)_freespace, size=$(2)MB\n\n");

    Indent_Printf(2, FALSE,
		  "then this command becomes:\n\n"
		  "allocate: sda_freespace, size=100MB\n\n");

    return;
}

static int
Init_Substitution_Table(ADDRESS Object,
			TAG ObjectTag,
			ADDRESS ObjectHandle, ADDRESS Parameters)
{
    Substitution_Table_Type *Substitution_Table =
	(Substitution_Table_Type *) Parameters;
    Substitution_Table_Entry *Table_Entry =
	(Substitution_Table_Entry *) Object;

    /* Sanity Checks */
    if ((ObjectTag != REPLACEMENT_PARAMETER) || (Object == NULL)) {
	Report_Standard_Error(DLIST_CORRUPTED);
	abort();
    }

    Substitution_Table->Replacement_Parameters[Substitution_Table->count] =
	*Table_Entry;
    Substitution_Table->count += 1;

    return DLIST_SUCCESS;

}

#define USER_CHOICE_BUFFER_SIZE   10

int GetUserChoice(char Buffer[], long int *UserChoice, int default_choice)
{
    int  I;
    int  data_len;
    int  loops = 5;
    char one_frame[MAX_FRAME_LEN];
    char *Bad_Characters;

    /* Get the user's choice. */
    Bad_Characters = NULL;
    for (I = 0; I < sizeof(Buffer); I++) {
	Buffer[I] = 0;
    }

    // Tell web module to send user choice
    SendEvmsFrame(g_sock_fd, DEFAULT_RETCODE, NULL, 0, EVMS_NEED_USER_INPUT);

    // Read one evms frame from web service module for at most loops times.
    while(loops > 0)
    {
    	if(RecvOneFrame(g_sock_fd, one_frame) <= 0)
    	{
		loops --;
		continue;
    	}

    	data_len = ((PPACKET_HDR)one_frame)->len - PACKET_HDR_LEN;

    	// check the frame type
    	if(((PPACKET_HDR)one_frame)->type != MSG_TYPE_EVMS
    	|| ((PPACKET_HDR)one_frame)->subtype != EVMS_NEED_USER_INPUT
    	|| data_len >= USER_CHOICE_BUFFER_SIZE)
    	{
		loops --;
	    	continue;
    	}

    	// FIXME fgets(Buffer, sizeof(Buffer), stdin);
    	memcpy(Buffer, one_frame + PACKET_HDR_LEN, data_len);
    	break;
    }

    if(loops <= 0)
    {
	    return -1;
    }

    /* Did the user make a choice? */
    if ((strlen(Buffer) == 0) || (strcmp(Buffer, "\n") == 0)) {
	/* No choice made -- use the default. */
	(*UserChoice) = default_choice + 1;
	return 0;
    }

    /* The user made a choice.  Is it valid? */
    (*UserChoice) = strtol(Buffer, &Bad_Characters, 10);
    if (strcmp(Bad_Characters, "\n") == 0) {
	/* So we got valid choice here, breat it */
	return 0;
    }

    return -1;
}

#ifndef DEBUG_PARSER

static int
Display_Engine_Message(char *message_text, int *answer, char **choices)
{
    int Index;
    int I;
    long int UserChoice;
    char Buffer[USER_CHOICE_BUFFER_SIZE];
    char *Bad_Characters;

    if (message_text != NULL) {
	My_Printf("%s\n", message_text);
    }

    if ((answer != NULL) && (choices != NULL)) {
	/* A choice needs to be made.  The default should be set in answer already.
	   If we are in batch mode (i.e. processing a command file) then we will
	   automatically accept the default and just print a message that the
	   default has been accepted and what the default is.  If we are not in
	   batch mode, then we will display a list of the choices and let the
	   user choose one.                                                           */
	if (!Interactive) {

	    My_Printf
		("The EVMS Command Line Interpreter is processing a command file - "
		 "therefore user interaction is not allowed.  "
		 "Thus, the EVMS Command Line Interpreter has accepted the default choice.  "
		 "The default choice is: %s\n", choices[*answer]);

	} else {
	    /* Initialize our loop control variables. */
	    Bad_Characters = NULL;
	    UserChoice = 1;
	    Index = UserChoice;

	    /* Remain in the loop until the user makes a valid selection. */
	    do {

		if ((Bad_Characters != NULL) || (UserChoice > Index)
		    || (UserChoice == 0)) {
		    My_Printf
			("%s does not correspond to any of the available responses.  Please try again.\n\n",
			 Buffer);
		}

		My_Printf("The following responses are available:\n");

		for (Index = 0; choices[Index] != NULL; Index++) {
		    if (Index != *answer) {
			My_Printf(" %d = %s\n", Index + 1, choices[Index]);
		    } else {
			My_Printf("*%d = %s\n", Index + 1, choices[Index]);
		    }
		}

		My_Printf("\nThe default choice is marked with an *.\n");
		My_Printf
		    ("Please enter the number corresponding to your choice: ");

		UserChoice = 0;
		if (0 == GetUserChoice(Buffer, &UserChoice, *answer)) {
		    Bad_Characters = NULL;
		} else {
		    // invalid user input, get one more user input
		    Bad_Characters = "error";
		}

	    }
	    while ((Bad_Characters != NULL) || (UserChoice > Index)
		   || (UserChoice == 0));

	    /* Save the user's choice. */
	    *answer = UserChoice - 1;

	}

    }

    return 0;
}

#endif

#ifdef DEBUG_PARSER

static int
Map_Command_List_Entries(ADDRESS Object,
			 TAG ObjectTag,
			 uint ObjectSize,
			 ADDRESS ObjectHandle, ADDRESS Parameters)
{

    /* Establish easy access to the current node. */
    Executable_Node *Node_To_Map = (Executable_Node *) Object;
    uint *Indentation_Level = (uint *) Parameters;
    uint count;
    int Error;
    char *Node_Name = NULL;

    /* Sanity check!  Is the node of the expected type? */
    if (ObjectTag != STACK_NODE) {

	/* We have an illegal node in the list!  Abort! */
	return DLIST_ITEM_TAG_WRONG;

    }

    /* Map the current node. */

    if (*Indentation_Level == 0) {
	My_Printf("");
    }

    /* Do the indentation spaces first */
    if (*Indentation_Level > 5)
	for (count = 0; count < *Indentation_Level; count++)
	    My_Printf(" ");

    /* Now do the vertical bar. */
    if (*Indentation_Level > 0) {
	My_Printf("|");

	/* Now do the dashes. */
	for (count = 0; count < 4; count++)
	    My_Printf("-");

    }

    /* Now output the information for the node. */
    switch (Node_To_Map->NodeType) {
    case StringData:
	Node_Name = StringDataStr;
	break;
    case IntegerData:
	Node_Name = IntegerDataStr;
	break;
    case RealNumberData:
	Node_Name = RealNumberDataStr;
	break;
    case Assign_Node:
	Node_Name = AssignStr;
	break;
    case Remove_Node:
	Node_Name = RemoveStr;
	break;
    case Allocate_Node:
	Node_Name = AllocateStr;
	break;
    case Probe_Node:
	Node_Name = ProbeStr;
	break;
    case Create_Node:
	Node_Name = CreateStr;
	break;
    case Commit_Node:
	Node_Name = CommitStr;
	break;
    case Set_Node:
	Node_Name = SetStr;
	break;
    case Delete_Node:
	Node_Name = DeleteStr;
	break;
    case Echo_Node:
	Node_Name = EchoStr;
	break;
    case Expand_Node:
	Node_Name = ExpandStr;
	break;
    case Shrink_Node:
	Node_Name = ShrinkStr;
	break;
    case Rename_Node:
	Node_Name = RenameStr;
	break;
    case Revert_Node:
	Node_Name = RevertStr;
	break;
    case Query_Node:
	Node_Name = QueryStr;
	break;
    case Help_Node:
	Node_Name = HelpStr;
	break;
    case Filter:
	Node_Name = FilterStr;
	break;
    case Feature_ID:
	Node_Name = FeatureStr;
	break;
    case Feature_Name:
	Node_Name = FeatureNameStr;
	break;
    case Plugin_ID:
	Node_Name = PluginStr;
	break;
    case Plugin_Name:
	Node_Name = PluginNameStr;
	break;
    case Format_Node:
	Node_Name = FormatStr;
	break;
    case Unformat_Node:
	Node_Name = UnformatStr;
	break;
    case Check_Node:
	Node_Name = CheckStr;
	break;
    case Add_Feature_Node:
	Node_Name = AFStr;
	break;
    case Convert_Node:
	Node_Name = ConvertStr;
	break;
    case Task_Node:
	Node_Name = TaskStr;
	break;
    case Quit_Node:
	Node_Name = QuitStr;
	break;
    case Name_Value_Pair:
	if (Node_To_Map->NodeValue != NULL) {
	    My_Printf("Node: Name_Value_Pair.  Node Value: %s\n",
		      (char *) Node_To_Map->NodeValue);
	} else {
	    My_Printf("Node: Name_Value_Pair.  Node Value is NULL.\n");
	}
	break;
    case ObjectList:
	if (Node_To_Map->NodeValue != NULL) {
	    My_Printf("Node: ObjectList.  Node Value: %s\n",
		      (char *) Node_To_Map->NodeValue);
	} else {
	    My_Printf("Node: ObjectList.  Node Value is NULL.\n");
	}
	break;
    case Translate_Name_To_Handle:
	if (Node_To_Map->NodeValue != NULL) {
	    My_Printf("Node: Translate_Name_To_Handle.  Node Value: %s\n",
		      (char *) Node_To_Map->NodeValue);
	} else {
	    My_Printf
		("Node: Translate_Name_To_Handle.  Node Value is NULL.\n");
	}
	break;
    case Name_To_Volume_Handle:
	if (Node_To_Map->NodeValue != NULL) {
	    My_Printf("Node: Name_To_Volume_Handle.  Node Value: %s\n",
		      (char *) Node_To_Map->NodeValue);
	} else {
	    My_Printf
		("Node: Name_To_Volume_Handle.  Node Value is NULL.\n");
	}
	break;
    case Name_To_Object_Handle:
	if (Node_To_Map->NodeValue != NULL) {
	    My_Printf("Node: Name_To_Object_Handle.  Node Value: %s\n",
		      (char *) Node_To_Map->NodeValue);
	} else
	    }
	{
	    My_Printf
		("Node: Name_To_Object_Handle.  Node Value is NULL.\n");
	    break;
    case Name_To_Region_Handle:
	    if (Node_To_Map->NodeValue != NULL) {
		My_Printf("Node: Name_To_Region_Handle.  Node Value: %s\n",
			  (char *) Node_To_Map->NodeValue);
	    } else {
		My_Printf
		    ("Node: Name_To_Region_Handle.  Node Value is NULL.\n");
	    }
	    break;
    case Name_To_Container_Handle:
	    if (Node_To_Map->NodeValue != NULL) {
		My_Printf
		    ("Node: Name_To_Container_Handle.  Node Value: %s\n",
		     (char *) Node_To_Map->NodeValue);
	    } else {
		My_Printf
		    ("Node: Name_To_Container_Handle.  Node Value is NULL.\n");
	    }
	    break;
    case Name_To_Segment_Handle:
	    if (Node_To_Map->NodeValue != NULL) {
		My_Printf
		    ("Node: Name_To_Segment_Handle.  Node Value: %s\n",
		     (char *) Node_To_Map->NodeValue);
	    } else {
		My_Printf
		    ("Node: Name_To_Segment_Handle.  Node Value is NULL.\n");
	    }
	    break;
    case Name_To_Disk_Handle:
	    if (Node_To_Map->NodeValue != NULL) {
		My_Printf("Node: Name_To_Disk_Handle.  Node Value: %s\n",
			  (char *) Node_To_Map->NodeValue);
	    } else {
		My_Printf
		    ("Node: Name_To_Disk_Handle.  Node Value is NULL.\n");
	    }
	    break;
    case Name_To_Plugin_Handle:
	    if (Node_To_Map->NodeValue != NULL) {
		My_Printf("Node: Name_To_Plugin_Handle.  Node Value: %s\n",
			  (char *) Node_To_Map->NodeValue);
	    } else {
		My_Printf
		    ("Node: Name_To_Plugin_Handle.  Node Value is NULL.\n");
	    }
	    break;
    case Units_Node:
	    if (Node_To_Map->NodeValue != NULL) {
		My_Printf("Node: Units_Node.  Node Value: %s\n",
			  (char *) Node_To_Map->NodeValue);
	    } else {
		My_Printf("Node: Units_Node.  Node Value is NULL.\n");
	    }
	    break;
    case Task_Adjust:
	    if (Node_To_Map->NodeValue != NULL) {
		My_Printf("Node: Task_Adjust.  Node Value: %s\n",
			  (char *) Node_To_Map->NodeValue);
	    } else {
		My_Printf("Node: Task_Adjust.  Node Value is NULL.\n");
	    }
	    break;
    case Topmost_Objects:
	    if (Node_To_Map->NodeValue != NULL) {
		My_Printf("Node: Topmost_Objects.  Node Value: %s\n",
			  (char *) Node_To_Map->NodeValue);
	    } else {
		My_Printf("Node: Topmost_Objects.  Node Value is NULL.\n");
	    }
	    break;
    case Precision_Node:
	    if (Node_To_Map->NodeValue != NULL) {
		My_Printf("Node: Precision_Node.  Node Value: %s\n",
			  (char *) Node_To_Map->NodeValue);
	    } else {
		My_Printf("Node: Precision_Node.  Node Value is NULL.\n");
	    }
	    break;
    case Query_Extended_Info:
	    if (Node_To_Map->NodeValue != NULL) {
		My_Printf("Node: Query_Extended_Info.  Node Value: %s\n",
			  (char *) Node_To_Map->NodeValue);
	    } else {
		My_Printf
		    ("Node: Query_Extended_Info.  Node Value is NULL.\n");
	    }
	    break;
    default:
	    My_Printf("Node type is not valid.\n");
	    break;
	}

	if (Node_Name != NULL) {

	    if (Node_To_Map->NodeValue != NULL) {
		My_Printf("Node: %s.  Node Value: %s\n", Node_Name,
			  (char *) Node_To_Map->NodeValue);
	    } else {
		My_Printf("Node: %s.  Node Value is NULL.\n", Node_Name);
	    }

	}

	/* If the node has children, we must map them also. */
	if (Node_To_Map->Children != NULL) {

	    /* Map the children. */
	    *Indentation_Level = *Indentation_Level + 5;
	    Error =
		ForEachItem(Node_To_Map->Children,
			    Map_Command_List_Entries, Indentation_Level,
			    TRUE);
	    *Indentation_Level = *Indentation_Level - 5;

	    if (Error != DLIST_SUCCESS)
		return Error;

	}

	return DLIST_SUCCESS;

    }

#endif


/* Find the current screen width, then break up the buffer on word boundaries
 * into lines that fit within the screen width and print the lines.
 */
#define WHITE_SPACE " \t\r\n"
#define WHITE_SPACE_NO_NL " \t\r"

    static int curr_col = 0;

    static char *Do_Indent(char *pch, int indent_level) {

	int i;

	for (i = 0; i < indent_level; i++) {
	    *pch = '\t';
	    pch++;
	    curr_col += 8;
	}

	return pch;
    }

    static int Marginize(char *string, int indent_level,
			 boolean indent_first_line) {

	struct winsize ws = { 0 };
	char *out_buf;
	int extra_space = 0;
	char *p_out;
	char *p_word;
	size_t white_space_size;
	size_t word_size;
	int count;
	int line_count;
	int len;
	int col_width;
	int col = 0;
	int i;
	int rc;


	rc = ioctl(1, TIOCGWINSZ, &ws);
	if (rc < 0 || ws.ws_col == 0) {
	    /* Fall back to default if ioctl fails. */
	    return 0;
	}

	/* Make sure the indent level fits within the screen width.  If not,
	   decrement it until it does.
	 */
	while (indent_level * 8 > ws.ws_col) {
	    indent_level--;
	}

	/* In order to do indentation there must be extra room in the output buffer
	 * for the tabs that are used to indent each line. */
	line_count = 0;
	len = strlen(string);
	col_width = ws.ws_col - indent_level * 8;
	col = 0;

	/* Crude line count */
	for (i = 0; i < len; i++) {
	    col++;
	    if ((string[i] == '\n') || (col % col_width == 0)) {
		line_count++;
		col = 0;
	    }
	}

	extra_space = line_count * (indent_level + 1);

	out_buf = malloc(strlen(string) + 1 + extra_space);
	if (out_buf == NULL) {
	    return 0;
	}

	p_out = out_buf;

	if (curr_col == 0) {
	    p_out =
		Do_Indent(p_out,
			  indent_first_line ? indent_level : indent_level -
			  1);
	}
	*p_out = '\0';

	while (*string != '\0') {
	    /* Skip over white space. */
	    white_space_size = 0;
	    p_word = string;
	    while ((*p_word != '\0')
		   && (strchr(WHITE_SPACE, *p_word) != NULL)) {
		if (*p_word == '\t') {
		    white_space_size +=
			(curr_col + white_space_size + 8) & (8 - 1);
		} else
		    white_space_size += 1;
		p_word++;
	    }

	    /* Skip over the next word. */
	    word_size = strcspn(p_word, WHITE_SPACE);

	    /* If the word won't fit on the line and... */
	    if ((curr_col + white_space_size + word_size > ws.ws_col) &&
		/* it's not the first word on the line... */
		!((curr_col == 0) || (curr_col == indent_level * 8))) {

		/* This word would go past the screen margin. Start a new line. */
		*p_out = '\n';
		p_out++;

		curr_col = 0;
		p_out = Do_Indent(p_out, indent_level);

		/* Skip over white space up to the next word or new line.  Any whitespace
		 * after a new line in the input string gets copied to the output buffer
		 * on the newly started line.
		 */
		string += strspn(string, WHITE_SPACE_NO_NL);
		if (*string == '\n') {
		    string++;

		    white_space_size = strspn(p_word, WHITE_SPACE);

		    if (white_space_size != 0) {
			memcpy(p_out, string, white_space_size);
			p_out += white_space_size;

			string += white_space_size;
		    }
		}

		*p_out = '\0';

	    } else {

		/* Copy the white space into the output buffer. */
		while (string != p_word) {
		    *p_out = *string;
		    p_out++;

		    switch (*string) {
		    case '\n':
			/* Add indents if the next character doesn't start a new line. */
			if ((string[1] != '\n') && (string[1] != '\0')) {
			    p_out = Do_Indent(p_out, indent_level);
			}
			/* fall through */

		    case '\r':
			curr_col = 0;
			break;

		    case '\t':
			curr_col += 8;
			curr_col &= ~(8 - 1);
			break;

		    default:
			curr_col++;
		    }
		    string++;
		}

		*p_out = '\0';

		/* Put the word into the output buffer. */
		if (word_size != 0) {
		    memcpy(p_out, string, word_size);
		    p_out += word_size;
		    *p_out = '\0';

		    curr_col += word_size;
		    string += word_size;
		}
	    }
	}

	count = (int) write(1, out_buf, strlen(out_buf));
	free(out_buf);

	return count;
    }

/* Expand the printf result into a buffer, then feed the buffer to Marginize
 * to print it.
 */
    int My_Printf(char *format, ...) {
	va_list args;
	char *Buf;
	int count;

	va_start(args, format);

	/* Expand the format and args, allocating a buffer big enough to hold the result. */
	vasprintf(&Buf, format, args);

	if (Buf != NULL) {
	    // count = Marginize(Buf, 0, TRUE);
	    /*
	    int msg_len;
	    char msg[MAX_CONTENT_LEN];

	    msg_len = strlen(Buf);
	    msg_len = msg_len > MAX_CONTENT_LEN -1 ? MAX_CONTENT_LEN - 1 : msg_len;
	    memcpy(msg, Buf, msg_len);
	    msg[msg_len] = '\0';
	    msg_len ++;
	    */

	    count =
		SendEvmsFrame(g_sock_fd, DEFAULT_RETCODE, Buf, strlen(Buf) + 1,
			      EVMS_PROMPT_MSG);

	    /* If Marginize failed (printed nothing) print it manually. */
	    if (count == 0) {
		count = (int) write(1, Buf, strlen(Buf));
	    } else if (count < 0) {
		// send error
	    }

	    free(Buf);

	} else {
	    count = vprintf(format, args);
	}

	return count;
    }

    int Indent_Printf(int indent_level, boolean indent_first_line,
		      char *format, ...) {
	va_list args;
	char *Buf;
	int count;

	va_start(args, format);

	/* Expand the format and args, allocating a buffer big enough to hold the result. */
	vasprintf(&Buf, format, args);

	if (Buf != NULL) {
	    // count = Marginize(Buf, indent_level, indent_first_line);
	    count =
		SendEvmsFrame(g_sock_fd, DEFAULT_RETCODE, Buf, strlen(Buf) + 1,
			      EVMS_PROMPT_MSG);
	    /* If Marginize failed (printed nothing) print it manually. */
	    if (count == 0) {
		count = (int) write(1, Buf, strlen(Buf));
	    }

	    free(Buf);

	} else {
	    count = vprintf(format, args);
	}

	return count;
    }


    int Row_Delimiter(char ch, char *header) {

	struct winsize ws = { 0 };
	char *Buf;
	int rc = 0;
	int offset1 = 0;
	int offset2 = 0;

	/* Set default column width in case the ioctl fails. */
	ws.ws_col = 80;

	/* Get the window size. */
	ioctl(1, TIOCGWINSZ, &ws);

	if (header != NULL) {
	    /* If the header won't fit in the screen width, send it to My_Printf()
	     * and return .*/
	    if (strlen(header) > ws.ws_col) {
		return My_Printf(header);
	    }

	    offset1 = (ws.ws_col - strlen(header)) / 2;
	    offset2 = offset1 + strlen(header);
	}

	Buf = malloc(ws.ws_col + 3);

	if (Buf != NULL) {
	    if (header != NULL) {
		memset(Buf, ch, offset1);
		memcpy(Buf + offset1, header, strlen(header));
		memset(Buf + offset2, ch, ws.ws_col - offset2);

	    } else {
		memset(Buf, ch, ws.ws_col);
	    }

	    Buf[ws.ws_col] = '\n';
	    Buf[ws.ws_col + 1] = '\n';
	    Buf[ws.ws_col + 2] = '\0';
	    rc = (int) write(1, Buf, strlen(Buf));
	    free(Buf);

	} else {
	    /* Have to do it by hand. */
	    int i;

	    if (header != NULL) {
		for (i = 0; i < offset1; i++) {
		    rc += printf("%c", ch);
		}
		rc += printf("%s", header);
		for (i = offset2; i < ws.ws_col; i++) {
		    rc += printf("%c", ch);
		}

	    } else {
		for (i = 0; i < ws.ws_col; i++) {
		    rc += printf("%c", ch);
		}
	    }
	    rc = printf("\n");
	}

	return rc;
    }
