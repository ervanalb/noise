#ifndef __CORE_ERROR_H__
#define __CORE_ERROR_H__

// Put your errors in here:
#define DECLARE_ERRORS \
    DECLARE_ERROR(NZ_NOMEM) \

// Errors are passed through this return code object:
#define DECLARE_ERROR(X) X ,
typedef enum {
    NZ_SUCCESS = 0,
    DECLARE_ERRORS
} nz_rc;
#undef DECLARE_ERROR

extern const char * nz_error_string;
extern const char * nz_error_file;
extern int nz_error_line;

#define NZ_THROW_MSG(STR) {nz_error_file = __FILE__; nz_error_line = __LINE__; nz_error_string = (STR);}
#define NZ_THROW() NZ_THROW_MSG(NULL)

const char * nz_error_rc_str(nz_rc rc);

#endif
