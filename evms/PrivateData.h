#ifndef __PRIVATE_DATA_H__
#define __PRIVATE_DATA_H__

#include <pthread.h>
#include <appAPI.h>

// expand or shrink points, 64 maybe small on some ultimate environment 
#define MAX_EXPAND_POINTS		128

#define PRIVATE_HASH_SIZE		64
#define GET_ENTRY(tid)			(tid % PRIVATE_HASH_SIZE)

typedef struct _expandable_obj_t
{
	object_handle_t handle;
	sector_count_t expandable_size;	
}expandable_obj_t;

typedef struct _hash_node_t
{
	pthread_t thread_id;

	int count;
	expandable_obj_t obj[MAX_EXPAND_POINTS];
	struct _hash_node_t * next;
}hash_node_t;

void InitHash();
void DestroyHash();

hash_node_t * AddNode();
hash_node_t * SearchNode();
int ClearNode();

int InsertPrivateData(object_handle_t handle, sector_count_t size);
sector_count_t QueryPrivateData(object_handle_t handle);

#endif
