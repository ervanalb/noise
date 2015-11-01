#include "noise.h"
#include <stdio.h>
#include <stdlib.h>

const size_t nz_chunk_size = 128;

nz_rc run()
{
    nz_rc rc = NZ_SUCCESS;
    struct nz_context * context;
    const struct nz_typeclass * my_typeclass;
    nz_type * my_type;
    nz_obj * my_obj;
    char * string;

    if((rc = nz_context_create(&context)) == NZ_SUCCESS)
    {
        // Create a type
        if((rc = nz_type_create(context, &my_typeclass, &my_type, "array<10,int>")) == NZ_SUCCESS)
        {
            // Instantiate it
            if((rc = my_typeclass->type_create_obj(my_type, &my_obj)) == NZ_SUCCESS)
            {
                if((my_typeclass->type_str_obj(my_type, my_obj, &string)) == NZ_SUCCESS)
                {
                    printf("my_obj default value: %s\n", string);
                    free(string);
                }
                if((rc = my_typeclass->type_init_obj(my_type, my_obj,
                    // "{0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 }"
                    "{1, 2, 3, 4, 5, 6, 7, 8, NULL, 10}"
                    )) == NZ_SUCCESS)
                {

                    if((my_typeclass->type_str_obj(my_type, my_obj, &string)) == NZ_SUCCESS)
                    {
                        printf("my_obj value: %s\n", string);
                        free(string);
                    }
                }
                my_typeclass->type_destroy_obj(my_type, my_obj);
            }
            my_typeclass->type_destroy(my_type);
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

