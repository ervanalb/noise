#include <string.h>

#include "core/error.h"

#define DECLARE_ERROR(X) case X: return #X;
const char * nz_strerror(nz_rc rc)
{
    switch(errnum) {
        case NZ_SUCCESS: return "NZ_SUCCESS"
        DECLARE_ERRORS
        default: return "NZ_UNKNOWN";
    }
}
#undef DECLARE_ERROR
