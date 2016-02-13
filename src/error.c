#include <string.h>
#include <stdlib.h>

#include "noise.h"

char * nz_error_string;
const char * nz_error_file;
int nz_error_line;

#define DECLARE_ERROR(X) case X : return #X ;
const char * nz_error_rc_str(nz_rc rc)
{
    switch(rc) {
        case NZ_SUCCESS: return "NZ_SUCCESS";
        DECLARE_ERRORS
        default: return "NZ_UNKNOWN";
    }
}
#undef DECLARE_ERROR
