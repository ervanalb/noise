#include "util.h"

// --
// strbuf

static const size_t strbuf_init_capacity = 16;

char * strbuf_alloc(struct strbuf * buf)
{
    char * str = malloc(strbuf_init_capacity);
    buf->capacity = strbuf_init_capacity;
    str = strbuf_clear(buf, str);
    return str;
}

char * strbuf_clear(struct strbuf * buf, char * str)
{
    if(!str) return NULL;

    *str = '\0';
    buf->len = 1;
    return str;
}

char * strbuf_grow(struct strbuf * buf, char * str, size_t capacity)
{
    if(!str) return NULL;

    size_t c = buf->capacity;
    while(c < capacity) c *= 2;
    buf->capacity = c;
    char * new_str = realloc(str, c);
    if(!new_str)
    {
        free(str);
        return NULL;
    }
    return new_str;
}

char * strbuf_shrink(struct strbuf * buf, char * str, size_t capacity)
{
    if(!str) return NULL;

    size_t c = buf->capacity;
    size_t last_c;
    while(c > capacity)
    {
        last_c = c;
        c /= 2;
        if(c < strbuf_init_capacity)
        {
            last_c = strbuf_init_capacity;
            break;
        }
    }
    buf->capacity = last_c;
    char * new_buf = realloc(str, last_c);
    if(!new_buf)
    {
        free(str);
        return NULL;
    }
    return new_buf;
}

char * strbuf_printf(struct strbuf * buf, char * str, const char * fmt, ...)
{
    if(!str) return NULL;

    va_list vargs;

    va_start(vargs, fmt);
    int addlen = vsnprintf(NULL, 0, fmt, vargs);
    va_end(vargs);

    if(addlen < 0)
    {
        free(str);
        return NULL;
    }

    if(buf->len + addlen > buf->capacity)
    {
        str = strbuf_grow(buf, str, buf->len + addlen);
        if(!str) return NULL;
    }

    va_start(vargs, fmt);
    addlen = vsnprintf(str + buf->len - 1, addlen + 1, fmt, vargs);
    va_end(vargs);

    buf->len += addlen;

    return str;
}

// --

double nz_note_to_freq(double note) {
    return pow(2,(note-69)/12)*440;
}

#ifndef __USE_BSD
char * strdup(const char * s) {
    size_t len = strlen(s);
    char * r = malloc(len + 1);
    strcpy(r, s);
    return r;
}

char * strndup(const char * s, size_t maxlen) {
    //size_t len = strnlen(s, maxlen);
    size_t len = 0;
    for(; s[len] && len < maxlen; len++);

    char * r = malloc(len + 1);
    memcpy(r, s, len);
    r[len] = '\0';
    return r;
}
#endif

int asprintf(char ** buf, const char * fmt, ...) 
{
    va_list vargs;
    va_start(vargs, fmt);

    int len = vsnprintf(NULL, 0, fmt, vargs) + 1;
    *buf = malloc(len);

    va_end(vargs);
    va_start(vargs, fmt);

    if(*buf == NULL)
        len = -1;
    else
        len = vsnprintf(*buf, len, fmt, vargs);

    va_end(vargs);
    
    return len; 
}

char * rsprintf(const char * fmt, ...)
{
    va_list vargs;
    va_start(vargs, fmt);

    int len = vsnprintf(NULL, 0, fmt, vargs) + 1;
    char * buf = malloc(len);
    if(!buf) return 0;

    va_end(vargs);
    va_start(vargs, fmt);

    vsnprintf(buf, len, fmt, vargs);

    va_end(vargs);
    
    return buf; 
}


