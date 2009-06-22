#include <stdlib.h>
#include <string.h>
#include "PrivateData.h"


hash_node_t * hash_entry_tbl[PRIVATE_HASH_SIZE];

void InitHash()
{
	unsigned int i;
	for(i = 0; i < PRIVATE_HASH_SIZE; i ++)
	{
		hash_entry_tbl[i] = NULL;
	}
}

/*
 * function: 
 * 	destroy all dynamiclly malloced memroy
 */
void DestroyHash()
{
	unsigned int i;
	hash_node_t * next, * cur;

	for(i = 0; i < PRIVATE_HASH_SIZE; i ++)
	{
		if(NULL == hash_entry_tbl[i])
		{
			continue;
		}

		cur = hash_entry_tbl[i];
		while(cur)
		{
			next = cur->next;	
			free(cur);
			cur = next;
		}

		hash_entry_tbl[i] = NULL;
	}
}

/*
 * function:
 * 	automatically adding current thread's storage block to hash table
 * parameter:
 * 	none.
 * return value:
 * 	pointer of hash node or already exist newlly added.
 */
hash_node_t * AddNode()
{
	u_int32_t entry;
	pthread_t tid;
	hash_node_t * cur;


	tid = pthread_self();

	entry = GET_ENTRY(tid);

	if(NULL == hash_entry_tbl[entry])
	{
		/* current thread info doesn't exist, create it */
		cur = (hash_node_t *)malloc(sizeof(hash_node_t));
		memset(cur, 0, sizeof(hash_node_t));
		cur->thread_id = tid;
		cur->next = NULL;
		hash_entry_tbl[entry] = cur;
	}
	else
	{
		cur = hash_entry_tbl[entry];
		while(cur)
		{
			if(tid == cur->thread_id)
			{
				break;
			}
			cur = cur->next;
		}

		if(NULL == cur)
		{
			/* current thread info doesn't exist, create it */
			cur = (hash_node_t *)malloc(sizeof(hash_node_t));
			memset(cur, 0, sizeof(hash_node_t));
			cur->thread_id = tid;

			/* insert new node into hash entry header */
			cur->next = hash_entry_tbl[entry];
			hash_entry_tbl[entry] = cur;
		}
	}

	/* return newlly created or already exist node's pointor */
	return cur;
}


/*
 * function:
 * 	search current thread's information block in hash table.
 * parameter:
 * return:
 * 	NULL, not found
 * 	! NULL, the node's pointer of current thread in hash table.
 */
hash_node_t * SearchNode()
{
	unsigned int entry;
	hash_node_t * cur;
	pthread_t tid;

	tid = pthread_self();

	entry = GET_ENTRY(tid);

	/* empty entry */
	if(NULL == hash_entry_tbl[entry])
	{
		/* not find */
		return NULL;
	}

	cur = hash_entry_tbl[entry];
	while(NULL != cur)
	{
		if(cur->thread_id == tid)
		{
			break;
		}
		else
		{
			cur = cur->next;
		}
	}

	if(NULL == cur)
	{
		/* not find */
		return NULL;
	}
	else
	{
		/* find */
		return cur;
	}
	
}

/*
 * function: 
 * 	Clear current thread's expandable object information. 
 * return value:
 * 	0, Success.
 * 	-1, Failed.
 * remark:
 * 	Because the node may be used frequentlly after prev cli command, 
 * 	We don't free the node untile system shutdown.
 */
int ClearNode()
{
	hash_node_t * cur = NULL;

	cur = SearchNode();

	if(NULL == cur)
	{
		return -1;
	}
	else
	{
		memset(cur->obj, 0, 
				sizeof(expandable_obj_t) * MAX_EXPAND_POINTS);

		return 0;
	}
}


int InsertPrivateData(object_handle_t handle, u_int64_t size)
{
	int i;
	hash_node_t * cur = NULL;

	cur = AddNode();

	if(cur->count >= MAX_EXPAND_POINTS)
	{
		return -1;
	}

	cur->obj[cur->count].handle = handle;
	cur->obj[cur->count].expandable_size = size;
	cur->count ++;

	return 0;
}

sector_count_t QueryPrivateData(object_handle_t handle)
{
	int i;
	hash_node_t * cur = NULL;

	if(NULL == (cur = SearchNode()))
	{
		return 0;
	}
	
	for(i = 0; i < cur->count; i ++)
	{
		if(handle == cur->obj[i].handle)
		{
			return cur->obj[i].expandable_size;
		}
	}

	return 0;
}
