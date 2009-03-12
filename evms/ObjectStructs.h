#ifndef __OBJECTSTRUCTS_H__
#define __OBJECTSTRUCTS_H__

/* must including this file other than appstructs.h or options.h */
#include <appAPI.h>

#include "ReplacedStructs.h"

#define MAX_TASK_NUM	  32
#define MAX_OPERATION_NUM MAX_TASK_NUM
#define MAX_FEATURE_NUM   5

#define DL_FEATURE	 (0x00000001 << 0)
#define BBR_FEATURE	 (0x00000001 << 1)
#define SNAPSHOT_FEATURE (0x00000001 << 2)

typedef enum {
	MY_OP_NULL	= 0,
	MY_OP_ALLOCATE,
	MY_OP_ASSIGN_SEGMENT_MANAGER,
	MY_OP_TASK_CREATE,
	MY_OP_TASK_CREATE_CONTAINER,
	MY_OP_TASK_CREATE_REGION,
	MY_OP_TASK_CREATE_VOLUME,
	MY_OP_TASK_CREATE_SO,			/* storage object */
	MY_OP_TASK_CREATE_SEGMENT,
	MY_OP_TASK_ASSIGN_PLUGIN,
	MY_OP_TASK_MKFS,
	MY_OP_TASK_FSCK,
	MY_OP_CHECK,
	MY_OP_CONVERT,
	MY_OP_DELETE,
	MY_OP_EXPAND,
	MY_OP_FORMAT,
	MY_OP_RENAME,
	MY_OP_REVERT,
	MY_OP_SHRINK,
	MY_OP_UNFORMAT,
	MY_OP_SETINFO,
	MY_OP_SLIDE,
	MY_OP_MOVE,
	MY_OP_REPLACE

} my_operation_type;


typedef struct __operation_s
{
	my_operation_type        type;		// basic operation info : type

	int 			 option_num;	// each operation may have many options
	my_option_descriptor_t   option[0];
}my_operation_t;


typedef struct __task_s
{
	my_function_info_t 	 function; 	// basic task info : description

	int 			 option_num;	// each task may have many options
	my_option_descriptor_t   option[0]; 

}my_task_t;


typedef struct __feature_s
{
	char name[EVMS_NAME_SIZE + 1];
}my_feature_t;

typedef struct __advance_info_s
{
	int operation_num;			// object may support many operations
	int task_num;				// object may support many tasks
	int feature_num;			// object may have many features

	my_operation_t * supported_operations[MAX_OPERATION_NUM];
	my_task_t      * supported_tasks[MAX_TASK_NUM];
	my_feature_t   * supported_features[MAX_FEATURE_NUM];
}my_advance_info_t;


typedef struct __basic_info_s
{
	int type;
	object_info_t  * info;
}my_basic_info_t;

/*
 * For all object types , the corresponding basic info's proto type is:
enum {
	OBJ_TYPE_PLUGIN = 1,
	OBJ_TYPE_DISK,
	OBJ_TYPE_SEGMENT,
	OBJ_TYPE_REGION,
	OBJ_TYPE_OBJECT,
	OBJ_TYPE_CONTAINER,
	OBJ_TYPE_VOLUME
} my_object_type;
 */

typedef struct __object_info_s
{
	my_basic_info_t   * basic;
	my_advance_info_t * advance;
}my_object_info_t;



void MY_CREATE_OPERATION_T(my_operation_t ** op, int option_num);
void MY_CREATE_FEATURE_T(my_feature_t ** feature);
void MY_CREATE_BASIC_INFO_T(my_basic_info_t ** basic);
void MY_CREATE_ADVANCE_INFO_T(my_advance_info_t ** advance);
void MY_FREE_BASIC_INFO_T(my_basic_info_t * basic);
void MY_FREE_ADVANCE_INFO_T(my_advance_info_t * advance);

void MY_FREE_MY_OBJECT_INFO_T(my_object_info_t * obj);

void MY_ADD_OPTION_DESCRIPTORS(void * options_header, option_descriptor_t **, int);
int  MY_ADD_OPERATION(my_advance_info_t * parent, int op_type, int option_num);
int  MY_ADD_FEATURE(my_advance_info_t *, const char * name);

// Get basic info from evms object.
int  MY_GET_BASIC_INFO(my_object_info_t * my_obj, handle_object_info_t * evms_obj);

// Send packet to the other peer.
int  MY_PACK_EVMS_OBJECT(my_object_info_t * obj, char * buffer, int * len);

// The function is a demo to lwplwp123, imulating unpack procedure in web 
// module.
int  MY_UNPACK_EVMS_OBJECT(char * buffer, int len, my_object_info_t * obj);


#endif
