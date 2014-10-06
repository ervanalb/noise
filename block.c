#include "block.h"

error_t object_alloc(object_info_t* object_info, object_state_t* object_state)
{
	object_state->type_info = object_info->type_info;
	object_state->copy_fn = object_info->copy_fn;
	return (*object_info->alloc_fn)(object_info->type_info,&object_state->object);
}

void object_free(object_info_t* object_info, object_state_t* object_state)
{
	(*object_info->alloc_fn)(object_info->type_info,object_state->object);
}

error_t object_copy(object_state_t* object_state, output_pt src)
{
	return (*(object_state->copy_fn))(object_state->type_info,object_state->object,src);
}


