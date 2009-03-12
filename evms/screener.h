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
 * Module: screener.h
 */

/*
 * Change History:
 *
 * 6/2001  B. Rafanello  Initial version.
 *
 */

#include "token.h"  /* TokenCharacterizations, TokenType */

#ifndef SCREENER_H

#define SCREENER_H 1


/* For ease of parsing, all tokens characterized as KeyWord are converted so that
   their TokenText pointers point to one of the following standard keywords:       */

#ifdef SCREENER_C

char *   AcceptableStr    =  "ACCEPTABLE";
char *   ActStr           =  "ACT";
char *   ActivateStr      =  "ACTIVATE";
char *   AddStr           =  "ADD";
char *   AFStr            =  "AF";          /* Abbreviation for Add Feature command. */
char *   AllStr           =  "ALL";
char *   AllocateStr      =  "ALLOCATE";
char *   AsmStr           =  "ASM";         /* Abbreviation for the Add Segment Manager command. */
char *   AssignStr        =  "ASSIGN";
char *   AutoStr          =  "AUTO";
char *   AvailableStr     =  "AVAILABLE";
char *   CheckStr         =  "CHECK";
char *   ChildrenStr      =  "CHILDREN";
char *   ClusterStr       =  "CLUSTER";
char *   CMStr            =  "CM";
char *   CommitStr        =  "COMMIT";
char *   CompatibilityStr =  "COMPATIBILITY";
char *   ContainerStr     =  "CONTAINER";
char *   ContainersStr    =  "CONTAINERS";
char *   ConvertStr       =  "CONVERT";
char *   ConvertibleStr   =  "CONVERTIBLE";
char *   CreateStr        =  "CREATE";
char *   DeactStr         =  "DEACT";
char *   DeactivateStr    =  "DEACTIVATE";
char *   DeleteStr        =  "DELETE";
char *   DeviceStr        =  "DEVICE";
char *   DiskStr          =  "DISK";
char *   DisksStr         =  "DISKS";
char *   DistributedStr   =  "DISTRIBUTED";
char *   DLMStr           =  "DLM";
char *   DrStr            =  "DR";          /* Abbreviation for Delete Recursive */
char *   EchoStr          =  "ECHO";
char *   EIStr            =  "EI";
char *   EQStr            =  "EQ";
char *   EndStr           =  "END";
char *   EPStr            =  "EP";
char *   EVMSStr          =  "EVMS";
char *   ExitStr          =  "EXIT";
char *   ExpandStr        =  "EXPAND";
char *   ExpandableStr    =  "EXPANDABLE";
char *   ExtendedStr      =  "EXTENDED";
char *   FeatureStr       =  "FEATURE";
char *   FileStr          =  "FILE";
char *   FilesystemStr    =  "FILESYSTEM";
char *   FSIMStr          =  "FSIM";
char *   FormatStr        =  "FORMAT";
char *   FreespaceStr     =  "FREESPACE";
char *   GigabyteStr      =  "GB";
char *   GTStr            =  "GT";
char *   HelpStr          =  "HELP";
char *   HoldStr          =  "HOLD";
char *   HoursStr         =  "HOURS";
char *   InfoStr          =  "INFO";
char *   InterfaceStr     =  "INTERFACE";
char *   KilobyteStr      =  "KB";
char *   ListStr          =  "LIST";
char *   LockStr          =  "LOCK";
char *   LOStr            =  "LO";
char *   LTStr            =  "LT";
char *   ManagementStr    =  "MANAGEMENT";
char *   ManagerStr       =  "MANAGER";
char *   ManualStr        =  "MANUAL";
char *   MegabyteStr      =  "MB";
char *   MicrosecondsStr  =  "MICROSECONDS";
char *   MillisecondsStr  =  "MILLISECONDS";
char *   MinutesStr       =  "MINUTES";
char *   MkfsStr          =  "MKFS";
char *   ModifyStr        =  "MODIFY";
char *   ModuleStr        =  "MODULE";
char *   MountStr         =  "MOUNT";
char *   NameStr          =  "NAME";
char *   ObjectStr        =  "OBJECT";
char *   ObjectsStr       =  "OBJECTS";
char *   OptionsStr       =  "OPTIONS";
char *   ParentStr        =  "PARENT";
char *   PluginStr        =  "PLUGIN";
char *   PluginsStr       =  "PLUGINS";
char *   PointsStr        =  "POINTS";
char *   ProbeStr         =  "PROBE";
char *   QueryStr         =  "QUERY";
char *   QuitStr          =  "QUIT";
char *   RecursiveStr     =  "RECURSIVE";
char *   RegionStr        =  "REGION";
char *   RegionsStr       =  "REGIONS";
char *   ReleaseStr       =  "RELEASE";
char *   RemoveStr        =  "REMOVE";
char *   RenameStr        =  "RENAME";
char *   ReplaceStr       =  "REPLACE";
char *   RevertStr        =  "REVERT";
char *   SaveStr          =  "SAVE";
char *   SecondsStr       =  "SECONDS";
char *   SectorsStr       =  "SECTORS";
char *   SegmentStr       =  "SEGMENT";
char *   SegmentsStr      =  "SEGMENTS";
char *   SetStr           =  "SET";
char *   ShrinkStr        =  "SHRINK";
char *   ShrinkableStr    =  "SHRINKABLE";
char *   SizeStr          =  "SIZE";
char *   SPStr            =  "SP";
char *   StartStr         =  "START";
char *   TaskStr          =  "TASK";
char *   TerrabyteStr     =  "TB";
char *   TypeStr          =  "TYPE";
char *   UnclaimedStr     =  "UNCLAIMED";
char *   UnformatStr      =  "UNFORMAT";
char *   UnMkfsStr        =  "UNMKFS";
char *   UnmountStr       =  "UNMOUNT";
char *   VolumeStr        =  "VOLUME";
char *   VolumesStr       =  "VOLUMES";

#else

extern char *   AcceptableStr;
extern char *   ActStr;
extern char *   ActivateStr;
extern char *   AddStr;
extern char *   AFStr;
extern char *   AllStr;
extern char *   AllocateStr;
extern char *   AsmStr;
extern char *   AssignStr;
extern char *   AutoStr;
extern char *   AvailableStr;
extern char *   CheckStr;
extern char *   ChildrenStr;
extern char *   ClusterStr;
extern char *   CMStr;
extern char *   CommitStr;
extern char *   CompatibilityStr;
extern char *   ContainerStr;
extern char *   ContainersStr;
extern char *   ConvertibleStr;
extern char *   ConvertStr;
extern char *   CreateStr;
extern char *   DeactStr;
extern char *   DeactivateStr;
extern char *   DeleteStr;
extern char *   DeviceStr;
extern char *   DiskStr;
extern char *   DisksStr;
extern char *   DistributedStr;
extern char *   DLMStr;
extern char *   DrStr;
extern char *   EchoStr;
extern char *   EIStr;
extern char *   EQStr;
extern char *   EndStr;
extern char *   EPStr;
extern char *   EVMSStr;
extern char *   ExitStr;
extern char *   ExpandStr;
extern char *   ExpandableStr;
extern char *   ExtendedStr;
extern char *   FeatureStr;
extern char *   FileStr;
extern char *   FilesystemStr;
extern char *   FSIMStr;
extern char *   FormatStr;
extern char *   FreespaceStr;
extern char *   GigabyteStr;
extern char *   GTStr;
extern char *   HelpStr;
extern char *   HoldStr;
extern char *   HoursStr;
extern char *   InfoStr;
extern char *   InterfaceStr;
extern char *   KilobyteStr;
extern char *   ListStr;
extern char *   LockStr;
extern char *   LOStr;
extern char *   LTStr;
extern char *   ManagementStr;
extern char *   ManagerStr;
extern char *   ManualStr;
extern char *   MegabyteStr;
extern char *   MicrosecondsStr;
extern char *   MillisecondsStr;
extern char *   MinutesStr;
extern char *   MkfsStr;
extern char *   ModifyStr;
extern char *   ModuleStr;
extern char *   MountStr;
extern char *   NameStr;
extern char *   ObjectStr;
extern char *   ObjectsStr;
extern char *   OptionsStr;
extern char *   ParentStr;
extern char *   PluginStr;
extern char *   PluginsStr;
extern char *   PointsStr;
extern char *   ProbeStr;
extern char *   QueryStr;
extern char *   QuitStr;
extern char *   RecursiveStr;
extern char *   RegionStr;
extern char *   RegionsStr;
extern char *   ReleaseStr;
extern char *   RemoveStr;
extern char *   RenameStr;
extern char *   ReplaceStr;
extern char *   RevertStr;
extern char *   SaveStr;
extern char *   SectorsStr;
extern char *   SecondsStr;
extern char *   SegmentStr;
extern char *   SegmentsStr;
extern char *   SetStr;
extern char *   ShrinkStr;
extern char *   ShrinkableStr;
extern char *   SizeStr;
extern char *   SPStr;
extern char *   StartStr;
extern char *   TaskStr;
extern char *   TerrabyteStr;
extern char *   TypeStr;
extern char *   UnclaimedStr;
extern char *   UnformatStr;
extern char *   UnMkfsStr;
extern char *   UnmountStr;
extern char *   VolumeStr;
extern char *   VolumesStr;

#endif


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
TokenType * GetScreenedToken(void);


#endif
