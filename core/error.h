#ifndef __CORE_ERROR_H__
#define __CORE_ERROR_H__

// Put your errors in here:
#define DECLARE_ERRORS \
    DECLARE_ERROR(NZ_NOT_IMPLEMENTED) \
    DECLARE_ERROR(NZ_NOT_ENOUGH_MEMORY) \
    DECLARE_ERROR(NZ_TYPE_NOT_FOUND) \
    DECLARE_ERROR(NZ_UNEXPECTED_TYPE_ARGS) \
    DECLARE_ERROR(NZ_EXPECTED_TYPE_ARGS) \
    DECLARE_ERROR(NZ_TYPE_ARG_PARSE) \
    DECLARE_ERROR(NZ_OBJ_ARG_PARSE) \
    DECLARE_ERROR(NZ_OBJ_ARG_VALUE) \

// Errors are passed through this return code object:
#define DECLARE_ERROR(X) X ,
typedef enum {
    NZ_SUCCESS = 0,
    DECLARE_ERRORS
} nz_rc;
#undef DECLARE_ERROR

extern char * nz_error_string;
extern const char * nz_error_file;
extern int nz_error_line;

#define NZ_ERR_MSG(STR) {nz_error_file = __FILE__; nz_error_line = __LINE__; nz_error_string = STR;}
#define NZ_ERR() NZ_ERR_MSG(NULL)
#define NZ_RETURN_ERR(ERR) {NZ_ERR(); return (ERR);}
#define NZ_RETURN_ERR_MSG(ERR, STR) {NZ_ERR_MSG(STR); return (ERR);}

const char * nz_error_rc_str(nz_rc rc);

#endif

