#ifndef __CORE_ARGPARSE_H__
#define __CORE_ARGPARSE_H__

#include "core/error.h"
#include "core/util.h"

typedef void nz_arg;

// Parse args manually
nz_rc next_arg(const char * string, const char ** pos,
               const char ** key_start, size_t * key_length,
               const char ** value_start, size_t * value_length);

// Parse args according to a format string
nz_rc arg_parse(const char * fmt, const char * args, nz_arg ** arg_p_array);

#endif
