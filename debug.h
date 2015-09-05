#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "noise.h"

void nz_debug_print_graph(struct nz_node * node);
void nz_debug_print_dot(struct nz_node * node, const char * filename);

#endif
