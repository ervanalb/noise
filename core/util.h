#ifndef __CORE_UTIL_H__
#define __CORE_UTIL_H__

// Do not include any noise header files

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

//#include "core/error.h" // is this one ok?
#include "error.h" // is this one ok?

struct strbuf {
    size_t len;
    size_t capacity;
};

char * strbuf_alloc  (struct strbuf * buf);
char * strbuf_clear  (struct strbuf * buf, char * str);
char * strbuf_resize (struct strbuf * buf, char * str, size_t capacity);
char * strbuf_printf (struct strbuf * buf, char * str, const char * fmt, ...) __attribute__ ((format (printf, 3, 4)));
char * strbuf_putc   (struct strbuf * buf, char * str, char c);

// --
// String functions

#ifndef __USE_BSD
char * strdup(const char * s);
char * strndup(const char * s, size_t maxlen);
#endif

int asprintf(char ** buf, const char * fmt, ...) __attribute__ ((format (printf, 2, 3)));
char * rsprintf(const char * fmt, ...) __attribute__ ((format (printf, 1, 2)));

// --

double nz_note_to_freq(double note);

// --
// Comma-separated list parsing

nz_rc nz_next_type_arg(const char * string, const char ** pos, const char ** start, size_t * length);
nz_rc nz_next_list_arg(const char * string, const char ** pos, const char ** start, size_t * length);
nz_rc nz_next_block_arg(const char * string, const char ** pos, const char ** start, size_t * length);

#endif
