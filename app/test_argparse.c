#include "noise.h"
#include "core/argparse.h"

nz_rc run() {
    nz_rc result;
    nz_arg * arg_p_array[3];

    result = arg_parse("required int thing1, required real thing2, required generic thing3", "13, 14.5, saw", arg_p_array);
    if(result != NZ_SUCCESS) return result;

    printf("Decoded %ld %lf %s\n", *(long *)arg_p_array[0], *(double *)arg_p_array[1], (char *)arg_p_array[2]);

    for(size_t i = 0; i < 3; i++) {
        free(arg_p_array[i]);
    }

    return NZ_SUCCESS;
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
            nz_error_string_free();
        }
        return 1;
    }
    return 0;
}

