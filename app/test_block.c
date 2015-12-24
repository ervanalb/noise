#include <stdio.h>
#include <stdlib.h>

#include "libnoise.h"

nz_rc run()
{
    nz_rc rc = NZ_SUCCESS;
    struct nz_context * context;
    const struct nz_blockclass * my_blockclass;
    struct nz_block my_block;
    struct nz_block_info my_block_info;
    int a;
    nz_obj * pull_result;
    char * string;

    if((rc = nz_context_create(&context)) == NZ_SUCCESS)
    {
        // Create a block
        if((rc = nz_context_create_block(context, &my_blockclass, &my_block.block_state_p, &my_block_info, "constant(int,10)")) == NZ_SUCCESS)
        {
            pull_result = my_block_info.block_pull_fns[0](my_block, 0, (nz_obj*)&a);
            if(pull_result == NULL) {
                printf("pulled NULL\n");
            } else {
                printf("pulled %d\n", a);
            }
            my_blockclass->block_destroy(my_block.block_state_p, &my_block_info);
        }
    }
    nz_context_destroy(context);
    return rc;
}

int main()
{
    nz_rc rc = run();
    if(rc != NZ_SUCCESS)
    {
        fprintf(stderr, "Noise error %d: %s\n", rc, nz_error_rc_str(rc));
        fprintf(stderr, "File: %s line %d\n", nz_error_file, nz_error_line);
        if(nz_error_string) {
            fprintf(stderr, "%s\n", nz_error_string);
            free(nz_error_string);
        }
        return 1;
    }
    return 0;
}

