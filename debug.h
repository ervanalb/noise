#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "block.h"
#include "typefns.h"

void debug_run(node_t * debug_block);
void debug_run_n(node_t * debug_block, int count);
void debug_print_graph(node_t * node);

#endif
