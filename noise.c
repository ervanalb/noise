#include "noise.h"
#include "log.h"
#include "libdill/list.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

// Channels
#define CH_TIMEOUT 10
int nz_chmake() {
    return chmake(NZ_CHUNK_SIZE * sizeof(nz_real));
}

int nz_chsend(const int * ch, const nz_real * chunk, int flags) {
    while (1) {
        if (*ch == -1) {
            msleep(now() + CH_TIMEOUT);
            continue;
        }
        int rc = chsend(*ch, chunk, NZ_CHUNK_SIZE * sizeof *chunk, now() + CH_TIMEOUT);
        if (rc == -1 && errno == ETIMEDOUT) {
            if ((flags & NZ_CH_NORETRY) && *ch == -1) return -1;
            else continue;
        }
        return rc;
    }
}

int nz_chrecv(const int * ch, nz_real * chunk, int flags) {
    while (1) {
        if (*ch == -1) {
            msleep(now() + CH_TIMEOUT);
            continue;
        }
        int rc = chrecv(*ch, chunk, NZ_CHUNK_SIZE * sizeof *chunk, now() + CH_TIMEOUT);
        if (rc == -1 && errno == ETIMEDOUT) {
            if ((flags & NZ_CH_NORETRY) && *ch == -1) return -1;
            else continue;
        }
        return rc;
    }
}

int nz_chdone(int ch) {
    return chdone(ch);
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
    int * channel_param;
    enum nz_direction channel_direction;
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

struct nz_param * nz_param_channel(const char * parent, const char * name, enum nz_direction dir, int * param) {
    struct nz_param * p = calloc(1, sizeof *p);
    if (p == NULL) return NULL;

    p->type = NZ_PARAM_CHANNEL;
    p->parent = parent;
    p->name = name;
    p->channel_param = param;
    p->channel_direction = dir;

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
                fprintf(stdout, "%d\n", *p->channel_param);
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
                    p->parent, p->name, *p->channel_param);
            fflush(stdout);
            const char * val = _getline();
            if (val != NULL && *val && !isspace(*val)) {
                int ch = atoi(val);
                *p->channel_param = ch;
            }
            break;
        default:
            FAIL("Invalid type %d", p->type);
            break;
        }
    }
    FAIL("UI Coroutine exited");
}
