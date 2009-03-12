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
 * Module: error.h
 */

/*
 * Change History:
 *  06/2001     B. Rafanello  Initial version
 *
 */

/*
 *
 */

#ifndef ERROR_H

#define ERROR_H 1

#include "token.h"


/* Error Codes */
#define INVALID_OPTION 300
#define INTERNAL_ERROR_BOD  301      /* Internal Error - Bad Option Descriptor */
#define INVALID_TASK        302      /* The task specified by the user is invalid. */

/* Scanner Error Codes */
typedef enum {
               Bad_Command_File,                  /* Parameter1 = name of file that we could not open.  Other parameters are ignored. */
               Invalid_Character,                 /* Parameter1 = contents of Buffer, Parameter2 = invalid character, Column and Row are output */
               Invalid_Character_In_Substitution, /* Parameter1 = invalid character, Parameter2 = not used, Column and Row are output. */
               Invalid_Substitution,              /* Parameter1 = the susbstitution value, Parameter2 = not used, Colume and Row are output. */
               Scanner_Out_Of_Memory              /* Column and Row are used to indicate how far we got before quitting. */
             } Scanner_Errors;


typedef enum {
               Engine_Open_Failure,
               SetInput_Failure,
               Out_Of_Memory,
               Expecting_Command_Start,
               Expecting_Command_Name,
               Expecting_Command_Name_Or_File,
               Expecting_Command_File_Name,
               Expecting_Command_Separator,
               Expecting_Colon_Separator,
               Expecting_Comma_Separator,
               Expecting_Allocate_Or_Assign_Command,
               Expecting_Create_Or_Check_Command,
               Expecting_Delete_Command,
               Expecting_Format_Command,
               Expecting_Feature_Or_Segment_Keyword,
               Expecting_Manager_Keyword,
               Expecting_Filter_Identifier,
               Expecting_Expand_Command,
               Expecting_Probe_Command,
               Expecting_Shrink_Command,
               Expecting_Revert_Or_Rename,
               Expecting_Help_Command,
               Expecting_Query_Command,
               Expecting_Creation_Parameters,
               Expecting_Volume_Parameter,
               Expecting_Object_Parameter,
               Expecting_Container_Parameter,
               Expecting_Expand_Parameters,
               Expecting_Query_Parameters,
               Expecting_Query_Expand_Points_Keywords,
               Expecting_Query_Shrink_Points_Keywords,
               Expecting_Query_Extended_Info_Keywords,
               Expecting_EVMS_Identifier,
               Expecting_Plugins_Or_Parent_Parameter,
               Expecting_Volumes_Parameter,
               Expecting_Regions_Parameter,
               Expecting_Disks_Parameter,
               Expecting_Segments_Parameter,
               Expecting_Objects_Parameter,
               Expecting_List_Options_Keywords,
               Expecting_Acceptable_Parameter,
               Expecting_Volume_Or_Object,
               Expecting_Containers_Or_Children_Parameter,
               Expecting_Help_Parameters,
               Expecting_Plugin_Type_Parameter,
               Expecting_Feature_Identifier,
               Expecting_Plugin_Identifier,
               Expecting_Object_Identifier,
               Expecting_Region_Or_Object,
               Expecting_Offset,
               Expecting_Start_KeyWord,
               Expecting_End_KeyWord,
               Expecting_Offset_Parameters,
               Expecting_Size_Parameters,
               Expecting_Size_Value,
               Expecting_EQ_Keyword,
               Expecting_LT_Keyword,
               Expecting_GT_Keyword,
               Expecting_GT_LT_Parameter,
               Expecting_Units_Parameter,
               Expecting_Disk_Units_Parameter,
               Expecting_Number_Or_String,
               Expecting_Number,
               Expecting_Name,                                          /* Name = "string" */
               Expecting_Field_Name,                                    /* Expecting an extended info field name. */
               Expecting_Name_Value,                                    /* Name portion of Name Value Pair */
               Expecting_Equal_Sign_Separator,
               Expecting_Open_Brace_Separator,
               Expecting_Closing_Brace_Separator,
               Expecting_Open_Paren_Separator,
               Expecting_Closing_Paren_Separator,
               Expecting_Name_Value_Pair,
               Expecting_Query_Volume_Parameters,
               Expecting_Query_Plugin_Parameters,
               Expecting_Query_Region_Parameters,
               Expecting_Query_Acceptable_Parameters,
               Expecting_Query_Acceptable_Create,
               Expecting_Region_Creation_Parameters,
               Expecting_Object_List,
               Expecting_Query_Disks_Parameters,
               Expecting_Volume_Identifier,
               Expecting_Region_Identifier,
               Expecting_Container_Identifier,
               Expecting_Unclaimed_Identifier,
               Expecting_Disk_Identifier,
               Expecting_Shrink_Parameters,
               Expecting_Freespace_Identifier,
               Expecting_Expandable_Shrinkable_Identifier,
               Expecting_Name_Or_Compatibility,
               Expecting_Compatibility_Keyword,
               Expecting_Commit_Parameters,
               Internal_Error_Encountered,
               Wrong_Filter,
               Duplicate_Filter,
               Bad_Command_File_Name
             } Parser_Errors;

/* Error reporting functions. */
void Report_Scanner_Error(Scanner_Errors Error, char * Parameter1, char * Parameter2, uint Column, uint Row );
void Report_Parser_Error(Parser_Errors Error, char * Filename, TokenType * Token);
void Report_Standard_Error(int Error);


#endif
