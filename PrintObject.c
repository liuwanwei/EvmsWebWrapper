#include "PrintObject.h"
#include <stdio.h>
#include <appAPI.h>

void PrintBasic(my_basic_info_t * basic);
void PrintAdvance(my_advance_info_t * advance);

void PrintObject(my_object_info_t * obj)
{
	if(NULL == obj)
	{
		return;
	}

	printf("\n");

	PrintBasic(obj->basic);
	
	PrintAdvance(obj->advance);

	printf("\n");
}

void PrintPlugin(plugin_info_t * info);
void PrintStorageObjectInfo(storage_object_info_t * info);
void PrintStorageContainerInfo(storage_container_info_t * info);
void PrintLogicalVolumeInfo(logical_volume_info_t * info);

void PrintBasic(my_basic_info_t * basic)
{
	if(NULL == basic)
	{
		return;
	}

	printf("+ BASIC INFO : \n");

	printf("	type: ");

	switch(basic->type)
	{
		case PLUGIN:
			printf("PLUGIN\n");
			PrintPlugin(&(basic->info->plugin));
			break;
		case DISK:
			printf("DISK\n");
			PrintStorageObjectInfo(&(basic->info->disk));
			break;
		case SEGMENT:
			printf("SEGMENT\n");
			PrintStorageObjectInfo(&(basic->info->segment));
			break;
		case REGION:
			printf("REGION\n");
			PrintStorageObjectInfo(&(basic->info->region));
			break;
		case EVMS_OBJECT:
			printf("EVMS_OBJECT\n");
			PrintStorageObjectInfo(&(basic->info->object));
			break;
		case CONTAINER:
			printf("CONTAINER\n");
			PrintStorageContainerInfo(&(basic->info->container));
			break;
		case VOLUME:
			printf("VOLUME\n");
			PrintLogicalVolumeInfo(&(basic->info->volume));
			break;
		case TASK:
			printf("TASK\n");
			break;
		default:
			printf("UNKNOWN %d\n", basic->type);
			break;
	}
}

void PrintPlugin(plugin_info_t * info)
{
	if(NULL == info)
	{
		return;
	}

	printf("	short_name: %s\n", info->short_name);
	printf("	long_name: %s\n", info->long_name);
	printf("	oem_name: %s\n", info->oem_name);
}

void PrintStorageObjectInfo(storage_object_info_t * info)
{
	if(NULL == info)
	{
		return;
	}

	printf("	type: %u\n", info->object_type);
	printf("	name: %s\n", info->name);
	printf("	data_type: %u\n", info->data_type);
	printf("	dev_major: %u\n", info->dev_major);
	printf("	dev_minor: %u\n", info->dev_minor);
	printf("        flags: %u\n",     info->flags);

}

void PrintStorageContainerInfo(storage_container_info_t * info)
{
	if(NULL == info)
	{
		return;
	}

	printf("	name: %s\n", info->name);
	printf("        flags: %u\n",     info->flags);
}

void PrintLogicalVolumeInfo(logical_volume_info_t * info)
{
	if(NULL == info)
	{
		return;
	}

	printf("	name: %s\n", info->name);
	printf("	dev_node: %s\n", info->dev_node);
	printf("	mount_point: %s\n", info->mount_point);
	printf("	dev_major: %u\n", info->dev_major);
	printf("	dev_minor: %u\n", info->dev_minor);
	printf("        flags: %u\n",     info->flags);
}


void PrintOperation(my_operation_t * op);
void PrintTask(my_task_t * task);
void PrintFeature(my_feature_t * feature);

void PrintAdvance(my_advance_info_t * advance)
{
	if(NULL == advance)
	{
		return;
	}

	int i;

	printf("+ ADVANCE INFO : \n");

	if(advance->operation_num > 0)
	{
		printf("	+ operation_num : %d\n", advance->operation_num);
		for(i = 0; i < advance->operation_num; i ++)
		{
			PrintOperation(advance->supported_operations[i]);
		}
	}

	if(advance->task_num > 0)
	{
		printf("	+ task_num: %d\n", advance->task_num);
		for(i = 0; i < advance->task_num; i ++)
		{
			PrintTask(advance->supported_tasks[i]);
		}
	}

	if(advance->feature_num > 0)
	{
		printf("	+ feature_num: %d\n", advance->feature_num);
		for(i = 0; i < advance->feature_num; i ++)
		{
			PrintFeature(advance->supported_features[i]);
		}
	}
}

void PrintOption(my_option_descriptor_t * option);

void PrintOperation(my_operation_t * op)
{
	if(NULL == op)
	{
		return;
	}

	int i;

	printf("		- operation type: %d\n", op->type);

	if(op->option_num > 0)
	{
		printf("		+ option num: %d :\n", op->option_num);
		for(i = 0; i < op->option_num; i ++)
		{
			PrintOption(&(op->option[i]));
		}
	}
}

void PrintTask(my_task_t * task)
{
	if(NULL == task)
	{
		return;
	}
	
	int i;

	printf("		- name: %s\n", task->function.name);
	printf("		- title: %s\n", task->function.title);
	printf("		- verb: %s\n", task->function.verb);
	printf("		- help: %s\n", task->function.help);

	if(task->option_num > 0)
	{
		printf("		+ option num: %d :\n", task->option_num);

		for(i = 0; i < task->option_num; i ++)
		{
			PrintOption(&(task->option[i]));
		}
	}
}

void PrintFeature(my_feature_t * feature)
{
	if(NULL == feature)
	{
		return;
	}

	printf("		- name: %s\n", feature->name);
}


void PrintOption(my_option_descriptor_t * option)
{
	if(NULL == option)
	{
		return;
	}

	printf("			- name: %s\n", option->name);
	printf("			- title: %s\n", option->title);
	printf("			- tip: %s\n", option->tip);
	printf("			- help: %s\n", option->help);
}
