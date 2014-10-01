#include "block.h"

error_t chunk_alloc(type_info_pt type_info, output_pt* output);
void chunk_free(type_info_pt type_info, output_pt output);
error_t chunk_copy(type_info_pt type_info, output_pt src, output_pt dest);

