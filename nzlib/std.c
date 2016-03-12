#include "std.h"

const struct nz_typeclass const * nz_typeclass_p_array[] = {
    &nz_int_typeclass,
    &nz_long_typeclass,
    &nz_real_typeclass,
    &nz_chunk_typeclass,
    &nz_string_typeclass,
    &nz_array_typeclass,
    &nz_midiev_typeclass,
    NULL
};

const struct nz_blockclass const * nz_blockclass_p_array[] = {
    &nz_constant_blockclass,
    &nz_debug_blockclass,
    &nz_accumulator_blockclass,
    &nz_ruler_blockclass,
    &nz_tee_blockclass,
    &nz_wye_blockclass,
    &nz_sum_blockclass,
    &nz_diff_blockclass,
    &nz_mul_blockclass,
    &nz_div_blockclass,
    &nz_mod_blockclass,
    &nz_pa_blockclass,
    &nz_mixer_blockclass,
    &nz_wave_blockclass,
    &nz_midireader_blockclass,
    &nz_midimelody_blockclass,
    &nz_mididrums_blockclass,
    &nz_wavfileout_blockclass,
    &nz_compressor_blockclass,
    &nz_drum_blockclass,
    &nz_envelope_blockclass,
    &nz_lpf_blockclass,
    NULL
};
