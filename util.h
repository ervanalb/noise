#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

static inline char * strdup(const char * s) {
    size_t len = strlen(s);
    char * r = malloc(len + 1);
    strcpy(r, s);
    return r;
}

static inline char * strndup(const char * s, size_t maxlen) {
    //size_t len = strnlen(s, maxlen);
    size_t len = 0;
    for(; s[len] && len < maxlen; len++);

    char * r = malloc(len + 1);
    memcpy(r, s, len);
    r[len] = '\0';
    return r;
}

static int asprintf(char ** buf, const char * fmt, ...) __attribute__ ((format (printf, 2, 3)));
static inline int asprintf(char ** buf, const char * fmt, ...) 
{
    va_list vargs;
    va_start(vargs, fmt);

    int len = vsnprintf(NULL, 0, fmt, vargs);
    *buf = malloc(len + 1);
    if(*buf == NULL)
        len = -1;
    else
        len = vsprintf(*buf, fmt, vargs);

    va_end(vargs);
    
    return len; 
}

static char * rsprintf(const char * fmt, ...) __attribute__ ((format (printf, 1, 2)));
static inline char * rsprintf(const char * fmt, ...)
{
    va_list vargs;
    va_start(vargs, fmt);

    int len = vsnprintf(NULL, 0, fmt, vargs);
    char * buf = malloc(len + 1);

    va_end(vargs);
    va_start(vargs, fmt);

    vsnprintf(buf, len, fmt, vargs);

    va_end(vargs);
    
    return buf; 
}

#endif
