#include <stdlib.h>
#include <stdio.h>
#include "debug.h"
#include "block.h"
#include "blockdef.h"

int main(void) {
    object_t * one_obj = object_alloc(double_type);
    CAST_OBJECT(double, one_obj) = 0.1;

    node_t * one = constant_create(one_obj);
    node_t * acc = accumulator_create(NULL);
    node_t * math = math_create(MATH_NOTE_TO_FREQ);
    node_t * fungen = fungen_create();
    node_t * tee = tee_create(double_type, 2);
    node_t * wye = tee_create(double_type, 2);
    node_t * debug = debug_create(NULL);

    connect(acc, 0, one, 0);
    connect(fungen, 0, acc, 0);
    //connect(debug, 0, fungen, 0);

    connect(math, 0, one, 0);
    connect(math, 1, one, 0);
    //connect(debug, 0, math, 0);

    connect(tee, 0, one, 0);
    connect(wye, 0, tee, 0);
    connect(wye, 1, tee, 1);
    connect(debug, 0, wye, 0);

    run_debug(debug);

    return 0;
}