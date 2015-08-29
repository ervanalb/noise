#ifndef __ERROR_H__
#define __ERROR_H__

#include <errno.h>
#include <assert.h>

// Random number to prevent collisions
#define NOISE_ERRNOBASE 829410

int noise_errno(void);
const char * noise_strerror(int errnum);

#endif
