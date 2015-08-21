#ifndef __ERROR_H
#define __ERROR_H

typedef enum error_t {SUCCESS = 0, ERR_MALLOC = 1, ERR_INVALID = 2} error_t;

typedef struct error_info_t {
	error_t type;
	char message[1024];
} error_info_t; 

error_t raise_error(error_t type, const char * message, ...);

extern error_info_t global_error;

#endif
