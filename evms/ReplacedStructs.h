#ifndef __REPLACEDSTRUCTS_H__
#define __REPLACEDSTRUCTS_H__

#include <appAPI.h>

#define MY_CHECK_LEFT_LEN_SIZEOF(left_len, proto_type)	do\
{\
	need_len = sizeof(proto_type);\
	if(left_len < need_len)\
	{\
		return -1;\
	}\
}while(0)
	
#define MY_CHECK_LEFT_LEN(left_len, need_len)		do\
{\
	if(left_len < need_len)\
	{\
		return -1;\
	}\
}while(0)


/*
 * These structures were redefined to make their size more slim,
 * and make some structures more acceptable in network transmission,
 * such as string buffer pointors used in old structures.
 */

#pragma pack(1)

typedef struct __my_value_s
{
	char			c;
	char			s[EVMS_NAME_SIZE + 1];
	u_char			uc;
	boolean			b;
	int			i;
	int8_t			i8;
	int16_t			i16;
	int32_t			i32;
	int64_t			i64;
	u_int			ui;
	u_int8_t		ui8;
	u_int16_t		ui16;
	u_int32_t		ui32;
	u_int64_t		ui64;
	float			r32;
	double			r64;
}my_value_t;

typedef struct __my_function_info_s
{
	task_action_t		function;			// enum

	char 			name[EVMS_NAME_SIZE  + 1];
	char 			title[EVMS_NAME_SIZE + 1];
	char 			verb[EVMS_NAME_SIZE  + 1];
	char 			help[EVMS_NAME_SIZE  + 1];

	u_int32_t 		flags;

}my_function_info_t;

typedef struct __my_option_descriptor_s
{
	char 			name[EVMS_NAME_SIZE  + 1];
	char 			title[EVMS_NAME_SIZE + 1];
	char 			tip[EVMS_NAME_SIZE   + 1];
	char 			help[EVMS_NAME_SIZE  + 1];

	value_type_t   		type;
	value_t			value;

}my_option_descriptor_t;

typedef struct __my_plugin_info_s
{
	char			short_name[EVMS_NAME_SIZE + 1];
	char 			long_name[EVMS_NAME_SIZE + 1];
	char			oem_name[EVMS_NAME_SIZE + 1];
	boolean			supports_containers;
}my_plugin_info_t;

typedef struct __my_storage_object_info_s 
{
	object_type_t		object_type;
	data_type_t		data_type;
	u_int32_t		flags;
	char 			name[EVMS_NAME_SIZE + 1];
	sector_count_t		size;
	sector_count_t 		resize;
}my_storage_object_info_t;

typedef struct __my_storage_container_info_s
{
	u_int32_t 		flags;
	char 			name[EVMS_NAME_SIZE + 1];
	sector_count_t		size;
	sector_count_t 		resize;

}my_storage_container_info_t;

typedef struct __my_logical_volume_info_s
{
	char			mount_point[EVMS_NAME_SIZE + 1];
	sector_count_t		fs_size;
	sector_count_t		min_fs_size;
	sector_count_t		max_fs_size;
	sector_count_t		vol_size;
	sector_count_t		max_vol_size;
	sector_count_t 		resize;
	u_int64_t		serial_number;
	u_int32_t		flags;
	char			name[EVMS_NAME_SIZE + 1];
	char			dev_node[EVMS_NAME_SIZE + 1];
	char                    file_system[EVMS_NAME_SIZE + 1];
}my_logical_volume_info_t;

#pragma pack()



int MY_COPY_OPTION_DESCRIPTOR(my_option_descriptor_t * dst, option_descriptor_t * src);
int MY_COPY_FUNCTION_INFO(my_function_info_t * dst, function_info_t * src);
int MY_CONVERT_PLUGIN_INFO(my_plugin_info_t * dst, plugin_info_t * src);
int MY_REVERT_PLUGIN_INFO(plugin_info_t * dst, my_plugin_info_t * src);
int MY_CONVERT_STORAGE_OBJECT_INFO(my_storage_object_info_t * dst, storage_object_info_t * src);
int MY_REVERT_STORAGE_OBJECT_INFO(storage_object_info_t * dst, my_storage_object_info_t * src);
int MY_CONVERT_STORAGE_CONTAINER_INFO(my_storage_container_info_t * dst, storage_container_info_t * src);
int MY_REVERT_STORAGE_CONTAINER_INFO(storage_container_info_t * dst, my_storage_container_info_t * src);
int MY_CONVERT_LOGICAL_VOLUME_INFO(my_logical_volume_info_t * dst, logical_volume_info_t * src);
int MY_REVERT_LOGICAL_VOLUME_INFO(logical_volume_info_t * dst, my_logical_volume_info_t * src);

#endif
