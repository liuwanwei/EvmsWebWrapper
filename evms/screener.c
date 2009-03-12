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
 */

/* Identify this file. */
#define SCREENER_C      1

/*--------------------------------------------------
 * Necessary include files
 --------------------------------------------------*/

#include <ctype.h>		/* toupper */
#include <stdlib.h>		/* malloc */
#include <string.h>		/* malloc */
#include "token.h"		/* TokenType, Token, MaxIdentifierLength */
#include "scanner.h"		/* GetToken */
#include "screener.h"


/*--------------------------------------------------
 * Private constants
 --------------------------------------------------*/


/*--------------------------------------------------
 * Private functions
 --------------------------------------------------*/



/*--------------------------------------------------
 * Public Functions
 --------------------------------------------------*/


/*********************************************************************/
/*                                                                   */
/*   Function Name: GetScreenedToken                                 */
/*                                                                   */
/*   Descriptive Name: Repeatedly calls the scanner for a token until*/
/*                     it gets EOF, Error, or a token which is       */
/*                     something other than a space or tab.  This    */
/*                     function will also recharacterize some tokens */
/*                     currently marked as KeyWords.                 */
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
TokenType *GetScreenedToken(void)
{

    TokenType *CurrentToken = NULL;	/* Points to the token we are examining. */
    uint Index;			/* Used when converting token text to upper case. */
    boolean Convert_To_String = FALSE;	/* Used to control when tokens of type KeyWord are converted to type String. */
    char Original_Character;	/* Used to hold the original value of TokenText when TokenText contains a single character. */
    char *Original_Token_Text = NULL;	/* Used to hold the original token text in order to preserve case should the token text not be a keyword. */

    do {

	/* Get a token to examine. */
	CurrentToken = GetToken();

    }
    while ((CurrentToken != NULL) &&
	   ((CurrentToken->Characterization == Tab) ||
	    (CurrentToken->Characterization == MultiTab) ||
	    (CurrentToken->Characterization == Space) ||
	    (CurrentToken->Characterization == MultiSpace)));

    /* Do we have a token? */
    if (CurrentToken == NULL) {

	/* We must be out of memory or something equally bad.  This problem should have been reported by the scanner already. */
	return NULL;

    }

    /* Do we have an EOF or InvalidCharacter token? */
    if ((CurrentToken->Characterization == EofToken)
	|| (CurrentToken->Characterization == InvalidCharacter)) {

	return CurrentToken;

    }

    /* Lets look at tokens characterized as KeyWords.  These could be actual command line key words or
       strings that were not within quotes.  We will convert all command line key words into pointers
       to the same set of strings, and all others will be recharacterized as strings.                   */
    if (CurrentToken->Characterization == KeyWord) {

	if (CurrentToken->TokenLength == 1) {
	    Original_Character = CurrentToken->TokenText[0];
	    CurrentToken->TokenText[0] =
		toupper(CurrentToken->TokenText[0]);

	    switch (CurrentToken->TokenText[0]) {

	    case 'A':		/* Acceptable, Add, All, Auto */
	    case 'C':		/* Check, Children, Cluster, CM, Container, Containers, Create */
	    case 'D':		/* Delete, Device, Disk, Disks, Distributed, DLM */
	    case 'E':		/* EI, EQ, End, Expand, Expandable, Extended */
	    case 'F':		/* Feature, File, Filesystem, FSIM, Format, Freespace */
	    case 'G':		/* GB, GT */
	    case 'H':		/* Hours, Help */
	    case 'I':		/* Info, Interface */
	    case 'K':		/* KB */
	    case 'L':		/* Lock, LT */
	    case 'M':		/* Management, Manager, Manual, MB,  Module */
	    case 'N':		/* Name */
	    case 'O':		/* Object, Objects */
	    case 'P':		/* Parent, Plugin, Plugins, Probe */
	    case 'Q':		/* Query */
	    case 'R':		/* Region, Regions, Remove, Rename, Revert */
	    case 'S':		/* Seconds, Sectors, Segment, Segments, Shrink, Shrinkable, Size, Start */
	    case 'T':		/* TYPE */
	    case 'U':		/* Unclaimed, Unformat */
	    case 'V':		/* Volume, Volumes */

		/* The single character will be treated as a KeyWord since it matches the first letter of a keyword. */
		break;
	    default:
		/* Restore TokenText and change its characterization to string. */
		CurrentToken->Characterization = String;
		CurrentToken->TokenText[0] = Original_Character;

		break;

	    }

	} else {

	    Convert_To_String = FALSE;

	    /* Save the original token text. */
	    Original_Token_Text = strdup(CurrentToken->TokenText);

	    /* Convert TokenText to upper case. */
	    for (Index = 0; Index < CurrentToken->TokenLength; Index++)
		CurrentToken->TokenText[Index] =
		    toupper(CurrentToken->TokenText[Index]);

	    /* Now we will see which key word, if any, this token may represent. */
	    switch (CurrentToken->TokenText[0]) {

	    case 'A':		/* Acceptable, Add, AF, All, Allocate, Assign, As, Auto, Available, ASM */

		switch (CurrentToken->TokenLength) {
		case 2:
		    if (strcmp(CurrentToken->TokenText, "AF") == 0) {

			/* We have a match.  Free TokenText and set it to AFStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = AFStr;
			break;

		    }
		    if (strcmp(CurrentToken->TokenText, "AS") == 0) {

			/* We have a match.  Free TokenText and set it to AssignStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = AssignStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 3:
		    if (strcmp(CurrentToken->TokenText, ActStr) == 0) {

			/* We have a match.  Free TokenText and set it to ActStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = ActStr;
			break;

		    }
		    if (strcmp(CurrentToken->TokenText, AddStr) == 0) {

			/* We have a match.  Free TokenText and set it to AddStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = AddStr;
			break;

		    }
		    if (strcmp(CurrentToken->TokenText, AllStr) == 0) {

			/* We have a match.  Free TokenText and set it to AllStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = AllStr;
			break;

		    }
		    if (strcmp(CurrentToken->TokenText, AsmStr) == 0) {

			/* We have a match.  Free TokenText and set it to AsmStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = AsmStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 4:
		    if (strcmp(CurrentToken->TokenText, AutoStr) == 0) {

			/* We have a match.  Free TokenText and set it to AutoStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = AutoStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 6:
		    if (strcmp(CurrentToken->TokenText, AssignStr) == 0) {

			/* We have a match.  Free TokenText and set it to AssignStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = AssignStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 8:
		    if (strcmp(CurrentToken->TokenText, ActivateStr) == 0) {

			/* We have a match.  Free TokenText and set it to ActivateStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = ActivateStr;
			break;

		    }
		    if (strcmp(CurrentToken->TokenText, AllocateStr) == 0) {

			/* We have a match.  Free TokenText and set it to AllocateStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = AllocateStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 9:
		    if (strcmp(CurrentToken->TokenText, AvailableStr) == 0) {

			/* We have a match.  Free TokenText and set it to AvailableStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = AvailableStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 10:
		    if (strcmp(CurrentToken->TokenText, AcceptableStr) ==
			0) {

			/* We have a match.  Free TokenText and set it to AcceptableStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = AcceptableStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		default:
		    Convert_To_String = TRUE;
		    break;
		}

		break;

	    case 'C':		/* Check, Children, Cluster, CM, Co, Commit, Con, Compatibility, Container, Containers, Convert, Convertible, Create */

		switch (CurrentToken->TokenLength) {
		case 2:
		    if (strcmp(CurrentToken->TokenText, CMStr) == 0) {

			/* We have a match.  Free TokenText and set it to CMStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = CMStr;
			break;

		    }

		    if (strcmp(CurrentToken->TokenText, "CH") == 0) {

			/* We have a match.  Free TokenText and set it to CheckStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = CheckStr;
			break;

		    }

		    if (strcmp(CurrentToken->TokenText, "CO") == 0) {

			/* We have a match.  Free TokenText and set it to ConvertStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = ConvertStr;
			break;

		    }

		    Convert_To_String = TRUE;
		    break;
		case 3:
		    if (strcmp(CurrentToken->TokenText, "CHI") == 0) {

			/* We have a match.  Free TokenText and set it to ChildrenStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = ChildrenStr;
			break;

		    }

		    if (strcmp(CurrentToken->TokenText, "CON") == 0) {

			/* We have a match.  Free TokenText and set it to ConvertibleStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = ConvertibleStr;
			break;

		    }

		    Convert_To_String = TRUE;
		    break;
		case 5:
		    if (strcmp(CurrentToken->TokenText, CheckStr) == 0) {

			/* We have a match.  Free TokenText and set it to CheckStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = CheckStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 6:
		    if (strcmp(CurrentToken->TokenText, CreateStr) == 0) {

			/* We have a match.  Free TokenText and set it to CreateStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = CreateStr;
			break;

		    }
		    if (strcmp(CurrentToken->TokenText, CommitStr) == 0) {

			/* We have a match.  Free TokenText and set it to CommitStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = CommitStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 7:
		    if (strcmp(CurrentToken->TokenText, ClusterStr) == 0) {

			/* We have a match.  Free TokenText and set it to ClusterStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = ClusterStr;
			break;

		    }

		    if (strcmp(CurrentToken->TokenText, ConvertStr) == 0) {

			/* We have a match.  Free TokenText and set it to ConvertStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = ConvertStr;
			break;

		    }

		    Convert_To_String = TRUE;
		    break;
		case 8:
		    if (strcmp(CurrentToken->TokenText, ChildrenStr) == 0) {

			/* We have a match.  Free TokenText and set it to ChildrenStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = ChildrenStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 9:
		    if (strcmp(CurrentToken->TokenText, ContainerStr) == 0) {

			/* We have a match.  Free TokenText and set it to ContainerStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = ContainerStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 10:
		    if (strcmp(CurrentToken->TokenText, ContainersStr) ==
			0) {

			/* We have a match.  Free TokenText and set it to ContainersStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = ContainersStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 11:
		    if (strcmp(CurrentToken->TokenText, ConvertibleStr) ==
			0) {

			/* We have a match.  Free TokenText and set it to ConvertibleStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = ConvertibleStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 13:
		    if (strcmp(CurrentToken->TokenText, CompatibilityStr)
			== 0) {

			/* We have a match.  Free TokenText and set it to CompatibilityStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = CompatibilityStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		default:
		    Convert_To_String = TRUE;
		    break;
		}
		break;

	    case 'D':		/* Delete, Device, Disk, Disks, Distributed, DLM, DR */

		switch (CurrentToken->TokenLength) {
		case 2:
		    if (strcmp(CurrentToken->TokenText, DrStr) == 0) {

			/* We have a match.  Free TokenText and set it to DrStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = DrStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 3:
		    if (strcmp(CurrentToken->TokenText, DLMStr) == 0) {

			/* We have a match.  Free TokenText and set it to DLMStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = DLMStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 4:
		    if (strcmp(CurrentToken->TokenText, DiskStr) == 0) {

			/* We have a match.  Free TokenText and set it to DiskStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = DiskStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 5:
		    if (strcmp(CurrentToken->TokenText, DeactStr) == 0) {

			/* We have a match.  Free TokenText and set it to DeactStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = DeactStr;
			break;

		    }
		    if (strcmp(CurrentToken->TokenText, DisksStr) == 0) {

			/* We have a match.  Free TokenText and set it to DisksStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = DisksStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 6:
		    if (strcmp(CurrentToken->TokenText, DeleteStr) == 0) {

			/* We have a match.  Free TokenText and set it to DeleteStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = DeleteStr;
			break;

		    }

		    if (strcmp(CurrentToken->TokenText, DeviceStr) == 0) {

			/* We have a match.  Free TokenText and set it to DeviceStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = DeviceStr;
			break;

		    }

		    Convert_To_String = TRUE;
		    break;
		case 10:
		    if (strcmp(CurrentToken->TokenText, DeactivateStr) ==
			0) {

			/* We have a match.  Free TokenText and set it to DeactivateStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = DeactivateStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 11:
		    if (strcmp(CurrentToken->TokenText, DistributedStr) ==
			0) {

			/* We have a match.  Free TokenText and set it to DistributedStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = DistributedStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		default:
		    Convert_To_String = TRUE;
		    break;
		}

		break;

	    case 'E':		/* Echo, EI, EQ, End, EP, Expand, Expandable, Extended, Exit */

		switch (CurrentToken->TokenLength) {
		case 2:
		    if (strcmp(CurrentToken->TokenText, EIStr) == 0) {

			/* We have a match.  Free TokenText and set it to EIStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = EIStr;
			break;

		    }

		    if (strcmp(CurrentToken->TokenText, EQStr) == 0) {

			/* We have a match.  Free TokenText and set it to EQStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = EQStr;
			break;

		    }

		    if (strcmp(CurrentToken->TokenText, EPStr) == 0) {

			/* We have a match.  Free TokenText and set it to EPStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = EPStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 3:
		    if (strcmp(CurrentToken->TokenText, EndStr) == 0) {

			/* We have a match.  Free TokenText and set it to EndStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = EndStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 4:
		    if (strcmp(CurrentToken->TokenText, EchoStr) == 0) {

			/* We have a match.  Free TokenText and set it to EndStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = EchoStr;
			break;

		    }

		    if (strcmp(CurrentToken->TokenText, ExitStr) == 0) {

			free(CurrentToken->TokenText);
			CurrentToken->TokenText = ExitStr;
			break;

		    }

		    Convert_To_String = TRUE;
		    break;
		case 6:
		    if (strcmp(CurrentToken->TokenText, ExpandStr) == 0) {

			/* We have a match.  Free TokenText and set it to ExpandStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = ExpandStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 8:
		    if (strcmp(CurrentToken->TokenText, ExtendedStr) == 0) {

			/* We have a match.  Free TokenText and set it to ExtendedStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = ExtendedStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 10:
		    if (strcmp(CurrentToken->TokenText, ExpandableStr) ==
			0) {

			/* We have a match.  Free TokenText and set it to ExpandableStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = ExpandableStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		default:
		    Convert_To_String = TRUE;
		    break;
		}
		break;
	    case 'F':		/* Feature, File, Filesystem, FSIM, Format, Freespace */

		switch (CurrentToken->TokenLength) {
		case 4:
		    if (strcmp(CurrentToken->TokenText, FileStr) == 0) {

			/* We have a match.  Free TokenText and set it to FileStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = FileStr;
			break;

		    } else if (strcmp(CurrentToken->TokenText, FSIMStr) ==
			       0) {

			/* We have a match.  Free TokenText and set it to FSIMStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = FSIMStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 6:
		    if (strcmp(CurrentToken->TokenText, FormatStr) == 0) {

			/* We have a match.  Free TokenText and set it to FormatStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = FormatStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 7:
		    if (strcmp(CurrentToken->TokenText, FeatureStr) == 0) {

			/* We have a match.  Free TokenText and set it to FeatureStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = FeatureStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 9:
		    if (strcmp(CurrentToken->TokenText, FreespaceStr) == 0) {

			/* We have a match.  Free TokenText and set it to FreespaceStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = FreespaceStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 10:
		    if (strcmp(CurrentToken->TokenText, FilesystemStr) ==
			0) {

			/* We have a match.  Free TokenText and set it to FilesystemStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = FilesystemStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		default:
		    Convert_To_String = TRUE;
		    break;
		}

		break;

	    case 'G':		/* GB, GT */

		if (strcmp(CurrentToken->TokenText, GigabyteStr) == 0) {

		    /* We have a match.  Free TokenText and set it to GigabyteStr. */
		    free(CurrentToken->TokenText);
		    CurrentToken->TokenText = GigabyteStr;
		    break;

		}

		if (strcmp(CurrentToken->TokenText, GTStr) == 0) {

		    /* We have a match.  Free TokenText and set it to GTStr. */
		    free(CurrentToken->TokenText);
		    CurrentToken->TokenText = GTStr;
		    break;

		}

		Convert_To_String = TRUE;

		break;

	    case 'H':		/* Help, Hold, Hours */

		if (strcmp(CurrentToken->TokenText, HoursStr) == 0) {

		    /* We have a match.  Free TokenText and set it to HoursStr. */
		    free(CurrentToken->TokenText);
		    CurrentToken->TokenText = HoursStr;
		    break;

		}

		if (strcmp(CurrentToken->TokenText, HoldStr) == 0) {

		    /* We have a match.  Free TokenText and set it to HoldStr. */
		    free(CurrentToken->TokenText);
		    CurrentToken->TokenText = HoldStr;
		    break;

		}

		if (strcmp(CurrentToken->TokenText, HelpStr) == 0) {

		    /* We have a match.  Free TokenText and set it to HelpStr. */
		    free(CurrentToken->TokenText);
		    CurrentToken->TokenText = HelpStr;
		    break;

		}

		Convert_To_String = TRUE;

		break;

	    case 'I':		/* Info, Interface */

		if (strcmp(CurrentToken->TokenText, InfoStr) == 0) {

		    /* We have a match.  Free TokenText and set it to InfoStr. */
		    free(CurrentToken->TokenText);
		    CurrentToken->TokenText = InfoStr;
		    break;

		}

		if (strcmp(CurrentToken->TokenText, InterfaceStr) == 0) {

		    /* We have a match.  Free TokenText and set it to InterfaceStr. */
		    free(CurrentToken->TokenText);
		    CurrentToken->TokenText = InterfaceStr;
		    break;

		}

		Convert_To_String = TRUE;

		break;

	    case 'K':		/* KB */

		if (strcmp(CurrentToken->TokenText, KilobyteStr) == 0) {

		    /* We have a match.  Free TokenText and set it to KilobyteStr. */
		    free(CurrentToken->TokenText);
		    CurrentToken->TokenText = KilobyteStr;
		    break;

		}

		Convert_To_String = TRUE;

		break;

	    case 'L':		/* List, LO, Lock, LT */

		switch (CurrentToken->TokenLength) {
		case 2:
		    if (strcmp(CurrentToken->TokenText, LOStr) == 0) {

			/* We have a match.  Free TokenText and set it to LOStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = LOStr;
			break;

		    }
		    if (strcmp(CurrentToken->TokenText, LTStr) == 0) {

			/* We have a match.  Free TokenText and set it to LTStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = LTStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 4:
		    if (strcmp(CurrentToken->TokenText, ListStr) == 0) {

			/* We have a match.  Free TokenText and set it to ListStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = ListStr;
			break;

		    }
		    if (strcmp(CurrentToken->TokenText, LockStr) == 0) {

			/* We have a match.  Free TokenText and set it to LockStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = LockStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		default:
		    Convert_To_String = TRUE;
		    break;
		}
		break;

	    case 'M':		/* Management, Manager, Manual, MB,  Minutes, Milliseconds, Microseconds, Module, Modify, Mkfs */
		switch (CurrentToken->TokenLength) {
		case 2:
		    if (strcmp(CurrentToken->TokenText, MegabyteStr) == 0) {

			/* We have a match.  Free TokenText and set it to MegabyteStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = MegabyteStr;
			break;

		    }
		    if (strcmp(CurrentToken->TokenText, "MS") == 0) {

			/* We have a match.  Free TokenText and set it to MillisecondsStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = MillisecondsStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 3:
		    if (strcmp(CurrentToken->TokenText, "MIN") == 0) {

			/* We have a match.  Free TokenText and set it to MinutesStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = MinutesStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 4:
		    if (strcmp(CurrentToken->TokenText, MkfsStr) == 0) {

			/* We have a match.  Free TokenText and set it to MkfsStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = MkfsStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 5:
		    if (strcmp(CurrentToken->TokenText, MountStr) == 0) {

			/* We have a match.  Free TokenText and set it to MountStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = MountStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 6:
		    if (strcmp(CurrentToken->TokenText, ManualStr) == 0) {

			/* We have a match.  Free TokenText and set it to ManualStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = ManualStr;
			break;

		    }
		    if (strcmp(CurrentToken->TokenText, ModuleStr) == 0) {

			/* We have a match.  Free TokenText and set it to ModuleStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = ModuleStr;
			break;

		    }
		    if (strcmp(CurrentToken->TokenText, ModifyStr) == 0) {

			/* We have a match.  Free TokenText and set it to ModifyStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = ModifyStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 7:
		    if (strcmp(CurrentToken->TokenText, ManagerStr) == 0) {

			/* We have a match.  Free TokenText and set it to ManagerStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = ManagerStr;
			break;

		    }
		    if (strcmp(CurrentToken->TokenText, MinutesStr) == 0) {

			/* We have a match.  Free TokenText and set it to MinutesStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = MinutesStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 10:
		    if (strcmp(CurrentToken->TokenText, ManagementStr) ==
			0) {

			/* We have a match.  Free TokenText and set it to ManagementStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = ManagementStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 12:
		    if (strcmp(CurrentToken->TokenText, MillisecondsStr) ==
			0) {

			/* We have a match.  Free TokenText and set it to MillisecondsStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = MillisecondsStr;
			break;

		    }
		    if (strcmp(CurrentToken->TokenText, MicrosecondsStr) ==
			0) {

			/* We have a match.  Free TokenText and set it to MicrosecondsStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = MicrosecondsStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		default:
		    Convert_To_String = TRUE;
		    break;
		}

		break;

	    case 'N':		/* Name */

		if (strcmp(CurrentToken->TokenText, NameStr) == 0) {

		    /* We have a match.  Free TokenText and set it to NameStr. */
		    free(CurrentToken->TokenText);
		    CurrentToken->TokenText = NameStr;
		    break;

		}

		Convert_To_String = TRUE;

		break;

	    case 'O':		/* Object, Objects, Options */

		if (strcmp(CurrentToken->TokenText, ObjectStr) == 0) {

		    /* We have a match.  Free TokenText and set it to ObjectStr. */
		    free(CurrentToken->TokenText);
		    CurrentToken->TokenText = ObjectStr;
		    break;

		}

		if (strcmp(CurrentToken->TokenText, ObjectsStr) == 0) {

		    /* We have a match.  Free TokenText and set it to ObjectsStr. */
		    free(CurrentToken->TokenText);
		    CurrentToken->TokenText = ObjectsStr;
		    break;

		}

		if (strcmp(CurrentToken->TokenText, OptionsStr) == 0) {

		    /* We have a match.  Free TokenText and set it to OptionsStr. */
		    free(CurrentToken->TokenText);
		    CurrentToken->TokenText = OptionsStr;
		    break;

		}

		Convert_To_String = TRUE;

		break;

	    case 'P':		/* Parent, Plugin, Plugins, Points, Probe */

		switch (CurrentToken->TokenLength) {
		case 3:
		    if (strcmp(CurrentToken->TokenText, "PAR") == 0) {

			/* We have a match.  Free TokenText and set it to ParentStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = ParentStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 5:
		    if (strcmp(CurrentToken->TokenText, ProbeStr) == 0) {

			/* We have a match.  Free TokenText and set it to PluginsStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = ProbeStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 6:
		    if (strcmp(CurrentToken->TokenText, ParentStr) == 0) {

			/* We have a match.  Free TokenText and set it to ParentStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = ParentStr;
			break;

		    }
		    if (strcmp(CurrentToken->TokenText, PluginStr) == 0) {

			/* We have a match.  Free TokenText and set it to PluginStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = PluginStr;
			break;

		    }
		    if (strcmp(CurrentToken->TokenText, PointsStr) == 0) {

			/* We have a match.  Free TokenText and set it to PointsStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = PointsStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 7:
		    if (strcmp(CurrentToken->TokenText, PluginsStr) == 0) {

			/* We have a match.  Free TokenText and set it to PluginsStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = PluginsStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		default:
		    Convert_To_String = TRUE;
		    break;
		}
		break;

	    case 'Q':		/* Query. Quit */

		if (strcmp(CurrentToken->TokenText, QueryStr) == 0) {

		    free(CurrentToken->TokenText);
		    CurrentToken->TokenText = QueryStr;
		    break;

		}

		if (strcmp(CurrentToken->TokenText, QuitStr) == 0) {

		    free(CurrentToken->TokenText);
		    CurrentToken->TokenText = QuitStr;
		    break;

		}

		Convert_To_String = TRUE;

		break;

	    case 'R':		/* Recursive, Region, Regions, Release, Remove, Rename, Revert */

		if (strcmp(CurrentToken->TokenText, RegionStr) == 0) {

		    /* We have a match.  Free TokenText and set it to RegionStr. */
		    free(CurrentToken->TokenText);
		    CurrentToken->TokenText = RegionStr;
		    break;

		}

		if (strcmp(CurrentToken->TokenText, RegionsStr) == 0) {

		    /* We have a match.  Free TokenText and set it to RegionsStr. */
		    free(CurrentToken->TokenText);
		    CurrentToken->TokenText = RegionsStr;
		    break;

		}

		if (strcmp(CurrentToken->TokenText, ReleaseStr) == 0) {

		    /* We have a match.  Free TokenText and set it to ReleaseStr. */
		    free(CurrentToken->TokenText);
		    CurrentToken->TokenText = ReleaseStr;
		    break;

		}

		if ((strcmp(CurrentToken->TokenText, RemoveStr) == 0) ||
		    (strcmp(CurrentToken->TokenText, "REM") == 0)) {

		    /* We have a match.  Free TokenText and set it to RemoveStr. */
		    free(CurrentToken->TokenText);
		    CurrentToken->TokenText = RemoveStr;
		    break;

		}

		if ((strcmp(CurrentToken->TokenText, RenameStr) == 0) ||
		    (strcmp(CurrentToken->TokenText, "REN") == 0)) {

		    /* We have a match.  Free TokenText and set it to RenameStr. */
		    free(CurrentToken->TokenText);
		    CurrentToken->TokenText = RenameStr;
		    break;

		}

		if ((strcmp(CurrentToken->TokenText, ReplaceStr) == 0) ||
		    (strcmp(CurrentToken->TokenText, "REP") == 0)) {

		    /* We have a match.  Free TokenText and set it to ReplaceStr. */
		    free(CurrentToken->TokenText);
		    CurrentToken->TokenText = ReplaceStr;
		    break;

		}

		if ((strcmp(CurrentToken->TokenText, RevertStr) == 0) ||
		    (strcmp(CurrentToken->TokenText, "REV") == 0)) {

		    /* We have a match.  Free TokenText and set it to RevertStr. */
		    free(CurrentToken->TokenText);
		    CurrentToken->TokenText = RevertStr;
		    break;

		}

		if (strcmp(CurrentToken->TokenText, RecursiveStr) == 0) {

		    /* We have a match.  Free TokenText and set it to RecursiveStr. */
		    free(CurrentToken->TokenText);
		    CurrentToken->TokenText = RecursiveStr;
		    break;

		}

		Convert_To_String = TRUE;

		break;

	    case 'S':		/* Seconds, Sectors, Segment, Segments, Set, Sh, Shrink, Shrinkable, Size, SP, Start, Save */

		switch (CurrentToken->TokenLength) {
		case 2:
		    if (strcmp(CurrentToken->TokenText, SPStr) == 0) {

			/* We have a match.  Free TokenText and set it to SPStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = SPStr;
			break;

		    }
		    if (strcmp(CurrentToken->TokenText, "SH") == 0) {

			/* We have a match.  Free TokenText and set it to ShrinkableStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = ShrinkableStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 3:
		    if (strcmp(CurrentToken->TokenText, SetStr) == 0) {

			/* We have a match.  Free TokenText and set it to SetStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = SetStr;
			break;

		    }
		    Convert_To_String = TRUE;	/* No match */
		    break;
		case 4:
		    if (strcmp(CurrentToken->TokenText, SizeStr) == 0) {

			/* We have a match.  Free TokenText and set it to SizeStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = SizeStr;
			break;

		    }
		    if (strcmp(CurrentToken->TokenText, SaveStr) == 0) {

			/* We have a match.  Free TokenText and set it to SaveStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = SaveStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 5:
		    if (strcmp(CurrentToken->TokenText, StartStr) == 0) {

			/* We have a match.  Free TokenText and set it to StartStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = StartStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 6:
		    if (strcmp(CurrentToken->TokenText, ShrinkStr) == 0) {

			/* We have a match.  Free TokenText and set it to ShrinkStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = ShrinkStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 7:
		    if (strcmp(CurrentToken->TokenText, SecondsStr) == 0) {

			/* We have a match.  Free TokenText and set it to SecondsStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = SecondsStr;
			break;

		    }
		    if (strcmp(CurrentToken->TokenText, SectorsStr) == 0) {

			/* We have a match.  Free TokenText and set it to SectorsStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = SectorsStr;
			break;

		    }
		    if (strcmp(CurrentToken->TokenText, SegmentStr) == 0) {

			/* We have a match.  Free TokenText and set it to SegmentStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = SegmentStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 8:
		    if (strcmp(CurrentToken->TokenText, SegmentsStr) == 0) {

			/* We have a match.  Free TokenText and set it to SegmentsStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = SegmentsStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 10:
		    if (strcmp(CurrentToken->TokenText, ShrinkableStr) ==
			0) {

			/* We have a match.  Free TokenText and set it to ShrinkableStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = ShrinkableStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		default:
		    Convert_To_String = TRUE;
		    break;
		}

		break;

	    case 'T':		/* Task, TB, TYPE */

		switch (CurrentToken->TokenLength) {
		case 2:
		    if (strcmp(CurrentToken->TokenText, TerrabyteStr) == 0) {

			/* We have a match.  Free TokenText and set it to TerrabyteStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = TerrabyteStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 4:
		    if (strcmp(CurrentToken->TokenText, TaskStr) == 0) {

			/* We have a match.  Free TokenText and set it to TaskStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = TaskStr;
			break;
		    }
		    if (strcmp(CurrentToken->TokenText, TypeStr) == 0) {

			/* We have a match.  Free TokenText and set it to TypeStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = TypeStr;
			break;
		    }
		    Convert_To_String = TRUE;
		    break;
		default:
		    Convert_To_String = TRUE;
		    break;
		}

		break;

	    case 'U':		/* Unclaimed, Unformat, US, Unmkfs */

		switch (CurrentToken->TokenLength) {
		case 2:
		    if (strcmp(CurrentToken->TokenText, "US") == 0) {

			/* We have a match.  Free TokenText and set it to MicrosecondsStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = MicrosecondsStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 6:
		    if (strcmp(CurrentToken->TokenText, UnMkfsStr) == 0) {

			/* We have a match.  Free TokenText and set it to UnMkfsStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = UnMkfsStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 7:
		    if (strcmp(CurrentToken->TokenText, UnmountStr) == 0) {

			/* We have a match.  Free TokenText and set it to UnmountStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = UnmountStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 8:
		    if (strcmp(CurrentToken->TokenText, UnformatStr) == 0) {

			/* We have a match.  Free TokenText and set it to UnformatStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = UnformatStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 9:
		    if (strcmp(CurrentToken->TokenText, UnclaimedStr) == 0) {

			/* We have a match.  Free TokenText and set it to UnclaimedStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = UnclaimedStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		default:
		    Convert_To_String = TRUE;
		    break;
		}

		break;

	    case 'V':		/* Volume, Volumes */

		switch (CurrentToken->TokenLength) {
		case 6:
		    if (strcmp(CurrentToken->TokenText, VolumeStr) == 0) {

			/* We have a match.  Free TokenText and set it to VolumeStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = VolumeStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		case 7:
		    if (strcmp(CurrentToken->TokenText, VolumesStr) == 0) {

			/* We have a match.  Free TokenText and set it to VolumesStr. */
			free(CurrentToken->TokenText);
			CurrentToken->TokenText = VolumesStr;
			break;

		    }
		    Convert_To_String = TRUE;
		    break;
		default:
		    Convert_To_String = TRUE;
		    break;
		}

		break;

	    default:
		/* Leave the TokenText alone but change its characterization to string. */
		Convert_To_String = TRUE;

		break;

	    }

	    if (Convert_To_String) {
		CurrentToken->Characterization = String;
		free(CurrentToken->TokenText);
		CurrentToken->TokenText = Original_Token_Text;
	    } else {
		free(Original_Token_Text);
	    }

	}

    }


    return CurrentToken;

}
