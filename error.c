#include <stdio.h>
#include <stdarg.h>
#include "error.h"
#include "node.h"
#include "globals.h"

error_t global_error;

int raise_error(errorcode_t code, node_t * node, const char * message, ...){
    va_list argptr;
    va_start(argptr, message);

    global_error.code = code;
    global_error.node = node;
    vsnprintf(global_error.message, 127, message, argptr);

    va_end(argptr);
    return -1;
}
