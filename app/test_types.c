#include "noise.h"
#include <stdio.h>

const size_t nz_chunk_size = 128;

nz_rc run()
{
    nz_rc rc = NZ_SUCCESS;
    nz_type_data_p my_int_type;
    nz_obj_p my_int;
    char* string;

    // Create an int type
    if((rc = nz_int_type_create(&my_int_type)) == NZ_SUCCESS)
    {
        // Use this int type to instantiate an int
        if((rc = nz_int_type.type_create_obj(my_int_type, &my_int)) == NZ_SUCCESS)
        {
            // Set the int
            *(int*)my_int = 5;

            if((nz_int_type.type_str_obj(my_int_type, my_int, &string)) == NZ_SUCCESS)
            {
                printf("my_int value: %s\n", string);
                free(string);
            }
            nz_int_type.type_destroy_obj(my_int_type, my_int);
        }
        nz_int_type_destroy(my_int_type);
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
        if(nz_error_string) fprintf(stderr, "%s\n", nz_error_string);
        return 1;
    }
    return 0;
}

