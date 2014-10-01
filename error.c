#include <stdio.h>
#include <stdarg.h>
#include "error.h"
#include "node.h"

error_info_t global_error;

error_t raise_error(error_t type, const char * message, ...){
	va_list argptr;
	global_error.type = type;
	va_start(argptr, message);
	vsnprintf(global_error.message, sizeof(global_error.message)/sizeof(*(global_error.message)), message, argptr);
	va_end(argptr);
	return type;
}
