#ifndef __BLOCKS_BLOCKS_H__
#define __BLOCKS_BLOCKS_H__

#include "core/block.h"

nz_rc nz_blocks_init(struct nz_context * context_p);

const struct nz_blockclass nz_constant_blockclass;
const struct nz_blockclass nz_debug_blockclass;
const struct nz_blockclass nz_accumulator_blockclass;
const struct nz_blockclass nz_tee_blockclass;
const struct nz_blockclass nz_wye_blockclass;
const struct nz_blockclass nz_sum_blockclass;
const struct nz_blockclass nz_diff_blockclass;
const struct nz_blockclass nz_mul_blockclass;
const struct nz_blockclass nz_div_blockclass;
const struct nz_blockclass nz_pa_blockclass;

// --

nz_rc pa_start(struct nz_block *);

#endif
