#include <math.h>
#include <stdlib.h>

#include "noise.h"
#include "blocks/blocks.h"
#include "core/util.h"

#include "blocks/instrument.h"

struct state {
    struct nz_obj * note_states; // Vector
    struct nz_obj * notes; // Vector<notes>
    nz_instr_render_fpt render;
};

// Synth<> (vector<note> ns) -> (chunk )

static enum nz_pull_rc nz_instrument_pull(struct nz_port * port) {
    struct nz_node * node = port->port_node; 

    struct nz_obj * input0 = NZ_NODE_PULL(node, 0);

    struct state * state = (struct state *) node->node_state;

    if (input0 == NULL) {
        return NZ_PULL_RC_NULL;
    }

    double * output = &NZ_CAST(double, port->port_value);
    memset(output, 0, sizeof(double) * nz_chunk_size);

    struct nz_note * inp_notes = NZ_CAST(struct nz_note *, input0);
    struct nz_note * orig_st_notes = NZ_CAST(struct nz_note *, state->notes);
    size_t n_inp_notes = nz_vector_get_size(input0);
    size_t n_st_notes = nz_vector_get_size(state->notes);

    enum fset {
        NONE = 0,
        NEW,
        OLD,
        BOTH
    };

    enum fset note_sets[n_st_notes + n_inp_notes];
    memset(note_sets, 0, sizeof(note_sets));
    double chunk[nz_chunk_size];

    for (size_t i = 0; i < n_inp_notes; i++) {
        int found = 0;

        for (size_t j = 0; j < n_st_notes; j++) {
            if (note_sets[j] != NONE) continue;

            if (inp_notes[i].note_pitch == orig_st_notes[j].note_pitch) {
                found = 1;
                note_sets[j] = BOTH;
                void * render_state = nz_vector_at(state->note_states, j);
                int rc = state->render(render_state, &inp_notes[i], NZ_INSTR_NOTE_ON, chunk); // Existing
                if (rc == 0) {
                    for(size_t k = 0; k < nz_chunk_size; k++) {
                        output[k] += chunk[k];
                    }
                }
                //if (rc == 0)  TODO
            }
        }
        if (found == 0) {
            nz_vector_push_back(state->notes, &inp_notes[i]);
            orig_st_notes = NZ_CAST(struct nz_note *, state->notes);
            n_st_notes++;
            nz_vector_set_size(state->note_states, n_st_notes);
            note_sets[n_st_notes - 1] = NEW;
            void * render_state = nz_vector_at(state->note_states, n_st_notes - 1);
            memset(render_state, 0, nz_vector_sizeofel(state->note_states));
            int rc = state->render(render_state, &inp_notes[i], NZ_INSTR_NOTE_NEW, chunk); // New 
            printf("New note: pitch=%f, vel=%f\n", inp_notes[i].note_pitch, inp_notes[i].note_velocity);
            if (rc == 0) {
                for(size_t k = 0; k < nz_chunk_size; k++) {
                    output[k] += chunk[k];
                }
            }
            //if (rc == 0)  TODO
        }
    }

    for (size_t j = 0; j < n_st_notes; j++) {
        if (note_sets[j] == NONE) {
            note_sets[j] = OLD;
            void * render_state = nz_vector_at(state->note_states, j);
            int rc = state->render(render_state, &orig_st_notes[j], NZ_INSTR_NOTE_OFF, chunk); // Old
            if (rc == 0) {
                for(size_t k = 0; k < nz_chunk_size; k++) {
                    output[k] += chunk[k];
                }
            } else {
                // Remove  
                nz_vector_erase(state->notes, j);
                nz_vector_erase(state->note_states, j);
                j--;
                n_st_notes--;
            }
        }
    }

    return NZ_PULL_RC_OBJECT;
}

static void instrument_term(struct nz_node * node) { 
    struct state * state = (struct state *) node->node_state;
    nz_obj_destroy(&state->notes);
    nz_obj_destroy(&state->note_states);
    nz_node_free_ports(node);
}

int nz_instrument_init(struct nz_node * node, size_t state_size, nz_instr_render_fpt render) {
    int rc = nz_node_alloc_ports(node, 1, 1);
    if (rc != 0) return rc;

    node->node_term = &instrument_term;

    // Define inputs
    node->node_inputs[0] = (struct nz_inport) {
        .inport_type = nz_note_vector_type,
        .inport_name = strdup("notes"),
    };
    
    // Define outputs
    node->node_outputs[0] = (struct nz_port) {
        .port_node = node,
        .port_name = strdup("chunk"),
        .port_pull = &nz_instrument_pull,
        .port_type = nz_chunk_type,
        .port_value = nz_obj_create(nz_chunk_type),
    };

    // Initialize state
    node->node_state = calloc(1, sizeof(struct state));
    struct state * state = (struct state *) node->node_state;
    if (state == NULL) 
        return (nz_node_term(node), -1);

    state->note_states = nz_obj_create(nz_type_create_vector(state_size));
    state->notes = nz_obj_create(nz_note_vector_type);
    state->render = render;

    return 0;
}

void nz_oscbank_render(struct nz_osc * osc, size_t n_oscs, double * chunk) {
    memset(chunk, 0, sizeof(double) * nz_chunk_size);
    while(n_oscs--) {
        for (size_t i = 0; i < nz_chunk_size; i++) {
            osc->osc_phase += osc->osc_freq / nz_frame_rate;
            chunk[i] += sin(osc->osc_phase * 2 * M_PI);
        }
        osc->osc_phase = fmod(osc->osc_phase, 1.0);
        osc++;
    }
}
