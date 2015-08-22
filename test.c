#include <stdlib.h>
#include <stdio.h>
#include "debug.h"
#include "block.h"

int main(void) {
    object_t * one = object_alloc(double_type);
    CAST_OBJECT(double, one) = 1.0;

    node_t * one_node = constant_create(one);
    node_t * acc = accumulator_create(NULL);
    node_t * debug = debug_create(NULL);

    connect(acc, 0, one_node, 0);
    connect(debug, 0, acc, 0);

    run_debug(debug);

    return 0;
}
