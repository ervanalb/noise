#ifndef __CORE_ERROR_H__
#define __CORE_ERROR_H__

#include <errno.h>
#include <assert.h>

// Random number to prevent collisions
#define NZ_ERRNOBASE 829410

int nz_errno(void);
const char * nz_strerror(int errnum);

#endif
