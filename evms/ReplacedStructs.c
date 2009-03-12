#include <stdio.h>
#include <string.h>
#include <appAPI.h>

#include "ReplacedStructs.h"
#include "PrivateData.h"


int MY_COPY_OPTION_DESCRIPTOR(my_option_descriptor_t * dst, option_descriptor_t * src)
{
	if(NULL == dst
	|| NULL == src)
	{
		return -1;
	}

	memset(dst, 0, sizeof(my_option_descriptor_t));

	int len;

	if(NULL != src->name)
	{
		len = strlen(src->name);
		len = len > EVMS_NAME_SIZE ? EVMS_NAME_SIZE : len;
		memcpy(dst->name, src->name, len);
	}

	if(NULL != src->title)
	{
		len = strlen(src->title);
		len = len > EVMS_NAME_SIZE ? EVMS_NAME_SIZE : len;
		memcpy(dst->title, src->title, len);
	}

	if(NULL != src->tip)
	{
		len = strlen(src->tip);
		len = len > EVMS_NAME_SIZE ? EVMS_NAME_SIZE : len;
		memcpy(dst->tip, src->tip, len);
	}

	if(NULL != src->help)
	{
		len = strlen(src->help);
		len = len > EVMS_NAME_SIZE ? EVMS_NAME_SIZE : len;
		memcpy(dst->help, src->help, len);
	}

	dst->type  = src->type;
	dst->value = src->value;

	return 0;
}

int MY_COPY_FUNCTION_INFO(my_function_info_t * dst, function_info_t * src)
{
	if(NULL == dst
	|| NULL == src)
	{
		return -1;
	}

	memset(dst, 0, sizeof(my_function_info_t));

	int len;

	if(NULL != src->name)
	{
		len = strlen(src->name);
		len = len > EVMS_NAME_SIZE ? EVMS_NAME_SIZE : len;
		memcpy(dst->name, src->name, len);
	}

	if(NULL != src->title)
	{
		len = strlen(src->title);
		len = len > EVMS_NAME_SIZE ? EVMS_NAME_SIZE : len;
		memcpy(dst->title, src->title, len);
	}

	if(NULL != src->verb)
	{
		len = strlen(src->verb);
		len = len > EVMS_NAME_SIZE ? EVMS_NAME_SIZE : len;
		memcpy(dst->verb, src->verb, len);
	}

	if(NULL != src->help)
	{
		len = strlen(src->help);
		len = len > EVMS_NAME_SIZE ? EVMS_NAME_SIZE : len;
		memcpy(dst->help, src->help, len);
	}

	dst->function = src->function;
	dst->flags    = src->flags;

	return 0;
}

