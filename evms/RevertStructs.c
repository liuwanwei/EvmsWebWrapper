#include <stdio.h>
#include <string.h>
#include <appAPI.h>

#include "ReplacedStructs.h"
#include "ObjectStructs.h"
#include "PrivateData.h"

#define GET_EXPAND_OR_SHRINK_SIZE() do\
{\
	if(0 != src->resize)\
	{\
		printf("Object expand or shrink size: %llu sectors\n", src->resize);\
		printf("sector size: %d bytes\n", EVMS_VSECTOR_SIZE);\
	}\
}while(0)

#define PRINT_FILE_SYSTEM() do\
{\
	if(src->file_system[0] != '\0')\
	{\
		printf("file system: %s\n", src->file_system);\
	}\
}while(0)

int MY_REVERT_PLUGIN_INFO(plugin_info_t * dst, my_plugin_info_t * src)
{
	if(NULL == dst
	|| NULL == src)
	{
		return -1;
	}

	dst->short_name = src->short_name;
	dst->long_name  = src->long_name;
	dst->oem_name   = src->oem_name;
	dst->supports_containers = src->supports_containers;

	return 0;
}

int MY_REVERT_STORAGE_OBJECT_INFO(storage_object_info_t * dst, my_storage_object_info_t * src)
{
	if(NULL == dst
	|| NULL == src)
	{
		return -1;
	}

	memset(dst, 0, sizeof(storage_object_info_t));

	int len;

	if(NULL != src->name)
	{
		len = strlen(src->name);
		len = len > EVMS_NAME_SIZE ? EVMS_NAME_SIZE : len;
		memcpy(dst->name, src->name, len);
	}

	dst->flags       = src->flags;
	dst->object_type = src->object_type;
	dst->data_type = src->data_type;
	dst->size = src->size;

	GET_EXPAND_OR_SHRINK_SIZE();

	return 0;
}


int MY_REVERT_STORAGE_CONTAINER_INFO(storage_container_info_t * dst, my_storage_container_info_t * src)
{
	if(NULL == dst
	|| NULL == src)
	{
		return -1;
	}

	memset(dst, 0, sizeof(storage_container_info_t));

	int len;

	if(NULL != src->name)
	{
		len = strlen(src->name);
		len = len > EVMS_NAME_SIZE ? EVMS_NAME_SIZE : len;
		memcpy(dst->name, src->name, len);
	}

	dst->flags = src->flags;
	dst->size  = src->size;

	GET_EXPAND_OR_SHRINK_SIZE();

	return 0;
}

int MY_REVERT_LOGICAL_VOLUME_INFO(logical_volume_info_t * dst, my_logical_volume_info_t * src)
{
	if(NULL == dst
	|| NULL == src)
	{
		return -1;
	}
	
	memset(dst, 0, sizeof(logical_volume_info_t));

	int len;

	if(NULL != src->name)
	{
		len = strlen(src->name);
		len = len > EVMS_NAME_SIZE ? EVMS_NAME_SIZE : len;
		memcpy(dst->name, src->name, len);
	}

	if(NULL != src->dev_node)
	{
		len = strlen(src->dev_node);
		len = len > EVMS_NAME_SIZE ? EVMS_NAME_SIZE : len;
		memcpy(dst->dev_node, src->dev_node, len);
	}

	dst->mount_point  = src->mount_point;
	dst->fs_size 	  = src->fs_size;
	dst->min_fs_size  = src->min_fs_size;
	dst->max_fs_size  = src->max_fs_size;
	dst->vol_size     = src->vol_size;
	dst->max_vol_size = src->max_vol_size;
	dst->serial_number= src->serial_number;
	dst->flags        = src->flags;

	GET_EXPAND_OR_SHRINK_SIZE();

	PRINT_FILE_SYSTEM();

	return 0;
}

int MY_UNPACK_BASIC_INFO(char * buffer, int * len, my_object_info_t * info);
int MY_UNPACK_ADVANCE_INFO(char * buffer, int * len, my_object_info_t * info);

// Reverse procedure in MY_SEND_EVMS_OBJECT()
// FIXME FIXME FIXME
// 1, memeory leak everywhere.
// 2, network bytes number maybe different with host machine bytes number
// 3, object_info_t should not be used.
int MY_UNPACK_EVMS_OBJECT(char * buffer, int len, my_object_info_t * info)
{
	int left_len = len;
	int used_len = 0;

	if(0 != MY_UNPACK_BASIC_INFO(buffer + used_len, &left_len, info))
	{
		return -1;
	}

	used_len += left_len;
	left_len = len - used_len;

	if(left_len > 0)
	{
		if(0 != MY_UNPACK_ADVANCE_INFO(buffer + used_len, &left_len, info))
		{
			return -1;
		}
	}

	return 0;
}


int MY_UNPACK_BASIC_INFO(char * buffer, int * len, my_object_info_t * info)
{
	if(NULL == buffer
	|| NULL == len
	|| NULL == info)
	{
		return -1;
	}

	int left_len = (* len);
	int used_len = 0;
	int need_len = 0;

	info->basic = NULL;
	MY_CREATE_BASIC_INFO_T(&(info->basic));

	// get my_basic_info_t::type
	MY_CHECK_LEFT_LEN_SIZEOF(left_len, int);
	memcpy(&(info->basic->type), buffer, need_len);

	used_len += need_len;
	left_len -= need_len;

	info->basic->info = NULL;
	MY_CREATE_OBJECT_INFO_T(&(info->basic->info));
	switch(info->basic->type)
	{
		case PLUGIN:
			{
				my_plugin_info_t * tmp;
				
				MY_CHECK_LEFT_LEN_SIZEOF(left_len, my_plugin_info_t);
				tmp = (my_plugin_info_t *)(buffer + used_len);
				MY_REVERT_PLUGIN_INFO(&(info->basic->info->plugin), tmp);
			}
			break;
		case VOLUME:
			{
				my_logical_volume_info_t * tmp;

				MY_CHECK_LEFT_LEN_SIZEOF(left_len, my_logical_volume_info_t);
				tmp = (my_logical_volume_info_t *)(buffer + used_len);
				MY_REVERT_LOGICAL_VOLUME_INFO(&(info->basic->info->volume), tmp);
			}
			break;
		case CONTAINER:
			{
				my_storage_container_info_t * tmp;

				MY_CHECK_LEFT_LEN_SIZEOF(left_len, my_storage_container_info_t);
				tmp = (my_storage_container_info_t *)(buffer + used_len);
				MY_REVERT_STORAGE_CONTAINER_INFO(&(info->basic->info->container), tmp);
			}
			break;
		case EVMS_OBJECT:
		case REGION:
		case SEGMENT:
		case DISK:
			{
				my_storage_object_info_t * tmp;

				MY_CHECK_LEFT_LEN_SIZEOF(left_len, my_storage_object_info_t);
				tmp = (my_storage_object_info_t *)(buffer + used_len);
				MY_REVERT_STORAGE_OBJECT_INFO(&(info->basic->info->region), tmp);
			}
			break;
		default:
			return -1;
			break;
	}

	used_len += need_len;
	left_len -= need_len;

	(* len) = used_len;

	return 0;
}

int MY_UNPACK_ADVANCE_INFO(char * buffer, int * len, my_object_info_t * info)
{
	if(NULL == buffer
	|| NULL == info
	|| (* len) <= 0)
	{
		return -1;
	}

	int i;
	int num;
	int need_len = 0;
	int used_len = 0;
	int left_len = (* len);

	MY_CREATE_ADVANCE_INFO_T(&(info->advance));

	// my_advance_info_t::operation_num ~::task_num ~::feature_num
	need_len = 3 * sizeof(int);

	memcpy(info->advance, buffer, need_len);
	used_len += need_len;
	left_len -= need_len;

	// handle supported operations.
	num = info->advance->operation_num;
	if(num > 0)
	{
		my_operation_t * op = NULL;
		for(i = 0; i < num; i ++)
		{
			op = (my_operation_t *)(buffer + used_len);

			need_len = sizeof(my_operation_t) + op->option_num * sizeof(my_option_descriptor_t);
			MY_CHECK_LEFT_LEN(left_len, need_len);

			MY_CREATE_OPERATION_T(&(info->advance->supported_operations[i]), op->option_num);
			memcpy(info->advance->supported_operations[i], buffer + used_len, need_len);
			used_len += need_len;
			left_len -= need_len;
		}
	}

	// handle supported tasks.
	num = info->advance->task_num;
	if(num > 0)
	{
		my_task_t * task = NULL;
		for(i = 0; i < num; i ++)
		{
			task = (my_task_t *)(buffer + used_len);
			need_len = sizeof(my_task_t) + task->option_num * sizeof(my_option_descriptor_t);
			MY_CHECK_LEFT_LEN(left_len, need_len);
			MY_CREATE_TASK_T(&(info->advance->supported_tasks[i]), task->option_num);
			memcpy(info->advance->supported_tasks[i], buffer + used_len, need_len);
			used_len += need_len;
			left_len -= need_len;
		}
	}

	// handle features.
	num = info->advance->feature_num;
	if(num > 0)
	{
		my_feature_t * feature = NULL;

		for(i = 0; i < num; i ++)
		{
			feature = (my_feature_t *)(buffer + used_len);
			need_len = sizeof(my_feature_t);
			MY_CHECK_LEFT_LEN(left_len, need_len);
			MY_CREATE_FEATURE_T(&(info->advance->supported_features[i]));
			memcpy(info->advance->supported_features[i], buffer + used_len, need_len);

			used_len += need_len;
			left_len -= need_len;
		}
	}

	(* len) -= used_len; 

	return 0;
}
