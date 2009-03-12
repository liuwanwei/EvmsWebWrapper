#include <string.h>

#include "ObjectStructs.h"
#include "ReplacedStructs.h"

#include "../Protocol.h"

inline void MY_CREATE_OPERATION_T(my_operation_t ** op, int option_num)
{
	// FIXME if(NULL == (*op))
	{
		(*op) = (my_operation_t *) malloc(sizeof(my_operation_t) + option_num * sizeof(my_option_descriptor_t));
		if(NULL == (* op))
		{
			abort();
		}

		(*op)->option_num = option_num;
	}
}

inline void MY_CREATE_FEATURE_T(my_feature_t ** feature)
{
	// FIXME if(NULL == (* feature))
	{
		(* feature) = (my_feature_t *) malloc(sizeof(my_feature_t));
		if(NULL == (* feature))
		{
			abort();
		}
	}
}

inline void MY_CREATE_TASK_T(my_task_t ** task, int option_num)
{
	// FIXME if(NULL == (* task))
	{
		(* task) = (my_task_t *) malloc(sizeof(my_task_t) + option_num * sizeof(my_option_descriptor_t));
		if(NULL == (* task))
		{
			abort();
		}

		(* task)->option_num = option_num;
	}
}

inline void MY_CREATE_BASIC_INFO_T(my_basic_info_t ** basic)
{
	if(NULL == (* basic))
	{
		(* basic) = (my_basic_info_t *) malloc(sizeof(my_basic_info_t));
		if(NULL == (* basic))
		{
			abort();
		}

		(* basic)->type     = 0;
		(* basic)->info     = NULL;
		// (* basic)->size     = 0;
		// (* basic)->units[0] = '\0';
	}
}

inline void MY_CREATE_ADVANCE_INFO_T(my_advance_info_t ** advance)
{
	if(NULL == (* advance))
	{
		(* advance) = (my_advance_info_t *) malloc(sizeof(my_advance_info_t));
		(* advance)->operation_num 	  	= 0;
		(* advance)->task_num 		  	= 0;
		(* advance)->feature_num		= 0;
	}
}

inline void MY_FREE_BASIC_INFO_T(my_basic_info_t * info)
{
	if(NULL == info)
	{
		return;
	}

	if(info->info)
	{
		// my_basic_info_t::info was assigned by stack memory
		// so we needn't free it.
		// free(info->info);
		info->info = NULL;
	}

	free(info);
}

inline void MY_FREE_ADVANCE_INFO_T(my_advance_info_t * info)
{
	if(NULL == info)
	{
		return;
	}

	int num = 0;

	num = info->operation_num;
	while(num > 0)
	{
		free(info->supported_operations[num - 1]);
		num --;
	}

	num = info->task_num;
	while(num > 0)
	{
		free(info->supported_tasks[num -1]);
		num --;
	}

	num = info->feature_num;
	while(num > 0)
	{
		free(info->supported_features[num - 1]);
		num --;
	}

	free(info);
}

inline void MY_CREATE_OBJECT_INFO_T(object_info_t ** obj)
{
	if(NULL == (* obj))
	{
		(* obj) = (object_info_t *) malloc(sizeof(object_info_t));
		memset((*obj), 0, sizeof(object_info_t));
	}
}

inline void MY_ADD_OPTION_DESCRIPTORS(void * options_header, 
		                   option_descriptor_t ** descriptors, 
				   int count)
{
	if(NULL == options_header
	|| NULL == descriptors
	|| 0 >= count)
	{
		return;
	}
	
	int i;
	my_option_descriptor_t * iterator = NULL;

	iterator = (my_option_descriptor_t *)options_header;
	for(i = 0; i < count; i ++)
	{
		MY_COPY_OPTION_DESCRIPTOR(iterator, descriptors[i]);
		iterator ++;
	}
}

int MY_ADD_OPERATION(my_advance_info_t * advance, int op_type, int option_num)
{
	if(NULL == advance
	|| option_num > MAX_OPERATION_NUM)
	{
		// invaild parameters
		return -1;
	}

	if(advance->operation_num >= MAX_OPERATION_NUM)
	{
		// beyond max operaton size 
		return -2;
	}

	my_operation_t * op = NULL;

	MY_CREATE_OPERATION_T(&op, option_num);
	op->type = op_type;
	op->option_num = option_num;

	advance->supported_operations[advance->operation_num] = op;
	advance->operation_num ++;

	// return the position in table where we stored operstion
	return (advance->operation_num - 1);
}


int MY_ADD_FEATURE(my_advance_info_t * advance, const char * name)
{
	if(NULL == advance
	|| NULL == name)
	{
		// invalid parameter
		return -1;
	}
	if(advance->feature_num >= MAX_FEATURE_NUM)
	{
		// beyond max features num
		return -2;
	}

	int len;

	my_feature_t * feature = 0;
	MY_CREATE_FEATURE_T(&feature);

	len = strlen(name);
	len = len > EVMS_NAME_SIZE ? EVMS_NAME_SIZE : len;
	memcpy(feature->name, name, len);

	advance->supported_features[advance->feature_num] = feature;
	advance->feature_num ++;

	return (advance->feature_num - 1);
}

int MY_GET_BASIC_INFO(my_object_info_t * my_obj, handle_object_info_t * evms_obj)
{
	if(NULL == my_obj
	|| NULL == evms_obj)
	{
		return -1;
	}

	if(NULL == my_obj->basic)
	{
		MY_CREATE_BASIC_INFO_T( &(my_obj->basic) );
	}

	// get type
	my_obj->basic->type = evms_obj->type;

	// get basic information
	my_obj->basic->info = &(evms_obj->info);

	/*
	// get size and units
	int size_in_sector = 0;
	switch(my_obj->basic->type)
	{
		case VOLUME:
			size_in_sector = evms_obj->info.volume.vol_size;
			break;
		case EVMS_OBJECT:
			size_in_sector = evms_obj->info.object.size;
			break;
		case REGION:
			size_in_sector = evms_obj->info.region.size;
			break;
		case CONTAINER:
			size_in_sector = evms_obj->info.container.size;
			break;
		case SEGMENT:
			size_in_sector = evms_obj->info.segment.size;
			break;
		case DISK:
			size_in_sector = evms_obj->info.disk.size;
			break;
		default:
			break;
	}

	if(0 != size_in_sector)
	{
		int len;
		char * units;
		double readable_size;

		units = Determine_Units(size_in_sector * 512, &readable_size);

		my_obj->basic->size = readable_size;
		len = strlen(units);
		len = len > EVMS_NAME_SIZE ? EVMS_NAME_SIZE : len;

		strncpy(my_obj->basic->units, units, len);
	}
	*/

	return 0;
}





#if 0
int main()
{
	char buffer[NET_BUFFER_LEN + 1];
	int len;
	my_object_info_t info;

	/*
	 * ......
	 * after some steps, the buffer was packed with network frame, and len indicates used buffer len.
	 * ......
	 *
	 */

	if(0 != MY_UNPACK_EVMS_OBJECT(buffer, len, &info))
}
#endif 



