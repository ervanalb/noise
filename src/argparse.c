#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "libnoise.h"

const char opening_group_symbols[] = {'(', '[', '{', '<'};
const char closing_group_symbols[] = {')', ']', '}', '>'};

static char is_opening(char c) {
    for(size_t i = 0; i < sizeof(opening_group_symbols) / sizeof(char); i++) {
        if(c == opening_group_symbols[i]) return closing_group_symbols[i];
    }
    return '\0';
}

static char is_closing(char c) {
    for(size_t i = 0; i < sizeof(closing_group_symbols) / sizeof(char); i++) {
        if(c == closing_group_symbols[i]) return opening_group_symbols[i];
    }
    return '\0';
}

nz_rc next_arg(const char * string, const char ** pos,
               const char ** key_start, size_t * key_length,
               const char ** value_start, size_t * value_length) {

    struct strbuf open_groups;
    char * open_groups_str = strbuf_alloc(&open_groups);

    size_t trailing_whitespace = 0;
    *key_start = NULL;
    *key_length = 0;

    const char * definitely_value = NULL;

    enum {
        SKIP_STARTING_WHITESPACE,
        KEY_OR_VALUE,
        IN_STRING,
        IN_STRING_ESCAPE,
    } state = SKIP_STARTING_WHITESPACE;

    for(;;) {
        char c;
        switch(state) {
            case SKIP_STARTING_WHITESPACE:
                switch(**pos) {
                    case ' ':
                        break;
                    case '"':
                        definitely_value = *pos;
                        *value_start = *pos;
                        state = IN_STRING;
                        break;
                    case '=':
                        // = not allowed as first non-whitespace character
                        free(open_groups_str);
                        NZ_RETURN_ERR_MSG(NZ_ARG_PARSE, rsprintf("%lu", *pos - string));
                        break;
                    case ',':
                        // Empty argument
                        free(open_groups_str);
                        NZ_RETURN_ERR_MSG(NZ_ARG_PARSE, rsprintf("%lu", *pos - string));
                    case '\0':
                        // Empty argument
                        free(open_groups_str);
                        NZ_RETURN_ERR_MSG(NZ_ARG_PARSE, rsprintf("%lu", *pos - string));
                    default:
                        if(is_opening(**pos) != '\0') {
                            *value_start = *pos;
                            open_groups_str = strbuf_putc(&open_groups, open_groups_str, **pos);
                            if(open_groups_str == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
                            definitely_value = *pos;
                            state = KEY_OR_VALUE;
                            break;
                        } else if(is_closing(**pos) != '\0') {
                            // Unmatched closing
                            free(open_groups_str);
                            NZ_RETURN_ERR_MSG(NZ_ARG_PARSE, rsprintf("%lu", *pos - string));
                        }
                        *value_start = *pos;
                        state = KEY_OR_VALUE;
                        break;
                }
                break;
            case KEY_OR_VALUE:
                switch(**pos) {
                    case '"':
                        definitely_value = *pos;
                        state = IN_STRING;
                        break;
                    case ',':
                        if(open_groups.len == 1) {
                            *value_length = *pos - *value_start - trailing_whitespace;
                            (*pos)++;
                            goto success;
                        }
                        break;
                    case '\0':
                        if(open_groups.len > 1) {
                            // Unmatched open
                            free(open_groups_str);
                            NZ_RETURN_ERR_MSG(NZ_ARG_PARSE, rsprintf("%lu", *pos - string));
                        }
                        *value_length = *pos - *value_start - trailing_whitespace;
                        *pos = NULL;
                        goto success;
                    case '=':
                        if(open_groups.len == 1) {
                            if(definitely_value != NULL) {
                                // Bad character
                                free(open_groups_str);
                                NZ_RETURN_ERR_MSG(NZ_ARG_PARSE, rsprintf("%lu", definitely_value - string));
                            }
                            *key_start = *value_start;
                            *key_length = *pos - *value_start - trailing_whitespace;
                            trailing_whitespace = 0;
                            state = SKIP_STARTING_WHITESPACE;
                        }
                        break;
                    default:
                        if(is_opening(**pos) != '\0') {
                            open_groups_str = strbuf_putc(&open_groups, open_groups_str, **pos);
                            if(open_groups_str == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
                            definitely_value = *pos;
                            break;
                        } else if((c = is_closing(**pos)) != '\0') {
                            if(open_groups.len == 1) {
                                // Unmatched close
                                free(open_groups_str);
                                NZ_RETURN_ERR_MSG(NZ_ARG_PARSE, rsprintf("%lu", *pos - string));
                            }
                            if(open_groups_str[open_groups.len - 2] != c) {
                                // Mismatched grouping
                                free(open_groups_str);
                                NZ_RETURN_ERR_MSG(NZ_ARG_PARSE, rsprintf("%lu", *pos - string));
                            }
                            open_groups_str[open_groups.len - 2] = '\0';
                            open_groups.len--;
                            break;
                        }
                }
                break;
            case IN_STRING:
                switch(**pos) {
                    case '"':
                        state = KEY_OR_VALUE;
                        break;
                    case '\\':
                        state = IN_STRING_ESCAPE;
                        break;
                    case '\0':
                        if(definitely_value != NULL) {
                            // Unmatched quote
                            free(open_groups_str);
                            NZ_RETURN_ERR_MSG(NZ_ARG_PARSE, rsprintf("%lu", *pos - string));
                        }
                }
                break;
            case IN_STRING_ESCAPE:
                switch(**pos) {
                    case '\0':
                        if(definitely_value != NULL) {
                            // Unmatched quote
                            free(open_groups_str);
                            NZ_RETURN_ERR_MSG(NZ_ARG_PARSE, rsprintf("%lu", *pos - string));
                        }
                    default:
                        state = IN_STRING;
                        break;
                }
                break;
        }

        if(**pos == ' ') {
            trailing_whitespace++;
        } else {
            trailing_whitespace = 0;
        }
        (*pos)++;
    }
    success:
    free(open_groups_str);
    return NZ_SUCCESS;
}

static nz_rc next_tok(const char * string, const char ** pos,
                      const char ** tok_start, size_t * tok_length) {

    *tok_length = 0;
    while(**pos == ' ') (*pos)++;
    if(**pos == '\0') NZ_RETURN_ERR(NZ_ARG_PARSE);
    *tok_start = *pos;
    while(**pos != ' ' && **pos != '\0') {
        (*tok_length)++;
        (*pos)++;
    }
    while(**pos == ' ') (*pos)++;
    if(**pos == '\0') *pos = NULL;

    return NZ_SUCCESS;
}

enum arg_spec_type {GENERIC, STRING, INT, REAL};
struct arg_spec {
    char * name;
    enum arg_spec_type type;
    int required;
};

static nz_rc arg_parse_fmt(const char * fmt, struct arg_spec ** arg_spec_array_p, size_t * arg_spec_array_length_p) {
    const char * pos = fmt;
    const char * key_start;
    size_t key_length;
    const char * value_start;
    size_t value_length;
    nz_rc result;

    size_t arg_spec_array_capacity = 8;
    *arg_spec_array_p = calloc(arg_spec_array_capacity, sizeof(struct arg_spec));
    if(*arg_spec_array_p == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
    *arg_spec_array_length_p = 0;
 
    while(pos) {
        char * arg_spec_str;
        const char * spec_pos;
        const char * tok_start;
        size_t tok_length;

        result = next_arg(fmt, &pos,
                          &key_start, &key_length,
                          &value_start, &value_length);

        if(result != NZ_SUCCESS || key_start != NULL) {
            for(size_t i = 0; i < *arg_spec_array_length_p; i++) free((*arg_spec_array_p)[i].name);
            free(*arg_spec_array_p);
            return NZ_INTERNAL_ERROR;
        }

        arg_spec_str = strndup(value_start, value_length);

        if(arg_spec_str == NULL) {
            for(size_t i = 0; i < *arg_spec_array_length_p; i++) free((*arg_spec_array_p)[i].name);
            free(*arg_spec_array_p);
            NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
        }

        (*arg_spec_array_length_p)++;
        if(*arg_spec_array_length_p > arg_spec_array_capacity) {
            arg_spec_array_capacity *= 2;
            struct arg_spec * new_arg_spec_array_p = realloc(*arg_spec_array_p, sizeof(struct arg_spec) * arg_spec_array_capacity);
            if(new_arg_spec_array_p == NULL) {
                free(arg_spec_str);
                for(size_t i = 0; i < *arg_spec_array_length_p - 1; i++) free((*arg_spec_array_p)[i].name);
                free(*arg_spec_array_p);
                NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
            }
            *arg_spec_array_p = new_arg_spec_array_p;
        }

        spec_pos = arg_spec_str;
        result = next_tok(arg_spec_str, &spec_pos,
                          &tok_start, &tok_length);
        if(result != NZ_SUCCESS) {
            free(arg_spec_str);
            for(size_t i = 0; i < *arg_spec_array_length_p - 1; i++) free((*arg_spec_array_p)[i].name);
            free(*arg_spec_array_p);
            return NZ_INTERNAL_ERROR;
        }
        if(tok_length == 8 && memcmp(tok_start, "required", 8) == 0) {
            (*arg_spec_array_p)[*arg_spec_array_length_p - 1].required = 1;
            result = next_tok(arg_spec_str, &spec_pos,
                              &tok_start, &tok_length);
            if(result != NZ_SUCCESS) {
                free(arg_spec_str);
                for(size_t i = 0; i < *arg_spec_array_length_p - 1; i++) free((*arg_spec_array_p)[i].name);
                free(*arg_spec_array_p);
                return NZ_INTERNAL_ERROR;
            }
            if(spec_pos == NULL) {
                free(arg_spec_str);
                for(size_t i = 0; i < *arg_spec_array_length_p - 1; i++) free((*arg_spec_array_p)[i].name);
                free(*arg_spec_array_p);
                NZ_RETURN_ERR(NZ_INTERNAL_ERROR);
            }
        } else {
            (*arg_spec_array_p)[*arg_spec_array_length_p - 1].required = 0;
        }
        if(tok_length == 7 && memcmp(tok_start, "generic", 7) == 0) {
            (*arg_spec_array_p)[*arg_spec_array_length_p - 1].type = GENERIC;
        } else if(tok_length == 6 && memcmp(tok_start, "string", 6) == 0) {
            (*arg_spec_array_p)[*arg_spec_array_length_p - 1].type = STRING;
        } else if(tok_length == 3 && memcmp(tok_start, "int", 3) == 0) {
            (*arg_spec_array_p)[*arg_spec_array_length_p - 1].type = INT;
        } else if(tok_length == 4 && memcmp(tok_start, "real", 4) == 0) {
            (*arg_spec_array_p)[*arg_spec_array_length_p - 1].type = REAL;
        } else {
            free(arg_spec_str);
            for(size_t i = 0; i < *arg_spec_array_length_p - 1; i++) free((*arg_spec_array_p)[i].name);
            free(*arg_spec_array_p);
            NZ_RETURN_ERR(NZ_INTERNAL_ERROR);
        }
        result = next_tok(arg_spec_str, &spec_pos,
                          &tok_start, &tok_length);
        if(result != NZ_SUCCESS) {
            free(arg_spec_str);
            for(size_t i = 0; i < *arg_spec_array_length_p - 1; i++) free((*arg_spec_array_p)[i].name);
            free(*arg_spec_array_p);
            return NZ_INTERNAL_ERROR;
        }
        if(spec_pos != NULL) {
            free(arg_spec_str);
            for(size_t i = 0; i < *arg_spec_array_length_p - 1; i++) free((*arg_spec_array_p)[i].name);
            free(*arg_spec_array_p);
            NZ_RETURN_ERR(NZ_INTERNAL_ERROR);
        }
        (*arg_spec_array_p)[*arg_spec_array_length_p - 1].name = strndup(tok_start, tok_length);
        if((*arg_spec_array_p)[*arg_spec_array_length_p - 1].name == NULL) {
            free(arg_spec_str);
            for(size_t i = 0; i < *arg_spec_array_length_p - 1; i++) free((*arg_spec_array_p)[i].name);
            free(*arg_spec_array_p);
            NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
        }
        free(arg_spec_str);
    }

    return NZ_SUCCESS;
}

static nz_rc parse_one_arg(enum arg_spec_type type, const char * value_start, size_t value_length, nz_arg ** arg_pp) {
    char * value = strndup(value_start, value_length);
    if(value == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    int n;
    int result;

    switch(type) {
        case GENERIC:
            *arg_pp = value;
            break;
        case STRING:
            // TODO
            *arg_pp = value;
            //free(value);
            //NZ_RETURN_ERR_MSG(NZ_NOT_IMPLEMENTED, strdup("lol you want string! need to write some code first!"));
            break;
        case INT:
            *arg_pp = calloc(1, sizeof(long));
            if(*arg_pp == NULL) {
                free(value);
                NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
            }
            result = sscanf(value, "%ld%n", (long *)*arg_pp, &n);
            if(result != 1 || n < 0 || value[n] != '\0') {
                char * v = strdup(value);
                free(value);
                free(*arg_pp);
                NZ_RETURN_ERR_MSG(NZ_ARG_VALUE, v);
            }
            free(value);
            break;
        case REAL:
            *arg_pp = calloc(1, sizeof(double));
            if(*arg_pp == NULL) {
                free(value);
                NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
            }
            result = sscanf(value, "%lf%n", (double *)*arg_pp, &n);
            if(result != 1 || n < 0 || value[n] != '\0') {
                char * v = strdup(value);
                free(value);
                free(*arg_pp);
                NZ_RETURN_ERR_MSG(NZ_ARG_VALUE, v);
            }
            free(value);
            break;
    }
    return NZ_SUCCESS;
}

nz_rc arg_parse(const char * fmt, const char * args, nz_arg ** arg_p_array) {
    if(fmt == NULL) {
        if(args == NULL) return NZ_SUCCESS;
        NZ_RETURN_ERR_MSG(NZ_UNEXPECTED_ARGS, strdup(args));
    }

    struct arg_spec * arg_spec_array;
    size_t arg_spec_array_length;
    nz_rc result;

    const char * pos = args;
    const char * key_start;
    size_t key_length;
    const char * value_start;
    size_t value_length;
    size_t pargs = 0;
    int kwargs = 0;

    result = arg_parse_fmt(fmt, &arg_spec_array, &arg_spec_array_length);
    if(result != NZ_SUCCESS) return result;

    for(size_t i = 0; i < arg_spec_array_length; i++) {
        arg_p_array[i] = NULL;
    }

    while(pos) {
        result = next_arg(args, &pos,
                          &key_start, &key_length,
                          &value_start, &value_length);

        if(key_start == NULL) {
            if(kwargs == 1 || pargs >= arg_spec_array_length) {
                for(size_t i = 0; i < arg_spec_array_length; i++) {
                    free(arg_spec_array[i].name);
                    free(arg_p_array[i]);
                }
                free(arg_spec_array);
                NZ_RETURN_ERR_MSG(NZ_BAD_PARG, rsprintf("%lu", value_start - args));
            }
            result = parse_one_arg(arg_spec_array[pargs].type, value_start, value_length, &arg_p_array[pargs]);
            if(result != NZ_SUCCESS) {
                for(size_t i = 0; i < arg_spec_array_length; i++) {
                    free(arg_spec_array[i].name);
                    free(arg_p_array[i]);
                }
                free(arg_spec_array);
                return result;
            }
            pargs++;
        } else {
            kwargs = 1;
            size_t i;
            for(i = pargs; i < arg_spec_array_length; i++) {
                if(arg_p_array[i] != NULL) continue;
                if(strncmp(arg_spec_array[i].name, key_start, key_length) == 0) {
                    result = parse_one_arg(arg_spec_array[i].type, value_start, value_length, &arg_p_array[i]);
                    if(result != NZ_SUCCESS) {
                        for(size_t i = 0; i < arg_spec_array_length; i++) {
                            free(arg_spec_array[i].name);
                            free(arg_p_array[i]);
                        }
                        free(arg_spec_array);
                        return result;
                    }
                    break;
                }
            }
            if(i == arg_spec_array_length) {
                for(size_t i = 0; i < arg_spec_array_length; i++) {
                    free(arg_spec_array[i].name);
                    free(arg_p_array[i]);
                }
                free(arg_spec_array);
                NZ_RETURN_ERR_MSG(NZ_BAD_KWARG, rsprintf("%lu", key_start - args));
            }
        }
    }

    for(size_t i = 0; i < arg_spec_array_length; i++) {
        if(arg_spec_array[i].required == 1 && arg_p_array[i] == NULL) {
            char * missing = strdup(arg_spec_array[i].name);
            for(size_t i = 0; i < arg_spec_array_length; i++) {
                free(arg_spec_array[i].name);
                free(arg_p_array[i]);
            }
            free(arg_spec_array);
            NZ_RETURN_ERR_MSG(NZ_ARG_MISSING, missing);
        }
    }

    for(size_t i = 0; i < arg_spec_array_length; i++) free(arg_spec_array[i].name);
    free(arg_spec_array);
    return NZ_SUCCESS;
}
