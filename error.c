#include "error.h"
#include <string.h>

int noise_errno(void) {
    return errno;    
}

const char * noise_strerror(int errnum) {
    switch(errnum) {
        default: 
            return strerror(errnum);
    }
}
