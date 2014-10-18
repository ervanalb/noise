#include "typefns.h"
#include <stdlib.h>
#include <string.h>
#include "globals.h"

error_t chunk_alloc(type_info_pt type_info, output_pt* output)
{
	*output = malloc(sizeof(double)*global_chunk_size);
	if(!(*output)) return raise_error(ERR_MALLOC,"");
	return SUCCESS;
}

void chunk_free(type_info_pt type_info, output_pt output)
{
	free(output);
}

error_t chunk_copy(type_info_pt type_info, output_pt src, output_pt dest)
{
	memcpy(src,dest,sizeof(double)*global_chunk_size);
	return SUCCESS;
}

error_t simple_alloc(type_info_pt type_info, output_pt* output)
{
	*output = malloc(*(int*)type_info);
	if(!(*output)) return raise_error(ERR_MALLOC,"");
	return SUCCESS;
}

void simple_free(type_info_pt type_info, output_pt output)
{
	free(output);
}

error_t simple_copy(type_info_pt type_info, output_pt dest, output_pt src)
{
	memcpy(dest,src,*(int*)type_info);
	return SUCCESS;
}

error_t array_alloc(type_info_pt type_info, output_pt* output)
{
	array_info_t* array_info = (array_info_t*)type_info;
	int n = sizeof(output_pt)*array_info->length;
	*output = malloc(n);
	if(!(*output)) return raise_error(ERR_MALLOC,"");
	memset(*output,0,n);
	return SUCCESS;
}

void array_free(type_info_pt type_info, output_pt output)
{
	int i;
	array_info_t* array_info = (array_info_t*)type_info;

	int len = array_info->length;

	output_free_fn_pt element_free_fn = array_info->element.free_fn;
	type_info_pt element_type_info = array_info->element.type_info;

	output_array_t* array_buf = (output_array_t*)output;

	for(i=0;i<len;i++)
	{
		if(array_buf[i]) (*element_free_fn)(element_type_info,&array_buf[i]);
	}
}

error_t array_copy(type_info_pt type_info, output_pt src, output_pt dest)
{
	int i;
	array_info_t* array_info = (array_info_t*)type_info;

	int len = array_info->length;
	output_alloc_fn_pt element_alloc_fn = array_info->element.alloc_fn;
	output_copy_fn_pt element_copy_fn = array_info->element.copy_fn;

	type_info_pt element_type_info = array_info->element.type_info;

	output_array_t* src_buf = (output_array_t*)src;
	output_array_t* dest_buf = (output_array_t*)dest;

	error_t e;

	for(i=0;i<len;i++)
	{
		if(src_buf[i])
		{
			e=(*element_alloc_fn)(element_type_info,&(dest_buf[i]));
			if(e != SUCCESS) return e;
			e=(*element_copy_fn)(element_type_info,dest_buf[i],src_buf[i]);
			if(e != SUCCESS) return e;
		}
	}
	return SUCCESS;
}

