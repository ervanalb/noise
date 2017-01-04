#include "noise.h"
#include "log.h"
#include "libdill/list.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

// Channels

struct nz_ch {
    struct hvfs hvfs;
    int self;

    int ctl; // libdill chan
    struct nz_ch * pair; // Corresponding reader/writer
    enum nz_dir dir;

    // Only used by writer channels
    int comm; // libdill chan
    int state;
    nz_chunk chunks[2];
};

static const int nz_ch_type_placeholder = 0;
static const void *nz_ch_type = &nz_ch_type_placeholder;

static void * nz_ch_query(struct hvfs * vfs, const void * type) {
    if (type == nz_ch_type)
        return (struct nz_ch *) vfs;
    errno = ENOTSUP;
    return NULL;
}

static void nz_ch_close(struct hvfs * vfs) {
    struct nz_ch * ch = (struct nz_ch *) vfs;
    nz_chjoin(ch->self, -1);
    ASSERT(ch->pair == NULL);

    if (ch->dir == NZ_WRITE)
        chdone(ch->comm);
    else
        ASSERT(ch->comm < 0);

    chdone(ch->ctl);
    free(ch);
}

int nz_chmake(enum nz_dir dir) {
    struct nz_ch * ch = calloc(1, sizeof *ch);
    ASSERT(ch != NULL);
    ch->hvfs.query = &nz_ch_query;
    ch->hvfs.close = &nz_ch_close;
    ch->ctl = chmake(0);
    ch->pair = NULL;
    ch->dir = dir;

    if (dir == NZ_WRITE) {
        ch->comm = chmake(sizeof(nz_real *));
        ch->state = 0;
    } else {
        ch->comm = -1;
    }

    ch->self = hmake(&ch->hvfs);
    ASSERT(ch->self >= 0);
    return ch->self;
}

nz_real * nz_challoc(int ch_handle) {
    struct nz_ch * ch = hquery(ch_handle, nz_ch_type);
    if (ch == NULL) return NULL;
    ASSERT(ch->dir == NZ_WRITE);
    return ch->chunks[ch->state];
}

int nz_chsend(int ch_handle, const nz_real * chunk, int flags) {
    struct nz_ch * ch = hquery(ch_handle, nz_ch_type);
    if (ch == NULL) return -1;
    if (chunk == ch->chunks[ch->state])
        ch->state = 1 - ch->state;
    while (1) {
        if (ch->pair == NULL && (flags & NZ_CANSKIP)) return -1;
        struct chclause clauses[2] = {
            { CHRECV, ch->ctl, NULL, 0 },
            { CHSEND, ch->comm, &chunk, sizeof chunk },
        };
        int n_clauses = (ch->pair != NULL) ? 2 : 1;
        int64_t deadline = (flags & NZ_NONBLOCK) ? 0 : -1;
        int rc = choose(clauses, n_clauses, deadline);
        if (rc == 1) return 0;
        if (rc == 0) continue;
        if (rc < 0 && errno == ETIMEDOUT) return -1;
        if (rc < 0) PFAIL("choose rc=%d", rc);
    }
}

const nz_real * nz_chrecv(int ch_handle, int flags) {
    struct nz_ch * ch = hquery(ch_handle, nz_ch_type);
    if (ch == NULL) return NULL;
    while (1) {
        if (ch->comm < 0 && (flags & NZ_CANSKIP)) return NULL;
        const nz_real * chunk = NULL;
        struct chclause clauses[2] = {
            { CHRECV, ch->ctl, NULL, 0 },
            { 0, 0, 0, 0 },
        };
        int n_clauses = 1;
        if (ch->pair != NULL) {
            clauses[1] = (struct chclause) { CHRECV, ch->pair->comm, &chunk, sizeof chunk };
            n_clauses = 2;
        }
        int64_t deadline = (flags & NZ_NONBLOCK) ? 0 : -1;
        int rc = choose(clauses, n_clauses, deadline);
        if (rc == 1) return chunk;
        if (rc == 0) continue;
        if (rc < 0 && errno == ETIMEDOUT) return NULL;
        if (rc < 0) PFAIL("choose rc=%d", rc);
    }
}

int nz_chjoin(int left_handle, int right_handle) {
    struct nz_ch * left_ch = NULL;
    if (left_handle >= 0) {
        left_ch = hquery(left_handle, nz_ch_type);
        if (left_ch == NULL) return -1;
    }
    struct nz_ch * right_ch = NULL;
    if (right_handle >= 0) {
        right_ch = hquery(right_handle, nz_ch_type);
        if (right_ch == NULL) return -1;
    }
    if (left_ch == NULL) {
        // Swap so that left is valid
        struct nz_ch * s = left_ch;
        left_ch = right_ch;
        right_ch = s;
    }
    if (left_ch == NULL) {
        // Both were invalid
        errno = EINVAL;
        return -1;
    }
    if (right_ch == NULL) {
        // Disconnect left
        if (left_ch->pair == NULL)
            return 0; // already disconnected
    } else {
        // Connect left & right
        if (left_ch->pair == right_ch &&
            right_ch->pair == left_ch)
            return 0; // already connected
        // Check there is a reader & writer side
        struct nz_ch * read_ch = NULL;
        struct nz_ch * write_ch = NULL;
        if (left_ch->dir == NZ_WRITE && right_ch->dir == NZ_READ) {
            write_ch = left_ch;
            read_ch = right_ch;
        } else if (left_ch->dir == NZ_READ && right_ch->dir == NZ_WRITE) {
            write_ch = right_ch;
            read_ch = left_ch;
        } else {
            errno = EINVAL;
            return -1;
        }
        // Disconnect any existing pair
        if (left_ch->pair != NULL) {
            int rc = nz_chjoin(left_handle, -1);
            if (rc < 0) return rc;
        }
        if (right_ch->pair != NULL) {
            int rc = nz_chjoin(right_handle, -1);
            if (rc < 0) return rc;
        }
        write_ch->pair = read_ch;
        read_ch->pair = write_ch;
    }
    // Notify
    if (left_ch != NULL) {
        int rc = chsend(left_ch->ctl, 0, 0, 0);
        if (rc < 0 && errno != ETIMEDOUT) return -1;
    }
    if (right_ch != NULL) {
        int rc = chsend(right_ch->ctl, 0, 0, 0);
        if (rc < 0 && errno != ETIMEDOUT) return -1;
    }
    INFO("Joined channels %d and %d", left_handle, right_handle);
    return 0;
}

int nz_chstate(int ch_handle) {
    struct nz_ch * ch = hquery(ch_handle, nz_ch_type);
    if (ch == NULL) return -1;
    if (ch->pair == NULL) return (errno = ENOTCONN), -1;
    return ch->pair->self;
}

// UI
static const char * keycodes = "qwertyuiopasdfghjklzxcvbnm";

struct nz_param {
    struct dill_list_item item;

    enum nz_param_type {
        NZ_PARAM_ENUM,
        NZ_PARAM_REAL,
        NZ_PARAM_CHANNEL,
    } type;
    const char * parent;
    const char * name;
    char keycode;

    // Enum
    int * enum_param;
    const struct nz_enum * enum_list;

    // Real
    nz_real * real_param;
    nz_real real_min;
    nz_real real_max;

    // Channel
    int channel_param;
};
struct dill_list nz_param_list;

struct nz_param * nz_param_enum(const char * parent, const char * name, const struct nz_enum * enums, int * param) {
    struct nz_param * p = calloc(1, sizeof *p);
    if (p == NULL) return NULL;

    p->type = NZ_PARAM_ENUM;
    p->parent = parent;
    p->name = name;
    p->enum_param = param;
    p->enum_list = enums;

    dill_list_item_init(&p->item);
    dill_list_insert(&nz_param_list, &p->item, NULL);

    return p;
}

static const char * nz_param_enum_name(const struct nz_param * p) {
    const int value = *p->enum_param;
    for (const struct nz_enum * e = p->enum_list; e->name != NULL; e++) {
        if (e->value == value)
            return e->name;
    }
    return NULL;
}

/*
static int nz_param_enum_set(const struct nz_param * p, const char * value) {
    for (struct nz_enum * e = p->enum_list; e->name != NULL; e++) {
        if (strcasecmp(value, e->name) == 0) {
            *p->enum_param = e->value;
            return 0;
        }
    }
    return -1;
}
*/

struct nz_param * nz_param_real(const char * parent, const char * name, nz_real min, nz_real max, nz_real * param) {
    struct nz_param * p = calloc(1, sizeof *p);
    if (p == NULL) return NULL;

    p->type = NZ_PARAM_REAL;
    p->parent = parent;
    p->name = name;
    p->real_param = param;
    p->real_min = min;
    p->real_max = max;

    dill_list_item_init(&p->item);
    dill_list_insert(&nz_param_list, &p->item, NULL);

    return p;
}

struct nz_param * nz_param_channel(const char * parent, const char * name, int param) {
    struct nz_param * p = calloc(1, sizeof *p);
    if (p == NULL) return NULL;

    p->type = NZ_PARAM_CHANNEL;
    p->parent = parent;
    p->name = name;
    p->channel_param = param;

    dill_list_item_init(&p->item);
    dill_list_insert(&nz_param_list, &p->item, NULL);

    return p;
}

void nz_param_destroy(struct nz_param * cookie) {
    dill_list_erase(&nz_param_list, &cookie->item);
    free(cookie);
}

static const char * _getline() {
    static char * buf = NULL;
    static size_t bufsz = 0;

    int rc = fdin(fileno(stdin), -1);
    if (rc < 0) PFAIL("fdin");

    ssize_t sz = getline(&buf, &bufsz, stdin);
    if (sz < 0) return NULL;

    return buf;
}

coroutine void nz_param_ui() {
    while (1) {
        // Draw UI
        int rc = fdout(fileno(stdout), -1);
        if (rc < 0) PFAIL("fdout");

        fprintf(stdout, "\nKnobs:\n");
        const char * kc = keycodes;
        for (struct dill_list_item * it = dill_list_begin(&nz_param_list); it != NULL; it = dill_list_next(it)){
            struct nz_param * p = (struct nz_param *) it;
            ASSERT(*kc != '\0');
            p->keycode = *kc++;

            fprintf(stdout, "[%c] %s::%s: ", p->keycode, p->parent, p->name);
            switch (p->type) {
            case NZ_PARAM_REAL:
                fprintf(stdout, "%0.3f [%0.3f %0.3f]\n", *p->real_param, p->real_min, p->real_max);
                break;
            case NZ_PARAM_ENUM:
                fprintf(stdout, "%s\n", nz_param_enum_name(p));
                break;
            case NZ_PARAM_CHANNEL:
                fprintf(stdout, "%d\n", p->channel_param);
                break;
            default:
                FAIL("Invalid type %d", p->type);
                break;
            }
        }

        // Handle Input
        fprintf(stdout, "> ");
        fflush(stdout);

        const char * ch1 = _getline();
        if (ch1 == NULL || !*ch1)
            continue;

        const struct nz_param * p = (void *) dill_list_begin(&nz_param_list);
        for (; p != NULL; p = (void *) dill_list_next(&p->item)){
            if (p->keycode == ch1[0])
                break;
        }
        if (p == NULL) {
            fprintf(stdout, "Unknown key '%c'\n", *ch1);
            continue;
        }

        switch (p->type) {
        case NZ_PARAM_REAL:
            fprintf(stdout, "Set %s::%s: %0.3f [%0.3f %0.3f] > ",
                    p->parent, p->name, *p->real_param, p->real_min, p->real_max);
            fflush(stdout);
            const char * real_val = _getline();
            if (real_val != NULL && *real_val && !isspace(*real_val)) {
                nz_real val = atof(real_val);
                if (val < p->real_min) val = p->real_min;
                if (val > p->real_max) val = p->real_max;
                *p->real_param = val;
            }
            break;
        case NZ_PARAM_ENUM:;
            int idx = 1;
            for (const struct nz_enum * e = p->enum_list; e->name != NULL; e++) {
                fprintf(stdout, "[%d] %s\n", idx++, e->name);
            }
            fprintf(stdout, "Set %s::%s: %s > ",
                    p->parent, p->name, nz_param_enum_name(p));
            fflush(stdout);

            const char * enum_val = _getline();
            if (enum_val != NULL && *enum_val && !isspace(*enum_val)) {
                int res = atoi(enum_val);
                if (res <= 0 || res >= idx) {
                    fprintf(stdout, "Invalid value %d\n", res);
                } else {
                    *p->enum_param = p->enum_list[res-1].value;
                }
            }
            break;
        case NZ_PARAM_CHANNEL:
            fprintf(stdout, "Set %s::%s: %d > ",
                    p->parent, p->name, p->channel_param);
            fflush(stdout);
            const char * val = _getline();
            if (val != NULL && *val && !isspace(*val)) {
                int ch = atoi(val);
                nz_chjoin(p->channel_param, ch);
            }
            break;
        default:
            FAIL("Invalid type %d", p->type);
            break;
        }
    }
    FAIL("UI Coroutine exited");
}
