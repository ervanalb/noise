#include "noise.h"
#include <stdio.h>

const size_t nz_chunk_size = 128;

nz_rc run()
{
    nz_rc rc = NZ_SUCCESS;
    const struct nz_typeclass * my_int_typeclass;
    nz_type_p my_int_type;
    nz_obj_p my_int;
    char* string;

    if((rc = nz_init_type_system()) == NZ_SUCCESS)
    {
        if((rc = nz_init_types()) == NZ_SUCCESS)
        {
            // Create a int type
            if((rc = nz_type_create(&my_int_typeclass, &my_int_type, "int<a>")) == NZ_SUCCESS)
            {
                // Use this int type to instantiate a int
                if((rc = nz_int_typeclass.type_create_obj(my_int_type, &my_int)) == NZ_SUCCESS)
                {
                    if((rc = nz_int_typeclass.type_init_obj(my_int_type, my_int,
                        // "{0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 }"
                        "12345"
                        )) == NZ_SUCCESS)
                    {

                        if((nz_int_typeclass.type_str_obj(my_int_type, my_int, &string)) == NZ_SUCCESS)
                        {
                            printf("my_int value: %s\n", string);
                            //free(string);
                        }
                    }
                    nz_int_typeclass.type_destroy_obj(my_int_type, my_int);
                }
                nz_int_typeclass.type_destroy(my_int_type);
            }
        }
        nz_deinit_type_system();
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

