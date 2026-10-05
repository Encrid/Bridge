/* Bridge-ASM700's inside: what bridge.c, store.c and abi.c share. Nothing here crosses a seam. */
#ifndef BRIDGE_INSIDE_H
#define BRIDGE_INSIDE_H

#include "bridge.h"
#include "bridge-abi.h"
#include "backend.h"
#include "os.h"
#include "rows.h"
#include "schema.h"

constexpr uint32_t PAGE = 65'536;

typedef struct module module;
typedef struct pool pool;

typedef struct {
    char            *name;
    void            *lib;
    backend_api      api;
    backend         *be;
    enum backend_capability capabilities;
    backend_memory  *shared;        /* the shared block, as this backend sees it */
} open_backend;

/* A component, by its namespaced name. Storage holds the rows; this is what load checks. */
typedef struct component component;
struct component {
    char        *name;
    rows_type   *type;              /* as storage knows it: a size */
    schema       s;                 /* its fields or its constants, read from its text */
    enum bridge_kind kind;
    uint32_t     size, align;
    uint64_t     fingerprint;       /* of the descriptor string - schema.md, *Fingerprints* */
    uint32_t     declarers;         /* how many loaded modules declared it */
    uint32_t     users;             /* how many components name it as a field's type */
    component  **deps;              /* the declared types its own fields name */
    uint32_t     dep_count;
    void        *zero;              /* `size` zero bytes: what an attach with no value copies */
};

typedef struct { uint64_t id; enum bridge_event events; } pending;

/* A system, or an observer: a query and a function either way. An observer's phase is the one
 * past the deployment's last, which is the barrier. */
typedef struct {
    module       *m;
    char         *name;
    char         *full;             /* how [order] names it: the module's stem, then the name */
    uint32_t      phase;
    rows_terms   *matched;          /* its terms, as storage holds them. nullptr with no terms */
    uint32_t      terms;
    uint32_t      sizes[BRIDGE_TERMS_MAX];
    component    *touches[BRIDGE_TERMS_MAX];    /* nullptr where a term reads nothing */
    uint8_t       writes[BRIDGE_TERMS_MAX];
    backend_func *fn;
    uint32_t      level;            /* within its phase: a level runs before the next */
    int           parked;           /* out of the schedule while a reload decides its fate */

    int           observer;
    pending      *pending;          /* who triggered it since the last barrier */
    uint32_t      pending_count, pending_room;

    uint32_t      when;             /* of the dispatch in progress */
    int           failed;           /* and how it ended, said after the join */
    char          why[256];
} bridge_system;

struct module {
    bridge           *b;
    char             *identity, *ns;
    char             *stem;         /* the identity without its version */
    open_backend     *on;
    backend_module   *image;
    backend_instance *instance;
    backend_memory   *memory;       /* its private block, as its backend sees it */

    /* The private block. A module can write all of it, so every offset Bridge-ASM700 acts on
     * is kept here and never read back from the header. */
    uint8_t  *priv;
    size_t    priv_len, priv_reserve;
    uint32_t  scratch, scratch_len;
    uint32_t  ring, ring_len, ring_tail;
    uint32_t  spans, spans_len;     /* the span region: query slots, column_base, spans */
    /* what storage last wrote there: whether it has, where its spans end, and whether an
     * observer's list was not empty */
    bool      published, had_triggers;
    uint32_t  spans_end;

    component    **declared;        /* by slot */
    enum bridge_on_unload *policy;  /* by slot: what it stated */
    uint32_t       declared_count;
    bridge_system **systems;        /* by slot */
    uint32_t       system_count;
    bridge_system **observers;      /* by slot */
    uint32_t       observer_count;

    int64_t   fuel;
    /* What is set aside for it alone - threads.md. Ids it may hand out, taken from storage at a
     * barrier; its share of the persistent arena; and what it asked to have changed, applied
     * at the next barrier in the order it asked. Only its own thread touches any of them. */
    uint64_t *ids;
    uint32_t  id_count, id_next, id_room;
    int       ids_short;            /* the entity ceiling cut its last refill short */
    int64_t   spawns, persistent;
    uint32_t  arena_at, arena_left, arena_base, arena_share;
    /* what a reload added to its share where it could not be added in place: used in order,
     * once the range before is too small for what is asked - budgets.md */
    struct { uint32_t at, left; } *more;
    uint32_t  more_count, more_next;
    uint8_t  *queue;
    uint32_t  queue_len, queue_room;
    char    **read, **write;        /* grants */
    uint32_t  read_count, write_count;

    int      pinned;
    uint64_t thread;                /* the one that loaded it */
    int      hot_reload;
    enum bridge_on_error on_error;
    backend_func *shutdown;
    int64_t  shutdown_timeout;
    /* failure.md, *The budget*: raised by a failed barrier, lowered by a clean one */
    int64_t  max_failures, failures;
    int      ran_now, failed_now;   /* since the last barrier */
    /* budgets.md, *What running out does*: how many entities it changed were left as they were
     * at this barrier, for want of a table and for want of columns */
    uint32_t left[2];
    /* config.md: the component its settings seeded, the row, and where their text is */
    component   *config;
    uint64_t     config_entity;
    uint32_t     text_at, text_room;
    int   in_init, stopped;
    int   refused;                  /* an entry refused during the call in progress */
    char  why[256];                 /* and this is why */
    module *next;
};

/* One queued change: `size` bytes of value follow it, padded to eight. */
enum change_op : uint32_t { CHANGE_DESTROY = 1, CHANGE_ATTACH = 2, CHANGE_DETACH = 3 };
typedef struct {
    uint64_t     id;
    component   *c;
    uint32_t     size;
    enum change_op op;
} change;

typedef struct { char *stem; uint32_t at, room; } kept_text;
typedef struct { uint32_t at, len; } gap;       /* in the column region, from its start */

struct bridge {
    bridge_sink_fn sink;
    void          *ud;
    open_backend  *backends;
    uint32_t       backend_count;
    /* the address space every block is allocated with: the largest any backend stated */
    uint64_t       reservation, guard;
    char          *stopped_by;      /* why nothing runs any more */
    char          *stopped_stem;    /* the module whose return lifts it, or nullptr if nothing can */

    char   **phases;
    uint32_t phase_count;
    uint32_t barriers;
    char   **before, **after;       /* [order], pair by pair */
    uint32_t order_count;
    pool    *pool;

    /* the shared block: a reserved page, the entity index, the persistent arena, the columns */
    uint8_t       *shared;
    size_t         shared_len, shared_reserve;
    bridge_entity *index;
    uint32_t       entities;
    uint32_t       arena, arena_len, arena_used;
    uint32_t       columns, columns_len, columns_used;
    gap           *gaps;            /* freed column space, by offset. kept outside the block */
    uint32_t       gap_count, gap_room;
    int            layout_moved;    /* a module came or went, or a declaration changed */
    uint32_t       tables;          /* [budget] tables: how many may exist at once */
    uint32_t       max_types, max_systems;      /* [budget] types, [budget] systems */

    rows          *rows;            /* who keeps the rows: rows.h */

    component    **components;
    uint32_t       component_count;
    kept_text     *kept;            /* where an unloaded module's settings text was */
    uint32_t       kept_count;
    bridge_system **systems;        /* every module's, in registration order */
    uint32_t       system_count;
    module        *modules;
    uint32_t       module_count, max_modules;
};

/* bridge.c */
void say(bridge *b, const module *m, const bridge_system *s, uint8_t level, const char *fmt, ...);
[[nodiscard]] char *copy(const char *s, size_t n);
/* 1, with a record, if nothing may run. `repairing` names a module being loaded or reloaded:
 * the one call a stopped bridge accepts is one that brings back the module that stopped it. */
[[nodiscard]] int bridge_stopped(bridge *b, const char *repairing);
void bridge_stop(bridge *b, const char *identity, const char *what);
void bridge_resume(bridge *b, const module *m);     /* if `m` is what it was waiting for */

/* lifecycle.c */
void module_discard(bridge *b, module *m);
void module_shut_down(bridge *b, module *m);    /* its shutdown, what it asked for, what it said */

/* config.c */
[[nodiscard]] int config_seed(bridge *b, module *m, const bridge_module_config *cfg, char *why, size_t len);
void config_keep(bridge *b, module *m);

/* schedule.c */
[[nodiscard]] int schedule_order(bridge *b, const bridge_order *order, uint32_t count);  /* at create */
[[nodiscard]] int schedule_build(bridge *b, char *why, size_t why_len);     /* 0, or why it cannot be */
void schedule_run(bridge *b, uint32_t phase);
void schedule_account(bridge *b);       /* at each barrier: the failure budgets, and the stops */

/* pool.c */
typedef void (*pool_fn)(void *job);
[[nodiscard]] pool *pool_create(uint32_t workers);
void  pool_destroy(pool *p);
/* fn(jobs[i]) for every i, on the workers and the caller; a job marked is the caller's alone */
void  pool_run(pool *p, pool_fn fn, void **jobs, const uint8_t *caller_only, uint32_t count);

/* store.c */
[[nodiscard]] int store_create(bridge *b, const bridge_budget *budget);    /* 0, or a record was emitted */
void     store_destroy(bridge *b);
int      store_refill(bridge *b, module *m);                      /* 1 if the ceiling cut its ids short */
void     store_drain(bridge *b, module *m);                       /* its log ring, to the sink */
void     store_barrier(bridge *b);
void     store_header(bridge *b, module *m);                      /* what a module reads at offset 0 */
uint32_t store_offset(bridge *b, const void *p);                  /* into the shared memory, 0 for null */
/* the column region, carved: what storage asks for is what it gets, to sixteen bytes.
 * `new_size` bytes holding what the `old_size` at `ptr` held, or fresh if `ptr` is null. Null,
 * and nothing changed, if the region cannot give them: nothing comes from outside it */
[[nodiscard]] void *store_column_regrow(bridge *b, void *ptr, uint32_t old_size, uint32_t new_size);
void     store_column_free(bridge *b, void *ptr, uint32_t size);

/* abi.c */
[[nodiscard]] int abi_define(module *m, backend_err *err);                      /* the eight imports */

#endif
