#include "libnoise.h"
#include "std.h"

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
    rc = nz_context_register_blockclass(context_p, &nz_lpf_blockclass); if(rc != NZ_SUCCESS) return rc;
    return NZ_SUCCESS;
}

nz_rc nz_types_init(struct nz_context * context_p) {
    nz_rc rc;
    rc = nz_context_register_typeclass(context_p, &nz_int_typeclass); if(rc != NZ_SUCCESS) return rc;
    rc = nz_context_register_typeclass(context_p, &nz_long_typeclass); if(rc != NZ_SUCCESS) return rc;
    rc = nz_context_register_typeclass(context_p, &nz_real_typeclass); if(rc != NZ_SUCCESS) return rc;
    rc = nz_context_register_typeclass(context_p, &nz_chunk_typeclass); if(rc != NZ_SUCCESS) return rc;
    rc = nz_context_register_typeclass(context_p, &nz_string_typeclass); if(rc != NZ_SUCCESS) return rc;
    rc = nz_context_register_typeclass(context_p, &nz_array_typeclass); if(rc != NZ_SUCCESS) return rc;
    rc = nz_context_register_typeclass(context_p, &nz_midiev_typeclass); if(rc != NZ_SUCCESS) return rc;
    return NZ_SUCCESS;
}
