#ifndef __CORE_GRAPH_H__
#define __CORE_GRAPH_H__

#include <stddef.h>

#include "noise.h"
#include "core/ntype.h"
#include "core/block.h"
#include "core/util.h"

struct nz_graph;

// --
// Public interface

nz_rc nz_graph_create(const struct nz_context * context_p, struct nz_graph ** graph_pp);
void nz_graph_destroy(struct nz_graph * graph_p);

nz_rc nz_graph_add_block(struct nz_graph * graph_p, const char * id, const char * block, struct nz_block ** block_pp);
nz_rc nz_graph_del_block(struct nz_graph * graph_p, const char * id);

nz_rc nz_graph_connect(struct nz_graph * graph_p, const char * id_upstream, const char * port_upstream, const char * id_downstream, const char * port_downstream);
nz_rc nz_graph_disconnect(struct nz_graph * graph_p, const char * id_upstream, const char * port_upstream, const char * id_downstream, const char * port_downstream);

#endif
