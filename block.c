#include <string.h>
#include <stdlib.h>
#include "block.h"
#include "typefns.h"

void generic_block_destroy(node_t * node)
{
    object_free(node->state);
}
