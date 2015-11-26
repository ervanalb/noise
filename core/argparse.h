#ifndef __CORE_ARGPARSE_H__
#define __CORE_ARGPARSE_H__

#include "core/error.h"
#include "core/util.h"

typedef void nz_arg;

nz_rc arg_parse(const char * args, const char * fmt, nz_arg ** arg_p_array);

#endif
