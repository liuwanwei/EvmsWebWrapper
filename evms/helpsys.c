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
 * Module: helpsys.c
 */

/*
 * Change History:
 *
 * 10/2001  B. Rafanello  Initial version.
 *
 */

/*
 *
 */

/* Identify this file. */
#define HELPSYS_C


/*--------------------------------------------------
 * Necessary include files
 --------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include "helpsys.h"
#include "evms.h"
#include "screener.h"

/*--------------------------------------------------
 * Private constants
 --------------------------------------------------*/


/*--------------------------------------------------
 * Private types
 --------------------------------------------------*/


/*--------------------------------------------------
 * Private variables
 --------------------------------------------------*/


/*--------------------------------------------------
 * Private Function Prototypes
 --------------------------------------------------*/
static void Do_Activate_Help(void);
static void Do_Allocate_Help(void);
static void Do_Assign_Help(void);
static void Do_Add_Feature_Help(void);
static void Do_Commit_Help(void);
static void Do_Convert_Help(void);
static void Do_Create_Help(void);
static void Do_Deactivate_Help(void);
static void Do_Delete_Help(void);
static void Do_Echo_Help(void);
static void Do_Exit_Help(void);
static void Do_Expand_Help(void);
static void Do_Probe_Help(void);
static void Do_Query_Help(void);
static void Do_Quit_Help(void);
static void Do_Remove_Help(void);
static void Do_Rename_Help(void);
static void Do_Replace_Help(void);
static void Do_Revert_Help(void);
static void Do_Shrink_Help(void);
static void Do_Default_Help(void);
static void Do_Set_Help(void);
static void Do_Check_Help(void);
static void Do_Format_Help(void);
static void Do_Unformat_Help(void);
static void Do_Task_Help(void);
static void Do_ASM_Help(void);
static void Do_Save_Help(void);
static void Do_Mkfs_Help(void);
static void Do_Modify_Help(void);
static void Do_UnMkfs_Help(void);
static void Do_Mount_Help(void);
static void Do_Unmount_Help(void);


/*--------------------------------------------------
 * Public Functions Available
 --------------------------------------------------*/
void Display_Help(Executable_Node * Current_Node)
{
    if (Current_Node->NodeValue == ActivateStr) {
	Do_Activate_Help();
    } else if (Current_Node->NodeValue == AllocateStr) {
	Do_Allocate_Help();
    } else if (Current_Node->NodeValue == AFStr) {
	Do_Add_Feature_Help();
    } else if (Current_Node->NodeValue == AssignStr) {
	Do_Assign_Help();
    } else if (Current_Node->NodeValue == CommitStr) {
	Do_Commit_Help();
    } else if (Current_Node->NodeValue == ConvertStr) {
	Do_Convert_Help();
    } else if (Current_Node->NodeValue == CreateStr) {
	Do_Create_Help();
    } else if (Current_Node->NodeValue == DeactivateStr) {
	Do_Deactivate_Help();
    } else if (Current_Node->NodeValue == DeleteStr) {
	Do_Delete_Help();
    } else if (Current_Node->NodeValue == EchoStr) {
	Do_Echo_Help();
    } else if (Current_Node->NodeValue == ExitStr) {
	Do_Exit_Help();
    } else if (Current_Node->NodeValue == ExpandStr) {
	Do_Expand_Help();
    } else if (Current_Node->NodeValue == ShrinkStr) {
	Do_Shrink_Help();
    } else if (Current_Node->NodeValue == RemoveStr) {
	Do_Remove_Help();
    } else if (Current_Node->NodeValue == RenameStr) {
	Do_Rename_Help();
    } else if (Current_Node->NodeValue == ReplaceStr) {
	Do_Replace_Help();
    } else if (Current_Node->NodeValue == RevertStr) {
	Do_Revert_Help();
    } else if (Current_Node->NodeValue == ProbeStr) {
	Do_Probe_Help();
    } else if (Current_Node->NodeValue == QueryStr) {
	Do_Query_Help();
    } else if (Current_Node->NodeValue == QuitStr) {
	Do_Quit_Help();
    } else if (Current_Node->NodeValue == SetStr) {
	Do_Set_Help();
    } else if (Current_Node->NodeValue == CheckStr) {
	Do_Check_Help();
    } else if (Current_Node->NodeValue == FormatStr) {
	Do_Format_Help();
    } else if (Current_Node->NodeValue == UnformatStr) {
	Do_Unformat_Help();
    } else if (Current_Node->NodeValue == TaskStr) {
	Do_Task_Help();
    } else if (Current_Node->NodeValue == AsmStr) {
	Do_ASM_Help();
    } else if (Current_Node->NodeValue == SaveStr) {
	Do_Save_Help();
    } else if (Current_Node->NodeValue == MkfsStr) {
	Do_Mkfs_Help();
    } else if (Current_Node->NodeValue == ModifyStr) {
	Do_Modify_Help();
    } else if (Current_Node->NodeValue == UnMkfsStr) {
	Do_UnMkfs_Help();
    } else if (Current_Node->NodeValue == MountStr) {
	Do_Mount_Help();
    } else if (Current_Node->NodeValue == UnmountStr) {
	Do_Unmount_Help();
    } else {
	Do_Default_Help();
    }

    return;
}


/*--------------------------------------------------
 * Private Functions Available
 --------------------------------------------------*/
static void Do_Activate_Help(void)
{
    My_Printf("The Activate Command\n"
	      "The Activate command is used to activate volumes or storage objects that are not active.  "
	      "If any of the storage objects that the specified volume or storage object comprises "
	      "are not active, they will be activated so that the specified volume or storage object can "
	      "be activated.\n\n"
	      "The Activate command has the following form:\n\n");

    Indent_Printf(1, FALSE, "Activate : <name>\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "<name> is the name of a volume or storage object.\n");

    Indent_Printf(1, FALSE,
		  "Note:  'Activate' may be abbreviated as 'Act' in commands.\n\n");

    Indent_Printf(1, FALSE,
		  "Example:  Given a volume /dev/evms/mydata that is not active, "
		  "activate the volume.\n\n");

    Indent_Printf(1, FALSE, "act:/dev/evms/mydata\n\n");

    return;
}

static void Do_Allocate_Help(void)
{
    My_Printf("The Allocate Command\n"
	      "The Allocate command is used to allocate regions or segments from blocks of freespace.  "
	      "It has the following form:\n\n");

    Indent_Printf(1, FALSE,
		  "Allocate : <freespace> , <name> = <value>\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "<freespace> is the name of a region or segment which represents "
		  "freespace (ex. sdb_freespace1).\n");
    Indent_Printf(1, FALSE,
		  "<name> is the name of an option accepted by the Region Manager or "
		  "Segment Manager which created <freespace>.\n");
    Indent_Printf(1, FALSE,
		  "<value> is an acceptable value for the option <name>.\n\n");

    Indent_Printf(1, FALSE,
		  "Example:  Given a disk with no partitions, which is under the control of the "
		  "DOS Segment Manager, create a 50 MB segment in an extended partition.  "
		  "The new segment does not have to be bootable.\n\n");

    Indent_Printf(1, FALSE,
		  "a:sdb_freespace1, size=50MB, offset=0, primary=false, bootable=false\n\n");

    return;
}

static void Do_Assign_Help(void)
{
    My_Printf("The Assign Command\n\n"
	      "The Assign command is used to assign a Segment Manager to any disk or segment "
	      "in the system which does not already have a Segment Manager assigned to it.  "
	      "The Assign command may be abbreviated to 'as' on the command line.\n\n"
	      "The Assign command has the following form:\n\n");

    Indent_Printf(1, FALSE,
		  "Assign : <Segment Mgr Name> = { <name> = <value> , <name> = <value> ... } , "
		  "<Disk Name>\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "<Segment Mgr Name> is the name of the Segment Manager that is to be assigned "
		  "to a disk.\n");
    Indent_Printf(1, FALSE,
		  "<name> is the name of an option accepted by the Segment Manager.\n");
    Indent_Printf(1, FALSE,
		  "<value> is an acceptable value for option <name>.\n");
    Indent_Printf(1, FALSE,
		  "<Disk Name> is the name of the disk which is to have the Segment Manager "
		  "assigned to it.\n\n");

    Indent_Printf(1, FALSE,
		  "Example:  You install a brand new disk into your computer.  "
		  "You wish to partition this disk into multiple partitions using "
		  "the DOS Segment Manager.  "
		  "What must you do before you can partition the disk?\n\n");

    My_Printf("You must assign the DOS Segment Manager to the disk.  "
	      "If EVMS identifies the disk as sdb, then the command to do this would be:\n\n");

    Indent_Printf(1, FALSE, "as:DosSegMgr={}, sdb\n\n");

    My_Printf
	("This command will cause the DOS Segment Manager to assume control of the disk sdb "
	 "and prepare the disk for partitioning.  "
	 "As a result of this, sdb_freespace1 will be created to represent the space available "
	 "on sdb, and this space can be divided into segments(partitions) using the Allocate "
	 "or Create commands.\n\n");
    return;
}


static void Do_ASM_Help(void)
{
    My_Printf("The Add Segment Manager Command\n\n"
	      "The Add Segment Manager command is used to add a Segment Manager to any "
	      "disk or segment in the system which does not already have a Segment Manager.  "
	      "The Add Segment Manager command may be abbreviated as 'asm' on the command line.\n\n"
	      "The Add Segment Manager command has the following form:\n\n");

    Indent_Printf(1, FALSE,
		  "Add Segment Manager : <Segment Mgr Name> = { <name> = <value> , "
		  "<name> = <value> ... } , <Disk Name>\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "<Segment Mgr Name> is the name of the Segment Manager that is to be "
		  "added to a disk.\n");
    Indent_Printf(1, FALSE,
		  "<name> is the name of an option accepted by the Segment Manager.\n");
    Indent_Printf(1, FALSE,
		  "<value> is an acceptable value for option <name>.\n");
    Indent_Printf(1, FALSE,
		  "<Disk Name> is the name of the disk which is to have the Segment Manager "
		  "assigned to it.\n\n");

    Indent_Printf(1, FALSE,
		  "Example:  You install a brand new disk into your computer.  "
		  "You wish to partition this disk into multiple partitions using "
		  "the DOS Segment Manager.  "
		  "What must you do before you can partition the disk?\n\n");

    My_Printf("You must assign the DOS Segment Manager to the disk.  "
	      "If EVMS identifies the disk as sdb, then the command to do this would be:\n\n");

    Indent_Printf(1, FALSE, "asm:DosSegMgr={}, sdb\n\n");

    My_Printf
	("This command will cause the DOS Segment Manager to assume control "
	 "of the disk sdb, store My_Disk as the name of the disk, and prepare "
	 "the disk for partitioning.  "
	 "As a result of this, sdb_freespace1 will be created to represent "
	 "the space available on sdb, and this space can be divided into "
	 "segments(partitions) using the Allocate or Create commands.\n\n");

    return;
}


static void Do_Commit_Help(void)
{
    My_Printf("The Commit Command\n\n"
	      "The EVMS Command Line Interpreter can be run in either of two modes.  "
	      "The default mode is to write to disk any changes made by a command immediately "
	      "after that command is executed.  "
	      "The alternate mode, which is invoked by using the -c option on the command line "
	      "which invoked the EVMS Command Line Interpreter, is to hold all changes in memory "
	      "until the EVMS Command Line Interpreter exits, at which time all of the changes "
	      "being held in memory are written to disk.  "
	      "This alternate mode can significantly improve performance in situations where "
	      "large command files are being executed, or where complex configurations are being used.  "
	      "However, it is not always desirable to hold all changes in memory until "
	      "the EVMS Command Line Interpreter exits.  "
	      "It is for these situations that the Commit Command exists.  "
	      "When the EVMS Command Line Interpreter is operating in its default mode, "
	      "the Commit Command does nothing.  "
	      "When the EVMS Command Line Interpreter is operating in its alternate mode, "
	      "then the Commit Command can be used to force the EVMS Command Line Interpreter to "
	      "save all of the changes that it has accumulated in memory up to that point.  "
	      "This effectively gives the user control over when changes are saved and when "
	      "they are held in memory.\n\n"
	      "The Commit command has the following form:\n\n");

    Indent_Printf(1, FALSE, "Commit <Optional Parameter>\n\n" "where:\n");
    Indent_Printf(1, FALSE,
		  "<Optional Parameter> if it exists, is a colon followed by either the "
		  "'Hold' or 'Release' keywords.  "
		  "The 'Hold' keyword tells the EVMS Command Line Interpreter to hold "
		  "all changes in memory and to commit those changes to disk when "
		  "the EVMS Command Line Interpreter exits.  "
		  "The 'Release' keyword tells the EVMS Command Line Interpreter to write "
		  "all pending changes to disk immediately, and write changes to disk "
		  "after each command has been completed.\n\n");

    Indent_Printf(1, FALSE,
		  "NOTE: There is no abbreviation for the Commit command.\n\n");

    Indent_Printf(1, FALSE,
		  "Example:  The EVMS Command Line Interpreter is operating in its default mode, "
		  "with changes being committed to disk after each command.  "
		  "Change the interpreter's mode of operation so that changes are held in memory "
		  "until the interpreter exits.\n\n");

    Indent_Printf(1, FALSE, "commit:hold\n\n");

    Indent_Printf(1, FALSE,
		  "Example:  The EVMS Command Line Interpreter is operating in its alternate mode, "
		  "with all changes being held in memory until the EVMS Command Line Interpreter "
		  "exits.  "
		  "Change the EVMS Command Line Interpreter's mode of operation so that changes "
		  "are committed to disk after each command, and write all changes which are "
		  "currently being held in memory to disk.\n\n");

    Indent_Printf(1, FALSE, "commit:release\n\n");

    Indent_Printf(1, FALSE,
		  "Example:  The EVMS Command Line Interpreter is operating in its alternate mode, "
		  "with all changes being held in memory until the EVMS Command Line Interpreter "
		  "exits.  "
		  "Force the EVMS Command Line Interpreter to commit to disk all the changes "
		  "it is currently holding in memory without changing the mode that the interpreter "
		  "is operating in.\n\n");

    Indent_Printf(1, FALSE, "commit\n\n");

    return;

}


static void Do_Save_Help(void)
{
    My_Printf("The Save Command\n\n"
	      "The EVMS Command Line Interpreter can be run in either of two modes.  "
	      "The default mode is to write to disk any changes made by a command immediately "
	      "after that command is executed.  "
	      "The alternate mode, which is invoked by using the -c option on the command line "
	      "which invoked the EVMS Command Line Interpreter, is to hold all changes in memory "
	      "until the EVMS Command Line Interpreter exits, at which time all of the changes "
	      "being held in memory are written to disk.  "
	      "This alternate mode can significantly improve performance in situations where "
	      "large command files are being executed, or where complex configurations are being used.  "
	      "However, it is not always desirable to hold all changes in memory until "
	      "the EVMS Command Line Interpreter exits.  "
	      "It is for these situations that the Save command exists.  "
	      "When the EVMS Command Line Interpreter is operating in its default mode, "
	      "the Save command does nothing.  "
	      "When the EVMS Command Line Interpreter is operating in its alternate mode, "
	      "then the Save command can be used to force the EVMS Command Line Interpreter "
	      "to save all of the changes that it has accumulated in memory up to that point.  "
	      "This effectively gives the user control over when changes are saved and "
	      "when they are held in memory.\n\n"
	      "The Save command has the following form:\n\n");

    Indent_Printf(1, FALSE, "Save <Optional Parameter>\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "<Optional Parameter> if it exists, is a colon followed by either the "
		  "'Hold' or 'Release' keywords.  "
		  "The 'Hold' keyword tells the EVMS Command Line Interpreter to hold "
		  "all changes in memory and to save those changes to disk when "
		  "the EVMS Command Line Interpreter exits.  "
		  "The 'Release' keyword tells the EVMS Command Line Interpreter to write "
		  "all pending changes to disk immediately, and write changes to disk "
		  "after each command has been completed.\n\n");

    Indent_Printf(1, FALSE,
		  "NOTE: There is no abbreviation for the Save command.\n\n");

    Indent_Printf(1, FALSE,
		  "Example:  The EVMS Command Line Interpreter is operating in its default mode, "
		  "with changes being saved to disk after each command.  "
		  "Change the interpreter's mode of operation so that changes are held in memory "
		  "until the interpreter exits.\n\n");

    Indent_Printf(1, FALSE, "save:hold\n\n");

    Indent_Printf(1, FALSE,
		  "Example:  The EVMS Command Line Interpreter is operating in its alternate mode, "
		  "with all changes being held in memory until the EVMS Command Line Interpreter "
		  "exits.  "
		  "Change the EVMS Command Line Interpreter's mode of operation so that changes "
		  "are saved to disk after each command, and write all changes which are currently "
		  "being held in memory to disk.\n\n");
    Indent_Printf(1, FALSE, "save:release\n\n");

    Indent_Printf(1, FALSE,
		  "Example:  The EVMS Command Line Interpreter is operating in its alternate mode, "
		  "with all changes being held in memory until the EVMS Command Line Interpreter "
		  "exits.  "
		  "Force the EVMS Command Line Interpreter to save to disk all the changes "
		  "it is currently holding in memory without changing the mode that the interpreter "
		  "is operating in.\n\n");

    Indent_Printf(1, FALSE, "save\n\n");

    return;
}


static void Do_Create_Help(void)
{
    My_Printf("The Create Command\n\n"
	      "The Create command is used to create volumes, EVMS objects, "
	      "regions, segments, and containers.  "
	      "It can be abbreviated as the letter 'c' on the command line.  "
	      "The different items that it can create can also be abbreviated "
	      "as a single letter on the command line.\n\n"
	      "The Create command has several forms:\n\n");

    Row_Delimiter('-', NULL);

    My_Printf("for creating containers:\n\n");

    Indent_Printf(1, FALSE,
		  "Create : Container , <Plug-in Name> = { <name> = <value> , "
		  "<name> = <value> ... } , <Segment or Disk name> , "
		  "<Segment or Disk name>, ...\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "<Plug-in Name> is the name of the EVMS plug-in to use "
		  "when creating the container.\n");
    Indent_Printf(1, FALSE,
		  "<name> is the name of an option accepted by the plug-in.\n");
    Indent_Printf(1, FALSE,
		  "<value> is an acceptable value for option <name>.\n");
    Indent_Printf(1, FALSE,
		  "<Segment or Disk name> is the name of a segment or disk which is "
		  "to be used in the creation of the container.\n\n");

    Indent_Printf(1, FALSE,
		  "Example:  Given a system with three available hard drives (sdc, sdd, hdc), "
		  "use the EVMS LVM Region Manager to combine these disks into a container "
		  "called Sample Container with a PE size of 16KB.\n\n");

    Indent_Printf(1, FALSE,
		  "c:c, LvmRegMgr={name=\"Sample Container\", pe_size=16KB}, sdc, sdd, hdc\n\n");

    Row_Delimiter('-', NULL);

    My_Printf("for creating EVMS objects:\n\n");

    Indent_Printf(1, FALSE,
		  "Create : Object , <Feature Name> = { <name> = <value> , <name> = <value> ...} , "
		  "<Backing Store> , <Backing Store> ...\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "<Feature Name> is the name of the EVMS Feature to use when creating "
		  "the EVMS object.\n");
    Indent_Printf(1, FALSE,
		  "<name> is the name of an option accepted by the Feature.\n");
    Indent_Printf(1, FALSE,
		  "<value> is an acceptable value for option <name>.\n");
    Indent_Printf(1, FALSE,
		  "<Backing Store> is the name of a region, segment, disk, or EVMS object "
		  "to be used in the creation of the new EVMS object.\n\n");

    Indent_Printf(1, FALSE,
		  "Example:  Given a system with a volume /dev/evms/Source, and an available "
		  "segment sdb5, create a snapshot object called My Snapshot Object using "
		  "the EVMS Snapshot feature.  /dev/evms/Source is the source of the snapshot, "
		  "and sdb5 is the backing store for the snapshot.\n\n");

    Indent_Printf(1, FALSE,
		  "c:o, Snapshot={original=/dev/evms/Source, snapshot=\"My Snapshot Object\", writeable=false}, sdb5\n\n");

    Row_Delimiter('-', NULL);

    My_Printf("for creating regions:\n\n");

    Indent_Printf(1, FALSE,
		  "Create : Region , <Region Mgr Name> = { <name> = <value> , <name> = <value> ...} , "
		  "<Backing Store> , <Backing Store> ...\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "<Region Mgr Name> is the name of the EVMS Region Manager to use.\n");
    Indent_Printf(1, FALSE,
		  "<name> is the name of an option accepted by the Region Manager.\n");
    Indent_Printf(1, FALSE,
		  "<value> is an acceptable value for option <name>.\n");
    Indent_Printf(1, FALSE,
		  "<Backing Store> is the name of a region, segment, or disk to be used "
		  "in the creation of the new region\n\n");

    Indent_Printf(1, FALSE,
		  "Example:  Given a system with three available segments, hda5, hdb9, hdc2, "
		  "combine these segments into a single region using the MD RAID5 Region Manager.\n\n");

    Indent_Printf(1, FALSE,
		  "c:r, MDRaid5RegMgr={chunksize=32, level=5, algorithm=\"Left Asymmetric\"}, hda5, hdb9, hdc2\n\n");

    Row_Delimiter('-', NULL);

    My_Printf("for creating segments:\n\n");

    Indent_Printf(1, FALSE,
		  "Create: Segment , <Freespace Segment> , <name> = <value> , <name> = <value> ...\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "<Freespace Segment> is the name of a segment representing freespace.\n");
    Indent_Printf(1, FALSE,
		  "<name> is the name of an option accepted by the Segment Manager that created "
		  "<Freespace Segment>.\n");
    Indent_Printf(1, FALSE,
		  "<value> is an acceptable value for option <name>.\n\n");

    Indent_Printf(1, FALSE,
		  "Example:  Given a disk which is under the control of the DOS Segment Manager, "
		  "and which has no primary partitions, and which has a 200 MB freespace segment "
		  "called sdb_freespace1, create a 50 MB logical segment.  "
		  "The new segment does not have to be bootable.\n\n");

    My_Printf
	("c:s, sdb_freespace1, size=50MB, offset=0, primary=false, bootable=false\n\n");

    Row_Delimiter('-', NULL);

    My_Printf("for creating compatibility volumes:\n\n");

    Indent_Printf(1, FALSE,
		  "Create : Volume , <Segment or Region> , Compatibility\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "<Segment or Region> is the name of the segment or region which is to become "
		  "a volume.\n\n");

    Indent_Printf(1, FALSE,
		  "Example:  Given a system with an unused segment, sda3, turn this segment into "
		  "a compatibility volume.\n\n");

    My_Printf("c:v, sda3, c\n\n");

    Row_Delimiter('-', NULL);

    My_Printf("for creating EVMS volumes:\n\n");

    Indent_Printf(1, FALSE,
		  "Create : Volume , <Backing Store> , Name = <User Defined Name>\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "<Backing Store> is any segment, disk, region, or EVMS object which is not "
		  "already a volume, or a part of volume or storage object.\n");
    Indent_Printf(1, FALSE,
		  "<User Defined Name> is the name that the user would like the volume to have.  "
		  "This name must be unique throughout the system, and the volume, "
		  "once created, will be known as /dev/evms/<User Defined Name>.\n\n");

    Indent_Printf(1, FALSE,
		  "Example:  Given a system with an unused segment sda3, make sda3 an EVMS "
		  "volume knows as \"Widow Maker\".\n\n");

    Indent_Printf(1, FALSE, "c:v, sda3, n=\"Widow Maker\"\n\n");

    return;
}

static void Do_Deactivate_Help(void)
{
    My_Printf("The Deactivate Command\n"
	      "The Deactivate command is used to deactivate volumes or storage objects that are active.  "
	      "When a storage object is deactivated, all of the storage objects and volumes that are built "
	      "using the specified storage object will also be deactivated, since active objects and volumes "
	      "cannot be built from inactive objects.\n\n"
	      "The Deactivate command has the following form:\n\n");

    Indent_Printf(1, FALSE, "Deactivate : <name>\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "<name> is the name of a volume or storage object.\n");

    Indent_Printf(1, FALSE,
		  "Note:  'Deactivate' may be abbreviated as 'Deact' in commands.\n\n");

    Indent_Printf(1, FALSE,
		  "Example:  Given a volume /dev/evms/mydata that is active, "
		  "deactivate the volume.\n\n");

    Indent_Printf(1, FALSE, "deact:/dev/evms/mydata\n\n");

    return;
}

static void Do_Delete_Help(void)
{
    My_Printf("The Delete Command\n\n"
	      "The Delete command deletes a volume, EVMS object, region, or segment "
	      "from the system.  "
	      "The delete command may be abbreviated to the single letter 'd' on "
	      "the command line.\n\n"
	      "The Delete command has two forms.  "
	      "The following form is used to tell the delete command to only delete "
	      "the specified item, leaving any children of the specified item intact:\n\n");

    Indent_Printf(1, FALSE, "Delete : <Item Name>\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "<Item Name> is the name of the volume, EVMS object, region, or segment that is to be deleted.\n\n");

    My_Printf
	("The second form of the Delete command is used to delete the specified item and "
	 "all of its children:\n\n");

    Indent_Printf(1, FALSE, "Delete Recursive : <Item Name>\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "<Item Name> is the name of the volume, object, region, or segment that is "
		  "to be deleted.\n");
    Indent_Printf(1, FALSE,
		  "Note:  The Delete Recursive command may be abbreviated as 'DR'.\n\n");

    My_Printf("Examples:\n\n"
	      "Given a volume called /dev/evms/Fred, which is constructed from the storage "
	      "object DL1, delete the volume /dev/evms/Fred while leaving DL1 intact.  "
	      "The volume /dev/evms/Fred is currently not mounted.\n\n"
	      "d:/dev/evms/Fred\n\n"
	      "Given a volume called /dev/evms/Fred, which is constructed from the storage "
	      "object DL1, which is constructed from sda1, sdb1, and sdc1 using drive linking, "
	      "delete the volume /dev/evms/Fred and all of its components (DL1, sda1, sdb1, sdc1).  "
	      "The volume /dev/evms/Fred is currently not mounted.\n\n"
	      "dr:/dev/evms/Fred\n\n"
	      "To delete an unused segment called sdb7, use:\n\n"
	      "d:sdb7\n\n"
	      "To delete an unused storage object called snap, use:\n\n"
	      "d:snap\n\n"
	      "To delete an empty container (i.e. - it exports no regions) called "
	      "lvm/Kevins_Stuff, use:\n\n"
	      "d:lvm/Kevins_Stuff\n\n"
	      "To delete an unused region called Temp, use:\n\n"
	      "d:Temp\n\n");

    return;
}

static void Do_Expand_Help(void)
{
    My_Printf("The Expand Command\n\n"
	      "The Expand command is used to increase the size of a volume, storage object, or "
	      "storage container.\n\n"
	      "A volume, storage object, or storage container may comprise of one or more EVMS objects, "
	      "regions, segments, or disks.  Whether or not a volume, storage object, or "
	      "storage container can be expanded depends upon how it is constructed.  "
	      "For example, if a volume consists of a single segment, then whether or not it "
	      "can be expanded depends upon whether or not the segment manager which created "
	      "that segment can increase the size of that segment.  "
	      "A more complicated volume may have several ways to expand.  "
	      "For example, a volume created from several segments using EVMS Drive Linking may be "
	      "expanded by increasing the size of the last segment linked to form the volume, "
	      "or by using EVMS Drive Linking to add another segment to the volume, or both.  "
	      "In this case, we say that the volume has multiple expansion points, because there are "
	      "multiple ways in which the volume can be expanded.  "
	      "EVMS allows the user complete control over how a volume is expanded.  "
	      "This means that, to expand a volume, the user must specify which expansion point is "
	      "to be used to expand the volume.  "
	      "Thus, in our last example, there were two expansion points.  "
	      "One was the storage object created by EVMS Drive Linking.  "
	      "This storage object could be expanded by adding another segment to it.  "
	      "The second expansion point would be the last segment used in the storage object formed by "
	      "EVMS Drive Linking.  "
	      "(While the other segments used to create this storage object may be capable of being "
	      "expanded, EVMS Drive Linking prohibits them from expanding as it is not set up to handle "
	      "that case.)\n\n"
	      "Similar logic is true for storage objects and for storage containers.  "
	      "A top level storage object may be expanded by expanding the object itself or by "
	      "expanding one of its descendents.  "
	      "A storage container can be expanded by adding an object to the container or by "
	      "expanding an object in the container if the plug-in allows.\n\n"
	      "To find the expansion points for a volume, storage object, or storage container, "
	      "use the Query Expand Points command.  "
	      "Once the expansion points for a volume, storage object, or storage container are known, "
	      "they can be used with this command to expand the volume, storage object, or "
	      "storage_container.\n\n"
	      "The Expand command has the following form:\n\n");

    Indent_Printf(1, FALSE,
		  "Expand : <Expansion Point> , <Name> = <Value> , ... , <Name> = <Value> , "
		  "<Storage to use> , ... , <Storage to use>\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "Expand may be abbreviated as the single letter 'E'.\n");
    Indent_Printf(1, FALSE,
		  "<Expansion Point> is the name of an expansion point as provided by the "
		  "QueryExpand Points command.\n");
    Indent_Printf(1, FALSE,
		  "<Name> is the name of an option supported by the plug-in module that "
		  "controls <Expansion Point>.\n");
    Indent_Printf(1, FALSE,
		  "<Value> is an acceptable value for option <Name>.\n");
    Indent_Printf(1, FALSE,
		  "<Storage to use> is the name of an acceptable EVMS object, region, "
		  "segment, freespace, or disk to use for expanding the volume.\n\n");

    Indent_Printf(1, FALSE,
		  "Example:  Given a volume created from three segments using EVMS Drive Linking.  "
		  "The three segments used are sda1, sdb1, and sdc1.  "
		  "The storage object created by EVMS Drive Linking is called DL1, "
		  "and the volume is called /dev/evms/Sample_Volume.  "
		  "The segment sdc1 is controlled by the DOS Segment Manager (DosSegMgr), "
		  "and has a 50 MB block of freespace (sdc_freespace1) adjacent to it on disk.  "
		  "Also, there is an unused 200MB segment known as hda7 available in the system.  "
		  "The Query Expand Points command indicates that this volume has two expansion points:  "
		  "DL1 and sdc1.  " "Expand this volume by 250MB.\n\n");

    My_Printf
	("First, we must consider the order in which we will use the expand points to expand "
	 "the volume.  " "Currently, sdc1 is an expand point.  "
	 "Since EVMS Drive Linking allows only the last segment it is linking to be expanded, "
	 "sdc1 must be the last segment that EVMS Drive Linking is using to create DL1.  "
	 "If we add hda7 to DL1, then hda7 will become the last segment used in DL1, "
	 "and sdc1 will not be an expansion point anymore.  "
	 "Without being able to expand sdc1, we will not be able to reach our goal of expanding "
	 "the volume by 250MB.  "
	 "However, expanding sdc1 has no affect on our ability to add hda7 to DL1, "
	 "so if we expand sdc1 before we add hda7 to DL1, we will be able to achieve our goal "
	 "of expanding the volume by 250MB.  "
	 "This will require two commands to accomplish.  "
	 "First, to expand sdc1, we use:\n\n"
	 "e:sdc1,size=50MB,sdc_freespace1\n\n"
	 "Now, to add hda7 to DL1, use the command:\n\n" "e:DL1,hda7\n\n"
	 "Now the volume will be 250MB larger than before.\n\n");

    return;
}

static void Do_Probe_Help(void)
{
    My_Printf("The Probe Command\n\n"
	      "The Probe command is used to probe the system for hardware changes.  "
	      "This is useful when the medium in a removable media device has been changed.  "
	      "The Probe command will commit any pending changes (from prior commands) "
	      "to disk before probing the system for hardware changes, even if the -c "
	      "option was used when invoking the EVMS Command Line Interpreter.\n\n"
	      "The form of the Probe command is:\n\n");

    Indent_Printf(1, FALSE, "Probe\n\n");

    Indent_Printf(1, FALSE,
		  "NOTE: The Probe command may be abbreviated as a single letter 'P'.\n\n");
    return;
}

static void Do_Query_Help(void)
{
    My_Printf("The Query Command\n\n"
	      "The Query command is used to obtain information about the state of EVMS and "
	      "the volumes, EVMS objects, regions, containers, segments, and disks in the system.  "
	      "The Query command has several different forms.  "
	      "One is used for gathering information about the volumes, EVMS objects, regions, "
	      "containers, segments, disks, and plug-in modules in the system.  "
	      "The other forms of the Query command are used for gathering information related to "
	      "various tasks that may be performed by the user on items in the system.\n\n"
	      "For gathering information about Volumes, EVMS Objects, Regions, Containers, "
	      "Segments, Disks, or Plug-in Modules, use this form of the Query command:\n\n");

    Indent_Printf(1, FALSE,
		  "Query : <Type of data to return> , <filter> , ... , <filter>\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "<Type of data to return> is either:\n"
		  "Plugins      (abbreviated as P)\n"
		  "Volumes      (abbreviated as V)\n"
		  "Regions      (abbreviated as R)\n"
		  "EVMS Objects (abbreviated as O)\n"
		  "Containers   (abbreviated as C)\n"
		  "Disks        (abbreviated as D)\n"
		  "Segments     (abbreviated as S)\n");
    Indent_Printf(1, FALSE,
		  "<filter> is one of the available filters for the type of data being returned.  "
		  "A filter restricts what is returned based upon some characteristic of "
		  "the items being returned.  "
		  "The available filters for a query depend upon <Type of data to return>.\n\n");

    My_Printf
	("When processing a query of this type, the Command Line Interpreter first "
	 "gathers all items in the system of <Type of data to return>, "
	 "puts them into a list, called the Return Values List, and then lets each of "
	 "the filters specified by the user examine the Return Values List and remove "
	 "any items they do not like.  "
	 "Once all of the filters have processed the Return Values List, "
	 "the Command Line Interpreter will display whatever items remain in the "
	 "Return Values List.\n\n"
	 "A note about filters:  Not all filters will work with every query.  "
	 "Furthermore, some filters may be mutually exclusive.  "
	 "See the EVMS Command Line Grammar to determine which combinations of filters "
	 "are allowed for a particular query.\n\n"
	 "The EVMS Command Line Interpreter supports the following filters:\n\n\n");


    Row_Delimiter('-', " Plug-in Type Filter ");

    My_Printf
	("This filter is only available when <Type of data to return> is equal to Plugins.  "
	 "This filter removes from the Return Values List any entries for plug-in modules "
	 "which are not of the type specified.  "
	 "The form of this filter is:\n\n");

    Indent_Printf(1, FALSE, "Type = <Plug-in Type>\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "Type may be abbreviated as a single letter 'T'.\n");
    Indent_Printf(1, FALSE,
		  "<Plug-in Type> is one of:\n"
		  "Device Manager               (abbreviated as D)\n"
		  "Feature                      (abbreviated as F)\n"
		  "Region Manager               (abbreviated as R)\n"
		  "Segment Manager              (abbreviated as S)\n"
		  "File System Interface Module (abbreviated as FSIM)\n"
		  "Distributed Lock Management  (abbreviated as DLM)\n"
		  "Cluster Management           (abbreviated as CM)\n\n\n");


    Row_Delimiter('-', " Plugin Filter ");

    My_Printf
	("This filter is available when <Type of data to return> is either Plugins, "
	 "Volumes, Objects, Regions, Containers, Segments, or Disks.  "
	 "This filter has the following behavior:\n\n"
	 "When <Type of data to return> is Plugins, this filter removes from the "
	 "Return Values List all plug-in modules other than the specified plug-in module.\n\n"
	 "When <Type of data to return> is Volumes, this filter removes from the "
	 "Return Values List any volume which was created without the use of the specified "
	 "plug-in module.\n\n"
	 "When <Type of data to return> is Objects, this filter removes from the "
	 "Return Values List any object which was created without the use of the specified "
	 "plug-in module.\n\n"
	 "When <Type of data to return> is Regions, this filter removes from the "
	 "Return Values List any region not created by the specified plug-in module.\n\n"
	 "When <Type of data to return> is Containers, this filter removes from the "
	 "Return Values List any container not created by the specified plug-in module.\n\n"
	 "When <Type of data to return> is Segments, this filter removes from the "
	 "Return Values List any segment not created by the specified plug-in module.\n\n"
	 "When <Type of data to return> is Disks, this filter removes from the "
	 "Return Values List any disk not claimed by the specified plug-in module.\n\n"
	 "The form of this filter is:\n\n");

    Indent_Printf(1, FALSE, "Plugin = <Plug-in Module Identifier>\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "Plugin may be abbreviated as a single letter 'P'.\n");
    Indent_Printf(1, FALSE,
		  "<Plug-in Module Identifier> is the name or ID number of a plug-in module.\n\n\n");


    Row_Delimiter('-', " Volume Filter ");

    My_Printf
	("This filter is available when <Type of data to return> is either Plugins, "
	 "Volumes, Objects, or Regions.  "
	 "This filter has the following behavior:\n\n"
	 "When <Type of data to return> is Plugins, this filter removes from the "
	 "Return Values List all plug-in modules other than the EVMS Native Feature plug-ins "
	 "used in the creation of the specified volume.\n\n"
	 "When <Type of data to return> is Volumes, this filter removes from the "
	 "Return Values List all volumes other than specified volume.\n\n"
	 "When <Type of data to return> is Objects, this filter removes from the "
	 "Return Values List all objects not used in the creation of the specified volume.\n\n"
	 "When <Type of data to return> is Regions, this filter removes from the "
	 "Return Values List all regions not used in the creation of the specified volume.\n\n"
	 "The form of this filter is:\n\n");

    Indent_Printf(1, FALSE, "Volume = <Volume Name>\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "Volume may be abbreviated as a single letter 'V'.\n");
    Indent_Printf(1, FALSE,
		  "<Volume Name> is the fully qualified name of a volume.\n\n\n");


    Row_Delimiter('-', " Object Filter ");

    My_Printf
	("This filter is available when <Type of data to return> is either Plugins, "
	 "Volumes, Objects, or Regions.  This filter has the following behavior:\n\n"
	 "When <Type of data to return> is Plugins, this filter removes from the "
	 "Return Values List all plug-in modules other than the EVMS Native Feature plug-ins "
	 "used in the creation of the specified object.\n\n"
	 "When <Type of data to return> is Volumes, this filter removes from the "
	 "Return Values List all volumes which do not consume the specified object.\n\n"
	 "When <Type of data to return> is Objects, this filter removes from the "
	 "Return Values List all objects other than the specified object.\n\n"
	 "When <Type of data to return> is Regions, this filter removes from the "
	 "Return Values List all regions not used in the creation of the specified object.\n\n"
	 "The form of this filter is:\n\n");

    Indent_Printf(1, FALSE, "Object = <Object Name>\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "Object may be abbreviated as a single letter 'O'.\n");
    Indent_Printf(1, FALSE,
		  "<Object Name> is the name of an object that EVMS knows about.\n\n\n");


    Row_Delimiter('-', " Container Filter ");

    My_Printf
	("This filter is available when <Type of data to return> is either Plugins, "
	 "Volumes, Objects, Regions, Containers, or Segments.  "
	 "This filter has the following behavior:\n\n"
	 "When <Type of data to return> is Plugins, this filter removes from the "
	 "Return Values List all plug-ins which were not used to create the specified container.\n\n"
	 "When <Type of data to return> is Volumes, this filter removes from the "
	 "Return Values List any volume which was constructed without using a region "
	 "from the specified container.\n\n"
	 "When <Type of data to return> is Objects, this filter removes from the "
	 "Return Values List any object which is not consumed by the container or which was "
	 "constructed without using a region from the specified container.\n\n"
	 "When <Type of data to return> is Regions, this filter removes from the "
	 "Return Values List any regions which do not come from the specified container.\n\n"
	 "When <Type of data to return> is Containers, this filter removes from the "
	 "Return Values List all containers other than the specified container.\n\n"
	 "When <Type of data to return> is Segments, this filter removes from the "
	 "Return Values List any segment which is was not used to create the container.\n\n"
	 "The form of this filter is:\n\n");

    Indent_Printf(1, FALSE, "Container = <Container Name>\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "Container may be abbreviated as a single letter 'C'.\n");
    Indent_Printf(1, FALSE,
		  "<Container Name> is the name of a container that EVMS knows about.\n\n\n");


    Row_Delimiter('-', " Region Filter ");

    My_Printf
	("This filter is available when <Type of data to return> is either Plugins, Volumes, "
	 "Objects, Regions, or Containers.  "
	 "This filter has the following behavior:\n\n"
	 "When <Type of data to return> is Plugins, this filter removes from the "
	 "Return Values List all plug-ins which were not used to create the specified region.\n\n"
	 "When <Type of data to return> is Volumes, this filter removes from the "
	 "Return Values List any volume which was constructed without using the\n"
	 "specified region.\n\n"
	 "When <Type of data to return> is Objects, this filter removes from the "
	 "Return Values List any object which was constructed without using the\n"
	 "specified region.\n\n"
	 "When <Type of data to return> is Regions, this filter removes from the "
	 "Return Values List all regions other than the specified region.\n\n"
	 "When <Type of data to return> is Containers, this filter removes from the "
	 "Return Values List any container other than the one from which "
	 "the specified region came.\n\n"
	 "The form of this filter is:\n\n");

    Indent_Printf(1, FALSE, "Region = <Region Name>\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "Region may be abbreviated as a single letter 'R'.\n");
    Indent_Printf(1, FALSE,
		  "<Region Name> is the name of a region that EVMS knows about.\n\n\n");


    Row_Delimiter('-', " Segment Filter ");

    My_Printf
	("This filter is available when <Type of data to return> is Plugins, Containers, "
	 "Segments, or Disks.  "
	 "This filter has the following behavior:\n\n"
	 "When <Type of data to return> is Plugins, this filter removes from the "
	 "Return Values List all plug-ins which were not used to create the specified segment.\n\n"
	 "When <Type of data to return> is Containers, this filter removes from the "
	 "Return Values List all containers which do not consume the specified segment.\n\n"
	 "When <Type of data to return> is Segments, this filter removes from the "
	 "Return Values List all but the specified segment.\n\n"
	 "When <Type of data to return> is Disks, this filter removes from the "
	 "Return Values List all disks other than the disk on which the specified segment lies.\n\n"
	 "The form of this filter is:\n\n");

    Indent_Printf(1, FALSE, "Segment = <Segment Name>\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "Segment may be abbreviated as a single letter 'D'.\n");
    Indent_Printf(1, FALSE,
		  "<Segment Name> is the name of a segment that EVMS knows about.\n\n\n");


    Row_Delimiter('-', " Disk Filter ");

    My_Printf
	("This filter is available when <Type of data to return> is Segments or Disks.  "
	 "This filter has the following behavior:\n\n"
	 "When <Type of data to return> is Segments, this filter removes from the "
	 "Return Values List any segment that does not lie on the specified disk\n\n"
	 "When <Type of data to return> is Disks, this filter removes from the "
	 "Return Values List all disks other than the specified disk.\n\n"
	 "The form of this filter is:\n\n");

    Indent_Printf(1, FALSE, "Disk = <Disk Name>\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "Disk may be abbreviated as a single letter 'D'.\n");
    Indent_Printf(1, FALSE,
		  "<Disk Name> is the name of a disk that EVMS knows about.\n\n\n");


    Row_Delimiter('-', " Size Filters ");

    My_Printf
	("There are three size filters:  Less Than, Greater Than, and Equal To.  "
	 "These filters are available when <Type of data to return> is either Volumes, "
	 "Regions, Containers, Objects, Disks, or Segments.  "
	 "These filters remove items from the Return Values List based upon their size.\n\n"
	 "The Less Than filter has the following form:\n\n");

    Indent_Printf(1, FALSE, "LT <XXX> <Unit>\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE, "<XXX> is a positive integer or 0.\n");
    Indent_Printf(1, FALSE, "<Unit> is either KB, MB, GB, or TB.\n\n");

    My_Printf("The Greater Than filter has the following form:\n\n"
	      "GT <XXX> <Unit>\n\n" "where:\n");
    Indent_Printf(1, FALSE, "<XXX> is a positive integer or 0.\n");
    Indent_Printf(1, FALSE, "<Unit> is either KB, MB, GB, or TB.\n\n");

    My_Printf("The Equal filter has the following form:\n\n"
	      "EQ <XXX> <Unit> , <Precision>\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE, "<XXX> is a positive integer or O.\n");
    Indent_Printf(1, FALSE,
		  "<Unit> if specified, must be either KB, MB, GB, or TB.\n");
    Indent_Printf(1, FALSE,
		  "<Precision> is a positive integer.  "
		  "It indicates how 'sloppy' a match to make.  "
		  "If no unit is specified, then Precision is interpreted to be a percentage, "
		  "in which case a value is considered to be equal to <XXX> if it is within <XXX> "
		  "plus or minus <Precision> percent of <XXX>.  "
		  "If a unit (KB, MB, GB, TB) is specified for <Precision>, then a value is "
		  "considered equal to <XXX> if it is within the range of <XXX> - <Precision> to "
		  "<XXX> + <Precision>.\n\n\n");


    Row_Delimiter('-', " Freespace Filter ");

    My_Printf
	("The Freespace filter removes from the Return Values List any items which do "
	 "not contain freespace.  "
	 "This filter is only useable when <Type of data to return> is Containers or Segments.  "
	 "The form of the Freespace filter is:\n\n");

    Indent_Printf(1, FALSE, "Freespace , <Size Filters>\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "Freespace may be abbreviated as the single letter 'F'.\n");
    Indent_Printf(1, FALSE,
		  "<Size Filters> are the same Size Filters as above, only now, instead of using "
		  "the size of the containers for comparisons, the <Size Filters> will use "
		  "the freespace size for comparisons.\n\n\n");


    Row_Delimiter('-', " Convertible Filter ");

    My_Printf
	("The Convertible filter is available when <Type of data to return> is Volumes.  "
	 "This filter removes from the Return Values List any volumes which are not "
	 "compatibility volumes, as well as any compatibility volumes which can NOT be "
	 "converted into EVMS native volumes.  "
	 "The form of this filter is:\n\n");

    Indent_Printf(1, FALSE, "Convertible\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "Convertible may be abbreviated as Con.\n\n\n");


    Row_Delimiter('-', " Expandable Filter ");

    My_Printf
	("The Expandable filter is available when <Type of data to return> is either "
	 "Volumes, Regions, Objects, or Segments.  "
	 "This filter removes from the Return Values List any items which can not be expanded.  "
	 "The form of this filter is:\n\n");

    Indent_Printf(1, FALSE, "Expandable\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "Expandable may be abbreviated as the single letter 'E'.\n\n\n");


    Row_Delimiter('-', " Shrinkable Filter ");

    My_Printf
	("The Shrinkable filter is available when <Type of data to return> is either "
	 "Volumes, Regions, Objects, or Segments.  "
	 "This filter removes from the Return Values List any items which can not be shrunk.  "
	 "The form of this filter is:\n\n");

    Indent_Printf(1, FALSE, "Shrinkable\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "Shrinkable may be abbreviated as the single letter 'S'.\n\n\n");


    Row_Delimiter('-', " Unclaimed Filter ");

    My_Printf
	("The Unclaimed filter is available when <Type of data to return> is Disks.  "
	 "It removes from the Return Values List all disks which have been claimed by "
	 "a Partition Manager plug-in module (which means that those disks have "
	 "a recognized partitioning scheme in place).  "
	 "Thus, the disks left in the Return Values List will either have no "
	 "partitioning scheme on them, or their partitioning scheme is unrecognized.  "
	 "The form of this filter is:\n\n");

    Indent_Printf(1, FALSE, "Unclaimed\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "Unclaimed can be abbreviated as a single letter 'U'.\n\n\n");


    Row_Delimiter('-', " List Options Pseudo Filter ");

    My_Printf
	("List Options is treated like a filter, but what it actually does is cause "
	 "the EVMS Command Line Interpreter to list what can be done with each item "
	 "in the Return Values List.  " "The form of this filter is:\n\n");

    Indent_Printf(1, FALSE, "List Options\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE, "List Options may be abbreviated as LO\n\n\n");


    Row_Delimiter('-', NULL);

    My_Printf("Sample Queries using Filters:\n\n"
	      "To query the plug-ins in the system and their options:\n\n"
	      "q:p,lo\n\n"
	      "To query only the Region Manager plug-ins in the system and their options:\n\n"
	      "q:p,t=r,lo\n\n"
	      "To query the volumes in the system:\n\n"
	      "q:v\n\n"
	      "To query the volumes in the system over 1GB in size:\n\n"
	      "q:v,gt 1GB\n\n"
	      "To query the EVMS objects in the system:\n\n"
	      "q:o\n\n"
	      "To query the segments in the system that reside on disk sda:\n\n"
	      "q:s,d=sda\n\n"
	      "To query the segments in the system that belong to the container lvm/MyStuff, "
	      "which are over 500MB in size yet less than 2 GB in size, and which come from "
	      "disk sda:\n\n"
	      "q:s,c=lvm/MyStuff,GT 500MB,lt 2GB,d=sda\n\n\n");


    Row_Delimiter('-', NULL);

    My_Printf("Specialty Query Commands\n\n");

    Row_Delimiter('*', " Query Children ");

    My_Printf
	("This command returns the items that were used to create the specified item.  "
	 "For example, the children of a storage object are the EVMS objects, regions, "
	 "disks, or segments that were used to create the specified storage object.  "
	 "The form of this command is:\n\n");

    Indent_Printf(1, FALSE, "Query : Children , <Item Name>\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE, "Children may be abbreviated as Chi.\n");
    Indent_Printf(1, FALSE,
		  "<Item Name> is the name of a Volume, Storage Object, Region, Segment, or Container.\n\n");

    Row_Delimiter('*', " Query Parent ");

    My_Printf
	("This command returns the item (if any) that uses the specified item.  "
	 "For example, if a storage object is specified, then this will return the volume "
	 "or storage object which was created using the specified storage object, if such "
	 "a volume or storage object exists.  "
	 "The form of this command is:\n\n");

    Indent_Printf(1, FALSE, "Query : Parent , <Item Name>\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE, "Parent may be abbreviated as Par.\n");
    Indent_Printf(1, FALSE,
		  "<Item Name> is the name of a Storage Object, Region, Segment, Container, or Disk.\n\n");

    Row_Delimiter('*', "Query Expand Points");
    My_Printf("\n");

    My_Printf
	("This query returns the expansion points for a volume, storage object, "
	 "or storage container.  "
	 "The expansion points are those entities within the volume, storage object, "
	 "or storage container which can be expanded without causing a loss of data.  "
	 "The form of this command is:\n\n");

    Indent_Printf(1, FALSE, "Query : Expand Points , <Item Name>\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE, "Expand Points may be abbreviated as EP\n");
    Indent_Printf(1, FALSE,
		  "<Item Name> is the name of a Volume, Storage Object or Storage Container.\n\n");

    Row_Delimiter('*', " Query Shrink Points ");

    My_Printf
	("This query returns the shrink points for a volume, storage object, or "
	 "storage container.  "
	 "The shrink points are those entities within the volume, storage object, "
	 "or storage container which can be shrunk without causing a loss of data.  "
	 "The form of this command is:\n\n");

    Indent_Printf(1, FALSE, "Query : Shrink Points , <Item Name>\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE, "Shrink Points may be abbreviated as SP\n");
    Indent_Printf(1, FALSE,
		  "<Item Name> is the name of a Storage Object or Volume.\n\n");

    Row_Delimiter('*', " Query Extended Info ");

    My_Printf
	("This command allows access to any additional information that may be available "
	 "for a plug-in module, a EVMS object, a disk, a region, a segment, or "
	 "a container.  " "The form of this command is:\n\n");

    Indent_Printf(1, FALSE,
		  "Query : Extended Info , <Item Name> , <Field Name>\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE, "Extended Info may be abbreviated as EI\n");
    Indent_Printf(1, FALSE,
		  "<Item Name> is the name of a EVMS object, disk, region, segment, "
		  "container, or plug-in module, or it can be the numeric ID of a plug-in module.\n");
    Indent_Printf(1, FALSE,
		  "<Field Name> is the name of a specific piece of extended information.  "
		  "Extended Information is grouped into fields.  "
		  "Each field has a name and one or more values associated with it. "
		  "<Field Name> is optional.\n\n");

    Row_Delimiter('*', " Query Acceptable ");

    My_Printf
	("The purpose of this query is to allow the user to find out which storage "
	 "objects, regions, segments, or disks are acceptable for creating or expanding "
	 "a volume, EVMS object, region, or segment.  "
	 "The command works as follows:\n\n"
	 "For use when creating containers, objects, or regions, the user will specify the "
	 "plug-in module being used, the values for the options that the plug-in is to use "
	 "when creating the container/object/region, and then the EVMS objects, "
	 "regions, segments, or disks that the user has already decided to use.  "
	 "The query will return any EVMS objects, regions, segments, or disks which "
	 "are still available and which are still acceptable for use in creating "
	 "the container/object/region.  "
	 "An example of using this command would be to determine what segments should be used "
	 "to create a software RAID 5 storage object.  "
	 "Initially no segments have been selected, so all unused segments in the system "
	 "may be available and may be returned by this query.  "
	 "Next, the user chooses an initial segment, and, as a result of that choice, "
	 "all segments on the same drive are not acceptable anymore.  "
	 "To find out what segments are still available to be chosen, the user may use "
	 "this query again, only now they specify the segments that they have already chosen, "
	 "and this query will return all of the segments that may still be chosen.  "
	 "The user will now choose another segment, and then use this query again, "
	 "specifying all of the segments they have already chosen in the order in which "
	 "they chose them, to see which segments are still available to be selected.  "
	 "This iterative process may be used to build an ordered list of segments that may "
	 "then be used in an actual create command to create the desired software RAID 5 "
	 "storage object.\n\n"
	 "For use when expanding volumes or storage objects, the user will specify the "
	 "volume or storage object to be expanded along with any EVMS objects, "
	 "segments, regions, or disks that have already been chosen to be used in the "
	 "expansion, and this query will return the EVMS objects, regions, segments, "
	 "or disks which may still be selected and used to expand the specified volume "
	 "or storage object.  "
	 "Again, as described above, an interative process may be used to build an ordered "
	 "list of the EVMS objects, regions, segments, or disks that can be used "
	 "to expand the specified volume or storage object with an actual expand command.\n\n"
	 "The query has two forms.  "
	 "For determining what is acceptable for use with a Create command use "
	 "the following form:\n\n");

    Indent_Printf(1, FALSE,
		  "Query : Acceptable , Create , <Item To Create> , <Plug-in Module> = "
		  "{ <name> = < value > ... , <name> = <value> } , <item to use> ... , <item to use>\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "Acceptable may be abbreviated by the single letter 'A'.\n");
    Indent_Printf(1, FALSE,
		  "Create may be abbreviated by the single letter 'C'.\n");
    Indent_Printf(1, FALSE,
		  "<Item To Create> is either Object, Region, or Container.  "
		  "Object may be abbreviated as a single letter 'O', Region may be abbreviated "
		  "as a single letter 'R', and Container may be abbreviated as a single letter 'C'.\n");
    Indent_Printf(1, FALSE,
		  "<Plug-in Module> is the name or ID number of the plug-in module to use when creating "
		  "<Item To Create>.\n");
    Indent_Printf(1, FALSE,
		  "<name> is the name of an option supported by <Plug-in Module>.\n");
    Indent_Printf(1, FALSE,
		  "<value> is a legitimate value for the option <name>.\n");
    Indent_Printf(1, FALSE,
		  "<item to use> is the EVMS name of a EVMS object, region, segment or disk "
		  "which is to be used to create <Item To Create>.\n\n");

    My_Printf
	("For determining what is acceptable for use with an Expand command, "
	 "use the following form:\n\n");

    Indent_Printf(1, FALSE,
		  "Query : Acceptable , Expand , <Item to Expand> , <item to use> ... <item to use>\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "Acceptable may be abbreviated by the single letter 'A'.\n");
    Indent_Printf(1, FALSE,
		  "Expand may be abbreviated by the single letter 'E'.\n");
    Indent_Printf(1, FALSE,
		  "<Item to Expand> is the name of a Volume or Storage Object that is to be expanded.\n");
    Indent_Printf(1, FALSE,
		  "<item to use> is the EVMS name of a EVMS object, region, segment or disk "
		  "which is to be used when expanding <Item To Expand>.\n\n");

    return;
}

static void Do_Remove_Help(void)
{
    My_Printf("The Remove Command\n\n"
	      "The remove command is used to remove a segment manager from a disk.  "
	      "A segment manager may only be removed from a disk when all of the data segments "
	      "on that disk have been deleted.  "
	      "The remove command actually causes a segment manager to remove its metadata from "
	      "the disk, thereby leaving an empty disk which may be used directly, "
	      "or to which another segment manager may be assigned.\n\n");

    Indent_Printf(1, FALSE,
		  "Example:  You have developed a new disk partitioning scheme and have created an "
		  "EVMS Segment Manager plug-in called MyScheme to test it.  "
		  "You have a disk, known as sdb, which you wish to use to test your new "
		  "partitioning scheme, but the disk is under the control of the "
		  "DOS Segment Manager.  "
		  "Remove the DOS Segment Manager from the disk so that you can use the disk "
		  "to test your new partitioning scheme.\n\n");

    My_Printf
	("After deleting all of the data partitions on the test disk, use the command:\n\n");

    My_Printf("Rem:sdb\n\n");

    return;
}

static void Do_Rename_Help(void)
{
    My_Printf("The Rename Command\n\n"
	      "The Rename command is used to change the name of an EVMS volume.\n\n"
	      "The Rename command has the following form:\n\n");

    Indent_Printf(1, FALSE,
		  "Rename : <Volume Name> , Name = <New Volume Name>\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "<Volume Name> is the name of the volume whose name is to be changed.\n");
    Indent_Printf(1, FALSE,
		  "<New Volume Name> is the new name for the volume.  "
		  "Please note that the new volume name must be in single quotes if "
		  "it contains spaces or any non-alphanumeric characters.\n\n");

    Indent_Printf(1, FALSE,
		  "Note:  Both the 'Rename' and 'Name' keywords may be abbreviated as their "
		  "first letters, as seen in the example below.\n\n");

    Indent_Printf(1, FALSE,
		  "Example:  Rename the volume /dev/evms/John to /dev/evms/Fred.\n\n");

    My_Printf("r:/dev/evms/John, n=Fred\n\n");

    return;
}

static void Do_Replace_Help(void)
{
    My_Printf("The Replace Command\n\n"
	      "The Replace command is used to replace one storage object, the source, "
	      "with another storage object, the target.  "
	      "The target object takes the place of the source object wherever the source was used.  "
	      "All the data from the source object are copied to the target object.  "
	      "If device-mapper is installed in the kernel and it has the mirroring target, "
	      "the replace can be done online, that is, the volume that contains "
	      "the source object can be mounted while the replace is being done.\n\n"
	      "If the source object is the child of another storage object, "
	      "the plug-in that owns the parent object of the source object must be able "
	      "to handle one of its children being replaced.  "
	      "If it cannot, then the source object cannot be replaced.  "
	      "The parent of the source object determines if the size of the target object is acceptable.\n\n"
	      "The child object of an EVMS volume can be replaced.  "
	      "(If the child object is an EVMS object, then the replace must be done offline, "
	      "that is, the volume must not be mounted.)  "
	      "The target child object must be at least as big as the source object.\n\n"
	      "The child object of a compatibility volume cannot be replaced.\n\n"
	      "The target object must be a topmost object.  "
	      "That is, it cannot have any parent objects and cannot be part of a volume.\n\n"
	      "The Replace command has the following form:\n\n");

    Indent_Printf(1, FALSE,
		  "Replace : <Source Object Name> , <Target Object Name>\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "<Source Object Name> is the name of the object to be replaced.\n");
    Indent_Printf(1, FALSE,
		  "<Target Object Name> is the name of the object that will replace "
		  "<Source Object Name>.\n\n");

    Indent_Printf(1, FALSE,
		  "Note:  'Replace' may be abbreviated as 'Rep' in commands.\n\n");

    Indent_Printf(1, FALSE,
		  "Example:  You have a 3GB drive link object named DL1 which has three 1GB children, "
		  "sda5, sdb5, and sdc5.  "
		  "Disk sdb is slow, so you want to replace sdb5 with a 1GB segment, sdd5, "
		  "on a faster disk.\n\n");

    My_Printf("Rep:sdb5, sdd5\n\n");

    Indent_Printf(1, FALSE,
		  "Example:  You have an EVMS volume /dev/evms/data made from a 3GB drive link object "
		  "named DL1.  "
		  "DL1 has three 1GB children, sda5, sdb5, and sdc5.  "
		  "You add a new, larger disk to your system and want /dev/evms/data to use one "
		  "3GB segment named sde5 on the new disk.\n\n");

    My_Printf("Rep:DL1, sde5\n\n");

    Indent_Printf(1, FALSE,
		  "Note:  The replace command can take a long time since it copies all the data "
		  "from the source object to the target object.  "
		  "If you are running the EVMS Command Line Interpreter in the default mode "
		  "where changes are saved after each command, it may take a while for the "
		  "replace command to finish.  "
		  "If your are running the EVMS Command Line Interpreter in the alternate mode "
		  "where changes are saved when the program exits, it may take a while for "
		  "the program to exit while it waits for the replace command to finish.\n\n");

    return;
}

static void Do_Revert_Help(void)
{
    My_Printf("The Revert Command\n\n"
	      "The Revert command strips away the topmost layer of an EVMS volume or "
	      "storage object, thereby restoring it to its previous EVMS state.  "
	      "Thus, an EVMS Volume will revert to the EVMS Storage Object from which it came.\n\n"
	      "The Revert command has the following form:\n\n");

    Indent_Printf(1, FALSE, "Revert : <Volume or Storage Object>\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "<Volume or Storage Object> is the name of the volume or storage object to revert.\n\n");

    Indent_Printf(1, FALSE,
		  "Note:  The 'Revert' keyword may be abbreviated as 'Rev' in commands.\n\n");

    Indent_Printf(1, FALSE,
		  "Example:  Lets say we just created a storage object named Fred from a "
		  "storage object named Barney.  "
		  "Fred is a rather complicated storage object that would take time to recreate.  "
		  "However, after creating Fred we find out that we applied the wrong feature "
		  "to Barney to create Fred.  "
		  "How do we remove the feature that was applied to Barney to create Fred "
		  "without having to delete Fred and start from scratch?\n\n")
	/ My_Printf("Rev:Fred\n\n");

    My_Printf
	("The Revert command will strip away the top most feature on Fred, "
	 "thereby undoing the creation of Fred, and leaving us with Barney.\n\n");

    return;
}

static void Do_Shrink_Help(void)
{
    My_Printf("The Shrink Command\n\n"
	      "The Shrink command is used to reduce the size of a volume, storage object, "
	      "or storage container.\n\n"
	      "A volume, storage object, or storage container may comprise of one or more EVMS objects, "
	      "regions, segments, or disks.  "
	      "Whether or not a volume, storage object, or storage container can be shrunk "
	      "depends upon how it is constructed.  "
	      "For example, if a volume consists of a single segment, then whether or not it "
	      "can be shrunk depends upon whether or not the segment manager which created "
	      "that segment can decrease the size of that segment.  "
	      "A more complicated volume may have several ways to shrink.  "
	      "For example, a volume created from several segments using EVMS Drive Linking may be "
	      "shrunk by decreasing the size of the last segment linked to form the volume, "
	      "or by using EVMS Drive Linking to remove a segment to the volume, or both.  "
	      "In this case, we say that the volume has multiple shrink points, because there are "
	      "multiple ways in which the volume can be shrunk.  "
	      "EVMS allows the user complete control over how a volume is shrunk.  "
	      "This means that, to shrink a volume, the user must specify which shrink point is "
	      "to be used to shrink the volume.  "
	      "Thus, in our last example, there were two shrink points.  "
	      "One was the storage object created by EVMS Drive Linking.  "
	      "This storage object could be shrunk by adding another segment to it.  "
	      "The second shrink point would be the last segment used in the storage object formed by "
	      "EVMS Drive Linking.  "
	      "(While the other segments used to create this storage object may be capable of being "
	      "shrunk, EVMS Drive Linking prohibits them from shrinking as it is not set up to handle "
	      "that case.)\n\n"
	      "Similar logic is true for storage objects and for storage containers.  "
	      "A top level storage object may be shrunk by shrinking the object itself or by "
	      "shrinking one of its descendents.  "
	      "A storage container can be shrunk by removing an object to the container or by "
	      "shrinking an object in the container if the plug-in allows.\n\n"
	      "To find the shrink points for a volume, storage object, or storage container, "
	      "use the Query Shrink Points command.  "
	      "Once the shrink points for a volume, storage object, or storage container are known, "
	      "they can be used with this command to shrink the volume, storage object, or "
	      "storage container.\n\n"
	      "The Shrink command has the following form:\n\n");

    Indent_Printf(1, FALSE,
		  "Shrink : <Shrink Point> , <Name> = <Value> , ... , <Name> = <Value> , "
		  "<Storage to remove> , ... , <Storage to remove>\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "Shrink may be abbreviated by the single letter 'S'.\n");
    Indent_Printf(1, FALSE,
		  "<Shrink Point> is the name of a shrink point as provided by the Query Shrink Points command.\n");
    Indent_Printf(1, FALSE,
		  "<Name> is the name of an option supported by the plug-in module that controls <Shrink Point>.\n");
    Indent_Printf(1, FALSE,
		  "<Value> is an acceptable value for option <Name>.\n");
    Indent_Printf(1, FALSE,
		  "<Storage to remove> is the name of an acceptable EVMS object, region, segment, or disk "
		  "to be shrunk or removed from the volume.\n\n");

    Indent_Printf(1, FALSE,
		  "Example:  Given a volume created from three segments using EVMS Drive Linking.\n"
		  "The three segments used are sda1, sdb1, and sdc1.  "
		  "The storage object created by EVMS Drive Linking is called DL1, and the volume "
		  "is called /dev/evms/Sample_Volume.  "
		  "The segment sdc1 is controlled by the DOS Segment Manager (DosSegMgr), and can be "
		  "shrunk by 50 MB.  "
		  "The Query Shrink Points command returns DL1 and sdc1 as the available shrink points.  "
		  "Shrink the volume by 50MB.\n\n");

    My_Printf("s:sdc1, size=50MB, sdc1\n\n");

    return;
}


static void Do_Set_Help(void)
{
    My_Printf("The Set Command\n\n"
	      "Some EVMS Objects, Regions, Containers, Segments, or Disks may have options "
	      "associated with them that can be changed.  "
	      "If a Query command which returns EVMS Objects, Regions, Containers, Segments, "
	      "or Disks is executed using the List Options Pseudo Filter, "
	      "then the changeable options for each item returned will be listed with that item.\n\n"
	      "The Set command has the following form:\n\n");

    Indent_Printf(1, FALSE,
		  "Set : <Object> , <Name> = <Value> , ... , <Name> = <Value>\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "<Object> is the name of a Storage Object, Region, Container, Segment or "
		  "Disk whose options are to be changed\n");
    Indent_Printf(1, FALSE,
		  "<Name> is the name of an option that can be set for <Object>\n");
    Indent_Printf(1, FALSE,
		  "<Value> is an acceptable value for option <Name>\n\n");

    Indent_Printf(1, FALSE,
		  "Note:  The 'Set' keyword can not be abbreviated.\n\n");

    return;
}


static void Do_Modify_Help(void)
{
    My_Printf("The Modify Command\n\n"
	      "Some EVMS Objects, Regions, Containers, Segments, or Disks may have options "
	      "associated with them that can be changed.  "
	      "If a Query command which returns EVMS Objects, Regions, Containers, Segments, "
	      "or Disks is executed using the List Options Pseudo Filter, "
	      "then the changeable options for each item returned will be listed with that item.\n\n"
	      "The Modify command has the following form:\n\n");

    Indent_Printf(1, FALSE,
		  "Modify : <Object> , <Name> = <Value> , ... , <Name> = <Value>\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "<Object> is the name of a Storage Object, Region, Container, Segment or "
		  "Disk whose options are to be changed\n");
    Indent_Printf(1, FALSE,
		  "<Name> is the name of an option that can be set for <Object>\n");
    Indent_Printf(1, FALSE,
		  "<Value> is an acceptable value for option <Name>\n\n");

    Indent_Printf(1, FALSE,
		  "Note:  The 'Modify' keyword can not be abbreviated.\n\n");

    return;
}


static void Do_Check_Help(void)
{
    My_Printf("The Check Command\n\n"
	      "The Check command performs a consistency check on the file system contained "
	      "within the specified volume.  "
	      "This is the equivalent of the Linux fsck command and is included here for "
	      "the convenience of the user.\n\n"
	      "The Check command has the following form:\n\n");

    Indent_Printf(1, FALSE,
		  "Check : <Volume> , <Name> = <Value> , ... , <Name> = <Value>\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "<Volume> is the name of the volume whose file system is to be checked.\n");
    Indent_Printf(1, FALSE,
		  "<Name> is the name of an option supported by the FSIM for the file system "
		  "found on the specified volume.\n");
    Indent_Printf(1, FALSE,
		  "<Value> is an acceptable value for option <Name>.\n\n");

    Indent_Printf(1, FALSE,
		  "Note:  The 'Check' keyword may be abbreviated as 'Ch' in commands.\n\n");

    return;
}

static void Do_Format_Help(void)
{
    My_Printf("The Format Command\n\n"
	      "The Format command creates a file system on a volume.  "
	      "This is the equivalent of the Linux mkfs command and is included here for "
	      "the convenience of the user.\n\n"
	      "The Format command has the following form:\n\n");

    Indent_Printf(1, FALSE,
		  "Format : <FSIM> = { <Name> = <Value> , ... , <Name> = <Value> } , <Volume>\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "<FSIM> is the name of a File system Interface Module recognized by EVMS.  "
		  "The specified FSIM will be used to create the file system on the volume "
		  "specified by <Volume>.  "
		  "Normally there will be one FSIM for each file system.  "
		  "Thus, specifying the FSIM is the same as specifying the file system.  "
		  "In those cases where one FSIM supports more than one file system, "
		  "then an option will be used to specify which file system should be created.\n");
    Indent_Printf(1, FALSE,
		  "<Name> is the name of an option supported by the FSIM for the file system found "
		  "on the specified volume.\n");
    Indent_Printf(1, FALSE,
		  "<Value> is an acceptable value for option <Name>.\n");
    Indent_Printf(1, FALSE,
		  "<Volume> is the name of the volume where the file system is to be created\n\n");

    Indent_Printf(1, FALSE,
		  "Note:  The 'Format' keyword may be abbreviated as the single letter 'F' in commands.\n\n");

    return;
}


static void Do_Mkfs_Help(void)
{
    My_Printf("The MKFS Command\n\n"
	      "The MKFS command creates a file system on a volume.  "
	      "This is the equivalent of the Linux mkfs command and is included here for "
	      "the convenience of the user.\n\n"
	      "The MKFS command has the following form:\n\n");

    Indent_Printf(1, FALSE,
		  "mkfs : <FSIM> = { <Name> = <Value> , ... , <Name> = <Value> } , <Volume>\n\n");


    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "<FSIM> is the name of a File system Interface Module recognized by EVMS.  "
		  "The specified FSIM will be used to create the file system on the volume "
		  "specified by <Volume>. "
		  "Normally there will be one FSIM for each file system.  "
		  "Thus, specifying the FSIM is the same as specifying the file system.  "
		  "In those cases where one FSIM supports more than one file system, "
		  "then an option will be used to specify which file system should be created.\n");
    Indent_Printf(1, FALSE,
		  "<Name> is the name of an option supported by the FSIM for the file system found "
		  "on the specified volume\n");
    Indent_Printf(1, FALSE,
		  "<Value> is an acceptable value for option <Name>\n");
    Indent_Printf(1, FALSE,
		  "<Volume> is the name of the volume where the file system is to be created\n\n");

    Indent_Printf(1, FALSE,
		  "Note:  The 'MKFS' keyword may be abbreviated as the single letter 'M' in commands.\n\n");

    return;
}


static void Do_Unformat_Help(void)
{
    My_Printf("The Unformat Command\n\n"
	      "The Unformat command destroys the file system contained within the specified volume.  "
	      "This is typically done by overwriting the metadata for the file system with zeros.  "
	      "All data on the volume is lost.\n\n"
	      "The Unformat command has the following form:\n\n"
	      "Unformat : <Volume>\n\n" "where:\n");
    Indent_Printf(1, FALSE,
		  "<Volume> is the name of the volume whose file system is to be destroyed.\n\n");


    Indent_Printf(1, FALSE,
		  "Note:  The 'Unformat' keyword may be abbreviated as a single letter 'U' in commands.\n\n");

    return;
}


static void Do_UnMkfs_Help(void)
{
    My_Printf("The UnMKFS Command\n\n"
	      "The UnMKFS command destroys the file system contained within the specified volume.  "
	      "This is typically done by overwriting the metadata for the file system with zeros.  "
	      "All data on the volume is lost.\n\n"
	      "The UnMKFS command has the following form:\n\n"
	      "unmkfs : <Volume>\n\n" "where:\n");
    Indent_Printf(1, FALSE,
		  "<Volume> is the name of the volume whose file system is to be destroyed.\n\n");

    Indent_Printf(1, FALSE,
		  "Note: The 'UnMKFS' keyword may be abbreviated as a single letter 'U' in commands.\n\n");

    return;
}


static void Do_Echo_Help(void)
{
    My_Printf("The Echo Command\n\n"
	      "The Echo command will 'echo' a string to standard out.  "
	      "This command allows the user to output user defined text from an EVMS command file.\n\n"
	      "The Echo command has the following form:\n\n"
	      "Echo : <String>\n\n" "where:\n");
    Indent_Printf(1, FALSE,
		  "<String> is any sequence of ascii characters except for the quote mark.  "
		  "A string should be enclosed in quote marks if it contains any non-alphabetic "
		  "or non-numeric characters or spaces.  "
		  "If a quote mark is embedded in the string as part of the string, "
		  "then two quote marks should be used as in the example below.\n\n");

    My_Printf("Example:\n\n"
	      "Echo:\"This is a \"\"sample\"\" string with embedded quote marks.\n\n");

    return;
}


static void Do_Convert_Help(void)
{
    My_Printf("The Convert Command\n\n"
	      "The Convert command has two forms.  "
	      "The first attempts to convert an existing compatibility volume into an EVMS native volume.  "
	      "The second form attempts to convert an EVMS native volume into a compatibility volume.\n\n"
	      "The Convert command attempts to convert an existing compatibility volume "
	      "into an EVMS native volume by adding EVMS metadata to it.  "
	      "If the compatibility volume does not contain a file system, "
	      "EVMS can easily add its metadata to the volume and convert it.\n\n"
	      "If the compatibility volume contains a file system, then "
	      "EVMS must shrink the file system in order to make room for the EVMS metadata.  "
	      "This means that EVMS must have an FSIM module corresponding to the file system "
	      "in use on the compatibility volume, and that FSIM must support shrinking of the file system.  "
	      "If the compatibility volume can not be unmounted (it is mounted as root, for instance), "
	      "then the FSIM must support on-line shrinking of the file system.  "
	      "If the file system can not be shrunk, then volume can not be converted.  "
	      "This command should not be used on compatibility volumes which are being used without a file system "
	      "(as is often the case with databases) as data loss may occur.\n\n"
	      "The Convert command attempts to convert an existing EVMS native volume into "
	      "a compatibility volume by removing any EVMS metadata on the volume.  "
	      "Currently this can only be done for EVMS volumes which have no features applied to them.\n\n"
	      "The Convert command has the following form when attempting to convert a compatibility volume "
	      "into an EVMS native volume:\n\n"
	      "Convert : <Volume Name> , Name = <New Volume Name>\n\n"
	      "where:\n");

    Indent_Printf(1, FALSE,
		  "<Volume Name> is the name of the compatibility volume which is to be converted into an EVMS volume.\n");
    Indent_Printf(1, FALSE,
		  "<New Volume Name> is the new name for the volume.  "
		  "Please note that the new volume name must be in quotes if it contains spaces or "
		  "any non-alphanumeric characters.\n\n");

    Indent_Printf(1, FALSE,
		  "Note: The 'Convert' keyword may be abbreviated as 'co' and the 'Name' keyword may be "
		  "abbreviated as the single letter 'n'.\n\n");

    My_Printf("Example: \n\n"
	      "Co:/dev/evms/hda3, N=\"New EVMS Volume\"\n\n"
	      "The Convert command has the following form when attempting to convert an EVMS volume "
	      "into a compatibility volume:\n\n"
	      "Convert : <Volume Name>, Compatibility\n\n" "where:\n");
    Indent_Printf(1, FALSE,
		  "<Volume Name> is the name of the EVMS volume which is to be "
		  "converted into a compatibility volume,\n");
    Indent_Printf(1, FALSE,
		  "'Compatibility' is the EVMS keyword used to indicate that a compatibility volume "
		  "is the desired result of the operation.\n\n");

    Indent_Printf(1, FALSE,
		  "Note:  The 'Convert' keyword may be abbreviated as 'co' and the 'Compatibility' "
		  "keyword may be abbreviated as the single letter 'c'.\n\n");

    My_Printf("Example: \n\n" "Co:/dev/evms/MyVol, c\n\n");

    return;
}


static void Do_Add_Feature_Help(void)
{
    My_Printf("The Add Feature Command\n\n"
	      "The Add Feature command attempts to add an EVMS feature to an existing EVMS native volume.  "
	      "This is done by shrinking the file system on the volume to make room for the metadata "
	      "of the new feature, then laying down the metadata for the new feature.  "
	      "In order for this to work, EVMS must have an FSIM for the file system on the volume, "
	      "and that FSIM must support shrinking of the file system.  "
	      "If the volume can not be unmounted (perhaps because it is root), then the FSIM must "
	      "support on-line shrinking of the file system.  "
	      "Also, the file system must be able to shrink by an amount that is equal to or larger "
	      "than the metadata of the feature to be added to the volume.  "
	      "Finally, the feature being added must support this operation.  "
	      "This command should not be used on volumes which are being used without a file system "
	      "(as is often the case with databases) as data loss may occur.\n\n"
	      "The Add Feature command has the following form:\n\n");

    Indent_Printf(1, FALSE,
		  "Add Feature : <Feature Name> = { <name> = <value> , <name> = <value> ...} , <Volume Name>\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "<Feature Name> is the name of the EVMS Feature to add to the volume.\n");
    Indent_Printf(1, FALSE,
		  "<name> is the name of an option accepted by the Feature.\n");
    Indent_Printf(1, FALSE,
		  "<value> is an acceptable value for option <name>.\n");
    Indent_Printf(1, FALSE,
		  "<Volume Name> is the name of an EVMS volume to which <Feature Name> is to be added.\n\n");

    My_Printf
	("Note: The 'Add Feature' keywords may be abbreviated as 'AF'.\n\n"
	 "Example: \n\n"
	 "AF:DriveLink={Name=My_Drive_Link}, /dev/evms/My_EVMS_Volume\n\n");

    return;
}


static void Do_Task_Help(void)
{
    My_Printf("The Task Command\n\n"
	      "The Task command is used to access features specific to an EVMS Plug-in module.  "
	      "The Task command may be used directly on an EVMS Plug-in module, "
	      "or it may be used on a volume, EVMS object, region, segment, or container. "
	      "When used on a EVMS object, region, segment, or container, the command will be "
	      "directed to the plug-in module which owns the EVMS object, region, segment, or container.  "
	      "When used on a volume, the command is directed to the File System Interface Module "
	      "associated with the volume.\n\n"
	      "The Task command has the following form:\n\n");

    Indent_Printf(1, FALSE,
		  "Task : <Task Name> , <Specify Target> , <Name> = <Value> ,..., <Name> = <Value> , "
		  "<Specify Source> , ... , <Specify Source>\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "<Task Name> is the name of the task as defined by the EVMS plug-in that is expected to perform the task.\n");
    Indent_Printf(1, FALSE,
		  "<Specify Target> is the Volume/Object/Region/Segment/Container/Plug-in that this task is to be performed on.\n");
    Indent_Printf(1, FALSE,
		  "<name> is the name of an option accepted by the Plug-in which controls the specified target.\n");
    Indent_Printf(1, FALSE,
		  "<value> is an acceptable value for option <name>.\n");
    Indent_Printf(1, FALSE,
		  "<Specify Source> is the name of a Volume/Object/Region/Segment to use "
		  "when performing the specified task on the specified target.\n\n");

    return;

}


static void Do_Mount_Help(void)
{
    My_Printf("The Mount Command\n\n"
	      "The Mount command mounts a volume on a directory.\n\n"
	      "The Mount command has the following form:\n\n");

    Indent_Printf(1, FALSE,
		  "Mount : <Volume> , <Mount point> [ , <Mount options> ]\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "<Volume> is the name of the Volume to be mounted.\n");
    Indent_Printf(1, FALSE,
		  "<Mount point> is the name of the directory on which to mount the volume.\n");
    Indent_Printf(1, FALSE,
		  "<Mount options> is a string of options to be passed to the mount command.\n\n");

    Indent_Printf(1, FALSE,
		  "Note:  The 'Mount' keyword can not be abbreviated.\n\n");

    return;
}


static void Do_Unmount_Help(void)
{
    My_Printf("The Unmount Command\n\n"
	      "The Unmount command unmounts a volume.\n\n"
	      "The Unmount command has the following form:\n\n");

    Indent_Printf(1, FALSE, "Unmount : <Volume>\n\n");

    My_Printf("where:\n");
    Indent_Printf(1, FALSE,
		  "<Volume> is the name of the Volume to be unmounted.\n\n");

    Indent_Printf(1, FALSE,
		  "Note:  The 'Unmount' keyword can not be abbreviated.\n\n");

    return;
}


static void Do_Exit_Help(void)
{
    My_Printf("The Exit Command\n\n"
	      "The Exit command is used to exit the EVMS Command Line Interpreter and return "
	      "to the Linux command prompt.\n\n"
	      "The Exit command has the following form:\n\n" "Exit\n\n");

    return;

}


static void Do_Quit_Help(void)
{
    My_Printf("The Quit Command\n\n"
	      "The Quit command is used to exit the EVMS Command Line Interpreter and return "
	      "to the Linux command prompt.\n\n"
	      "The Quit command has the following form:\n\n" "Quit\n\n");

    return;

}


static void Do_Default_Help(void)
{

    My_Printf("The EVMS Commands\n\n"
	      "The EVMS Command Line Interpreter supports the following commands:\n\n");

    Indent_Printf(1, FALSE,
		  "Activate - activates volumes and storage objects.\n");
    Indent_Printf(1, FALSE,
		  "Add Feature - attempts to add the specified feature to an existing EVMS volume.\n");
    Indent_Printf(1, FALSE,
		  "Add Segment Manager - attempts to add the specified segment manager "
		  "to a disk or segment.\n");
    Indent_Printf(1, FALSE,
		  "Allocate - allocates regions and segments from freespace.\n");
    Indent_Printf(1, FALSE,
		  "Assign - assigns a segment manager to a disk/segment.\n");
    Indent_Printf(1, FALSE,
		  "Check - performs an fsck operation on a volume.\n");
    Indent_Printf(1, FALSE,
		  "Commit - causes any changes which have not yet been written to disk "
		  "to be written to disk.\n");
    Indent_Printf(1, FALSE,
		  "Convert - Attempts to convert a compatibility volume into "
		  "an EVMS native volume, or an EVMS native volume into a "
		  "compatibility volume.\n");
    Indent_Printf(1, FALSE,
		  "Create - creates volumes, EVMS objects, regions, segments and containers.\n");
    Indent_Printf(1, FALSE,
		  "Deactivate - deactivates volumes and storage objects.\n");
    Indent_Printf(1, FALSE,
		  "Delete - deletes volumes, containers, regions, segments, and EVMS objects.\n");
    Indent_Printf(1, FALSE,
		  "Echo  - Echoes a string to standard output.\n");
    Indent_Printf(1, FALSE,
		  "Exit  - exits the EVMS command line interpreter.\n");
    Indent_Printf(1, FALSE,
		  "Expand - expands the size of volumes, containers, regions, segments, and EVMS objects.\n");
    Indent_Printf(1, FALSE,
		  "Format - creates a filesystem on a volume.\n");
    Indent_Printf(1, FALSE,
		  "Help  - provides help on a specific EVMS command.\n");
    Indent_Printf(1, FALSE, "Mkfs  - creates a filesystem on a volume.\n");
    Indent_Printf(1, FALSE,
		  "Modify - Some of the options associated with a EVMS object, "
		  "region, container, disk or segment may be modified.  "
		  "This command or the Set command may be used to modify such options.\n");
    Indent_Printf(1, FALSE, "Mount  - mounts a volume on a directory.\n");
    Indent_Printf(1, FALSE,
		  "Probe - causes EVMS to check the system for hardware changes.\n");
    Indent_Printf(1, FALSE,
		  "Query - gathers information about the volumes, containers, "
		  "EVMS objects, regions, and segments in the system.\n");
    Indent_Printf(1, FALSE,
		  "Quit  - exits the EVMS command line interpreter.\n");
    Indent_Printf(1, FALSE,
		  "Remove - removes the segment manager assigned to a disk/segment.\n");
    Indent_Printf(1, FALSE, "Rename - changes the names of volumes.\n");
    Indent_Printf(1, FALSE,
		  "Replace - replace a storage object with another storage object.\n");
    Indent_Printf(1, FALSE,
		  "Revert - deconstructs a volume or storage object a layer at a time.\n");
    Indent_Printf(1, FALSE,
		  "Save  - causes any changes which have not yet been written to disk to be written to disk.\n");
    Indent_Printf(1, FALSE,
		  "Set   - Some of the options associated with a EVMS object, "
		  "region, container, disk or segment may be modified.  "
		  "This command or the Modify command may be used to modify such options.\n");
    Indent_Printf(1, FALSE,
		  "Shrink - reduces the size of volumes, containers, regions, segments, and EVMS objects.\n");
    Indent_Printf(1, FALSE,
		  "Task  - allows access to plug-in specific features.\n");
    Indent_Printf(1, FALSE,
		  "Unformat - removes a file system from a volume.  "
		  "The contents of the volume are lost.\n");
    Indent_Printf(1, FALSE,
		  "UnMkfs - removes a file system from a volume.  "
		  "The contents of the volume are lost.\n");
    Indent_Printf(1, FALSE, "Unmount - unmounts a volume.\n\n");

    My_Printf
	("To obtain detailed help on an EVMS command, enter the following at the EVMS command prompt:\n\n"
	 "Help: <command>\n\n" "where:\n");
    Indent_Printf(1, FALSE,
		  " <command> is one of the aforementioned commands.\n\n");

    Indent_Printf(1, FALSE,
		  "Example: Get detailed help on the Create command.\n\n");

    My_Printf("Help:Create\n\n");

    return;
}
