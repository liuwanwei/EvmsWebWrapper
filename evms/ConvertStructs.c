#include <stdio.h>
#include <string.h>
#include <appAPI.h>

#include "ObjectStructs.h"
#include "ReplacedStructs.h"
#include "PrivateData.h"
#include "../Protocol.h"

#define SEND_EXPAND_OR_SHRINK_SIZE() do\
{\
	if(0 != dst->resize)\
	{\
		printf("Obejct expand or shrink size: %llu sectors\n", dst->resize);\
		printf("sector size: %d bytes\n", EVMS_VSECTOR_SIZE);\
	}\
}while(0)



int MY_CONVERT_PLUGIN_INFO(my_plugin_info_t * dst, plugin_info_t * src)
{
	if(NULL == dst
	|| NULL == src)
	{
		return -1;
	}

	memset(dst, 0, sizeof(my_plugin_info_t));

	int len;

	if(NULL != src->short_name)
	{
		len = strlen(src->short_name);
		len = len > EVMS_NAME_SIZE ? EVMS_NAME_SIZE : len;
		memcpy(dst->short_name, src->short_name, len);
	}

	if(NULL != src->long_name)
	{
		len = strlen(src->long_name);
		len = len > EVMS_NAME_SIZE ? EVMS_NAME_SIZE : len;
		memcpy(dst->long_name, src->long_name, len);
	}

	if(NULL != src->oem_name)
	{
		len = strlen(src->oem_name);
		len = len > EVMS_NAME_SIZE ? EVMS_NAME_SIZE : len;
		memcpy(dst->oem_name, src->oem_name, len);
	}

	dst->supports_containers = src->supports_containers;

	return 0;
}

int MY_CONVERT_STORAGE_OBJECT_INFO(my_storage_object_info_t * dst, storage_object_info_t * src)
{
	if(NULL == dst
	|| NULL == src)
	{
		return -1;
	}

	memset(dst, 0, sizeof(my_storage_object_info_t));

	int len;

	if(NULL != src->name)
	{
		len = strlen(src->name);
		len = len > EVMS_NAME_SIZE ? EVMS_NAME_SIZE : len;
		memcpy(dst->name, src->name, len);
	}

	dst->flags       = src->flags;
	dst->object_type = src->object_type;
	dst->data_type   = src->data_type;
	dst->size	 = src->size;
	dst->resize      = QueryPrivateData(src->handle);

	SEND_EXPAND_OR_SHRINK_SIZE();

	return 0;
}


int MY_CONVERT_STORAGE_CONTAINER_INFO(my_storage_container_info_t * dst, storage_container_info_t * src)
{
	if(NULL == dst
	|| NULL == src)
	{
		return -1;
	}

	memset(dst, 0, sizeof(my_storage_container_info_t));

	int len;

	if(NULL != src->name)
	{
		len = strlen(src->name);
		len = len > EVMS_NAME_SIZE ? EVMS_NAME_SIZE : len;
		memcpy(dst->name, src->name, len);
	}

	dst->flags  = src->flags;
	dst->size   = src->size;
	dst->resize = QueryPrivateData(src->handle);

	SEND_EXPAND_OR_SHRINK_SIZE();

	return 0;
}


int MY_CONVERT_LOGICAL_VOLUME_INFO(my_logical_volume_info_t * dst, logical_volume_info_t * src)
{
	if(NULL == dst
	|| NULL == src)
	{
		return -1;
	}

	memset(dst, 0, sizeof(my_logical_volume_info_t));

	int len;

	if(NULL != src->name)
	{
		len = strlen(src->name);
		len = len > EVMS_NAME_SIZE ? EVMS_NAME_SIZE : len;
		memcpy(dst->name, src->name, len);
	}

	if(NULL != src->mount_point)
	{
		len = strlen(src->mount_point);
		len = len > EVMS_NAME_SIZE ? EVMS_NAME_SIZE : len;
		memcpy(dst->mount_point, src->mount_point, len);
	}

	if(NULL != src->dev_node)
	{
		len = strlen(src->dev_node);
		len = len > EVMS_NAME_SIZE ? EVMS_NAME_SIZE : len;
		memcpy(dst->dev_node, src->dev_node, len);
	}

	dst->fs_size 	  = src->fs_size;
	dst->min_fs_size  = src->min_fs_size;
	dst->max_fs_size  = src->max_fs_size;
	dst->vol_size     = src->vol_size;
	dst->max_vol_size = src->max_vol_size;
	dst->flags        = src->flags;
	dst->serial_number= src->serial_number;
	/* get expanding or shrink size */
	dst->resize = QueryPrivateData(src->handle);
	SEND_EXPAND_OR_SHRINK_SIZE();

	/* get file system type */
    	handle_object_info_t *Temp_Info = NULL;
	if ((src->file_system_manager != 0) 
	&&  (evms_get_info(src->file_system_manager, &Temp_Info) == 0)
	&& (Temp_Info != NULL)) 
	{
		if(NULL != Temp_Info->info.plugin.short_name)
		{
			len = strlen(Temp_Info->info.plugin.short_name);
			len = len > EVMS_NAME_SIZE ? EVMS_NAME_SIZE : len;
			memcpy(dst->file_system, Temp_Info->info.plugin.short_name, len);
			dst->file_system[len] = '\n';
		}

	    	evms_free(Temp_Info);
	} 


	return 0;
}


int MY_PACK_BASIC_INFO(my_basic_info_t *, char *, int *);
int MY_PACK_ADVANCE_INFO(my_advance_info_t *, char *, int *);

int MY_PACK_EVMS_OBJECT(my_object_info_t * obj, char * buffer, int * len)
{
	if(NULL == obj)
	{
		return -1;
	}

	int  left_len = * len;
	int  used_len = 0;

	// Packing basic information into buffer
	if(NULL != obj->basic)
	{
		if(0 != MY_PACK_BASIC_INFO(obj->basic, buffer + used_len, & left_len))
		{
			return -1;
		}

		used_len += left_len;
		left_len = NET_BUFFER_LEN - used_len;
	}

	// Packing advance information into buffer
	if(NULL != obj->advance)
	{
		if(0 != MY_PACK_ADVANCE_INFO(obj->advance, buffer + used_len, & left_len))
		{
			return -1;
		}

		used_len += left_len;
		left_len = NET_BUFFER_LEN - used_len;
	}

	* len = used_len;

	return 0;
}




/*
 * basic	:	[in] the source information to be used
 * buffer	:	[in/out] the buffer to be packed
 * len		:	[in/out] when used as input parameter, 
 * 				 it means buffer length usable.
 * 				 when used as output parameter,
 * 				 it means bytes number totally 
 * 				 packed into the buffer.
 */
int MY_PACK_BASIC_INFO(my_basic_info_t * basic, char * buffer, int * len)
{
	if(NULL == buffer
	|| NULL == len
	|| NULL == basic)
	{
		return -1;
	}

	int used_len = 0;
	int left_len = (* len);
	int need_len = 0;

	memcpy(buffer, &(basic->type), sizeof(int));
	used_len += sizeof(int);
	left_len -= used_len;

	switch(basic->type)
	{
		case PLUGIN:
			{
				my_plugin_info_t info;

				MY_CHECK_LEFT_LEN_SIZEOF(left_len, my_plugin_info_t);

				MY_CONVERT_PLUGIN_INFO(&info, &(basic->info->plugin));
				memcpy(buffer + used_len, &info, need_len);
				used_len += need_len;
			}
			break;
		case VOLUME:
			{
				my_logical_volume_info_t info;

				MY_CHECK_LEFT_LEN_SIZEOF(left_len, my_logical_volume_info_t);

				MY_CONVERT_LOGICAL_VOLUME_INFO(&info, &(basic->info->volume));
				memcpy(buffer + used_len, &info, need_len);
				used_len += need_len;
			}
			break;
		case CONTAINER:
			{
				my_storage_container_info_t info;

				MY_CHECK_LEFT_LEN_SIZEOF(left_len, my_storage_container_info_t);
				MY_CONVERT_STORAGE_CONTAINER_INFO(&info, &(basic->info->container));
				memcpy(buffer + used_len, &info, need_len);
				used_len += need_len;
			}
			break;
		// The below four object used the same storage container: storage_object_info_t
		// So, in union, they consume the same memory position.
		case EVMS_OBJECT:
		case REGION:
		case SEGMENT:
		case DISK:
			{
				my_storage_object_info_t info;

				MY_CHECK_LEFT_LEN_SIZEOF(left_len, my_storage_object_info_t);
				MY_CONVERT_STORAGE_OBJECT_INFO(&info, &(basic->info->disk));
				memcpy(buffer + used_len, &info, need_len);
				used_len += need_len;
			}
			break;
		default:
			return -1;
			break;
	}

	(* len) = used_len;

	return 0;
}

/*
 * almost the same as MY_PACK_BASIC_INFO() except the advance parameter.
 */
int MY_PACK_ADVANCE_INFO(my_advance_info_t * advance, char * buffer, int * len)
{
	if(NULL == advance
	|| NULL == buffer
	|| NULL == len)
	{
		return -1;
	}

	int i;	

	int left_len = (* len);
	int used_len = 0;
	int need_len = 0;

	need_len = sizeof(int) * 3;
	MY_CHECK_LEFT_LEN(left_len, need_len);

	// copy operation_num, task_num and featrue_num fields.
	memcpy(buffer, advance, need_len);
	used_len += need_len;
	left_len -= need_len;

	// copy operations one by one.
	for(i = 0; i < advance->operation_num;  i++)
	{
		// my_operation_t header, plus all operation options.
		MY_CHECK_LEFT_LEN_SIZEOF(left_len, my_operation_t);
		memcpy(buffer + used_len, advance->supported_operations[i], need_len);

		used_len += need_len;
		left_len -= need_len;

 		need_len = advance->supported_operations[i]->option_num * sizeof(my_option_descriptor_t);
		MY_CHECK_LEFT_LEN(left_len, need_len);
		memcpy(buffer + used_len, advance->supported_operations[i]->option, need_len);

		used_len += need_len;
		left_len -= need_len;
	}

	// copy tasks one by one.
	for(i = 0; i < advance->task_num; i ++)
	{
		MY_CHECK_LEFT_LEN_SIZEOF(left_len, my_task_t);
		memcpy(buffer + used_len, advance->supported_tasks[i], need_len);

		used_len += need_len;
		left_len -= need_len;

		need_len = advance->supported_tasks[i]->option_num * sizeof(my_option_descriptor_t);
		MY_CHECK_LEFT_LEN(left_len, need_len);
		memcpy(buffer + used_len, advance->supported_tasks[i]->option, need_len);

		used_len += need_len;
		left_len -= need_len;
	}

	// copy features one by one
	for(i = 0; i < advance->feature_num; i ++)
	{
		MY_CHECK_LEFT_LEN_SIZEOF(left_len, my_feature_t);
		memcpy(buffer + used_len, advance->supported_features[i], need_len);

		used_len += need_len;
		left_len -= need_len;
	}

	// FIXME added this line
	(* len) = used_len;

	return 0;
}
