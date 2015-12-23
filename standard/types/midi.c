#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "noise.h"
#include "core/argparse.h"
#include "core/context.h"
#include "core/ntype.h"
#include "core/util.h"

#include "types/midi.h"

static nz_rc midiev_type_init_obj (const nz_type * type_p, nz_obj * obj_p, const char * string) {
    nz_arg * args[3];
    nz_rc rc = arg_parse("required int status, required int data1, required int data2", string, args);
    if(rc != NZ_SUCCESS) return rc;
    int status = *(int *)args[0];
    int data1 = *(int *)args[1];
    int data2 = *(int *)args[2];
    free(args[0]);
    free(args[1]);
    free(args[2]);

    struct nz_midiev * midiev_p  = (struct nz_midiev *) obj_p;
    midiev_p->midiev_status = status;
    midiev_p->midiev_data1 = data1;
    midiev_p->midiev_data2 = data2;

    return NZ_SUCCESS;
}

static nz_rc midiev_type_str_obj (const nz_type * type_p, const nz_obj * obj_p, char ** string) {
    struct nz_midiev * midiev_p  = (struct nz_midiev *) obj_p;

    *string = rsprintf("MIDI Event (%#02X, %#02X, %02X)", 
            midiev_p->midiev_status,
            midiev_p->midiev_data1,
            midiev_p->midiev_data2);

    if(*string == 0) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
    return NZ_SUCCESS;
}

GEN_SIMPLE_TYPE_FNS(midiev)
GEN_STATIC_OBJ_FNS(midiev, sizeof(struct nz_midiev))
GEN_SHALLOW_COPY_FN(midiev, sizeof(struct nz_midiev))
DECLARE_TYPECLASS(midiev)

