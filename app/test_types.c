#include "noise.h"
#include <stdio.h>

const size_t nz_chunk_size = 128;

nz_rc run()
{
    nz_rc rc = NZ_SUCCESS;
    nz_type_p my_chunk_type;
    nz_obj_p my_chunk;
    char* string;

    // Create a chunk type
    if((rc = nz_chunk_typeclass.type_create(&my_chunk_type, NULL)) == NZ_SUCCESS)
    {
        // Use this chunk type to instantiate a chunk
        if((rc = nz_chunk_typeclass.type_create_obj(my_chunk_type, &my_chunk)) == NZ_SUCCESS)
        {
            if((rc = nz_chunk_typeclass.type_init_obj(my_chunk_type, my_chunk,
                "{0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 }"
                // "12345"
                )) == NZ_SUCCESS)
            {

                if((nz_chunk_typeclass.type_str_obj(my_chunk_type, my_chunk, &string)) == NZ_SUCCESS)
                {
                    printf("my_chunk value: %s\n", string);
                    //free(string);
                }
            }
            nz_chunk_typeclass.type_destroy_obj(my_chunk_type, my_chunk);
        }
        nz_chunk_typeclass.type_destroy(my_chunk_type);
    }
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

