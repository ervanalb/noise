#include "error.h"
#include <string.h>

int nz_errno(void) {
    return errno;    
}

const char * nz_strerror(int errnum) {
    switch(errnum) {
        default: 
            return strerror(errnum);
    }
}
