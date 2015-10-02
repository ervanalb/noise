#include <stdlib.h>
#include <string.h>

#include "noise.h"
#include "blocks/io/blocks.h"
#include "blocks/io/midi.h"

struct state {
    struct nz_obj * notes;
};

static enum nz_pull_rc midiintegrator_pull(struct nz_port * port) {
    struct nz_node * node = port->port_node; 
    struct state * state = (struct state *) node->node_state;

    struct nz_obj * inp_midi = NZ_NODE_PULL(node, 0);

    if (inp_midi == NULL) {
        // Clear notes that are on
        nz_vector_set_size(state->notes, 0);
        nz_vector_set_size(port->port_value, 0);
        return NZ_PULL_RC_NULL;
    }

    size_t n_midievs = nz_vector_get_size(inp_midi);
    for (size_t i = 0; i < n_midievs; i++) {
        struct nz_midiev * ev = (struct nz_midiev *) nz_vector_at(inp_midi, i);
        switch( ev->midiev_status & 0xF0 ) {
            case 0x90: ; // Note on
                double pitch = nz_note_to_freq((double) ev->midiev_data1);
                double velocity = ((double) ev->midiev_data2) / 127.0;

                struct nz_note note;
                nz_note_init(&note, pitch, velocity);

                nz_vector_push_back(port->port_value, &note);
                nz_vector_push_back(state->notes, ev);
                break;
            case 0x80: ; // Note off
                size_t n_notes = nz_vector_get_size(state->notes);
                for (size_t j = 0; j < n_notes; j++) {
                    struct nz_midiev * note = (struct nz_midiev *) nz_vector_at(state->notes, j);
                    if (note->midiev_data1 == ev->midiev_data1 &&
                        note->midiev_data2 == ev->midiev_data2 &&
                        (note->midiev_status & 0xF) == (ev->midiev_status & 0xF)) {
                        nz_vector_erase(state->notes, j);
                        nz_vector_erase(port->port_value, j);
                        break;
                    }
                }
                break;
            default:
                break;
        }
    }

    assert(nz_vector_get_size(port->port_value) == nz_vector_get_size(state->notes));
    printf("note len: %ld\n", nz_vector_get_size(port->port_value));
    return NZ_PULL_RC_OBJECT;
}

int nz_midiintegrator_init(struct nz_node * node) {
    if (nz_midi_vector_type == NULL) {
        nz_midi_vector_type = nz_type_create_vector(sizeof(struct nz_midiev));
        if (nz_midi_vector_type == NULL) return -1;
    }

    int rc = nz_node_alloc_ports(node, 1, 1);
    if (rc != 0) return rc;

    node->node_term = &nz_node_term_generic; // FIXME: need to del midi vector
    node->node_name = strdup("Midi Integrator");

    // Define inputs
    node->node_inputs[0] = (struct nz_inport) {
        .inport_type = nz_midi_vector_type,
        .inport_name = strdup("midievs"),
    };
    
    // Define outputs
    node->node_outputs[0] = (struct nz_port) {
        .port_node = node,
        .port_name = strdup("out"),
        .port_pull = &midiintegrator_pull,
        .port_type = nz_note_vector_type,
        .port_value = nz_obj_create(nz_note_vector_type),
    };
    if (node->node_outputs[0].port_value == NULL) 
        return (nz_node_term(node), -1);

    // Initialize state
    node->node_state = calloc(1, sizeof(struct state));
    struct state * state = (struct state *) node->node_state;
    if (state == NULL) 
        return (nz_node_term(node), -1);

    state->notes = nz_obj_create(nz_midi_vector_type);
    if (state->notes == NULL) 
        return (nz_node_term(node), -1);
    
    return 0;
}
