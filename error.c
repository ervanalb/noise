#include "error.h"
#include "node.h"
#include "globals.h"

error_t global_error;

int raise_error(errorcode_t code, node_t * node, char * message){
    global_error.code = code;
    global_error.node = node;
    strncpy(global_error.message, message, 127);
    return -1;
}
