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
 * Module: dlist.c
 *
 * Functions: dlist_t     CreateList
 *            int         InsertObject
 *            int         ExclusiveInsertObject
 *            int         DeleteAllItems
 *            int         GetObject
 *            int         BlindGetObject
 *            int         GetNextObject
 *            int         ExtractObject
 *            int         BlindExtractObject
 *            int         ReplaceObject
 *            int         GetHandle
 *            int         GetListSize
 *            boolean     ListEmpty
 *            boolean     AtEndOfList
 *            int         DestroyList
 *            int         NextItem
 *            int         PreviousItem
 *            int         GoToStartOfList
 *            int         GoToEndOfList
 *            int         GoToSpecifiedItem
 *            int         SortList
 *            int         ForEachItem
 *            int         PruneList
 *            int         AppendList
 *            int         CopyList
 *
 * Description:  This module implements a simple, generic, doubly linked list.
 *               Data objects of any type can be placed into a linked list
 *               created by this module.  Furthermore, data objects of different
 *               types may be placed into the same linked list.
 *
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dlist.h"		/*  Import dlist.h so that the compiler can check the  */
		      /*  consistency of the declarations in dlist.h against */
		      /*  those in this module.                              */

/*--------------------------------------------------
 * Private Constants
 --------------------------------------------------*/

/* Each list contains a Verify field.  Before any operations are performed on
   the list, this module checks the Verify field to see if the VerifyValue
   is in the field.  If the VerifyValue is not found in the Verify field,
   the operation is aborted.                                                 */

#define VerifyValue 39646966L


/*--------------------------------------------------
 * Private Type definitions
 --------------------------------------------------*/


/* A list has the following structure:

                                   dlist_t         An item of type dlist_t is a pointer
                                    |            to the following structure:
                                    |
                                    V
                          ----------------------
                          |                    |
                          |    ControlNode     |
                          ----------------------
                         /           /          \
         Pointer to the /           /            \
         LinkNode of   /           /              \  Pointer to the LinkNode of
         the first    /           /                \ the last item in the list.
         item in the /           / Pointer to the   \
         list.      /           /  LinkNode of the   \
                   /           /   Current Item in    \             NOTE:  All LinkNodes
                  /           /    the list.           \                   have a pointer
                 V           V                          V                  to the ControlNode.
        -------------      -------------                -------------
NULL <--|  LinkNode | <--> | LinkNode  |<-->  ...  <--> |  LinkNode |--> NULL
        -------------      -------------                -------------
             |                   |                            |
             |                   |                            |
             V                   V                            V
           Data                Data                         Data


NOTES:  The ControlNode does the beekeeping for things which affect the
        entire list.  It tracks the number of items in the list, where the
        first, current, and last items in the list reside.  It also holds
        the Verify field which is used to see if a pointer passed to us
        really does point to a list created by this module.

        Each data item placed in the list gets its own LinkNode.  The
        LinkNode tracks where in memory the data item is, how big the
        data item is, what item tag the user gave that data item, and
        where the LinkNodes for the previous and next items in the list
        are located.  By tracking this information here, the user does
        not have to worry about placing fields for this information
        inside of his data in order for his data items to be compatible
        with this module.  Thus, the operation of the list module is
        decoupled from the data that is placed in the list.

*/


/*--------------------------------------------------
 Private global variables.
--------------------------------------------------*/
boolean ErrorsFound = FALSE;	/* Used to track whether or not errors have
				   been found.  Can be used with a memory access
				   breakpoint to stop program execution when
				   an error is detected so that the type of
				   error can be seen.                              */


/*********************************************************************/
/*                                                                   */
/*   Function Name:  CreateList                                      */
/*                                                                   */
/*   Descriptive Name: This function allocates and initializes the   */
/*                     data structures associated with a list and    */
/*                     then returns a pointer to these structures.   */
/*                                                                   */
/*   Input: None.                                                    */
/*                                                                   */
/*   Output: If Success : The function return value will be non-NULL */
/*                                                                   */
/*           If Failure : The function return value will be NULL.    */
/*                                                                   */
/*   Error Handling:  The function will only fail if it can not      */
/*                    allocate enough memory to create the new list. */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
dlist_t CreateList(void)
{

    /* This function returns a DLIST.  A variable of type dlist_t is really a
       pointer to a ControlNode which has been typecast to (void *).
       We will therefore declare a variable of type (ControlNode *) to
       work with while in this function and then, if successful, typecast
       this variable to dlist_t for the return value.                           */
    ControlNode *ListData;

    /* We must now initialize the data structures for the dlist_t we are
       creating.  We will start by allocating the memory.                    */
    ListData = (ControlNode *) malloc(sizeof(ControlNode));
    if (ListData == NULL) {

	return NULL;
    }

    /* Now that we have the memory, lets initialize the fields in the master record. */
    ListData->ItemCount = 0;	/* No items in the list. */
    ListData->StartOfList = NULL;	/* Since the list is empty, there is no first item */

    ListData->EndOfList = NULL;	/* Since the list is empty, there is no last item */
    ListData->CurrentItem = NULL;	/* Since the list is empty, there is no current item */

#ifdef EVMS_DEBUG

    ListData->Verify = VerifyValue;	/* Initialize the Verify field so that this list will recognized as being valid. */

#endif

    /* Set the return value. */
    return (dlist_t) ListData;
}


/*********************************************************************/
/*                                                                   */
/*   Function Name: InsertObject                                     */
/*                                                                   */
/*   Descriptive Name:  This function inserts an object into a DLIST.*/
/*                      The object can be inserted before or after   */
/*                      the current item in the list.                */
/*                                                                   */
/*   Input:  dlist_t          ListToAddTo : The list to which the    */
/*                                          data object is to be     */
/*                                          inserted.                */
/*           ADDRESS       ItemLocation : The address of the data    */
/*                                        to append to the list      */
/*           TAG           ItemTag : The item tag to associate with  */
/*                                   the item being appended to the  */
/*                                   list                            */
/*           ADDRESS TargetHandle : The item in ListToAddTo which    */
/*                                   is used to determine where      */
/*                                   the item being transferred will */
/*                                   be placed.  If this is NULL,    */
/*                                   then the current item in        */
/*                                   ListToAddTo will be used.       */
/*           Insertion_Modes Insert_Mode : This indicates where,     */
/*                                   relative to the item in         */
/*                                   ListToAddTo specified by        */
/*                                   Target_Handle, the item being   */
/*                                   inserted can be placed.         */
/*           boolean MakeCurrent : If TRUE, the item being inserted  */
/*                                 into ListToAddTo becomes the      */
/*                                 current item in ListToAddTo.      */
/*           ADDRESS    * Handle : The address of a variable to hold */
/*                                 the handle for the item that was  */
/*                                 inserted into the list.           */
/*                                                                   */
/*   Output:  If all went well, the return value will be             */
/*            DLIST_SUCCESS and *Handle will contain the ADDRESS of  */
/*            the new item.  If errors were encountered, the   .     */
/*            return value will be the error code and *Handle will   */
/*            be NULL.                                               */
/*                                                                   */
/*   Error Handling: This function will fail under the following     */
/*                   conditions:                                     */
/*                       ListToAddTo does not point to a valid       */
/*                           list                                    */
/*                       ItemLocation is NULL                        */
/*                       The memory required for a LINK NODE can not */
/*                           be allocated.                           */
/*                       TargetHandle is invalid or is for an item   */
/*                           in another list.                        */
/*                   If this routine fails, an error code is returned*/
/*                   and any memory allocated by this function is    */
/*                   freed.                                          */
/*                                                                   */
/*   Side Effects: None.                                             */
/*                                                                   */
/*   Notes:  The item to insert is NOT copied to the heap.  Instead, */
/*           the location of the item is stored in the list.         */
/*           InsertObject stores the address provided by the user.   */
/*                                                                   */
/*           It is assumed that TargetHandle is valid, or is at least*/
/*           the address of an accessible block of storage.  If      */
/*           TargetHandle is invalid, or is not the address of an    */
/*           accessible block of storage, then a trap or exception   */
/*           may occur.                                              */
/*                                                                   */
/*           It is assumed that if ItemLocation is not NULL, then    */
/*           it is a valid address that can be dereferenced.  If     */
/*           this assumption is violated, an exception or trap may   */
/*           occur.                                                  */
/*                                                                   */
/*********************************************************************/
int
InsertObject(dlist_t ListToAddTo,
	     ADDRESS ItemLocation,
	     TAG ItemTag,
	     ADDRESS TargetHandle,
	     Insertion_Modes Insert_Mode,
	     boolean MakeCurrent, ADDRESS * Handle)
{

    /* Since ListToAddTo is of type DLIST, we can not use it without having
       to type cast it each time.  To avoid all of the type casting, we
       will declare a local variable of type ControlNode * and then
       initialize it once using ListToAddTo.  This way we just do the
       cast once.                                                            */

    ControlNode *ListData;

    LinkNode *NewNode;		/* Used to create the LinkNode for the new item. */
    LinkNode *CurrentNode;	/* Used to hold the reference point for the insertion. */
    LinkNode *PreviousNode;	/* Used to point to the item prior to CurrentNode in the list while
				   the new item is being inserted. */
    LinkNode *NextNode;		/* Used to point to the item after CurrentNode in the list while
				   the new item is being inserted.                                   */

    /* Initialize the return handle in case we bail out with an error. */

    *Handle = NULL;

    /* We will assume that ListToAppendTo points to a valid list.  Given this,
       we will initialize ListData to point to the ControlNode of this
       list.                                                              */
    ListData = (ControlNode *) ListToAddTo;

    /* Has the user specified a specific item in the list as a reference point for this insertion? */
    if (TargetHandle != NULL) {

	/* Since the user has specified a reference point for this insertion, set up to use it. */
	CurrentNode = (LinkNode *) TargetHandle;

    } else {

	/* The user did not specify a reference point, so use the current item in the list as the reference point. */
	CurrentNode = ListData->CurrentItem;

    }

#ifdef EVMS_DEBUG

    /* We must now validate the list before we attempt to use it.  We will
       do this by checking the Verify field in the ControlNode.               */
    if ((ListData == NULL) || (ListData->Verify != VerifyValue)) {
	return DLIST_NOT_INITIALIZED;
    }

    /* Since the list is valid, we must now see if the TargetHandle is valid.  We
       will assume that, if the TargetHandle is not NULL, it points to a LinkNode.
       If the ControlNodeLocation field of the LinkNode points to the
       ControlNode for the list we are working with, then the LinkNode is in
       the list and can therefore be used safely.

       At this point, CurrentNode has been set equal to TargetHandle if TargetHandle
       is not NULL.  If TargetHandle is NULL, then CurrentNode was set to the current
       item in the list.                                                              */
    if (TargetHandle != NULL) {

	/* Is CurrentNode part of this list? */
	if (CurrentNode->ControlNodeLocation != ListData) {

	    /* The handle either did not point to a ControlNode or it pointed to the wrong ControlNode! */
	    return DLIST_BAD_HANDLE;
	}

    }

    /* We must check the insertion mode. */
    if (Insert_Mode > AppendToList) {
	return DLIST_INVALID_INSERTION_MODE;
    }

    /* Lets check the item being added to the DLIST. */
    if (ItemLocation == NULL) {
	return DLIST_BAD_ITEM_POINTER;
    }
#endif

    /* Since both the list and item are valid, lets make a LinkNode. */
    NewNode = (LinkNode *) malloc(sizeof(LinkNode));

    /* Did we get the memory? */
    if (NewNode == NULL) {
	return DLIST_OUT_OF_MEMORY;
    }

    /* Now that all memory has been allocated, lets finish initializing the LinkNode. */
    NewNode->DataLocation = ItemLocation;
    NewNode->DataTag = ItemTag;
    NewNode->NextLinkNode = NULL;
    NewNode->PreviousLinkNode = NULL;
    NewNode->ControlNodeLocation = ListData;	/* Initialize the link to the control node
						   of the list containing this link node.   */

    /* Now we can add the node to the list. */

    /* Is the list empty?  If so, then the Insertion_Mode does not matter! */
    if (ListData->CurrentItem == NULL) {
	/* The List is empty.  This will be the first (and only) item in the list.
	   Also, since this will be the only item in the list, it automatically
	   becomes the current item.                                               */
	ListData->EndOfList = NewNode;
	ListData->StartOfList = NewNode;
	ListData->CurrentItem = NewNode;
    } else {
	/* The list was not empty.  */

	/* Now lets insert the item according to the specified Insert_Mode. */
	switch (Insert_Mode) {

	case InsertAtStart:	/* Get the first item in the list. */
	    CurrentNode = ListData->StartOfList;

	    /* Now insert NewNode before CurrentNode. */
	    NewNode->NextLinkNode = CurrentNode;
	    CurrentNode->PreviousLinkNode = NewNode;

	    /* Now update the ControlNode. */
	    ListData->StartOfList = NewNode;

	    break;
	case InsertBefore:	/* CurrentNode already points to the Item we are to insert NewNode before. */

	    /* Is CurrentNode the first item in the list? */
	    if (ListData->StartOfList != CurrentNode) {

		/* Since CurrentNode is not the first item in the list, we need the node prior to CurrentNode
		   so we can adjust its link fields.                                                           */
		PreviousNode = CurrentNode->PreviousLinkNode;

		/* Now make PreviousLinkNode point to NewNode and vice versa. */
		PreviousNode->NextLinkNode = NewNode;
		NewNode->PreviousLinkNode = PreviousNode;

	    } else {

		/* Since CurrentNode is the first item in the list, that means that NewNode will be
		   the first item in the list after it is inserted.  Update the ControlNode for this
		   list to reflect that NewNode will be the first item in the list.                     */
		ListData->StartOfList = NewNode;

	    }

	    /* Now make NewNode point to CurrentNode and vice versa. */
	    NewNode->NextLinkNode = CurrentNode;
	    CurrentNode->PreviousLinkNode = NewNode;

	    break;
	case InsertAfter:	/* CurrentNode already points to the Item we are to insert NewNode after. */

	    /* Is CurrentNode the last item in the list? */
	    if (ListData->EndOfList != CurrentNode) {

		/* Since CurrentNode is not the last item in the list, we need the node after to CurrentNode
		   so we can adjust its link fields.                                                           */
		NextNode = CurrentNode->NextLinkNode;

		/* Now make NextLinkNode point to NewNode and vice versa. */
		NextNode->PreviousLinkNode = NewNode;
		NewNode->NextLinkNode = NextNode;

	    } else {

		/* Since CurrentNode is the last item in the list, that means that NewNode will be
		   the last item in the list after it is inserted.  Update the ControlNode for this
		   list to reflect that NewNode will be the last item in the list.                     */
		ListData->EndOfList = NewNode;

	    }

	    /* Now make NewNode point to CurrentNode and vice versa. */
	    CurrentNode->NextLinkNode = NewNode;
	    NewNode->PreviousLinkNode = CurrentNode;

	    break;
	case AppendToList:	/* Get the last item in the list. */
	    CurrentNode = ListData->EndOfList;

	    /* Now insert NewNode after CurrentNode. */
	    CurrentNode->NextLinkNode = NewNode;
	    NewNode->PreviousLinkNode = CurrentNode;

	    /* Now update the ControlNode. */
	    ListData->EndOfList = NewNode;

	    break;
	default:
	    NewNode->ControlNodeLocation = NULL;
	    free(NewNode->DataLocation);
	    free(NewNode);
	    return DLIST_INVALID_INSERTION_MODE;

	}

    }

    /* Adjust the count of the number of items in the list. */
    ListData->ItemCount++;

    /* Should the new node become the current item in the list? */
    if (MakeCurrent) {

	/* Adjust the control node so that NewNode becomes the current item in the list. */
	ListData->CurrentItem = NewNode;

    }

    /* All done.  Return the handle.  Signal successful operation. */
    *Handle = NewNode;

    return DLIST_SUCCESS;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: ExclusiveInsertObject                            */
/*                                                                   */
/*   Descriptive Name:  This function inserts an object into a       */
/*                      dlist_t.  The object can be inserted before  */
/*                      or after the current item in the list. If    */
/*                      object is already in the list, it is not     */
/*                      added again.                                 */
/*                                                                   */
/*   Input:  dlist_t        ListToAddTo : The list to which the      */
/*                                        data object is to be       */
/*                                        inserted.                  */
/*           ADDRESS       ItemLocation : The address of the data    */
/*                                        to append to the list      */
/*           TAG           ItemTag : The item tag to associate with  */
/*                                   the item being appended to the  */
/*                                   list                            */
/*           ADDRESS TargetHandle : The item in ListToAddTo which    */
/*                                   is used to determine where      */
/*                                   the item being transferred will */
/*                                   be placed.  If this is NULL,    */
/*                                   then the current item in        */
/*                                   ListToAddTo will be used.       */
/*           Insertion_Modes Insert_Mode : This indicates where,     */
/*                                   relative to the item in         */
/*                                   ListToAddTo specified by        */
/*                                   Target_Handle, the item being   */
/*                                   inserted can be placed.         */
/*           boolean MakeCurrent : If TRUE, the item being inserted  */
/*                                 into ListToAddTo becomes the      */
/*                                 current item in ListToAddTo.      */
/*           ADDRESS    * Handle : The address of a variable to hold */
/*                                 the handle for the item that was  */
/*                                 inserted into the list.           */
/*                                                                   */
/*   Output:  If all went well, the return value will be             */
/*            DLIST_SUCCESS and *Handle will contain the ADDRESS of  */
/*            the new item.  If errors were encountered, the   .     */
/*            return value will be the error code and *Handle will   */
/*            be NULL.                                               */
/*                                                                   */
/*   Error Handling: This function will fail under the following     */
/*                   conditions:                                     */
/*                       ListToAddTo does not point to a valid       */
/*                           list                                    */
/*                       ItemLocation is NULL                        */
/*                       The memory required for a LINK NODE can not */
/*                           be allocated.                           */
/*                       TargetHandle is invalid or is for an item   */
/*                           in another list.                        */
/*                   If this routine fails, an error code is returned*/
/*                   and any memory allocated by this function is    */
/*                   freed.                                          */
/*                                                                   */
/*   Side Effects: None.                                             */
/*                                                                   */
/*   Notes:  The item to insert is NOT copied to the heap.  Instead, */
/*           the location of the item is stored in the list.         */
/*           InsertObject stores the address provided by the user.   */
/*                                                                   */
/*           It is assumed that TargetHandle is valid, or is at least*/
/*           the address of an accessible block of storage.  If      */
/*           TargetHandle is invalid, or is not the address of an    */
/*           accessible block of storage, then a trap or exception   */
/*           may occur.                                              */
/*                                                                   */
/*           It is assumed that if ItemLocation is not NULL, then    */
/*           it is a valid address that can be dereferenced.  If     */
/*           this assumption is violated, an exception or trap may   */
/*           occur.                                                  */
/*                                                                   */
/*********************************************************************/
int
ExclusiveInsertObject(dlist_t ListToAddTo,
		      ADDRESS ItemLocation,
		      TAG ItemTag,
		      ADDRESS TargetHandle,
		      Insertion_Modes Insert_Mode,
		      boolean MakeCurrent, ADDRESS * Handle)
{
    /* Since ListToProcess is of type DLIST, we can not use it without
       having to type cast it each time.  To avoid all of the type casting,
       we will declare a local variable of type ControlNode * and then
       initialize it once using ListToProcess.  This way we just do the
       cast once.                                                            */
    ControlNode *ListData;

    LinkNode *CurrentLinkNode;	/* Used to point to the LinkNode of the
				   current item while we access its data.
				   This limits the levels of indirection
				   to one, which should result in faster
				   execution. */

    int Error;

    /* We will assume that ListToAddTo points to a valid list.  Given this,
       we will initialize ListData to point to the ControlNode of this
       list.                                                                     */
    ListData = (ControlNode *) ListToAddTo;


#ifdef EVMS_DEBUG

    /* We must now validate the list before we attempt to use it.  We will
       do this by checking the Verify field in the ControlNode.               */
    if ((ListData == NULL) || (ListData->Verify != VerifyValue)) {
	return DLIST_NOT_INITIALIZED;
    }
#endif

    /* If the list has items in it, search for the specified item */
    if (ListData->ItemCount != 0) {
	/* Get the first link node in the list. */
	CurrentLinkNode = ListData->StartOfList;

	/* Now loop through the items in the list. */
	while (CurrentLinkNode != NULL) {
	    if (CurrentLinkNode->DataLocation == ItemLocation) {
		/* Item is already in the list. */
		return DLIST_SUCCESS;
	    }
	    CurrentLinkNode = CurrentLinkNode->NextLinkNode;
	}
    }

    /* The item is not already in the list, so add it */
    Error = InsertObject(ListToAddTo,
			 ItemLocation,
			 ItemTag,
			 TargetHandle, Insert_Mode, MakeCurrent, Handle);

    /* All items in the list have been processed. */
    return Error;
}


/*********************************************************************/
/*                                                                   */
/*   Function Name:  DeleteObject                                    */
/*                                                                   */
/*   Descriptive Name:  This function removes the specified object   */
/*                      from the list.                               */
/*                                                                   */
/*   Input:  dlist_t     ListToDeleteFrom : The list whose current   */
/*                                          item is to be deleted.   */
/*           ADDRESS Object : The address of the object to be removed*/
/*                            from the list.                         */
/*                                                                   */
/*   Output:  Return DLIST_SUCCESS if successful, else an error code.*/
/*                                                                   */
/*   Error Handling: This function will fail if ListToDeleteFrom is  */
/*                   not a valid list, or if ListToDeleteFrom is     */
/*                   empty, or if Handle is invalid.                 */
/*                   If this routine fails, an error code is         */
/*                   returned.                                       */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  This function does not alter which item is the current  */
/*           item in the list, unless the handle specified belongs   */
/*           to the current item in the list.                        */
/*                                                                   */
/*********************************************************************/
int DeleteObject(dlist_t ListToDeleteFrom, ADDRESS Object)
{

    /* Since ListToProcess is of type DLIST, we can not use it without
       having to type cast it each time.  To avoid all of the type casting,
       we will declare a local variable of type ControlNode * and then
       initialize it once using ListToProcess.  This way we just do the
       cast once.                                                            */
    ControlNode *ListData;


    LinkNode *CurrentLinkNode;	/* Used to point to the LinkNode of the
				   current item while we access its data.
				   This limits the levels of indirection
				   to one, which should result in faster
				   execution. */

    int Error;
    ADDRESS TmpObject;

    /* We will assume that ListToProcess points to a valid list.  Given this,
       we will initialize ListData to point to the ControlNode of this
       list.                                                                     */
    ListData = (ControlNode *) ListToDeleteFrom;


#ifdef EVMS_DEBUG

    /* We must now validate the list before we attempt to use it.  We will
       do this by checking the Verify field in the ControlNode.               */
    if ((ListData == NULL) || (ListData->Verify != VerifyValue)) {
	return DLIST_NOT_INITIALIZED;
    }
#endif

    /* Assume success. */
    Error = DLIST_OBJECT_NOT_FOUND;

    /* Check for empty list. */
    if (ListData->ItemCount == 0) {
	return DLIST_OBJECT_NOT_FOUND;
    }

    /* Get the first link node in the list. */
    CurrentLinkNode = ListData->StartOfList;

    /* Now loop through the items in the list. */
    while (CurrentLinkNode != NULL) {

	if (CurrentLinkNode->DataLocation == Object) {
	    Error =
		ExtractObject(ListToDeleteFrom, CurrentLinkNode->DataTag,
			      CurrentLinkNode, &TmpObject);
	    break;

	} else {
	    CurrentLinkNode = CurrentLinkNode->NextLinkNode;
	}
    }

    /* All items in the list have been processed. */
    return Error;
}

/*********************************************************************/
/*                                                                   */
/*   Function Name:  DeleteAllItems                                  */
/*                                                                   */
/*   Descriptive Name:  This function deletes all of the items in the*/
/*                      specified list and optionally frees the      */
/*                      memory associated with each item deleted.    */
/*                                                                   */
/*   Input:  dlist_t       ListToDeleteFrom : The list whose items   */
/*                                            are to be deleted.     */
/*           boolean    FreeMemory : If TRUE, then the memory        */
/*                                   associated with each item in the*/
/*                                   list will be freed.  If FALSE   */
/*                                   then the each item will be      */
/*                                   removed from the list but its   */
/*                                   memory will not be freed.       */
/*                                                                   */
/*   Output:  Return DLIST_SUCCESS if successful, else an error code.*/
/*                                                                   */
/*   Error Handling: This function will fail if ListToDeleteFrom is  */
/*                   not a valid list, or if ListToDeleteFrom is     */
/*                   empty.                                          */
/*                   If this routine fails, an error code is         */
/*                   returned.                                       */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  Items in a list can be accessed in two ways:  A copy of */
/*           the item can be obtained using GetObject and its related*/
/*           calls, or a pointer to the item can be obtained using   */
/*           GetObject and its related calls.  If you have a copy of */
/*           the data and wish to remove the item from the list, set */
/*           FreeMemory to TRUE.  This will remove the item from the */
/*           list and deallocate the memory used to hold it.  If you */
/*           have a pointer to the item in the list (from one of the */
/*           GetObject style functions) and wish to remove the item  */
/*           from the list, set FreeMemory to FALSE.  This removes   */
/*           the item from the list without freeing its memory, so   */
/*           that the pointer obtained with the GetObject style      */
/*           functions is still useable.                             */
/*                                                                   */
/*********************************************************************/
int DeleteAllItems(dlist_t ListToDeleteFrom, boolean FreeMemory)
{

    /* Since ListToDeleteFrom is of type DLIST, we can not use it without
       having to type cast it each time.  To avoid all of the type casting,
       we will declare a local variable of type ControlNode * and then
       initialize it once using ListToDeleteFrom.  This way we just do the
       cast once.                                                            */
    ControlNode *ListData;


    LinkNode *CurrentLinkNode;	/* This is used to walk through the
				   linked list of LinkNodes.        */


    /* We will assume that ListToDeleteFrom points to a valid list.  Given this,
       we will initialize ListData to point to the ControlNode of this
       list.                                                                 */
    ListData = (ControlNode *) (ListToDeleteFrom);


#ifdef EVMS_DEBUG

    /* We must now validate the list before we attempt to use it.  We will
       do this by checking the Verify field in the ControlNode.               */
    if ((ListData == NULL) || (ListData->Verify != VerifyValue)) {
	return DLIST_NOT_INITIALIZED;
    }
#endif

  /*--------------------------------------------------
     To empty a DLIST, we must traverse the linked
     list of LinkNodes and dispose of each LinkNode,
     as well as the data item associated with each
     LinkNode.
  --------------------------------------------------*/

    /* Loop to dispose of the ListNodes. */
    while (ListData->ItemCount > 0) {
	CurrentLinkNode = ListData->StartOfList;	/* Get the first dlist_t node. */
	ListData->StartOfList = CurrentLinkNode->NextLinkNode;	/* Remove that dlist_t node from the DLIST. */
	ListData->ItemCount--;	/* Decrement the number of items in the list or we will never leave the loop! */
	if ((CurrentLinkNode->DataLocation != NULL) && FreeMemory) {

	    free(CurrentLinkNode->DataLocation);

	}

	CurrentLinkNode->ControlNodeLocation = NULL;

	free(CurrentLinkNode);
    }

    /* Since there are no items in the list, set the CurrentItem and EndOfList pointers to NULL. */
    ListData->CurrentItem = NULL;
    ListData->EndOfList = NULL;

    /* Signal success. */
    return DLIST_SUCCESS;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:  GetObject                                       */
/*                                                                   */
/*   Descriptive Name:  This function returns the address of the data*/
/*                      associated with the specified item in the    */
/*                      list.                                        */
/*                                                                   */
/*   Input:  dlist_t ListToGetItemFrom : The list whose current item */
/*                                       is to have its address      */
/*                                       returned to the caller.     */
/*           TAG     ItemTag : What the caller thinks the item tag   */
/*                             of the current item is.               */
/*           ADDRESS Handle : The handle of the item to get.  This   */
/*                            handle must be of an item which resides*/
/*                            in ListToGetItemFrom, or NULL.  If     */
/*                            NULL, then the current item in the list*/
/*           boolean MakeCurrent : If TRUE, the item to get will     */
/*                                 become the current item in the    */
/*                                 list.                             */
/*           ADDRESS   * Object : The address of a variable to hold  */
/*                                the ADDRESS of data associated     */
/*                                with the current item.             */
/*                                                                   */
/*   Output:  If Successful :                                        */
/*                 Return DLIST_SUCCESS.                             */
/*                 *Object will be the address of the data           */
/*                 associated with the current item in the list.     */
/*            If Failure :                                           */
/*                 Return an error code.                             */
/*                 *Object will be NULL.                             */
/*                                                                   */
/*   Error Handling: This function will fail under any of the        */
/*                   following conditions:                           */
/*                         ListToGetItemFrom is not a valid list     */
/*                         ItemTag does not match the item tag       */
/*                             of the current item in the list       */
/*                         Handle is invalid, or is for an item      */
/*                             which is not in ListToGetItemFrom     */
/*                   If any of these conditions occur, an error code */
/*                   will be returned.                               */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  The user should not free the memory associated with     */
/*           the address returned by this function as the object is  */
/*           still in the list.                                      */
/*                                                                   */
/*           It is assumed that Handle is valid, or is at least the  */
/*           address of an accessible block of storage.  If Handle   */
/*           is invalid, or is not the address of an accessible block*/
/*           of storage, then a trap or exception may occur.         */
/*           NOTE: For this function, NULL is considered a valid     */
/*                 handle designating the current item in the list.  */
/*                                                                   */
/*           It is assumed that Object is a valid address.  If not,  */
/*           an exception or trap may occur.                         */
/*                                                                   */
/*           This function does not alter which item is the current  */
/*           item in the list.                                       */
/*                                                                   */
/*********************************************************************/
int
GetObject(dlist_t ListToGetItemFrom,
	  TAG ItemTag, ADDRESS Handle, boolean MakeCurrent,
	  ADDRESS * Object)
{
    /* Since ListToGetItemFrom is of type DLIST, we can not use it without
       having to type cast it each time.  To avoid all of the type casting,
       we will declare a local variable of type ControlNode * and then
       initialize it once using ListToGetItemFrom.  This way we just do the
       cast once.                                                            */

    ControlNode *ListData;


    LinkNode *CurrentLinkNode;	/* Used to point to the LinkNode of the item to return. */


    /* Initialize the return address in case we bail out with an error. */

    *Object = NULL;

    /* We will assume that ListToGetItemFrom points to a valid list.  Given this,
       we will initialize ListData to point to the ControlNode of this
       list.                                                                     */

    ListData = (ControlNode *) ListToGetItemFrom;


#ifdef EVMS_DEBUG

    /* We must now validate the list before we attempt to use it.  We will
       do this by checking the Verify field in the ControlNode.               */
    if ((ListData == NULL) || (ListData->Verify != VerifyValue)) {
	return DLIST_NOT_INITIALIZED;
    }
#endif

    /* Check to see if the dlist_t is empty. */
    if (ListData->ItemCount == 0) {
	return DLIST_EMPTY;
    }

    /* Were we given a handle? */
    if (Handle != NULL) {

	/* Lets check the handle we were given. */
	CurrentLinkNode = (LinkNode *) Handle;

	/* Is the handle valid? */
	if (CurrentLinkNode->ControlNodeLocation != ListData) {

	    /* The handle is not valid!  Abort! */
	    return DLIST_BAD_HANDLE;

	}

    } else {

	/* Since we were not given a handle, we will use the current item in the list for this operation. */
	CurrentLinkNode = ListData->CurrentItem;

    }

    /* We must check the actual item tag against the item tag expected by the user.  A mismatch could lead to errors! */
    if (CurrentLinkNode->DataTag != ItemTag) {
	return DLIST_ITEM_TAG_WRONG;
    }

    /* Does the user want this item made the current item in the list? */
    if (MakeCurrent) {

	/* Make this item the current item in the list. */
	ListData->CurrentItem = CurrentLinkNode;

    }

    /* Since everything checks out, lets signal success and return the address of the data. */
    *Object = CurrentLinkNode->DataLocation;
    return DLIST_SUCCESS;

}

/*********************************************************************/
/*                                                                   */
/*   Function Name:  BlindGetObject                                  */
/*                                                                   */
/*   Descriptive Name:  This function returns the address of the data*/
/*                      associated with the specified item in the    */
/*                      list.                                        */
/*                                                                   */
/*   Input:  dlist_t ListToGetItemFrom : The list whose current      */
/*                                       item is to have its address */
/*                                       returned to the caller.     */
/*           TAG *   ItemTag : The tag of the current item           */
/*           ADDRESS Handle : The handle of the item to get.  This   */
/*                            handle must be of an item which resides*/
/*                            in ListToGetItemFrom, or NULL.  If     */
/*                            NULL, then the current item in the list*/
/*           boolean MakeCurrent : If TRUE, the item to get will     */
/*                                 become the current item in the    */
/*                                 list.                             */
/*           ADDRESS   * Object : The address of a variable to hold  */
/*                                the ADDRESS of data associated     */
/*                                with the current item.             */
/*                                                                   */
/*   Output:  If Successful :                                        */
/*                 Return DLIST_SUCCESS.                             */
/*                 *Object will be the address of the data           */
/*                 associated with the current item in the list.     */
/*            If Failure :                                           */
/*                 Return an error code.                             */
/*                 *Object will be NULL.                             */
/*                                                                   */
/*   Error Handling: This function will fail under any of the        */
/*                   following conditions:                           */
/*                         ListToGetItemFrom is not a valid list     */
/*                         ItemTag does not match the item tag       */
/*                             of the current item in the list       */
/*                         Handle is invalid, or is for an item      */
/*                             which is not in ListToGetItemFrom     */
/*                   If any of these conditions occur, an error code */
/*                   will be returned.                               */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  The user should not free the memory associated with     */
/*           the address returned by this function as the object is  */
/*           still in the list.                                      */
/*                                                                   */
/*           It is assumed that Handle is valid, or is at least the  */
/*           address of an accessible block of storage.  If Handle   */
/*           is invalid, or is not the address of an accessible block*/
/*           of storage, then a trap or exception may occur.         */
/*           NOTE: For this function, NULL is considered a valid     */
/*                 handle designating the current item in the list.  */
/*                                                                   */
/*           It is assumed that Object is a valid address.  If not,  */
/*           an exception or trap may occur.                         */
/*                                                                   */
/*           This function does not alter which item is the current  */
/*           item in the list.                                       */
/*                                                                   */
/*********************************************************************/
int
BlindGetObject(dlist_t ListToGetItemFrom,
	       TAG * ItemTag,
	       ADDRESS Handle, boolean MakeCurrent, ADDRESS * Object)
{

    /* Since ListToGetItemFrom is of type DLIST, we can not use it without
       having to type cast it each time.  To avoid all of the type casting,
       we will declare a local variable of type ControlNode * and then
       initialize it once using ListToGetItemFrom.  This way we just do the
       cast once.                                                            */

    ControlNode *ListData;


    LinkNode *CurrentLinkNode;	/* Used to point to the LinkNode of the item to return. */


    /* Initialize the return address in case we bail out with an error. */

    *Object = NULL;

    /* We will assume that ListToGetItemFrom points to a valid list.  Given this,
       we will initialize ListData to point to the ControlNode of this
       list.                                                                     */

    ListData = (ControlNode *) ListToGetItemFrom;


#ifdef EVMS_DEBUG

    /* We must now validate the list before we attempt to use it.  We will
       do this by checking the Verify field in the ControlNode.               */
    if ((ListData == NULL) || (ListData->Verify != VerifyValue)) {
	return DLIST_NOT_INITIALIZED;
    }
#endif

    /* Check to see if the dlist_t is empty. */
    if (ListData->ItemCount == 0) {
	return DLIST_EMPTY;
    }

    /* Were we given a handle? */
    if (Handle != NULL) {

	/* Lets check the handle we were given. */
	CurrentLinkNode = (LinkNode *) Handle;

	/* Is the handle valid? */
	if (CurrentLinkNode->ControlNodeLocation != ListData) {

	    /* The handle is not valid!  Abort! */
	    return DLIST_BAD_HANDLE;

	}

    } else {

	/* Since we were not given a handle, we will use the current item in the list for this operation. */
	CurrentLinkNode = ListData->CurrentItem;

    }

    /* Return the tag to the user */
    *ItemTag = CurrentLinkNode->DataTag;

    /* Does the user want this item made the current item in the list? */
    if (MakeCurrent) {

	/* Make this item the current item in the list. */
	ListData->CurrentItem = CurrentLinkNode;

    }

    /* Since everything checks out, lets signal success and return the address of the data. */
    *Object = CurrentLinkNode->DataLocation;
    return DLIST_SUCCESS;
}

/*********************************************************************/
/*                                                                   */
/*   Function Name:  GetNextObject                                   */
/*                                                                   */
/*   Descriptive Name:  This function advances the current item      */
/*                      pointer and then returns the address of the  */
/*                      data associated with the current item in the */
/*                      list.                                        */
/*                                                                   */
/*   Input:  dlist_t ListToGetItemFrom : The list whose current item */
/*                                       is to be copied and returned*/
/*                                       to the caller.              */
/*           TAG     ItemTag : What the caller thinks the item tag   */
/*                             of the current item is.               */
/*           ADDRESS   * Object : The address of a variable to hold  */
/*                                the ADDRESS of data associated     */
/*                                with the next item.                */
/*                                                                   */
/*   Output:  If Successful :                                        */
/*                 Return DLIST_SUCCESS.                             */
/*                 *Object will be the address of the data           */
/*                 associated with the current item in the list.     */
/*            If Failure :                                           */
/*                 Return an error code.                             */
/*                 *Object will be NULL.                             */
/*                 The current item pointer will NOT be advanced.    */
/*                     The current item in the list will be the same */
/*                     as before the call to this function.          */
/*                                                                   */
/*   Error Handling: This function will fail under any of the        */
/*                   following conditions:                           */
/*                         ListToGetItemFrom is not a valid list     */
/*                         ItemTag does not match the item tag       */
/*                             of the current item in the list       */
/*                         The current item in the list before this  */
/*                             function is called is the last item   */
/*                             item in the list.                     */
/*                   If any of these conditions occur, an error code */
/*                   will be returned.                               */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  The user should not free the memory associated with     */
/*           the address returned by this function as the object is  */
/*           still in the list.                                      */
/*                                                                   */
/*           It is assumed that Object is a valid address.  If not,  */
/*           an exception or trap may occur.                         */
/*                                                                   */
/*********************************************************************/
int GetNextObject(dlist_t ListToGetItemFrom, TAG ItemTag, ADDRESS * Object)
{
    /* Since ListToGetItemFrom is of type DLIST, we can not use it without
       having to type cast it each time.  To avoid all of the type casting,
       we will declare a local variable of type ControlNode * and then
       initialize it once using ListToAdvance.  This way we just do the
       cast once.                                                            */
    ControlNode *ListData;


    LinkNode *CurrentLinkNode;	/* Used to point to the LinkNode of the
				   current item while we access its data.
				   This limits the levels of indirection
				   to one, which should result in faster
				   execution. */
    LinkNode *OriginalCurrentLinkNode;	/* This is used to hold the value
					   of the Current Item pointer
					   as it was upon entry to this
					   function.  If there is an
					   error, the Current Item
					   pointer will be reset to the
					   value stored in this variable. */

    /* Initialize the return address in case we bail out with an error. */

    *Object = NULL;

    /* We will assume that ListToGetItemFrom points to a valid list.  Given this,
       we will initialize ListData to point to the ControlNode of this
       list.                                                                     */
    ListData = (ControlNode *) ListToGetItemFrom;


#ifdef EVMS_DEBUG

    /* We must now validate the list before we attempt to use it.  We will
       do this by checking the Verify field in the ControlNode.               */
    if ((ListData == NULL) || (ListData->Verify != VerifyValue)) {
	return DLIST_NOT_INITIALIZED;
    }
#endif

    /* Check for empty list. */
    if (ListData->ItemCount == 0) {
	return DLIST_EMPTY;
    }

    /* Check for end of list. */
    if (ListData->CurrentItem == ListData->EndOfList) {
	return DLIST_END_OF_LIST;
    }

    /* Save the current item pointer so that we can restore it should something
       go wrong later.                                                          */
    OriginalCurrentLinkNode = ListData->CurrentItem;

    /* Advance the current item pointer. */
    CurrentLinkNode = ListData->CurrentItem;
    ListData->CurrentItem = CurrentLinkNode->NextLinkNode;

    /* Lets get the current node from the DLIST. */
    CurrentLinkNode = ListData->CurrentItem;

    /* We must check the actual item tag against the item tag expected by the user.  A mismatch could lead to errors! */
    if (CurrentLinkNode->DataTag != ItemTag) {
	ListData->CurrentItem = OriginalCurrentLinkNode;
	return DLIST_ITEM_TAG_WRONG;
    }

    /* Since everything checks out, lets signal success and return the address of the data. */
    *Object = CurrentLinkNode->DataLocation;
    return DLIST_SUCCESS;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:  ExtractObject                                   */
/*                                                                   */
/*   Descriptive Name:  This function returns the address of the data*/
/*                      associated with the specified item in the    */
/*                      list and then removes that item from the list*/
/*                                                                   */
/*   Input:  dlist_t ListToGetItemFrom : The list whose current item */
/*                                       is to be copied and returned*/
/*                                       to the caller.              */
/*           TAG     ItemTag : What the caller thinks the item tag   */
/*                             of the current item is.               */
/*           ADDRESS Handle : The handle of the item to get.  This   */
/*                            handle must be of an item which resides*/
/*                            in ListToGetItemFrom, or NULL.  If     */
/*                            NULL, then the current item in the     */
/*                            list will be used.                     */
/*           ADDRESS   * Object : The address of a variable to hold  */
/*                                the ADDRESS of data associated     */
/*                                with the current item.             */
/*                                                                   */
/*   Output:  If Successful :                                        */
/*                 Return DLIST_SUCCESS.                             */
/*                 *Object will be the address of the data           */
/*                 associated with the current item in the list.     */
/*            If Failure :                                           */
/*                 Return an error code.                             */
/*                 *Object will be NULL.                             */
/*                                                                   */
/*   Error Handling: This function will fail under any of the        */
/*                   following conditions:                           */
/*                         ListToGetItemFrom is not a valid list     */
/*                         ItemTag does not match the item tag       */
/*                             of the current item in the list       */
/*                         Handle is invalid, or is for an item      */
/*                             which is not in ListToGetItemFrom     */
/*                   If any of these conditions occur, an error code */
/*                   will be returned.                               */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  The user is responsible for the memory associated with  */
/*           the address returned by this function since this        */
/*           function removes that object from the list.  This means */
/*           that, when the user is through with the object, they    */
/*           should free it.                                         */
/*                                                                   */
/*           It is assumed that Handle is valid, or is at least the  */
/*           address of an accessible block of storage.  If Handle   */
/*           is invalid, or is not the address of an accessible block*/
/*           of storage, then a trap or exception may occur.         */
/*           NOTE: For this function, NULL is considered a valid     */
/*                 handle which refers to the current item in the    */
/*                 list.                                             */
/*                                                                   */
/*           It is assumed that Object is a valid address.  If not,  */
/*           an exception or trap may occur.                         */
/*                                                                   */
/*           This function does not alter which item is the current  */
/*           item in the list, unless the handle specified belongs   */
/*           to the current item in the list, in which case the      */
/*           item following the current item becomes the current     */
/*           item in the list.  If there is no item following the    */
/*           current item in the list, then the item proceeding the  */
/*           current item will become the current item in the list.  */
/*                                                                   */
/*********************************************************************/
int
ExtractObject(dlist_t ListToGetItemFrom,
	      TAG ItemTag, ADDRESS Handle, ADDRESS * Object)
{
    /* Since ListToGetItemFrom is of type DLIST, we can not use it without
       having to type cast it each time.  To avoid all of the type casting,
       we will declare a local variable of type ControlNode * and then
       initialize it once using ListToGetItemFrom.  This way we just do the
       cast once.                                                            */

    ControlNode *ListData;


    LinkNode *CurrentLinkNode;	/* Used to point to the LinkNode of the current item. */
    void *DataLocation;		/* Used to store the address that will be returned
				   as the function value.                             */
    LinkNode *NextLinkNode;	/* Used to point to the item immediately following
				   the one being extracted so that its fields can
				   be updated.                                      */
    LinkNode *PreviousLinkNode;	/* Used to point to the item immediately before
				   the one being deleted so that its fields can
				   be updated.                                  */

    /* Initialize the return address in case we bail out with an error. */

    *Object = NULL;

    /* We will assume that ListToGetItemFrom points to a valid list.  Given this,
       we will initialize ListData to point to the ControlNode of this
       list.                                                                     */

    ListData = (ControlNode *) ListToGetItemFrom;


#ifdef EVMS_DEBUG

    /* We must now validate the list before we attempt to use it.  We will
       do this by checking the Verify field in the ControlNode.               */
    if ((ListData == NULL) || (ListData->Verify != VerifyValue)) {
	return DLIST_NOT_INITIALIZED;
    }
#endif

    /* Check to see if the dlist_t is empty. */
    if (ListData->ItemCount == 0) {
	return DLIST_EMPTY;
    }

    /* Were we given a handle? */
    if (Handle != NULL) {

	/* Lets check the handle we were given. */
	CurrentLinkNode = (LinkNode *) Handle;

	/* Is the handle valid? */
	if (CurrentLinkNode->ControlNodeLocation != ListData) {

	    /* The handle is not valid!  Abort! */
	    return DLIST_BAD_HANDLE;

	}

    } else {

	/* Since we were not given a handle, we will use the current item in the list for this operation. */
	CurrentLinkNode = ListData->CurrentItem;

    }

    /* We must check the actual item tag against the item tag expected by the user.  A mismatch could lead to errors! */
    if (CurrentLinkNode->DataTag != ItemTag) {
	return DLIST_ITEM_TAG_WRONG;
    }

    /* Since everything checks out, lets store the address of the data so that we can return it later. */
    DataLocation = CurrentLinkNode->DataLocation;

    /* Now we must remove the current item from the DLIST. */

    /* Find the next and previous link nodes in the source list. */
    NextLinkNode = CurrentLinkNode->NextLinkNode;
    PreviousLinkNode = CurrentLinkNode->PreviousLinkNode;

    /* Take the current node out of the source list. */
    if (PreviousLinkNode != NULL) {
	/* The current item was not the first item in the list. */

	/* Remove the current item from the list. */
	PreviousLinkNode->NextLinkNode = NextLinkNode;

    }

    if (NextLinkNode != NULL) {

	/* The current item was not the last item in the list. */
	NextLinkNode->PreviousLinkNode = PreviousLinkNode;

    }

    /* Was the current link node the first item in the list? */
    if (ListData->StartOfList == CurrentLinkNode)
	ListData->StartOfList = NextLinkNode;

    /* Was the current link node the last item in the list? */
    if (ListData->EndOfList == CurrentLinkNode)
	ListData->EndOfList = PreviousLinkNode;

    /* Was the node being extracted the current item in the list? */
    if (ListData->CurrentItem == CurrentLinkNode) {

	/* The current item in the list will be the item which follows the
	   item we are extracting.  If the item being extracted is the last
	   item in the list, then the current item becomes the item immediately
	   before the item being extracted.  If there are no items before or
	   after the item being extracted, then the list is empty!                */
	if (NextLinkNode != NULL)
	    ListData->CurrentItem = NextLinkNode;
	else if (PreviousLinkNode != NULL)
	    ListData->CurrentItem = PreviousLinkNode;
	else
	    ListData->CurrentItem = NULL;

    }

    /* Adjust the count of items in the list. */
    ListData->ItemCount = ListData->ItemCount - 1;

    /* Now we must free the memory associated with the current node. */
    CurrentLinkNode->ControlNodeLocation = NULL;
    free(CurrentLinkNode);

    /* Time to return the address of the data. */
    *Object = DataLocation;

    /* Signal success. */
    return DLIST_SUCCESS;

}

/*********************************************************************/
/*                                                                   */
/*   Function Name:  BlindExtractObject                              */
/*                                                                   */
/*   Descriptive Name:  This function returns the address of the data*/
/*                      associated with the specified item in the    */
/*                      list and then removes that item from the list*/
/*                                                                   */
/*   Input:  dlist_t ListToGetItemFrom : The list whose current      */
/*                                       item is to be copied and    */
/*                                       returned to the caller.     */
/*           TAG *   ItemTag : The tag of the current item           */
/*           ADDRESS Handle : The handle of the item to get.  This   */
/*                            handle must be of an item which resides*/
/*                            in ListToGetItemFrom, or NULL.  If     */
/*                            NULL, then the current item in the     */
/*                            list will be used.                     */
/*           ADDRESS   * Object : The address of a variable to hold  */
/*                                the ADDRESS of data associated     */
/*                                with the current item.             */
/*                                                                   */
/*   Output:  If Successful :                                        */
/*                 Return DLIST_SUCCESS.                             */
/*                 *Object will be the address of the data           */
/*                 associated with the current item in the list.     */
/*            If Failure :                                           */
/*                 Return an error code.                             */
/*                 *Object will be NULL.                             */
/*                                                                   */
/*   Error Handling: This function will fail under any of the        */
/*                   following conditions:                           */
/*                         ListToGetItemFrom is not a valid list     */
/*                         ItemTag does not match the item tag       */
/*                             of the current item in the list       */
/*                         Handle is invalid, or is for an item      */
/*                             which is not in ListToGetItemFrom     */
/*                   If any of these conditions occur, an error code */
/*                   will be returned.                               */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  The user is responsible for the memory associated with  */
/*           the address returned by this function since this        */
/*           function removes that object from the list.  This means */
/*           that, when the user is through with the object, they    */
/*           should free it.                                         */
/*                                                                   */
/*           It is assumed that Handle is valid, or is at least the  */
/*           address of an accessible block of storage.  If Handle   */
/*           is invalid, or is not the address of an accessible block*/
/*           of storage, then a trap or exception may occur.         */
/*           NOTE: For this function, NULL is considered a valid     */
/*                 handle which refers to the current item in the    */
/*                 list.                                             */
/*                                                                   */
/*           It is assumed that Object is a valid address.  If not,  */
/*           an exception or trap may occur.                         */
/*                                                                   */
/*           This function does not alter which item is the current  */
/*           item in the list, unless the handle specified belongs   */
/*           to the current item in the list, in which case the      */
/*           item following the current item becomes the current     */
/*           item in the list.  If there is no item following the    */
/*           current item in the list, then the item preceding the   */
/*           current item will become the current item in the list.  */
/*                                                                   */
/*********************************************************************/
int
BlindExtractObject(dlist_t ListToGetItemFrom,
		   TAG * ItemTag, ADDRESS Handle, ADDRESS * Object)
{

    /* Since ListToGetItemFrom is of type DLIST, we can not use it without
       having to type cast it each time.  To avoid all of the type casting,
       we will declare a local variable of type ControlNode * and then
       initialize it once using ListToGetItemFrom.  This way we just do the
       cast once.                                                            */

    ControlNode *ListData;


    LinkNode *CurrentLinkNode;	/* Used to point to the LinkNode of the current item. */
    void *DataLocation;		/* Used to store the address that will be returned
				   as the function value.                             */
    LinkNode *NextLinkNode;	/* Used to point to the item immediately following
				   the one being extracted so that its fields can
				   be updated.                                      */
    LinkNode *PreviousLinkNode;	/* Used to point to the item immediately before
				   the one being deleted so that its fields can
				   be updated.                                  */

    /* Initialize the return address in case we bail out with an error. */

    *Object = NULL;

    /* We will assume that ListToGetItemFrom points to a valid list.  Given this,
       we will initialize ListData to point to the ControlNode of this
       list.                                                                     */

    ListData = (ControlNode *) ListToGetItemFrom;


#ifdef EVMS_DEBUG

    /* We must now validate the list before we attempt to use it.  We will
       do this by checking the Verify field in the ControlNode.               */
    if ((ListData == NULL) || (ListData->Verify != VerifyValue)) {
	return DLIST_NOT_INITIALIZED;
    }
#endif

    /* Check to see if the dlist_t is empty. */
    if (ListData->ItemCount == 0) {
	return DLIST_EMPTY;
    }

    /* Were we given a handle? */
    if (Handle != NULL) {

	/* Lets check the handle we were given. */
	CurrentLinkNode = (LinkNode *) Handle;

	/* Is the handle valid? */
	if (CurrentLinkNode->ControlNodeLocation != ListData) {

	    /* The handle is not valid!  Abort! */
	    return DLIST_BAD_HANDLE;

	}

    } else {

	/* Since we were not given a handle, we will use the current item in the list for this operation. */
	CurrentLinkNode = ListData->CurrentItem;

    }

    /* return the Tag to the user. */
    *ItemTag = CurrentLinkNode->DataTag;

    /* Since everything checks out, lets store the address of the data so that we can return it later. */
    DataLocation = CurrentLinkNode->DataLocation;

    /* Now we must remove the current item from the DLIST. */

    /* Find the next and previous link nodes in the source list. */
    NextLinkNode = CurrentLinkNode->NextLinkNode;
    PreviousLinkNode = CurrentLinkNode->PreviousLinkNode;

    /* Take the current node out of the source list. */
    if (PreviousLinkNode != NULL) {
	/* The current item was not the first item in the list. */

	/* Remove the current item from the list. */
	PreviousLinkNode->NextLinkNode = NextLinkNode;

    }

    if (NextLinkNode != NULL) {

	/* The current item was not the last item in the list. */
	NextLinkNode->PreviousLinkNode = PreviousLinkNode;

    }

    /* Was the current link node the first item in the list? */
    if (ListData->StartOfList == CurrentLinkNode)
	ListData->StartOfList = NextLinkNode;

    /* Was the current link node the last item in the list? */
    if (ListData->EndOfList == CurrentLinkNode)
	ListData->EndOfList = PreviousLinkNode;

    /* Was the node being extracted the current item in the list? */
    if (ListData->CurrentItem == CurrentLinkNode) {

	/* The current item in the list will be the item which follows the
	   item we are extracting.  If the item being extracted is the last
	   item in the list, then the current item becomes the item immediately
	   before the item being extracted.  If there are no items before or
	   after the item being extracted, then the list is empty!                */
	if (NextLinkNode != NULL)
	    ListData->CurrentItem = NextLinkNode;
	else if (PreviousLinkNode != NULL)
	    ListData->CurrentItem = PreviousLinkNode;
	else
	    ListData->CurrentItem = NULL;

    }

    /* Adjust the count of items in the list. */
    ListData->ItemCount = ListData->ItemCount - 1;

    /* Now we must free the memory associated with the current node. */
    CurrentLinkNode->ControlNodeLocation = NULL;
    free(CurrentLinkNode);

    /* Time to return the address of the data. */
    *Object = DataLocation;

    /* Signal success. */
    return DLIST_SUCCESS;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: ReplaceObject                                    */
/*                                                                   */
/*   Descriptive Name:  This function replaces the specified object  */
/*                      in the list with the one provided as its     */
/*                      argument.                                    */
/*                                                                   */
/*   Input: dlist_t ListToReplaceItemIn : The list whose current     */
/*                                        object is to be replaced   */
/*          ADDRESS ItemLocation : The address of the replacement    */
/*                                 item                              */
/*          TAG     ItemTag : The item tag that the user wishes to   */
/*                            associate with the replacement item    */
/*          ADDRESS Handle : The handle of the item to get.  This    */
/*                           handle must be of an item which resides */
/*                           in ListToGetItemFrom, or NULL.  If NULL */
/*                           then the current item in the list will  */
/*                           be used.                                */
/*          boolean MakeCurrent : If TRUE, the item to get will      */
/*                                become the current item in the     */
/*                                list.                              */
/*           ADDRESS   * Object : The address of a variable to hold  */
/*                                the ADDRESS of the object that     */
/*                                was replaced.                      */
/*                                                                   */
/*   Output:  If Successful then return DLIST_SUCCESS and the        */
/*              *Object will contain the address of the object that  */
/*              was replaced.                                        */
/*            If Unsuccessful, then return an error code and         */
/*              *Object will be NULL.                                */
/*                                                                   */
/*   Error Handling:  This function will fail under the following    */
/*                    conditions:                                    */
/*                         ListToReplaceItemIn is empty              */
/*                         ItemLocation is NULL                      */
/*                         The memory required can not be allocated. */
/*                         Handle is invalid, or is for an item      */
/*                             which is not in ListToGetItemFrom     */
/*                    If any of these conditions occurs, an error    */
/*                    code will be returned.                         */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  The user is responsible for the memory associated with  */
/*           the object returned by this function as that object is  */
/*           removed from the list.  This means that, when the user  */
/*           is through with the object returned by this function,   */
/*           they should free it.                                    */
/*                                                                   */
/*           It is assumed that if ItemLocation is not NULL, then    */
/*           it is a valid address that can be dereferenced.  If     */
/*           these assumptions are violated, an exception or trap    */
/*           may occur.                                              */
/*                                                                   */
/*           It is assumed that Handle is valid, or is at least the  */
/*           address of an accessible block of storage.  If Handle   */
/*           is invalid, or is not the address of an accessible block*/
/*           of storage, then a trap or exception may occur.         */
/*           NOTE: For this function, NULL is a valid handle for the */
/*                 current item in the list.                         */
/*                                                                   */
/*           It is assumed that Object is a valid address.  If not,  */
/*           an exception or trap may occur.                         */
/*                                                                   */
/*           This function does not alter which item is the current  */
/*           item in the list.                                       */
/*                                                                   */
/*********************************************************************/
int ReplaceObject(dlist_t ListToReplaceItemIn, ADDRESS ItemLocation, TAG * ItemTag,	/* On input - TAG of new object.  On return = TAG of old object. */
		  ADDRESS Handle, boolean MakeCurrent, ADDRESS * Object)
{

    /* Since ListToReplaceItemIn is of type DLIST, we can not use it without
       having to type cast it each time.  To avoid all of the type casting,
       we will declare a local variable of type ControlNode * and then
       initialize it once using ListToReplaceItemIn.  This way we just do the
       cast once.                                                            */

    ControlNode *ListData;

    LinkNode *CurrentLinkNode;	/* Used to point to the LinkNode of the
				   item while we replace its data.
				   This limits the levels of indirection
				   to one, which should result in faster
				   execution. */
    TAG OldItemTag;
    ADDRESS OldItemLocation;

    /* Initialize the return address in case we bail out with an error. */

    *Object = NULL;

    /* We will assume that ListToReplaceItemIn points to a valid list.  Given this,
       we will initialize ListData to point to the ControlNode of this
       list.                                                                     */
    ListData = (ControlNode *) ListToReplaceItemIn;


#ifdef EVMS_DEBUG

    /* We must now validate the list before we attempt to use it.  We will
       do this by checking the Verify field in the ControlNode.               */
    if ((ListData == NULL) || (ListData->Verify != VerifyValue)) {
	return DLIST_NOT_INITIALIZED;
    }

    /* Lets check the replacement data. */
    if (ItemLocation == NULL) {
	return DLIST_BAD_ITEM_POINTER;
    }
#endif

    /* Check to see if the dlist_t is empty. */
    if (ListData->ItemCount == 0) {
	return DLIST_EMPTY;
    }

    /* Were we given a handle? */
    if (Handle != NULL) {

	/* Lets check the handle we were given. */
	CurrentLinkNode = (LinkNode *) Handle;

	/* Is the handle valid? */
	if (CurrentLinkNode->ControlNodeLocation != ListData) {

	    /* The handle is not valid!  Abort! */
	    return DLIST_BAD_HANDLE;

	}

    } else {

	/* Since we were not given a handle, we will use the current item in the list for this operation. */
	CurrentLinkNode = ListData->CurrentItem;

    }

    /* Save the old values of DataTag, and DataLocation for return to
       the caller.                                                              */
    OldItemTag = CurrentLinkNode->DataTag;
    OldItemLocation = CurrentLinkNode->DataLocation;

    /* Now lets put our replacement into the list. */
    CurrentLinkNode->DataTag = *ItemTag;
    CurrentLinkNode->DataLocation = ItemLocation;

    /* Setup return values for user. */
    *ItemTag = OldItemTag;

    /* Did the user want this item to become the current item in the list? */
    if (MakeCurrent) {

	/* Make this item the current item in the list. */
	ListData->CurrentItem = CurrentLinkNode;

    }

    /* Signal success. */
    *Object = OldItemLocation;

    return DLIST_SUCCESS;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:  GetHandle                                       */
/*                                                                   */
/*   Descriptive Name:  This function returns a handle for the       */
/*                      current item in the list.  This handle is    */
/*                      then associated with that item regardless of */
/*                      its position in the list.  This handle can be*/
/*                      used to make its associated item the current */
/*                      item in the list.                            */
/*                                                                   */
/*   Input:  dlist_t ListToGetHandleFrom : The list from which a     */
/*                                         handle is needed.         */
/*           ADDRESS * Handle   : The address of a variable to hold  */
/*                                the handle                         */
/*                                                                   */
/*   Output:  If successful, the function returns DLIST_SUCCESS and  */
/*               *Handle is set to the handle for the current item   */
/*               in ListToGetHandleFrom.                             */
/*            If unsuccessful, an error code is returned and *Handle */
/*               is set to 0.                                        */
/*                                                                   */
/*   Error Handling: This function will fail if ListToGetHandleFrom  */
/*                   is not a valid list or is an empty list.  In    */
/*                   either of these cases, an error code is         */
/*                   returned.                                       */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  The handle returned is a pointer to the LinkNode of the */
/*           current item in the list.  This allows the item to move */
/*           around in the list without losing its associated handle.*/
/*           However, if the item is deleted from the list, then the */
/*           handle is invalid and its use could result in a trap.   */
/*                                                                   */
/*********************************************************************/
int GetHandle(dlist_t ListToGetHandleFrom, ADDRESS * Handle)
{

    /* Since ListToGetHandleFrom is of type DLIST, we can not use it without
       having to type cast it each time.  To avoid all of the type casting,
       we will declare a local variable of type ControlNode * and then
       initialize it once using ListToGetHandleFrom.  This way we just do the
       cast once.                                                            */
    ControlNode *ListData;

    /* Initialize the return variable in case we bail out with an error. */

    *Handle = NULL;

    /* We will assume that ListToGetHandleFrom points to a valid list.  Given this,
       we will initialize ListData to point to the ControlNode of this
       list.                                                                     */
    ListData = (ControlNode *) ListToGetHandleFrom;


#ifdef EVMS_DEBUG

    /* We must now validate the list before we attempt to use it.  We will
       do this by checking the Verify field in the ControlNode.               */
    if ((ListData == NULL) || (ListData->Verify != VerifyValue)) {
	return DLIST_NOT_INITIALIZED;
    }
#endif

    /* Check to see if the dlist_t is empty. */
    if (ListData->ItemCount == 0) {
	return DLIST_EMPTY;
    }

    /* Return the address of the CurrentItem. This will serve as the handle. */
    *Handle = ListData->CurrentItem;

    /* Indicate success */
    return DLIST_SUCCESS;

}



/*********************************************************************/
/*                                                                   */
/*   Function Name:  GetListSize                                     */
/*                                                                   */
/*   Descriptive Name:  This function returns the number of items in */
/*                      a list.                                      */
/*                                                                   */
/*   Input:  dlist_t ListToGetSizeOf : The list whose size we wish to*/
/*                                     know                          */
/*           uint       * Size  : The address of a variable to hold  */
/*                                the size of the list.              */
/*                                                                   */
/*   Output:  If successful, the function returns DLIST_SUCCESS and  */
/*               *Size contains the a count of the number of items   */
/*               in the list.                                        */
/*            If unsuccessful, an error code is returned and *Size   */
/*               is set to 0.                                        */
/*                                                                   */
/*   Error Handling: This function will fail if ListToGetSizeOf is   */
/*                   not a valid list.  If this happens, then an     */
/*                   error code is returned.        .                */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  It is assumed that Size contains a valid address. If    */
/*           this assumption is violated, an exception or trap       */
/*           may occur.                                              */
/*                                                                   */
/*********************************************************************/
int GetListSize(dlist_t ListToGetSizeOf, uint * Size)
{

    /* Since ListToGetSizeOf is of type DLIST, we can not use it without
       having to type cast it each time.  To avoid all of the type casting,
       we will declare a local variable of type ControlNode * and then
       initialize it once using ListToGetSizeOf.  This way we just do the
       cast once.                                                            */
    ControlNode *ListData;

    /* Initialize the return variable in case we bail out with an error. */

    *Size = 0;

    /* We will assume that ListToGetSizeOf points to a valid list.  Given this,
       we will initialize ListData to point to the ControlNode of this
       list.                                                                     */
    ListData = (ControlNode *) ListToGetSizeOf;


#ifdef EVMS_DEBUG

    /* We must now validate the list before we attempt to use it.  We will
       do this by checking the Verify field in the ControlNode.               */
    if ((ListData == NULL) || (ListData->Verify != VerifyValue)) {
	return DLIST_NOT_INITIALIZED;
    }
#endif

    /* Return the size of the list. */
    *Size = ListData->ItemCount;

    /* Indicate success. */
    return DLIST_SUCCESS;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:  ListEmpty                                       */
/*                                                                   */
/*   Descriptive Name:  This function returns TRUE if the            */
/*                      specified list is empty, otherwise it returns*/
/*                      FALSE.                                       */
/*                                                                   */
/*   Input:  dlist_t  ListToCheck : The list to check to see if it   */
/*                                  is empty                         */
/*                                                                   */
/*   Output:  If successful, the function returns TRUE if the        */
/*               number of items in the list is 0, otherwise it      */
/*               returns FALSE.                                      */
/*            If unsuccessful, the function returns TRUE.            */
/*                                                                   */
/*   Error Handling: This function will return TRUE if ListToCheck   */
/*                   is not a valid list.                            */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*********************************************************************/
boolean ListEmpty(dlist_t ListToCheck)
{
    /* Since ListToCheck is of type DLIST, we can not use it without
       having to type cast it each time.  To avoid all of the type casting,
       we will declare a local variable of type ControlNode * and then
       initialize it once using ListToCheck.  This way we just do the
       cast once.                                                            */
    ControlNode *ListData;


    /* We will assume that ListToCheck points to a valid list.  Given this,
       we will initialize ListData to point to the ControlNode of this
       list.                                                                     */
    ListData = (ControlNode *) ListToCheck;


#ifdef EVMS_DEBUG

    /* We must now validate the list before we attempt to use it.  We will
       do this by checking the Verify field in the ControlNode.               */
    if ((ListData == NULL) || (ListData->Verify != VerifyValue)) {
	/* DLIST_NOT_INITIALIZED */
	return TRUE;
    }
#endif

    /* Check to see if the dlist_t is empty. */
    if (ListData->ItemCount == 0)
	return (TRUE);
    else
	return (FALSE);
}


/*********************************************************************/
/*                                                                   */
/*   Function Name:  AtEndOfList                                     */
/*                                                                   */
/*   Descriptive Name:  This function returns TRUE if the            */
/*                      current item in the list is the last item    */
/*                      in the list.  Returns FALSE otherwise.       */
/*                                                                   */
/*   Input:  dlist_t     ListToCheck : The list to check.            */
/*                                                                   */
/*   Output:  If successful, the function returns TRUE if the        */
/*               current item in the list is the last item in the    */
/*               list.  If it is not the last item in the list,      */
/*               FALSE is returned.                                  */
/*            If unsuccessful, the function returns FALSE.           */
/*                                                                   */
/*   Error Handling: This function will return FALSE ListToCheck is  */
/*                   not a valid list.                               */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*********************************************************************/
boolean AtEndOfList(dlist_t ListToCheck)
{

    /* Since ListToCheck is of type DLIST, we can not use it without
       having to type cast it each time.  To avoid all of the type casting,
       we will declare a local variable of type ControlNode * and then
       initialize it once using ListToCheck.  This way we just do the
       cast once.                                                            */
    ControlNode *ListData;


    /* We will assume that ListToCheck points to a valid list.  Given this,
       we will initialize ListData to point to the ControlNode of this
       list.                                                                     */
    ListData = (ControlNode *) ListToCheck;


#ifdef EVMS_DEBUG

    /* We must now validate the list before we attempt to use it.  We will
       do this by checking the Verify field in the ControlNode.               */
    if ((ListData == NULL) || (ListData->Verify != VerifyValue)) {
	/* DLIST_NOT_INITIALIZED */
	return FALSE;
    }
#endif

    /* Check to see if the current item in the list is also the last item in the list. */
    if (ListData->CurrentItem == ListData->EndOfList)
	return (TRUE);
    else
	return (FALSE);

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:  DestroyList                                     */
/*                                                                   */
/*   Descriptive Name:  This function releases the memory associated */
/*                      with the internal data structures of a DLIST.*/
/*                      Once a dlist_t has been destroyed by this    */
/*                      function, it must be reinitialized before it */
/*                      can be used again.                           */
/*                                                                   */
/*   Input:  dlist_t     ListToDestroy : The list to be eliminated   */
/*                                       from memory.                */
/*           boolean FreeItemMemory : If TRUE, all items in the list */
/*                                    will be freed.  If FALSE, all  */
/*                                    items in the list are not      */
/*                                    freed, only the list structures*/
/*                                    associated with them are.      */
/*                                                                   */
/*   Output:  If successful, return DLIST_SUCCESS                    */
/*            If unsuccessful, return an error code.                 */
/*                                                                   */
/*   Error Handling: This function will fail if ListToDestroy is not */
/*                   a valid list.  If this happens, then an error   */
/*                   code is returned.                               */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  If FreeItemMemory is TRUE, then this function will try  */
/*           to delete any items which may be in the list.  However, */
/*           since this function has no way of knowing the internal  */
/*           structure of an item, items which contain embedded      */
/*           pointers will not be entirely freed.  This can lead to  */
/*           memory leaks.  The programmer should ensure that any    */
/*           list passed to this function when the FreeItemMemory    */
/*           parameter is TRUE is empty or does not contain any      */
/*           items with embedded pointers.                           */
/*                                                                   */
/*********************************************************************/
int DestroyList(dlist_t * ListToDestroy, boolean FreeItemMemory)
{

    /* Since ListToDestroy is of type DLIST, we can not use it without
       having to type cast it each time.  To avoid all of the type casting,
       we will declare a local variable of type ControlNode * and then
       initialize it once using ListToDestroy.  This way we just do the
       cast once.                                                            */
    ControlNode *ListData;


    LinkNode *CurrentLinkNode;	/* This is used to walk through the
				   linked list of LinkNodes.        */


    /* We will assume that ListToDestroy points to a valid list.  Given this,
       we will initialize ListData to point to the ControlNode of this
       list.                                                                 */
    ListData = (ControlNode *) (*ListToDestroy);


#ifdef EVMS_DEBUG

    /* We must now validate the list before we attempt to use it.  We will
       do this by checking the Verify field in the ControlNode.               */
    if ((ListData == NULL) || (ListData->Verify != VerifyValue)) {
	return DLIST_NOT_INITIALIZED;
    }
#endif

  /*--------------------------------------------------
     To dispose of a DLIST, we must traverse the linked
     list of LinkNodes and dispose of each LinkNode,
     as well as the data item associated with each
     LinkNode.  Once all of the LinkNodes (and their
     data items) have been freed, we can then free the
     ControlNode.
  --------------------------------------------------*/

    /* Loop to dispose of the ListNodes. */
    while (ListData->ItemCount > 0) {
	CurrentLinkNode = ListData->StartOfList;	/* Get the first dlist_t node. */
	ListData->StartOfList = CurrentLinkNode->NextLinkNode;	/* Remove that dlist_t node from the DLIST. */
	ListData->ItemCount--;	/* Decrement the number of items in the list or we will never leave the loop! */
	if ((CurrentLinkNode->DataLocation != NULL) && FreeItemMemory) {
	    free(CurrentLinkNode->DataLocation);
	}
	CurrentLinkNode->ControlNodeLocation = NULL;
	free(CurrentLinkNode);
    }

#ifdef EVMS_DEBUG

    /* Set Verify to 0 so that, if the same block of
       memory is reused for another list, the InitializeList
       function does not get fooled into thinking that
       the block of memory already contains a valid list.    */
    ListData->Verify = 0;

#endif



    /* Now free the memory used to store the master dlist_t node. */
    free(*ListToDestroy);
    *ListToDestroy = NULL;

    /* Signal success. */
    return DLIST_SUCCESS;


}


/*********************************************************************/
/*                                                                   */
/*   Function Name:  NextItem                                        */
/*                                                                   */
/*   Descriptive Name:  This function makes the next item in the list*/
/*                      the current item in the list (i.e. it        */
/*                      advances the current item pointer).          */
/*                                                                   */
/*   Input:  dlist_t    ListToAdvance : The list whose current item  */
/*                                      pointer is to be advanced    */
/*                                                                   */
/*   Output:  If successful, return DLIST_SUCCESS.                   */
/*            If unsuccessful, return error code.                    */
/*                                                                   */
/*   Error Handling: This function will fail under the following     */
/*                   conditions:                                     */
/*                        ListToAdvance is not a valid list          */
/*                        ListToAdvance is empty                     */
/*                        The current item is the last item in the   */
/*                           list                                    */
/*                   If any of these conditions occurs, then an      */
/*                   error code is returned.                         */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*********************************************************************/
int NextItem(dlist_t ListToAdvance)
{

    /* Since ListToAdvance is of type DLIST, we can not use it without
       having to type cast it each time.  To avoid all of the type casting,
       we will declare a local variable of type ControlNode * and then
       initialize it once using ListToAdvance.  This way we just do the
       cast once.                                                            */
    ControlNode *ListData;


    LinkNode *CurrentLinkNode;	/* Used to point to the LinkNode of the
				   current item while we access its data.
				   This limits the levels of indirection
				   to one, which should result in faster
				   execution. */



    /* We will assume that ListToAdvance points to a valid list.  Given this,
       we will initialize ListData to point to the ControlNode of this
       list.                                                                     */
    ListData = (ControlNode *) ListToAdvance;


#ifdef EVMS_DEBUG

    /* We must now validate the list before we attempt to use it.  We will
       do this by checking the Verify field in the ControlNode.               */
    if ((ListData == NULL) || (ListData->Verify != VerifyValue)) {
	return DLIST_NOT_INITIALIZED;
    }
#endif

    /* Check for empty list. */
    if (ListData->ItemCount == 0) {
	return DLIST_EMPTY;
    }

    /* Check for end of list. */
    if (ListData->CurrentItem == ListData->EndOfList) {
	return DLIST_END_OF_LIST;
    }

    /* Advance the current item pointer. */
    CurrentLinkNode = ListData->CurrentItem;
    ListData->CurrentItem = CurrentLinkNode->NextLinkNode;

    /* Signal success. */
    return DLIST_SUCCESS;


}


/*********************************************************************/
/*                                                                   */
/*   Function Name:  PreviousItem                                    */
/*                                                                   */
/*   Descriptive Name:  This function makes the previous item in the */
/*                      list the current item in the list.           */
/*                                                                   */
/*   Input:  dlist_t     ListToChange : The list whose current item  */
/*                                      pointer is to be changed     */
/*                                                                   */
/*   Output:  If successful, return DLIST_SUCCESS.                   */
/*            If unsuccessful, return an error code.                 */
/*                                                                   */
/*   Error Handling: This function will fail under the following     */
/*                   conditions:                                     */
/*                        ListToChange is not a valid list           */
/*                        ListToChange is empty                      */
/*                        The current item is the first item in the  */
/*                           list                                    */
/*                   If any of these conditions occurs, then return  */
/*                   an error code.                                  */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*********************************************************************/
int PreviousItem(dlist_t ListToChange)
{

    /* Since ListToAdvance is of type DLIST, we can not use it without
       having to type cast it each time.  To avoid all of the type casting,
       we will declare a local variable of type ControlNode * and then
       initialize it once using ListToAdvance.  This way we just do the
       cast once.                                                            */
    ControlNode *ListData;


    LinkNode *CurrentLinkNode;	/* Used to point to the LinkNode of the
				   current item while we access its data.
				   This limits the levels of indirection
				   to one, which should result in faster
				   execution. */



    /* We will assume that ListToChange points to a valid list.  Given this,
       we will initialize ListData to point to the ControlNode of this
       list.                                                                     */
    ListData = (ControlNode *) ListToChange;


#ifdef EVMS_DEBUG

    /* We must now validate the list before we attempt to use it.  We will
       do this by checking the Verify field in the ControlNode.               */
    if ((ListData == NULL) || (ListData->Verify != VerifyValue)) {
	return DLIST_NOT_INITIALIZED;
    }
#endif

    /* Check for empty list. */
    if (ListData->ItemCount == 0) {
	return DLIST_EMPTY;
    }

    /* Check for beginning of list. */
    if (ListData->CurrentItem == ListData->StartOfList) {
	return DLIST_ALREADY_AT_START;
    }

    /* Position the current item pointer. */
    CurrentLinkNode = ListData->CurrentItem;
    ListData->CurrentItem = CurrentLinkNode->PreviousLinkNode;

    /* Signal success. */
    return DLIST_SUCCESS;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: GoToStartOfList                                  */
/*                                                                   */
/*   Descriptive Name:  This function makes the first item in the    */
/*                      list the current item in the list.           */
/*                                                                   */
/*   Input:  dlist_t    ListToReset : The list whose current item    */
/*                                    is to be set to the first item */
/*                                    in the list                    */
/*                                                                   */
/*   Output:  If successful, return DLIST_SUCCESS.                   */
/*            If unsuccessful, return an error code                  */
/*                                                                   */
/*   Error Handling: This function will fail if ListToAdvance is not */
/*                   a valid list.  If this occurs, then an error    */
/*                   code is returned.                               */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*********************************************************************/
int GoToStartOfList(dlist_t ListToReset)
{

    /* Since ListToReset is of type DLIST, we can not use it without
       having to type cast it each time.  To avoid all of the type casting,
       we will declare a local variable of type ControlNode * and then
       initialize it once using ListToReset.  This way we just do the
       cast once.                                                            */
    ControlNode *ListData;



    /* We will assume that ListToReset points to a valid list.  Given this,
       we will initialize ListData to point to the ControlNode of this
       list.                                                                 */
    ListData = (ControlNode *) ListToReset;


#ifdef EVMS_DEBUG

    /* We must now validate the list before we attempt to use it.  We will
       do this by checking the Verify field in the ControlNode.               */

    if ((ListData == NULL) || (ListData->Verify != VerifyValue)) {
	return DLIST_NOT_INITIALIZED;
    }
#endif

    /* Set the current item pointer. */
    ListData->CurrentItem = ListData->StartOfList;


    /* Signal success. */
    return DLIST_SUCCESS;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: GoToEndOfList                                    */
/*                                                                   */
/*   Descriptive Name:  This function makes the last item in the     */
/*                      list the current item in the list.           */
/*                                                                   */
/*   Input:  dlist_t     ListToSet : The list whose current item     */
/*                                   is to be set to the last item   */
/*                                   in the list                     */
/*                                                                   */
/*   Output:  If successful, return DLIST_SUCCESS.                   */
/*            If unsuccessful, return an error code                  */
/*                                                                   */
/*   Error Handling: This function will fail if ListToAdvance is not */
/*                   a valid list.  If this occurs, then an error    */
/*                   code is returned.                               */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*********************************************************************/
int GoToEndOfList(dlist_t ListToSet)
{

    /* Since ListToSet is of type DLIST, we can not use it without
       having to type cast it each time.  To avoid all of the type casting,
       we will declare a local variable of type ControlNode * and then
       initialize it once using ListToSet.  This way we just do the
       cast once.                                                            */

    ControlNode *ListData;



    /* We will assume that ListToSet points to a valid list.  Given this,
       we will initialize ListData to point to the ControlNode of this
       list.                                                                 */
    ListData = (ControlNode *) ListToSet;


#ifdef EVMS_DEBUG

    /* We must now validate the list before we attempt to use it.  We will
       do this by checking the Verify field in the ControlNode.               */
    if ((ListData == NULL) || (ListData->Verify != VerifyValue)) {
	return DLIST_NOT_INITIALIZED;
    }
#endif

    /* Set the current item pointer. */
    ListData->CurrentItem = ListData->EndOfList;

    /* Signal success. */
    return DLIST_SUCCESS;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: GoToSpecifiedItem                                */
/*                                                                   */
/*   Descriptive Name:  This function makes the item associated with */
/*                      Handle the current item in the list.         */
/*                                                                   */
/*   Input:  dlist_t ListToReposition:  The list whose current item  */
/*                                      is to be set to the item     */
/*                                      associated with Handle.      */
/*           ADDRESS Handle : A handle obtained by using the         */
/*                            GetHandle function.  This handle       */
/*                            identifies a unique item in the list.  */
/*                                                                   */
/*   Output:  If successful, return DLIST_SUCCESS.                   */
/*            If unsuccessful, return an error code                  */
/*                                                                   */
/*   Error Handling: This function will fail if ListToAdvance is not */
/*                   a valid list.  If this occurs, then an error    */
/*                   code is returned.                               */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  It is assumed that Handle is a valid handle and that    */
/*           the item associated with Handle is still in the list.   */
/*           If these conditions are not met, an exception or trap   */
/*           may occur.                                              */
/*                                                                   */
/*********************************************************************/
int GoToSpecifiedItem(dlist_t ListToReposition, ADDRESS Handle)
{
    /* Since ListToReposition is of type DLIST, we can not use it without
       having to type cast it each time.  To avoid all of the type casting,
       we will declare a local variable of type ControlNode * and then
       initialize it once using ListToReposition.  This way we just do the
       cast once.                                                            */
    ControlNode *ListData;

    LinkNode *CurrentNode = (LinkNode *) Handle;	/* Used to minimize type casting
							   when manipulating and testing
							   the handle.                   */

    /* We will assume that ListToReposition points to a valid list.  Given this,
       we will initialize ListData to point to the ControlNode of this
       list.                                                                 */
    ListData = (ControlNode *) ListToReposition;


#ifdef EVMS_DEBUG

    /* We must now validate the list before we attempt to use it.  We will
       do this by checking the Verify field in the ControlNode.               */
    if ((ListData == NULL) || (ListData->Verify != VerifyValue)) {
	return DLIST_NOT_INITIALIZED;
    }
#endif

    /* Since the list is valid, we must now see if the Handle is valid.  We
       will assume that, if the Handle is not NULL, it points to a LinkNode.
       If the ControlNodeLocation field of the LinkNode points to the
       ControlNode for ListToReposition, then the LinkNode is in ListToReposition
       and can therefore become the current item in ListToReposition. */
    if ((CurrentNode != NULL)
	&& (CurrentNode->ControlNodeLocation == ListData)) {
	/* The handle pointed to a valid LinkNode which is in ListToReposition.
	   Lets make that node the current item in ListToReposition.            */
	ListData->CurrentItem = CurrentNode;
    } else {
	/* The handle was either NULL or the LinkNode it pointed to was not
	   in ListToReposition. */
	return DLIST_BAD_HANDLE;
    }


    /* Signal success. */
    return DLIST_SUCCESS;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:  SortList                                        */
/*                                                                   */
/*   Descriptive Name:  This function sorts the contents of a list.  */
/*                      The sorting algorithm used is a stable sort  */
/*                      whose performance is not dependent upon the  */
/*                      initial order of the items in the list.      */
/*                                                                   */
/*   Input: dlist_t ListToSort : The dlist_t that is to be sorted.   */
/*                                                                   */
/*          int (*Compare) ( ... )                                   */
/*                                                                   */
/*              This is a pointer to a function that can compare any */
/*              two items in the list.  It should return -1 if       */
/*              Object1 is less than Object2, 0 if Object1 is equal  */
/*              to Object2, and 1 if Object1 is greater than Object2.*/
/*              This function will be called during the sort whenever*/
/*              the sorting algorithm needs to compare two objects.  */
/*                                                                   */
/*              The Compare function takes the following parameters: */
/*                                                                   */
/*              ADDRESS Object1 : The address of the data for the    */
/*                                first object to be compared.       */
/*              TAG Object1Tag : The user assigned TAG value for the */
/*                               first object to be compared.        */
/*              ADDRESS Object2 : The address of the data for the    */
/*                                second object to be compared.      */
/*              TAG Object2Tag : The user assigned TAG value for the */
/*                               second object to be compared.       */
/*              uint       * Error : The address of a variable to    */
/*                                   hold the error return value.    */
/*                                                                   */
/*              If this function ever sets *Error to a non-zero value*/
/*              the sort will terminate and the error code will be   */
/*              returned to the caller of the SortList function.     */
/*                                                                   */
/*                                                                   */
/*   Output:  If successful, this function will return DLIST_SUCCESS */
/*               and ListToSort will have been sorted.               */
/*            If unsuccessful, an error code will be returned.       */
/*               The order of the items in ListToSort is undefined   */
/*               and may have changed.                               */
/*                                                                   */
/*   Error Handling: This function will terminate if *Compare sets   */
/*                   *Error to a non-zero value, or if ListToSort    */
/*                   is invalid.  If this function does terminate in */
/*                   the middle of a sort, the order of the items in */
/*                   ListToSort may be different than it was before  */
/*                   the function was called.                        */
/*                                                                   */
/*   Side Effects: None.                                             */
/*                                                                   */
/*   Notes:  This function works by breaking the list into sublists  */
/*           and merging the sublists back into one list.  The size  */
/*           of the sublists starts at 1, and with each pass, the    */
/*           of the sublists is doubled.  The sort ends when the size*/
/*           of a sublist is greater than the size of the original   */
/*           list.                                                   */
/*                                                                   */
/*********************************************************************/
int
SortList(dlist_t ListToSort,
	 int (*Compare) (ADDRESS Object1,
			 TAG Object1Tag,
			 ADDRESS Object2, TAG Object2Tag, uint * Error))
{
    ControlNode *ListData;

    LinkNode *NodeToMove;

    LinkNode *MergeList1;
    uint MergeList1Size;

    LinkNode *MergeList2;
    uint MergeList2Size;

    uint MergeListMaxSize;
    uint ListSize;

    int CompareResult;

    uint Error;

    /* We will assume that ListToSort points to a valid list.  Given this,
       we will initialize ListData to point to the ControlNode of this
       list.                                                                 */
    ListData = (ControlNode *) ListToSort;


#ifdef EVMS_DEBUG

    /* We must now validate the list before we attempt to use it.  We will
       do this by checking the Verify field in the ControlNode.               */
    if ((ListData == NULL) || (ListData->Verify != VerifyValue)) {
	return DLIST_NOT_INITIALIZED;
    }
#endif

    /* We will assume success until proven otherwise. */
    Error = DLIST_SUCCESS;

    /* Is the list big enough to sort? */
    if (ListData->ItemCount > 1) {

	/* The original list will be repeatedly broken into sublists, which are then
	   merged back into one list.  This process is done two sublists at a time.
	   The two sublists are MergeList1 and MergeList2.  Both sublists are the
	   same size.  The only exception occurs when there are not enough items
	   remaining to create a MergeList2 of the same size as the MergeList1.
	   The size of MergeList1 and MergeList2 starts out at 1, and will be doubled
	   with each iteration of the outer "do" loop below.                            */
	MergeListMaxSize = 1;

	/* This is the outer "do" loop which controls the size of the sublists being
	   merged.  The sublists are merged two at a time, with MergeList1 and
	   MergeList2 representing the two sublists being merged.                     */
	do {

	    /* The first sublist will always start with the first element of the
	       list being sorted.                                                  */
	    MergeList1 = ListData->StartOfList;

	    /* This loop controls the merging of sublists back into one list. */
	    do {

		/* The maximum number of items in each of the sublists to be merged
		   is MergeListMaxSize.  As items are merged, they are removed from
		   the sublist they were in and placed in the single list which results
		   from the merging process.                                             */
		MergeList1Size = MergeListMaxSize;
		MergeList2Size = MergeListMaxSize;

		/* Find the start of the second list for merging. */
		ListSize = MergeList1Size;
		MergeList2 = MergeList1;
		while ((MergeList2 != NULL) && (ListSize > 0)) {

		    MergeList2 = MergeList2->NextLinkNode;
		    ListSize--;

		}

		/* Now merge the two lists */
		while ((MergeList1 != NULL) && (MergeList2 != NULL) &&
		       (MergeList1Size > 0) && (MergeList2Size > 0)) {

		    /* Compare the first item in MergeList1 with the first item in MergeList2. */
		    CompareResult =
			(*Compare) (MergeList1->DataLocation,
				    MergeList1->DataTag,
				    MergeList2->DataLocation,
				    MergeList2->DataTag, &Error);

		    /* If there was an error during the comparison, bail out! */
		    if (Error != DLIST_SUCCESS) {

			return Error;

		    }

		    /* See who gets moved. */
		    if (CompareResult > 0) {
			/* Object1 is greater than Object2. */

			/* Object2 must be placed before Object 1. */
			NodeToMove = MergeList2;

			/* Make MergeList2 point to the new start of the second list. */
			MergeList2 = MergeList2->NextLinkNode;

			/* If NodeToMove was the last item in the list, we must update EndOfList since
			   NodeToMove will no longer be the last item in the list!                           */
			if (NodeToMove == ListData->EndOfList) {
			    ListData->EndOfList =
				NodeToMove->PreviousLinkNode;
			}

			/* Remove NodeToMove from the list. */
			if (NodeToMove->PreviousLinkNode != NULL) {
			    NodeToMove->PreviousLinkNode->NextLinkNode =
				MergeList2;

			    if (MergeList2 != NULL) {
				MergeList2->PreviousLinkNode =
				    NodeToMove->PreviousLinkNode;
			    }

			}

			/* NodeToMove must go in front of the current item in the first list.  The
			   current item in the first list is given by MergeList1.                          */
			if (MergeList1->PreviousLinkNode != NULL) {
			    /* Make the item before MergeList1 point to NodeToMove. */
			    MergeList1->PreviousLinkNode->NextLinkNode =
				NodeToMove;
			}

			/* Make NodeToMove->PreviousLinkNode point to the item before MergeList1. */
			NodeToMove->PreviousLinkNode =
			    MergeList1->PreviousLinkNode;

			/* Make NodeToMove->NextLinkNode point to MergeList1. */
			NodeToMove->NextLinkNode = MergeList1;

			/* Complete the process by making MergeList1->PreviousLinkNode point to NodeToMove. */
			MergeList1->PreviousLinkNode = NodeToMove;

			/* If MergeList1 was the first item in the list, we must update StartOfList since
			   MergeList1 is no longer the first item in the list!                             */
			if (MergeList1 == ListData->StartOfList) {
			    ListData->StartOfList = NodeToMove;
			}

			MergeList2Size--;
		    } else {
			/* Object1 is less than or equal to Object2. */

			/* Remove Object1 from the first list.  To do this, we just need to
			   advance the MergeList1 pointer, since it always points to the
			   first item in the first of the lists which are being merged.      */
			MergeList1 = MergeList1->NextLinkNode;
			MergeList1Size--;
		    }

		}

		/* We have left the while loop.  All of the items in one of the merge lists
		   must have been used.  We must now setup MergeList1 to point to the first
		   of the next two lists to be merged.                                      */
		if ((MergeList2Size == 0) || (MergeList2 == NULL)) {

		    /* MergeList2 is empty.  Either MergeList2 now points to the first
		       item in the next list to be merged, or MergeList2 is NULL.  Thus,
		       MergeList2 points to what MergeList1 should point to.  So make
		       MergeList1 equal to MergeList2.  When we reach the top of the
		       "do" loop, MergeList2 will be set to point to the proper location. */
		    MergeList1 = MergeList2;

		} else {

		    /* The first of the next two lists to be merged starts after the end of the
		       list pointed to by MergeList2.  Thus, we must start MergeList1 at
		       MergeList2 and advance it past the remaining items in MergeList2.        */
		    ListSize = MergeList2Size;
		    MergeList1 = MergeList2;
		    while ((MergeList1 != NULL) && (ListSize > 0)) {

			MergeList1 = MergeList1->NextLinkNode;
			ListSize--;

		    }

		}

	    }
	    while (MergeList1 != NULL);

	    MergeListMaxSize = MergeListMaxSize * 2;

	}
	while (ListData->ItemCount > MergeListMaxSize);

    }

    return Error;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:  ForEachItem                                     */
/*                                                                   */
/*   Descriptive Name:  This function passes a pointer to each item  */
/*                      in a list to a user provided function for    */
/*                      processing by the user provided function.    */
/*                                                                   */
/*   Input:  dlist_t ListToProcess : The dlist_t whose items are to  */
/*                                   be processed by the user        */
/*                                   provided function.              */
/*                                                                   */
/*           int (*ProcessItem) (...)                                */
/*                                                                   */
/*               This is a pointer to the user provided function.    */
/*               This user provided function takes the following     */
/*                  parameters:                                      */
/*                                                                   */
/*                  ADDRESS Object : A pointer to an item in         */
/*                                   ListToProcess.                  */
/*                  TAG Object1Tag : The user assigned TAG value for */
/*                                   the item pointed to by Object.  */
/*                  ADDRESS Parameter : The address of a block of    */
/*                                      memory containing any        */
/*                                      parameters that the user     */
/*                                      wishes to have passed to this*/
/*                                      function.                    */
/*                                                                   */
/*           ADDRESS Parameters : This field is passed through to    */
/*                                *ProcessItem.  This function does  */
/*                                not even look at the contents of   */
/*                                this field.  This field is here to */
/*                                provide the user a way to pass     */
/*                                additional data to *ProcessItem    */
/*                                that *ProcessItem may need to      */
/*                                function correctly.                */
/*                                                                   */
/*   Output:  If successful, return DLIST_SUCCESS.                   */
/*            If unsuccessful, return an error code.                 */
/*                                                                   */
/*   Error Handling: This function aborts immediately when an error  */
/*                   is detected, and any remaining items in the list*/
/*                   will not be processed.                          */
/*                                                                   */
/*   Side Effects: None.                                             */
/*                                                                   */
/*   Notes: This function allows the user to access all of the items */
/*          in a list and perform an operation on them.  The         */
/*          operation performed must not free any items in the list, */
/*          or perform any list operations on the list being         */
/*          processed.                                               */
/*                                                                   */
/*          As an example of when this would be useful, consider a   */
/*          a list of graphic objects (rectangles, triangles, circles*/
/*          etc.)  which comprise a drawing.  To draw the picture    */
/*          that these graphic objects represent, one could build a  */
/*          loop which gets and draws each item.  Another way to     */
/*          do this would be to build a drawing function which can   */
/*          draw any of the graphic objects, and then use that       */
/*          function as the ProcessItem function in a call to        */
/*          ForEachItem.                                             */
/*                                                                   */
/*          If the ProcessItem function returns an error code        */
/*          other than DLIST_SUCCESS, then ForEachItem will terminate*/
/*          and return an error to whoever called it.  The single    */
/*          exception to this is if ProcessItem returns              */
/*          DLIST_SEARCH_COMPLETE, in which case ForEachItem         */
/*          terminates and returns DLIST_SUCCESS.  This is           */
/*          useful for using ForEachItem to search a list and then   */
/*          terminating the search once the desired item is found.   */
/*                                                                   */
/*          A word about the Parameters parameter.  This parameter   */
/*          is passed through to *ProcessItem and is never looked at */
/*          by this function.  This means that the user can put any  */
/*          value they desire into Parameters as long as it is the   */
/*          same size (in bytes) as Parameters.  The intended use of */
/*          Parameters is to allow the user to pass information to   */
/*          *ProcessItem that *ProcessItem may need.  Either way,    */
/*          how Parameters is used is literally up to the user.      */
/*                                                                   */
/*********************************************************************/
int
ForEachItem(dlist_t ListToProcess,
	    int (*ProcessItem) (ADDRESS Object,
				TAG ObjectTag,
				ADDRESS ObjectHandle,
				ADDRESS Parameters),
	    ADDRESS Parameters, boolean Forward)
{

    /* Since ListToProcess is of type DLIST, we can not use it without
       having to type cast it each time.  To avoid all of the type casting,
       we will declare a local variable of type ControlNode * and then
       initialize it once using ListToProcess.  This way we just do the
       cast once.                                                            */
    ControlNode *ListData;


    LinkNode *CurrentLinkNode;	/* Used to point to the LinkNode of the
				   current item while we access its data.
				   This limits the levels of indirection
				   to one, which should result in faster
				   execution. */

    int Error;

    /* We will assume that ListToProcess points to a valid list.  Given this,
       we will initialize ListData to point to the ControlNode of this
       list.                                                                     */
    ListData = (ControlNode *) ListToProcess;


#ifdef EVMS_DEBUG

    /* We must now validate the list before we attempt to use it.  We will
       do this by checking the Verify field in the ControlNode.               */
    if ((ListData == NULL) || (ListData->Verify != VerifyValue)) {
	return DLIST_NOT_INITIALIZED;
    }
#endif

    /* Assume success. */
    Error = DLIST_SUCCESS;

    /* Check for empty list. */
    if (ListData->ItemCount == 0) {
	return Error;
    }

    /* Set CurrentLinkNode based upon the direction we are going to traverse the list. */
    if (Forward) {

	/* Get the first link node in the list. */
	CurrentLinkNode = ListData->StartOfList;

    } else {

	/* Get the last link node in the list. */
	CurrentLinkNode = ListData->EndOfList;

    }

    /* Now loop through the items in the list. */
    while (CurrentLinkNode != NULL) {

	/* Call the user provided function to process the current item in the list. */
	Error =
	    ProcessItem(CurrentLinkNode->DataLocation,
			CurrentLinkNode->DataTag, CurrentLinkNode,
			Parameters);
	if (Error != DLIST_SUCCESS) {

	    if (Error == DLIST_SEARCH_COMPLETE)
		Error = DLIST_SUCCESS;

	    return Error;

	}

	/* Advance to the next item in the list based upon the direction that we are traversing the list in. */
	if (Forward) {

	    CurrentLinkNode = CurrentLinkNode->NextLinkNode;

	} else {

	    CurrentLinkNode = CurrentLinkNode->PreviousLinkNode;

	}

    }

    /* All items in the list have been processed. */
    return Error;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:  PruneList                                       */
/*                                                                   */
/*   Descriptive Name:  This function allows the caller to examine   */
/*                      each item in a list and optionally delete    */
/*                      it from the list.                            */
/*                                                                   */
/*   Input:  dlist_t ListToProcess : The dlist_t to be pruned.       */
/*                                                                   */
/*           boolean (*KillItem) (...)                               */
/*                                                                   */
/*               This is a pointer to a user provided function.      */
/*               This user provided function takes the following     */
/*                  parameters:                                      */
/*                                                                   */
/*                  ADDRESS Object : A pointer to an item in         */
/*                                   ListToProcess.                  */
/*                  TAG Object1Tag : The user assigned TAG value for */
/*                                   the item pointed to by Object.  */
/*                  ADDRESS Parameter : The address of a block of    */
/*                                      memory containing any        */
/*                                      parameters that the user     */
/*                                      wishes to have passed to this*/
/*                                      function.                    */
/*                  boolean * FreeMemory : The address of a boolean  */
/*                                         variable which this       */
/*                                         function will set to      */
/*                                         either TRUE or FALSE.     */
/*                                         If the function return    */
/*                                         value is TRUE, then the   */
/*                                         value in *FreeMemory will */
/*                                         be examined.  If it is    */
/*                                         TRUE, then PruneList will */
/*                                         free the memory associated*/
/*                                         with the item being       */
/*                                         deleted.  If *FreeMemory  */
/*                                         is FALSE, then the item   */
/*                                         being removed from the    */
/*                                         dlist_t will not be freed,*/
/*                                         and it is up to the user  */
/*                                         to ensure that this memory*/
/*                                         is handled properly.      */
/*                  uint       * Error : The address of a variable to*/
/*                                       hold the error return value.*/
/*                                                                   */
/*           ADDRESS Parameters : This field is passed through to    */
/*                                *KillItem.  This function does     */
/*                                not even look at the contents of   */
/*                                this field.  This field is here to */
/*                                provide the user a way to pass     */
/*                                additional data to *ProcessItem    */
/*                                that *ProcessItem may need to      */
/*                                function correctly.                */
/*                                                                   */
/*                                                                   */
/*   Output:  If successful, return DLIST_SUCCESS.                   */
/*            If unsuccessful, return an error code.                 */
/*                                                                   */
/*   Error Handling: This function aborts immediately when an error  */
/*                   is detected, and any remaining items in the list*/
/*                   will not be processed.                          */
/*                                                                   */
/*   Side Effects: None.                                             */
/*                                                                   */
/*   Notes: This function allows the user to access all of the items */
/*          in a list, perform an operation on them, and then        */
/*          optionally delete ("remove") them from the DLIST.  The   */
/*          operation performed must not free any items in the list, */
/*          or perform any list operations on the list being         */
/*          processed.                                               */
/*                                                                   */
/*          If the KillItem function sets *Error to something other  */
/*          than DLIST_SUCCESS, then PruneList will terminate and    */
/*          return an error to whoever called it.  The single        */
/*          exception to this is if KillItem sets *Error to          */
/*          DLIST_SEARCH_COMPLETE, in which case KillItem            */
/*          terminates and sets *Error to DLIST_SUCCESS.  This is    */
/*          useful for using KillItem to search a list and then      */
/*          terminating the search once the desired item is found.   */
/*                                                                   */
/*          A word about the Parameters parameter.  This parameter   */
/*          is passed through to *ProcessItem and is never looked at */
/*          by this function.  This means that the user can put any  */
/*          value they desire into Parameters as long as it is the   */
/*          same size (in bytes) as Parameters.  The intended use of */
/*          Parameters is to allow the user to pass information to   */
/*          *ProcessItem that *ProcessItem may need.  Either way,    */
/*          how Parameters is used is literally up to the user.      */
/*                                                                   */
/*********************************************************************/
int
PruneList(dlist_t ListToProcess,
	  boolean(*KillItem) (ADDRESS Object,
			      TAG ObjectTag,
			      ADDRESS ObjectHandle,
			      ADDRESS Parameters,
			      boolean * FreeMemory,
			      uint * Error), ADDRESS Parameters)
{

    /* Since ListToProcess is of type DLIST, we can not use it without
       having to type cast it each time.  To avoid all of the type casting,
       we will declare a local variable of type ControlNode * and then
       initialize it once using ListToProcess.  This way we just do the
       cast once.                                                            */
    ControlNode *ListData;


    LinkNode *CurrentLinkNode;	/* Used to point to the LinkNode of the
				   current item while we access its data.
				   This limits the levels of indirection
				   to one, which should result in faster
				   execution. */
    LinkNode *PreviousLinkNode;	/* Used to point to the LinkNode immediately
				   prior to the CurrentLinkNode.  This is
				   needed if an item is deleted.             */
    boolean FreeMemory;		/* Used as a parameter to KillItem to let the
				   user indicate whether or not to free the
				   memory associated with an item that is being
				   removed from the list.                    */

    uint Error;


    /* We will assume that ListToProcess points to a valid list.  Given this,
       we will initialize ListData to point to the ControlNode of this
       list.                                                                     */
    ListData = (ControlNode *) ListToProcess;


#ifdef EVMS_DEBUG

    /* We must now validate the list before we attempt to use it.  We will
       do this by checking the Verify field in the ControlNode.               */
    if ((ListData == NULL) || (ListData->Verify != VerifyValue)) {
	return DLIST_NOT_INITIALIZED;
    }
#endif

    /* Assume success. */
    Error = DLIST_SUCCESS;

    /* Check for empty list. */
    if (ListData->ItemCount == 0) {
	return Error;
    }

    /* Get the first link node in the list. */
    CurrentLinkNode = ListData->StartOfList;

    /* Now loop through the items in the list. */
    while (CurrentLinkNode != NULL) {

	/* Call the user provided function to decide whether or not to keep the
	   current item in the list.                                             */
	if (KillItem
	    (CurrentLinkNode->DataLocation, CurrentLinkNode->DataTag,
	     CurrentLinkNode, Parameters, &FreeMemory, &Error)) {

	    if ((Error != DLIST_SUCCESS)
		&& (Error != DLIST_SEARCH_COMPLETE)) {

		return Error;

	    }

	    /* We are to remove the current item from the list. */

	    /* Initialize PreviousLinkNode. */
	    PreviousLinkNode = CurrentLinkNode->PreviousLinkNode;

	    /* Is the item being removed from the list the first item in the list? */
	    if (PreviousLinkNode == NULL) {

		/* We are at the start of the list.  Update the StartOfList field. */
		ListData->StartOfList = CurrentLinkNode->NextLinkNode;

	    } else {

		/* We are somewhere in the middle of the list, or possibly even the
		   last item in the list.                                            */
		PreviousLinkNode->NextLinkNode =
		    CurrentLinkNode->NextLinkNode;

	    }

	    /* Is the item we are removing the last item in the list?  If so, update the
	       pointer to the last item in the list.                                     */
	    if (CurrentLinkNode == ListData->EndOfList) {

		ListData->EndOfList = PreviousLinkNode;

	    } else {

		/* Since CurrentLinkNode is not the last item in the list, we must adjust the
		   PreviousLinkNode field of the item following CurrentLinkNode.                 */
		CurrentLinkNode->NextLinkNode->PreviousLinkNode =
		    PreviousLinkNode;

	    }

	    /* CurrentLinkNode has now been removed from the list. */

	    /* Is the item we are deleting the current item in the list? */
	    if (CurrentLinkNode == ListData->CurrentItem) {

		/* Since the item we are deleting is the current item, we must choose
		   a new current item.  If the item we are deleting is NOT the last
		   item in the list, then we will choose the item following it as the
		   new current item.  If the item we are deleting is the last item
		   in the list, then we will choose the item immediately before it to
		   be the new current item.                                            */
		if (CurrentLinkNode->NextLinkNode != NULL) {

		    /* We are removing an item from the beginning or middle of the list. */
		    ListData->CurrentItem = CurrentLinkNode->NextLinkNode;

		} else {

		    /* We are removing the last item in the list. */
		    ListData->CurrentItem = PreviousLinkNode;

		}

	    }

	    /* Adjust the count of items in the list. */
	    ListData->ItemCount = ListData->ItemCount - 1;

	    /* Now we must free the memory associated with the current node. */
	    if (FreeMemory) {
		/* Free the memory associated with the actual item stored in the list. */
		free(CurrentLinkNode->DataLocation);
	    }

	    /* Free the memory associated with the control structures used to manage items in the list. */
	    CurrentLinkNode->ControlNodeLocation = NULL;
	    free(CurrentLinkNode);

	    /* Resume our traversal of the tree. */

	    /* Are we at the start of the list?  If so, then PreviousLinkNode will be
	       NULL since there is no previous link node.                             */
	    if (PreviousLinkNode != NULL) {

		/* Since we did not delete the first item in the list, we can resume our
		   tree traversal with the item following PreviousLinkNode.               */
		CurrentLinkNode = PreviousLinkNode->NextLinkNode;

	    } else {

		/* The item we deleted was the first item in the list.  We can resume our
		   list traversal with the item which is now the first item in the list.  */
		CurrentLinkNode = ListData->StartOfList;

	    }

	    /* Did the user indicate that we are to stop the list traversal? */
	    if (Error == DLIST_SEARCH_COMPLETE) {

		/* Convert the error code to success and stop the list traversal. */
		Error = DLIST_SUCCESS;
		return Error;

	    }

	} else {

	    if (Error != DLIST_SUCCESS) {

		if (Error == DLIST_SEARCH_COMPLETE)
		    Error = DLIST_SUCCESS;

		return Error;

	    }

	    /* We are keeping the current item in the list. */

	    /* Advance to the next item in the list. */
	    CurrentLinkNode = CurrentLinkNode->NextLinkNode;

	}

    }

    /* All items in the list have been processed. */
    return Error;

}

/*********************************************************************/
/*                                                                   */
/*   Function Name:  AppendList                                      */
/*                                                                   */
/*   Descriptive Name: Removes the items in SourceList and appends   */
/*                     them to TargetList.                           */
/*                                                                   */
/*   Input:  dlist_t TargetList : The dlist_t which is to have the   */
/*                                items from SourceList appended to  */
/*                                it.                                */
/*           dlist_t SourceList : The dlist_t whose items are to be  */
/*                                removed and appended to TargetList.*/
/*                                                                   */
/*   Output: If successful, return DLIST_SUCCESS.                    */
/*              SourceList will be empty, and TargetList will contain*/
/*              all of its original items and all of the items that  */
/*              were in SourceList.                                  */
/*           If unsuccessful, return an error code.  SourceList and  */
/*              TargetList will be unmodified.                       */
/*                                                                   */
/*   Error Handling:  This function will abort immediately upon      */
/*                    detection of an error.  All errors that can be */
/*                    detected are detected before the contents of   */
/*                    SourceList are appended to TargetList, so if an*/
/*                    error is detected and the function aborts,     */
/*                    SourceList and TargetList are unaltered.       */
/*                                                                   */
/*   Side Effects: None.                                             */
/*                                                                   */
/*   Notes: None.                                                    */
/*                                                                   */
/*********************************************************************/
int AppendList(dlist_t TargetList, dlist_t SourceList)
{

    /* Since TargetList is of type DLIST, we can not use it without
       having to type cast it each time.  To avoid all of the type casting,
       we will declare a local variable of type ControlNode * and then
       initialize it once using TargetList.  This way we just do the
       cast once.                                                            */
    ControlNode *TargetListData;

    /* Since SourceList is of type DLIST, we can not use it without
       having to type cast it each time.  To avoid all of the type casting,
       we will declare a local variable of type ControlNode * and then
       initialize it once using SourceList.  This way we just do the
       cast once.                                                            */
    ControlNode *SourceListData;

    ControlNode TempListData;	/* Used to hold the control node for
				   list when the control nodes for
				   TargetList and SourceList are being
				   swapped.                             */

    LinkNode *CurrentLinkNode;	/* Used to point to the LinkNode of the
				   current item  in TargetList while we
				   access its data.  This limits the
				   levels of indirection to one, which
				   should result in faster execution. */
    LinkNode *SourceLinkNode;	/* Used to point to the LinkNode of
				   the first item in the SourceList.  */

    /* We will assume that TargetList and SourceList both point to a valid lists.
       Given this, we will initialize TargetListData and SourceListData to point
       to the ControlNode of TargetList and SourceList, respectively.            */
    TargetListData = (ControlNode *) TargetList;
    SourceListData = (ControlNode *) SourceList;


#ifdef EVMS_DEBUG

    /* We must now validate the lists before we attempt to use them.  We will
       do this by checking the Verify field in the ControlNode.               */
    if ((TargetListData == NULL)
	|| (TargetListData->Verify != VerifyValue)) {
	return DLIST_NOT_INITIALIZED;
    }

    if ((SourceListData == NULL)
	|| (SourceListData->Verify != VerifyValue)) {
	return DLIST_NOT_INITIALIZED;
    }
#endif

    /* Is the source list empty?  If so, we have nothing to do! */
    if (SourceListData->ItemCount == 0) {

	/* The source list is empty!  We are done! */
	return DLIST_SUCCESS;

    }

    /* Is the target list currently empty? */
    if (TargetListData->ItemCount == 0) {

	/* Since the target list is empty but the source list is not, we will just swap the
	   control record from the source list with the control record of the target list.    */
	TempListData = *TargetListData;
	*TargetListData = *SourceListData;
	*SourceListData = TempListData;

	/* Get the first item in the target list. */
	CurrentLinkNode = TargetListData->StartOfList;

	/* Adjust the ControlNodeLocation field of this Link Node. */
	CurrentLinkNode->ControlNodeLocation = TargetListData;

    } else {

	/* Here's where we do the most work.  Both the TargetList and the SourceList contain
	   items.  We must append the items from the SourceList to the TargetList.             */

	/* Get the last item in the target list. */
	CurrentLinkNode = TargetListData->EndOfList;

	/* Get the first item in the source list. */
	SourceLinkNode = SourceListData->StartOfList;

	/* Attach the items from the Source List to the end of those from the Target List. */
	CurrentLinkNode->NextLinkNode = SourceLinkNode;
	SourceLinkNode->PreviousLinkNode = CurrentLinkNode;

	/* Update the Target List Control Information. */
	TargetListData->EndOfList = SourceListData->EndOfList;
	TargetListData->ItemCount =
	    TargetListData->ItemCount + SourceListData->ItemCount;

	/* Update the Source List Control Information. */
	SourceListData->StartOfList = NULL;
	SourceListData->EndOfList = NULL;
	SourceListData->CurrentItem = NULL;
	SourceListData->ItemCount = 0;

    }

    /* Adjust the ControlNodeLocation field of all of the items being moved from the Source List to the Target List. */
    while (CurrentLinkNode->NextLinkNode != NULL) {
	CurrentLinkNode = CurrentLinkNode->NextLinkNode;
	CurrentLinkNode->ControlNodeLocation = TargetListData;
    }


    /* All done! */
    return DLIST_SUCCESS;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:  CopyList                                        */
/*                                                                   */
/*   Descriptive Name: Copies the items in SourceList to the         */
/*                     TargetList.                                   */
/*                                                                   */
/*   Input:  dlist_t TargetList : The dlist_t which is to have the   */
/*                                items from SourceList copied to it.*/
/*           dlist_t SourceList : The dlist_t whose items are to be  */
/*                                copied to TargetList.              */
/*                                                                   */
/*   Output: If successful, return DLIST_SUCCESS.                    */
/*              SourceList will be unchanged and TargetList will     */
/*              contain all of its original items and all of the     */
/*              items that were in SourceList.                       */
/*           If unsuccessful, return an error code.  SourceList and  */
/*              TargetList will be unmodified.                       */
/*                                                                   */
/*   Error Handling:  This function will abort immediately upon      */
/*                    detection of an error.  All errors that can be */
/*                    detected are detected before the contents of   */
/*                    SourceList are appended to TargetList, so if an*/
/*                    error is detected and the function aborts,     */
/*                    SourceList and TargetList are unaltered.       */
/*                                                                   */
/*   Side Effects: None.                                             */
/*                                                                   */
/*   Notes: None.                                                    */
/*                                                                   */
/*********************************************************************/
int
CopyList(dlist_t TargetList, dlist_t SourceList,
	 Insertion_Modes Insert_Mode)
{

    /* Since TargetList is of type DLIST, we can not use it without
       having to type cast it each time.  To avoid all of the type casting,
       we will declare a local variable of type ControlNode * and then
       initialize it once using TargetList.  This way we just do the
       cast once.                                                            */
    ControlNode *TargetListData;

    /* Since SourceList is of type DLIST, we can not use it without
       having to type cast it each time.  To avoid all of the type casting,
       we will declare a local variable of type ControlNode * and then
       initialize it once using SourceList.  This way we just do the
       cast once.                                                            */
    ControlNode *SourceListData;

    ControlNode TempListData;	/* Used to build a copy of the
				   SourceList before inserting it into
				   the TargetList.                   */

    LinkNode *CurrentLinkNode;	/* Used to point to the LinkNode of the
				   current item  in TargetList while we
				   access its data.  This limits the
				   levels of indirection to one, which
				   should result in faster execution. */

    int Error;
    ADDRESS Handle;		/* Needed to catch returned handle
				   from call to InsertObject().       */

    /* We will assume that TargetList and SourceList both point to a valid lists.
       Given this, we will initialize TargetListData and SourceListData to point
       to the ControlNode of TargetList and SourceList, respectively.            */
    TargetListData = (ControlNode *) TargetList;
    SourceListData = (ControlNode *) SourceList;

#ifdef EVMS_DEBUG

    /* We must now validate the lists before we attempt to use them.  We will
       do this by checking the Verify field in the ControlNode.               */
    if ((TargetListData == NULL)
	|| (TargetListData->Verify != VerifyValue)) {
	return DLIST_NOT_INITIALIZED;
    }

    if ((SourceListData == NULL)
	|| (SourceListData->Verify != VerifyValue)) {
	return DLIST_NOT_INITIALIZED;
    }
#endif

    /* Is the source list empty?  If so, we have nothing to do! */
    if (SourceListData->ItemCount == 0) {

	/* The source list is empty!  We are done! */
	return DLIST_SUCCESS;

    }


    /* Validate the Insert_Mode. */
    if (Insert_Mode > AppendToList) {
	return DLIST_INVALID_INSERTION_MODE;
    }

    /* Build a copy of the SourceList in a temporary location so that we can
       check for errors (such as "out of memory") and bail out without having
       modified the TargetList.                                                             */


    TempListData.ItemCount = 0;	/* No items in the list. */
    TempListData.StartOfList = NULL;	/* Since the list is empty, there is no first item */
    TempListData.EndOfList = NULL;	/* Since the list is empty, there is no last item */
    TempListData.CurrentItem = NULL;	/* Since the list is empty, there is no current item */

#ifdef EVMS_DEBUG
    TempListData.Verify = VerifyValue;	/* Initialize the Verify field so that this list will recognized as being valid. */
#endif

    CurrentLinkNode = SourceListData->StartOfList;

    Error = DLIST_SUCCESS;
    while ((Error == DLIST_SUCCESS) && (CurrentLinkNode != NULL)) {
	Error = InsertObject(&TempListData,
			     CurrentLinkNode->DataLocation,
			     CurrentLinkNode->DataTag,
			     NULL, AppendToList, FALSE, &Handle);

	CurrentLinkNode = CurrentLinkNode->NextLinkNode;
    }

    if (Error != DLIST_SUCCESS) {

	/* Something went wrong when trying to make copies of the items in the SourceList.
	   Delete any items we may have created in the temporary list.                    */

	DeleteAllItems(&TempListData, FALSE);
	return Error;
    }

    /* Set the ControlNodeLocation of all the LinkNodes in the new list to be  the TargetListData. */

    CurrentLinkNode = TempListData.StartOfList;

    while (CurrentLinkNode != NULL) {
	CurrentLinkNode->ControlNodeLocation = TargetListData;
	CurrentLinkNode = CurrentLinkNode->NextLinkNode;
    }

    /* Put the copy of the SourceList into the TargetList at the place the caller specified. */

    if (TargetListData->CurrentItem == NULL) {

	// the Target list is empty.  Just slap in the temporary list.
	TargetListData->StartOfList = TempListData.StartOfList;
	TargetListData->EndOfList = TempListData.EndOfList;
	TargetListData->CurrentItem = TempListData.StartOfList;

    } else if ((Insert_Mode == InsertAtStart) ||
	       ((Insert_Mode == InsertBefore) &&
		(TargetListData->CurrentItem ==
		 TargetListData->StartOfList))) {

	TempListData.EndOfList->NextLinkNode = TargetListData->StartOfList;
	TargetListData->StartOfList->PreviousLinkNode =
	    TempListData.EndOfList;
	TargetListData->StartOfList = TempListData.StartOfList;

    } else if (Insert_Mode == InsertBefore) {

	CurrentLinkNode = TargetListData->CurrentItem;

	TempListData.StartOfList->PreviousLinkNode =
	    CurrentLinkNode->PreviousLinkNode;
	TempListData.EndOfList->NextLinkNode = CurrentLinkNode;
	CurrentLinkNode->PreviousLinkNode->NextLinkNode =
	    TempListData.StartOfList;
	CurrentLinkNode->PreviousLinkNode = TempListData.EndOfList;

    } else if ((Insert_Mode == AppendToList) ||
	       ((Insert_Mode == InsertAfter) &&
		(TargetListData->CurrentItem ==
		 TargetListData->EndOfList))) {

	TempListData.StartOfList->PreviousLinkNode =
	    TargetListData->EndOfList;
	TargetListData->EndOfList->NextLinkNode = TempListData.StartOfList;
	TargetListData->EndOfList = TempListData.EndOfList;

    } else {

	/* Insert_Mode must be InsertAfter and the current item is not the last one. */
	CurrentLinkNode = TargetListData->CurrentItem;

	TempListData.StartOfList->PreviousLinkNode = CurrentLinkNode;
	TempListData.EndOfList->NextLinkNode =
	    CurrentLinkNode->NextLinkNode;
	CurrentLinkNode->NextLinkNode->PreviousLinkNode =
	    TempListData.EndOfList;
	CurrentLinkNode->NextLinkNode = TempListData.StartOfList;

    }

    /* Update the count of items in the TargetList. */

    TargetListData->ItemCount += TempListData.ItemCount;

    /* All done! */
    return DLIST_SUCCESS;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:  dlist_strerror                                  */
/*                                                                   */
/*   Descriptive Name: REturns a string that describes the dlist     */
/*                     error code.                                   */
/*                                                                   */
/*   Input:  int err_num:  dlist error code                          */
/*                                                                   */
/*   Output: const char * description string                         */
/*                                                                   */
/*   Error Handling:  If the error number is not a vlaid dlist       */
/*                    error code, dlist_strerror() returns NULL      */
/*                                                                   */
/*   Side Effects: None.                                             */
/*                                                                   */
/*   Notes: None.                                                    */
/*                                                                   */
/*********************************************************************/

static char *err_msg[] = {
// DLIST_CORRUPTED                  201
    "Dlist is corrupted",
// DLIST_BAD                        202
    "Dlist is bad",
// DLIST_NOT_INITIALIZED            203
    "Dlist is not initialized",
// DLIST_EMPTY                      204
    "Dlist is empty",
// DLIST_ITEM_SIZE_WRONG            205
    "Item size is wrong",
// DLIST_BAD_ITEM_POINTER           206
    "Bad item pointer",
// DLIST_ITEM_SIZE_ZERO             207
    "Item size is zero",
// DLIST_ITEM_TAG_WRONG             208
    "Item tag is wrong",
// DLIST_END_OF_LIST                209
    "At end of dlist",
// DLIST_ALREADY_AT_START           210
    "Already at start of dlist",
// DLIST_BAD_HANDLE                 211
    "Bad handle"
// DLIST_INVALID_INSERTION_MODE     212
	"Insertion mode is not valid",
// DLIST_OBJECT_NOT_FOUND           213
    "Object not found",
// DLIST_OBJECT_ALREADY_IN_LIST     214
    "Object already in list"
};

const char *dlist_strerror(int err_num)
{

    if (IS_DLIST_ERROR(err_num)) {
	return err_msg[abs(err_num) - DLIST_ERROR_BASE];

    } else {
	return NULL;
    }

}
