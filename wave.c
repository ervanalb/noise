#include <stdlib.h>
#include "error.h"
#include "block.h"
#include "blockdef.h"
#include "globals.h"
#include "util.h"
#include "math.h"

static type_t * wave_state_type = NULL;

static double sine(double t)
{
	return sin(t*2*M_PI);
}

static double saw(double t)
{
	return 2*fmod(t,1)-1;
}

static double square(double t)
{
	return 2*(fmod(t,1)<.5)-1;
}

static error_t wave_pull(node_t * node, object_t ** output)
{
    error_t e = SUCCESS;
    object_t * inp_freq = NULL;
    e |= node_pull(node, 0, &inp_freq);

    object_t * inp_wave = NULL;
    e |= node_pull(node, 0, &inp_wave);

    //TODO - null behavior
    if (inp_freq == NULL || inp_wave == NULL) {
        *output = NULL;
        return e;
    }

    *output = (&CAST_OBJECT(object_t *, node->state))[1];

    double freq = CAST_OBJECT(double, inp_freq);
    enum wave_type wave = CAST_OBJECT(long, inp_wave);
    double * t = &CAST_TUPLE(double, 0, node->state);
    double * chunk = &CAST_TUPLE(double, 1, node->state);

	for (size_t i=0; i < global_chunk_size; i++) {
        switch(wave) {
        case WAVE_SINE:
            chunk[i] = sine(*t);
            break;
        case WAVE_SAW:
            chunk[i] = saw(*t);
            break;
        case WAVE_SQUARE:
            chunk[i] = square(*t);
            break;
        }

        *t += freq / global_frame_rate;
	}

    return e;
}

node_t * wave_create()
{
    if (wave_state_type == NULL)
        wave_state_type = make_tuple_type(2);
    if (wave_state_type == NULL) return NULL;

    type_t * chunk_type = get_chunk_type();
    node_t * node = node_alloc(2, 1, wave_state_type);
    node->name = strdup("Wave");
    node->destroy = &node_destroy_generic;

    // Define inputs
    node->inputs[0] = (struct node_input) {
        .type = double_type,
        .name = strdup("freq"),
    };
    node->inputs[1] = (struct node_input) {
        .type = long_type,
        .name = strdup("type"),
    };
    
    // Define outputs
    node->outputs[0] = (struct endpoint) {
        .node = node,
        .pull = &wave_pull,
        .type = chunk_type,
        .name = strdup("chunk"),
    };

    // Setup state
    object_t ** tuple = &CAST_OBJECT(object_t *, node->state);
    tuple[0] = object_alloc(double_type);
    tuple[1] = object_alloc(chunk_type);

    return node;
}
