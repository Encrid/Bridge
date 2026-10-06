/* Bridge-ASM700's inside: what bridge.c, block.c and abi.c share. Nothing here crosses a seam. */
#ifndef BRIDGE_INSIDE_H
#define BRIDGE_INSIDE_H

#include "bridge.h"
#include "bridge-abi.h"
#include "backend.h"
#include "os.h"
#include "storage.h"
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
typedef struct declared_type declared_type;
struct declared_type {
    char        *name;
    storage_type   *type;              /* as storage knows it: a size */
    schema       s;                 /* its fields or its constants, read from its text */
    enum bridge_kind kind;
    uint32_t     size, align;
    uint64_t     fingerprint;       /* of the descriptor string - schema.md, *Fingerprints* */
    uint32_t     declarers;         /* how many loaded modules declared it */
    uint32_t     users;             /* how many components name it as a field's type */
    declared_type  **deps;              /* the declared types its own fields name */
    uint32_t     dep_count;
    void        *zero;              /* `size` zero bytes: what an attach with no value copies */
};

typedef struct { uint64_t id; enum bridge_event events; } pending;
typedef struct { uint32_t at, len; } range;

/* A system, or an observer: a query and a function either way. An observer's phase is the one
 * past the deployment's last, which is the barrier. */
typedef struct {
    module       *m;
    char         *name;
    char         *full;             /* how [order] names it: the module's stem, then the name */
    uint32_t      phase;
    storage_terms   *matched;          /* its terms, as storage holds them. nullptr with no terms */
    uint32_t      terms;
    backend_func *fn;
    uint32_t      level;            /* within its phase: a level runs before the next */
    int           parked;           /* out of the schedule while a reload decides its fate */

    int           observer;
    pending      *pending;          /* who triggered it since the last barrier */
    uint32_t      pending_count, pending_room;

    uint32_t      when;             /* of the dispatch in progress */
    uint32_t      turn;             /* and where it comes among all that ran before the barrier */
    int           failed;           /* and how it ended, said after the join */
    char          why[256];
    int64_t       failures;         /* failure.md, *The budget*: its own count */
    /* schedule.md, *Order within a phase is never implicit*: the components it stated that it
     * attaches or detaches. They lie after the terms, in the same block */
    declared_type **changes;
    uint32_t      change_count;
    /* what the conflict graph reads of each term: the component, or nullptr where the term reads
     * nothing, and whether it is written. `terms` of them */
    struct { declared_type *touches; bool writes; } term[];
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

    declared_type    **declared;        /* by slot */
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
    uint32_t  arena_at, arena_left, arena_share;
    /* The ranges of the arena that are its share: one from its load, and one more for each
     * reload that could not add to the last where it stood. `alloc` moves to the next once
     * what is asked no longer fits in the one before - budgets.md. They are what goes back to
     * the arena at unload, if nothing left alive points into them - strings.md */
    range    *owned;
    uint32_t  own_count, own_next;
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
    /* failure.md, *The budget*. Each system has a count of its own. `failures` is the
     * module's, for what is nobody's system: a barrier that could not place an entity it
     * changed raises it, and one that placed all it changed lowers it */
    int64_t  max_failures, failures;
    int      changed_now, unplaced_now;     /* at this barrier */
    const void *reached;            /* the system whose count reached the number, if one did */
    /* budgets.md, *What running out does*: how many changes it asked were not applied at this
     * barrier, for want of a table and for want of columns */
    uint32_t left[2];
    /* grants.md: the types it may write, as storage numbers them, and how many entities it
     * asked to destroy at this barrier that carry none of them */
    uint64_t *writable;
    uint32_t barred;
    /* config.md: the component its settings seeded, the row, and where their text is */
    declared_type   *config;
    uint64_t     config_entity;
    uint32_t     text_at, text_room;
    int   in_init, stopped;
    /* threads.md: what is running in it now, or nullptr for a lifecycle call, and its turn.
     * `marks` is where in the queue each turn's changes begin: one for every call that
     * queued something, so the barrier can apply the queues in the order the calls ran */
    const void *running;
    uint32_t turn;
    struct { uint32_t turn, at; } *marks;
    uint32_t mark_count, mark_room, mark_at;
    int   refused;                  /* an entry refused during the call in progress */
    char  why[256];                 /* and this is why */
    module *next;
};

/* One queued change: `size` bytes of value follow it, padded to eight. */
enum change_op : uint32_t { CHANGE_DESTROY = 1, CHANGE_ATTACH = 2, CHANGE_DETACH = 3 };
typedef struct {
    uint64_t     id;
    declared_type   *c;
    uint32_t     size;
    enum change_op op;
} change;

/* A turn: observers of the last barrier, then lifecycle calls, then the phase's systems, each
 * in the order it ran. */
constexpr uint32_t TURN_LIFECYCLE = 1u << 16, TURN_SYSTEM = 2u << 16;

typedef struct { char *stem; uint32_t at, room; } kept_text;
/* A stretch of the shared block handed out in pieces and taken back: the column region, and
 * the persistent arena. What is free is kept here, outside the block, where no module's stray
 * write reaches it: by offset from the region's start, joined to its neighbours. */
typedef struct {
    range   *gaps;
    uint32_t count, room;
    uint32_t used;                  /* everything past it has never been handed out */
    uint32_t len;
} region;

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
    uint32_t turns;                 /* lifecycle calls since the last barrier: threads.md */
    /* what a phase's dispatch works in, [budget] systems long: budgets.md, *Nothing is
     * allocated in a phase* */
    void *run_jobs, **run_handed, **run_lists;
    uint8_t *run_callers;
    char   **before, **after;       /* [order], pair by pair */
    uint32_t order_count;
    pool    *pool;

    /* the shared block: a reserved page, the entity index, the persistent arena, the columns */
    uint8_t       *shared;
    size_t         shared_len, shared_reserve;
    bridge_entity *index;
    uint32_t       entities;
    uint32_t       arena, arena_len;
    uint32_t       columns, columns_len;
    region         arena_space, column_space;   /* what of each is handed out */
    int            layout_moved;    /* a module came or went, or a declaration changed */
    uint32_t       tables;          /* [budget] tables: how many may exist at once */
    uint32_t       max_types, max_systems;      /* [budget] types, [budget] systems */
    uint32_t       max_terms;                   /* [budget] terms */

    storage          *storage;            /* who keeps the rows: storage.h */

    declared_type    **types;
    uint32_t       type_count;
    kept_text     *kept;            /* where an unloaded module's settings text was */
    uint32_t       kept_count;
    bridge_system **systems;        /* every module's, in registration order */
    uint32_t       system_count;
    module        *modules;
    uint32_t       module_count, max_modules;
};

/* bridge.c */
void say(bridge *b, const module *m, const bridge_system *s, uint8_t level, const char *fmt, ...);
void abi_marks(module *m);          /* room for a mark from every call it may yet take */
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
void config_forget(bridge *b, const module *m);

/* schedule.c */
[[nodiscard]] int schedule_order(bridge *b, const bridge_order *order, uint32_t count);  /* at create */
[[nodiscard]] int schedule_build(bridge *b, char *why, size_t why_len);     /* 0, or why it cannot be */
void schedule_room(bridge *b);          /* at create: what a dispatch works in */
void schedule_run(bridge *b, uint32_t phase);
void schedule_account(bridge *b);       /* at each barrier: the failure budgets, and the stops */

/* pool.c */
typedef void (*pool_fn)(void *job);
[[nodiscard]] pool *pool_create(uint32_t workers, uint32_t most_jobs);
void  pool_destroy(pool *p);
/* fn(jobs[i]) for every i, on the workers and the caller; a job marked is the caller's alone */
void  pool_run(pool *p, pool_fn fn, void **jobs, const uint8_t *caller_only, uint32_t count);

/* block.c */
[[nodiscard]] int block_create(bridge *b, const bridge_budget *budget);    /* 0, or a record was emitted */
void     block_destroy(bridge *b);
int      block_refill(bridge *b, module *m);                      /* 1 if the ceiling cut its ids short */
void     block_drain(bridge *b, module *m);                       /* its log ring, to the sink */
void     block_barrier(bridge *b);
void     block_header(bridge *b, module *m);                      /* what a module reads at offset 0 */
uint32_t block_offset(bridge *b, const void *p);                  /* into the shared memory, 0 for null */
/* a piece of a region: 0 if it has none that long; and a piece given back */
[[nodiscard]] int region_take(region *r, uint32_t len, uint32_t *at);
void     region_give(region *r, uint32_t at, uint32_t len);
/* the piece at `at` made longer where it stands: false, and nothing changed, if what follows it
 * is not free */
[[nodiscard]] bool region_grow(region *r, uint32_t at, uint32_t had, uint32_t want);
uint32_t region_most(const region *r);          /* the longest piece it could give now */
/* the column region, carved: what storage asks for is what it gets, to sixteen bytes.
 * `new_size` bytes holding what the `old_size` at `ptr` held, or fresh if `ptr` is null. Null,
 * and nothing changed, if the region cannot give them: nothing comes from outside it */
[[nodiscard]] void *block_column_regrow(bridge *b, void *ptr, uint32_t old_size, uint32_t new_size);
void     block_column_free(bridge *b, void *ptr, uint32_t size);

/* abi.c */
[[nodiscard]] int abi_define(module *m, backend_err *err);                      /* the eight imports */

#endif
