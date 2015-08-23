#include <stdlib.h>
#include <stdio.h>

#include "debug.h"
#include "error.h"
#include "block.h"
#include "typefns.h"
#include "util.h"
#include "blockdef.h"

node_t * debug_create()
{
    node_t * node = node_alloc(1, 0, double_type);
    node->name = strdup("Debug printer");
    node->destroy = &node_destroy_generic;

    // Define inputs
    node->inputs[0] = (struct node_input) {
        .type = double_type,
        .name = strdup("debug input"),
    };
    
    return node;
}

void debug_run(node_t * debug_block) 
{
    char inp;
    do {
        object_t * obj;

        node_pull(debug_block, 0, &obj);
        printf(" > %f\n", CAST_OBJECT(double, obj));
    } while(scanf("%c", &inp), inp != 'q');
}

void debug_run_n(node_t * debug_block, int count)
{
    for (int i = 0; i < count; i++) {
        object_t * obj;

        node_pull(debug_block, 0, &obj);
        printf(" %4d/%4d > %f\n", i, count, CAST_OBJECT(double, obj));
    }
}

void debug_print_graph(node_t * node)
{
    printf("%p %s:\n", node, node->name);

    char * state_value = object_str(node->state);
    printf("    [s] %s\n", state_value);
    free(state_value);

    for (size_t i = 0; i < node->n_inputs; i++) {
        if (node->inputs[i].connected_input != NULL) {
            printf("    [i] '%s' %s.'%s' (%p)\n",
                    node->inputs[i].name,
                    node->inputs[i].connected_input->node->name,
                    node->inputs[i].connected_input->name,
                    node->inputs[i].connected_input);
        } else {
            printf("    [i] '%s' --\n", node->inputs[i].name);
        }
    }

    for (size_t i = 0; i < node->n_outputs; i++) {
        printf("    [o] %s\n", node->outputs[i].name);
    }
    //printf("    - inputs (%lu):\n", node->n_inputs);
    //printf("    - outputs (%lu):\n", node->n_outputs);
    
    for (size_t i = 0; i < node->n_inputs; i++) {
        debug_print_graph(node->inputs[i].connected_input->node);
    }

    for (size_t i = 0; i < node->n_outputs; i++) {
        //debug_print_graph(node->outputs[i].node);
    }
}
