#include "noise.h"
#include "log.h"
#include "libdill/list.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

// Channels
// Pipes
struct pipe {
    struct hvfs hvfs;
    int self;

    struct nz_ch * read_ch;
    struct nz_ch * write_ch;
    int comm;
};

struct nz_ch {
    int ctl; // libdill chan
    int comm; // libdill chan
    int pipe;
    enum nz_dir dir;
    // Only used by writer channels
    int state;
    nz_chunk chunks[2];
};

static const int pipe_type_placeholder = 0;
static const void *pipe_type = &pipe_type_placeholder;

static void * pipe_query(struct hvfs * vfs, const void * type) {
    if (type == pipe_type)
        return (struct pipe *) vfs;
    errno = ENOTSUP;
    return NULL;
}

static void pipe_disconn(struct pipe * pipe, struct nz_ch * ch) {
    if (ch == NULL) return;
    switch (ch->dir) {
    case NZ_READ:
        ASSERT(pipe->read_ch == ch);
        pipe->read_ch = NULL;
        break;
    case NZ_WRITE:
        ASSERT(pipe->write_ch == ch);
        pipe->write_ch = NULL;
        break;
    default:
        ERROR("Unknown channel direction %d", ch->dir);
        break;
    }

    ch->pipe = -1;
    ch->comm = -1;
    int rc = chsend(ch->ctl, 0, 0, 0);
    if (rc < 0 && errno != ETIMEDOUT) PFAIL("chsend rc = %d", rc);
}

static void pipe_conn(struct pipe * pipe, struct nz_ch * ch) {
    switch (ch->dir) {
    case NZ_READ:
        pipe_disconn(pipe, pipe->read_ch);
        pipe->read_ch = ch;
        break;
    case NZ_WRITE:
        pipe_disconn(pipe, pipe->write_ch);
        pipe->write_ch = ch;
        break;
    default:
        ERROR("Unknown channel direction %d", ch->dir);
        break;
    }
    ASSERT(ch->pipe < 0);
    ASSERT(ch->comm < 0);
    ch->pipe = pipe->self;
    ch->comm= pipe->comm;
    int rc = chsend(ch->ctl, 0, 0, 0);
    if (rc < 0 && errno != ETIMEDOUT) PFAIL("chsend rc = %d", rc);
}

static void pipe_close(struct hvfs * vfs) {
    struct pipe * pipe = (struct pipe *) vfs;
    pipe_disconn(pipe, pipe->read_ch);
    pipe_disconn(pipe, pipe->write_ch);
    free(pipe);
}

int nz_pipe(void) {
    struct pipe * pipe = calloc(1, sizeof *pipe);
    ASSERT(pipe != NULL);
    pipe->hvfs.query = &pipe_query;
    pipe->hvfs.close = &pipe_close;
    pipe->read_ch = NULL;
    pipe->write_ch = NULL;
    pipe->comm = chmake(sizeof(nz_real *));
    ASSERT(pipe->comm >= 0);
    pipe->self = hmake(&pipe->hvfs);
    ASSERT(pipe->self >= 0);
    return pipe->self;
}

// Channels

struct nz_ch * nz_chmake(enum nz_dir dir) {
    struct nz_ch * ch = calloc(1, sizeof *ch);
    ASSERT(ch != NULL);
    ch->ctl = chmake(0);
    ASSERT(ch->ctl >= 0);
    ch->comm = -1;
    ch->pipe = -1;
    ch->dir = dir;
    ch->state = 0;
    return ch;
}

nz_real * nz_challoc(struct nz_ch * ch) {
    ASSERT(ch->dir == NZ_WRITE);
    return ch->chunks[ch->state];
}

int nz_chsend(struct nz_ch * ch, const nz_real * chunk, int flags) {
    if (chunk == ch->chunks[ch->state])
        ch->state = 1 - ch->state;
    while (1) {
        struct chclause clauses[2] = {
            { CHRECV, ch->ctl, NULL, 0 },
            { CHSEND, ch->comm, &chunk, sizeof chunk },
        };
        int n_clauses = (ch->comm >= 0) ? 2 : 1;
        int64_t deadline = (flags & NZ_NONBLOCK) ? 0 : -1;
        int rc = choose(clauses, n_clauses, deadline);
        if (rc == 1) return 0;
        if (rc == 0) continue;
        if (rc < 0 && errno == ETIMEDOUT) return -1;
        if (rc < 0) PFAIL("choose rc=%d", rc);
    }
}

const nz_real * nz_chrecv(struct nz_ch * ch, int flags) {
    while (1) {
        if (ch->comm < 0 && (flags & NZ_CANSKIP)) return NULL;
        const nz_real * chunk = NULL;
        struct chclause clauses[2] = {
            { CHRECV, ch->ctl, NULL, 0 },
            { CHRECV, ch->comm, &chunk, sizeof chunk },
        };
        int n_clauses = (ch->comm >= 0) ? 2 : 1;
        int64_t deadline = (flags & NZ_NONBLOCK) ? 0 : -1;
        int rc = choose(clauses, n_clauses, deadline);
        if (rc == 1) return chunk;
        if (rc == 0) continue;
        if (rc < 0 && errno == ETIMEDOUT) return NULL;
        if (rc < 0) PFAIL("choose rc=%d", rc);
    }
}

void nz_chdone(struct nz_ch * ch) {
    if (ch->pipe >= 0) {
        struct pipe * pipe = (void *) hquery(ch->pipe, pipe_type);
        ASSERT(pipe != NULL);
        pipe_disconn(pipe, ch);
    }
    free(ch);
}

int nz_chjoin(struct nz_ch * ch, int pipe) {
    if (ch->pipe >= 0) {
        struct pipe * old_pipe = (void *) hquery(ch->pipe, pipe_type);
        ASSERT(old_pipe != NULL);
        pipe_disconn(old_pipe, ch);
    }

    struct pipe * new_pipe = (void *) hquery(pipe, pipe_type);
    if (new_pipe == NULL) return -1;
    pipe_conn(new_pipe, ch);
    return 0;
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
    struct nz_ch * channel_param;
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

struct nz_param * nz_param_channel(const char * parent, const char * name, struct nz_ch * param) {
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
                fprintf(stdout, "%d\n", p->channel_param->pipe);
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
                    p->parent, p->name, p->channel_param->pipe);
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
