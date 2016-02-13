#include "blocks/blocks.h"
#include "core/context.h"

nz_rc nz_blocks_init(struct nz_context * context_p) {
    nz_rc rc;
    rc = nz_context_register_blockclass(context_p, &nz_constant_blockclass); if(rc != NZ_SUCCESS) return rc;
    rc = nz_context_register_blockclass(context_p, &nz_debug_blockclass); if(rc != NZ_SUCCESS) return rc;
    rc = nz_context_register_blockclass(context_p, &nz_accumulator_blockclass); if(rc != NZ_SUCCESS) return rc;
    rc = nz_context_register_blockclass(context_p, &nz_tee_blockclass); if(rc != NZ_SUCCESS) return rc;
    rc = nz_context_register_blockclass(context_p, &nz_sum_blockclass); if(rc != NZ_SUCCESS) return rc;
    rc = nz_context_register_blockclass(context_p, &nz_diff_blockclass); if(rc != NZ_SUCCESS) return rc;
    rc = nz_context_register_blockclass(context_p, &nz_mul_blockclass); if(rc != NZ_SUCCESS) return rc;
    rc = nz_context_register_blockclass(context_p, &nz_div_blockclass); if(rc != NZ_SUCCESS) return rc;
    rc = nz_context_register_blockclass(context_p, &nz_mod_blockclass); if(rc != NZ_SUCCESS) return rc;
    rc = nz_context_register_blockclass(context_p, &nz_pa_blockclass); if(rc != NZ_SUCCESS) return rc;
    rc = nz_context_register_blockclass(context_p, &nz_mixer_blockclass); if(rc != NZ_SUCCESS) return rc;
    rc = nz_context_register_blockclass(context_p, &nz_wave_blockclass); if(rc != NZ_SUCCESS) return rc;
    rc = nz_context_register_blockclass(context_p, &nz_midireader_blockclass); if(rc != NZ_SUCCESS) return rc;
    rc = nz_context_register_blockclass(context_p, &nz_midimelody_blockclass); if(rc != NZ_SUCCESS) return rc;
    rc = nz_context_register_blockclass(context_p, &nz_mididrums_blockclass); if(rc != NZ_SUCCESS) return rc;
    rc = nz_context_register_blockclass(context_p, &nz_wavfileout_blockclass); if(rc != NZ_SUCCESS) return rc;
    rc = nz_context_register_blockclass(context_p, &nz_envelope_blockclass); if(rc != NZ_SUCCESS) return rc;
    rc = nz_context_register_blockclass(context_p, &nz_drum_blockclass); if(rc != NZ_SUCCESS) return rc;
    rc = nz_context_register_blockclass(context_p, &nz_lpf_blockclass); if(rc != NZ_SUCCESS) return rc;
    return NZ_SUCCESS;
}
