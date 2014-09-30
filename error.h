#ifndef __ERROR_H
#define __ERROR_H

#include "node.h"

typedef enum {
    ERR_NONE = 0,
    ERR_MALLOC,
} errorcode_t;

typedef struct {
    errorcode_t code;
    node_t * node;
    char message[128];
} error_t; 

int raise_error(errorcode_t code, node_t * node, const char * message, ...);

extern error_t global_error;

#endif
