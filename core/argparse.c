#include "core/argparse.h"

struct arg_spec {
    const char * name;
    size_t name_len;
    enum {GENERIC, STRING, INT, REAL} type;
    int required;
};

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

static nz_rc next_arg(const char * string, const char ** pos,
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

nz_rc arg_parse(char * args, const char * fmt, nz_arg *** arg_p_array_p) {
    size_t fmt_len = strlen(fmt);
    size_t item_cnt = 1;
    for(size_t i = 0; i < fmt_len; i++) {
        if(fmt[i] == ',') item_cnt++;
    }
    struct arg_spec * arg_spec_array = calloc(item_cnt, sizeof(struct arg_spec));
    for(size_t i = 0; i < fmt_len; i++) {
        
    }

    return NZ_SUCCESS;
}
