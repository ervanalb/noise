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

error_t simple_copy(type_info_pt type_info, output_pt src, output_pt dest)
{
	memcpy(src,dest,*(int*)type_info);
	return SUCCESS;
}

