#include <stdlib.h>
#include <stdio.h>

#include "debug.h"
#include "error.h"
#include "block.h"
#include "typefns.h"
#include "blockdef.h"

node_t * debug_create()
{
    node_t * node = allocate_node(1, 0, double_type);
    node->destroy = &generic_block_destroy;

    // Define inputs
    node->inputs[0] = (struct node_input) {
        .type = double_type,
        .name = "debug input",
    };
    
    return node;
}

void run_debug(node_t * debug_block) 
{
    char inp;
    do {
        object_t * obj;

        pull(debug_block, 0, &obj);
        printf(" > %f", CAST_OBJECT(double, obj));
    } while(scanf("%c", &inp), inp != 'q');
}

