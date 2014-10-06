#ifndef __TYPEFNS_H
#define __TYPEFNS_H

#include "block.h"

typedef output_pt output_array_t;

typedef struct
{
	int length;
	object_info_t element;
} array_info_t;

error_t chunk_alloc(type_info_pt type_info, output_pt* output);
void chunk_free(type_info_pt type_info, output_pt output);
error_t chunk_copy(type_info_pt type_info, output_pt src, output_pt dest);

error_t simple_alloc(type_info_pt type_info, output_pt* output);
void simple_free(type_info_pt type_info, output_pt output);
error_t simple_copy(type_info_pt type_info, output_pt src, output_pt dest);

error_t array_alloc(type_info_pt type_info, output_pt* output);
void array_free(type_info_pt type_info, output_pt output);
error_t array_copy(type_info_pt type_info, output_pt src, output_pt dest);

#endif
