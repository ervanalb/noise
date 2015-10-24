#include <stdlib.h>
#include <string.h>

#include "noise.h"
#include "blocks/blocks.h"

struct state {
    struct nz_obj * in_tnotes;
    struct nz_obj * out_tnotes;
    double last_time;
};

int tnote_start_cmp(const void * a, const void * b) {
    double va = ((struct nz_tnote *) a)->tnote_start;
    double vb = ((struct nz_tnote *) b)->tnote_start;
    if (va == vb) return 0;
    if (va < vb) return -1;
    return 1;
}

static enum nz_pull_rc notesequencer_pull(struct nz_port * port) {
    struct nz_node * node = port->port_node; 
    struct state * state = (struct state *) node->node_state;

    struct nz_obj * inp_time = NZ_NODE_PULL(node, 0);
    double time = NZ_CAST(double, inp_time);

    if (state->in_tnotes == NULL) {
        state->in_tnotes = NZ_NODE_PULL(node, 1);
        if (state->in_tnotes == NULL) {
            nz_vector_set_size(state->out_tnotes, 0);
            nz_vector_set_size(port->port_value, 0);
            return NZ_PULL_RC_NULL;
        }
        printf("# notes: %lu\n", nz_vector_get_size(state->in_tnotes));
    }

    if (state->last_time == time) {
        return NZ_PULL_RC_OBJECT;
    } else if (state->last_time > time) {
        state->last_time = -1;
    }

    // Remove expired notes
    size_t n_out_tnotes = nz_vector_get_size(state->out_tnotes);
    for (size_t i = 0; i < n_out_tnotes; i++) {
        struct nz_tnote * tnote = (struct nz_tnote *) nz_vector_at(state->out_tnotes, i);
        double start_time = tnote->tnote_start;
        double end_time = tnote->tnote_start + tnote->tnote_duration;
        if (start_time < time || end_time >= time) {
            nz_vector_erase(port->port_value, i);
            nz_vector_erase(state->out_tnotes, i); 
            i--, n_out_tnotes--;
        }
    }
    
    // Add new notes
    size_t n_in_tnotes = nz_vector_get_size(state->in_tnotes);
    struct nz_tnote * in_tnotes = nz_vector_at(state->in_tnotes, 0);
    size_t i;
    for (i = 0; i < n_in_tnotes && in_tnotes[i].tnote_start <= state->last_time; i++);
    //struct nz_tnote key = {.tnote_start = state->last_time};
    //struct nz_tnote * new_tnote = bsearch(&key, (struct nz_tnote *) nz_vector_at(state->in_tnotes, 0), n_in_tnotes, sizeof(key), tnote_start_cmp);

    //printf("i = %lu; %f %f\n", i, time, in_tnotes[i].tnote_start);
    while (i < n_in_tnotes && in_tnotes[i].tnote_start <= time) {
        struct nz_note new_note;
        printf("Adding note!\n");
        nz_note_dup(&new_note, &in_tnotes[i].tnote_note);
        nz_vector_push_back(state->out_tnotes, &in_tnotes[i]);
        nz_vector_push_back(port->port_value, &new_note);
        i++;
    }

    state->last_time = time;

    assert(nz_vector_get_size(port->port_value) == nz_vector_get_size(state->out_tnotes));
    //printf("notes: %lu\n", nz_vector_get_size(port->port_value));
    return NZ_PULL_RC_OBJECT;
}

void notesequencer_term(struct nz_node * node) {
    struct state * state = (struct state *) node->node_state;
    nz_obj_destroy(&state->in_tnotes);
    nz_obj_destroy(&state->out_tnotes);
    nz_node_term_generic(node);
}

int nz_notesequencer_init(struct nz_node * node) {
    int rc = nz_node_alloc_ports(node, 2, 1);
    if (rc != 0) return rc;

    node->node_term = &notesequencer_term;
    node->node_name = strdup("Note Sequencer");

    // Define inputs
    node->node_inputs[0] = (struct nz_inport) {
        .inport_type = nz_double_type,
        .inport_name = strdup("time"),
    };
    node->node_inputs[1] = (struct nz_inport) {
        .inport_type = nz_tnote_vector_type,
        .inport_name = strdup("tnote vector"),
    };
    
    // Define outputs
    node->node_outputs[0] = (struct nz_port) {
        .port_node = node,
        .port_name = strdup("note out"),
        .port_pull = &notesequencer_pull,
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

    state->out_tnotes = nz_obj_create(nz_tnote_vector_type);
    if (state->out_tnotes == NULL) 
        return (nz_node_term(node), -1);

    state->last_time = -1;
    state->in_tnotes = NULL;
    
    return 0;
}
