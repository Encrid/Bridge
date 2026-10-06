/* Bridge-ASM700's inside: what bridge.c, block.c and abi.c share. Nothing here crosses a seam. */
#ifndef BRIDGE_INSIDE_H
#define BRIDGE_INSIDE_H

#include "bridge.h"
#include "bridge-abi.h"
#include "backend.h"
#include "os.h"
#include "storage.h"
#include "schema.h"


typedef struct module module_t;
typedef struct pool pool_t;

typedef struct {
    char            *name;
    void            *lib;
    backend_api_t      api;
    backend_t         *be;
    enum backend_capability capabilities;
    backend_memory_t  *shared;        /* the shared block, as this backend sees it */
    /* backend.md, *Memory is adopted, not created*: the address space it needs around a
     * block it maps. A module's private block has this; the shared block the largest */
    uint64_t         reservation, guard, granule;
} open_backend_t;

/* A component, by its namespaced name. Storage holds the rows; this is what load checks. */
typedef struct declared_type declared_type_t;
struct declared_type {
    char        *name;
    storage_type_t   *type;              /* as storage knows it: a size */
    schema_t       s;                 /* its fields or its constants, read from its text */
    enum bridge_kind kind;
    enum bridge_storage storage;    /* the storage layout every declarer stated */
    uint32_t     size, align;
    uint64_t     fingerprint;       /* of the descriptor string - schema.md, *Fingerprints* */
    uint32_t     declarers;         /* how many loaded modules declared it */
    uint32_t     users;             /* how many components name it as a field's type */
    declared_type_t  **deps;              /* the declared types its own fields name */
    uint32_t     dep_count;
    void        *zero;              /* `size` zero bytes: what an attach with no value copies */
    /* lifetime.md, *When a component is moved*: at a reload, the type of this name whose rows
     * become this one's when the reload holds; and that the other has such a successor */
    declared_type_t   *replaces;
    bool         superseded;
    /* and, on the one that is superseded, which bytes of a value of it are which bytes of a
     * value of its successor: nullptr if they are the same bytes */
    storage_run_t *runs;
    uint32_t     run_count;
};

typedef struct { uint64_t id; enum bridge_event events; } pending_t;
typedef struct { uint32_t at, len; } range_t;

/* A system, or an observer: a query and a function either way. An observer's phase is the one
 * past the deployment's last, which is the barrier. */
typedef struct {
    module_t       *m;
    char         *name;
    char         *full;             /* how [order] names it: the module's stem, then the name */
    uint32_t      phase;
    storage_terms_t   *matched;          /* its terms, as storage holds them. nullptr with no terms */
    /* budgets.md: what it asked storage for, kept so that a system taken out of storage for
     * the length of a reload can be put back as it was */
    storage_term_t    *asked;
    enum bridge_event events;
    uint32_t      terms;
    backend_func_t *fn;
    uint32_t      level;            /* within its phase: a level runs before the next */
    int           parked;           /* out of the schedule while a reload decides its fate */

    int           observer;
    pending_t      *pending;          /* who triggered it since the last barrier */
    uint32_t      pending_count, pending_room;

    uint32_t      when;             /* of the dispatch in progress */
    uint32_t      turn;             /* and where it comes among all that ran before the barrier */
    int           failed;           /* and how it ended, said after the join */
    char          why[256];
    int64_t       failures;         /* failure.md, *The budget*: its own count */
    /* schedule.md, *Order within a phase is never implicit*: the components it stated that it
     * attaches or detaches. They lie after the terms, in the same block */
    declared_type_t **changes;
    uint32_t      change_count;
    /* what the conflict graph reads of each term: the component, or nullptr where the term reads
     * nothing, and whether it is written. `terms` of them */
    struct { declared_type_t *touches; bool writes; } term[];
} bridge_system_t;

struct module {
    bridge_t           *b;
    char             *identity, *ns;
    char             *stem;         /* the identity without its version */
    open_backend_t     *on;
    backend_module_t   *image;
    backend_instance_t *instance;
    backend_memory_t   *memory;       /* its private block, as its backend sees it */

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

    declared_type_t    **declared;        /* by slot */
    enum bridge_on_unload *policy;  /* by slot: what it stated */
    bool          *may_write;       /* by slot: its grants let it write the component. Worked
                                     * out once, at declare: an attach asks it every time */
    uint32_t       declared_count;
    bridge_system_t **systems;        /* by slot */
    uint32_t       system_count;
    bridge_system_t **observers;      /* by slot */
    uint32_t       observer_count;

    int64_t   work_limit;
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
    range_t    *owned;
    uint32_t  own_count, own_next;
    uint8_t  *queue;
    uint32_t  queue_len, queue_room;
    char    **read, **write;        /* grants */
    uint32_t  read_count, write_count;
    /* schedule.md: the phases it names that the deployment calls otherwise, and which of the
     * deployment's each is */
    char    **phase_from;
    uint32_t *phase_to;
    uint32_t  phase_names;

    int      pinned;
    uint64_t thread;                /* the one that loaded it */
    int      hot_reload;
    enum bridge_on_error on_error;
    backend_func_t *shutdown;
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
    declared_type_t   *config;
    uint32_t     text_at, text_room;
    int   in_init, stopped;
    /* threads.md: what is running in it now, or nullptr for a lifecycle call, and its turn.
     * `marks` is where in the queue each turn's changes begin: one for every call that
     * queued something, so the barrier can apply the queues in the order the calls ran */
    const void *running;
    uint32_t turn;
    struct { uint32_t turn, at; } *marks;
    uint32_t mark_count, mark_room, mark_at;
    /* a reload: the instance this one is to take the place of, and what it accepts to lose */
    module_t *reloads;
    const char *const *losses;
    uint32_t loss_count;
    uint32_t *loss_rows;            /* by loss: how many rows lost something under that name,
                                     * plus one. Zero: nothing was */
    int   refused;                  /* an entry refused during the call in progress */
    char  why[256];                 /* and this is why */
    module_t *next;
};

/* One queued change: `size` bytes of value follow it, padded to eight. */
enum change_op : uint32_t { CHANGE_DESTROY = 1, CHANGE_ATTACH = 2, CHANGE_DETACH = 3 };
typedef struct {
    uint64_t     id;
    declared_type_t   *c;
    uint32_t     size;
    enum change_op op;
} change_t;

/* A turn: observers of the last barrier, then lifecycle calls, then the phase's systems, each
 * in the order it ran. */
constexpr uint32_t TURN_LIFECYCLE = 1u << 16, TURN_SYSTEM = 2u << 16;

typedef struct { char *stem; uint32_t at, room; } kept_text_t;
/* A stretch of the shared block handed out in pieces and taken back: the column region, and
 * the persistent arena. What is free is kept here, outside the block, where no module's stray
 * write reaches it: by offset from the region's start, joined to its neighbours. */
typedef struct {
    range_t   *gaps;
    uint32_t count, room;
    uint32_t used;                  /* everything past it has never been handed out */
    uint32_t len;
} region_t;

struct bridge {
    bridge_sink_fn_t sink;
    void          *ud;
    open_backend_t  *backends;
    uint32_t       backend_count;
    /* the address space every block is allocated with: the largest any backend stated */
    uint64_t       reservation, guard, granule;    /* of the shared block: backend.md */
    /* why nothing runs any more, and the module whose return lifts it: nullptr while the
     * bridge runs. They point at the two arrays below, since a bridge is stopped at a barrier,
     * where nothing is allocated - budgets.md, *Nothing is allocated in a phase* */
    char          *stopped_by, *stopped_stem;
    char           stopped_why[256], stopped_name[256];

    char   **phases;
    uint32_t phase_count;
    uint32_t barriers;
    uint32_t turns;                 /* lifecycle calls since the last barrier: threads.md */
    uint32_t parked;                /* systems out of storage while their module reloads */
    /* how many ranges of the arena modules hold: its list of gaps has room for one more than
     * that, found when a range is taken, so that giving one back never asks for memory */
    uint32_t arena_ranges;
    /* what a phase's dispatch works in, [budget] systems long: budgets.md, *Nothing is
     * allocated in a phase* */
    void *run_jobs, **run_handed, **run_lists;
    uint8_t *run_callers;
    /* and what the schedule is worked out in, when a module comes or goes: a load, a reload
     * and an unload ask for no memory to do it, so none of them can fail there */
    void **build_in;
    uint64_t *build_bits;
    char   **before, **after;       /* [order], pair by pair */
    uint32_t order_count;
    pool_t    *pool;

    /* the shared block: eight bytes nothing is at, so that offset 0 is none; the entity index,
     * the persistent arena, the columns */
    uint8_t       *shared;
    size_t         shared_len, shared_reserve;
    bridge_entity_t *index;
    uint32_t       entities;
    uint32_t       arena, arena_len;
    uint32_t       columns, columns_len;
    /* [budget] sparse_sets, and where their indexes begin in the shared memory: one after
     * another, each a u32 for every entity there may be */
    uint32_t       sparse_sets, sparse;
    /* [budget] direct_indexed, and where their bits begin: one after another, each a bit for
     * every entity there may be, in 64-bit words */
    uint32_t       direct_indexed, direct;
    region_t         arena_space, column_space;   /* what of each is handed out */
    int            layout_moved;    /* a module came or went, or a declaration changed */
    uint32_t       tables;          /* [budget] tables: how many may exist at once */
    uint32_t       max_types, max_systems;      /* [budget] types, [budget] systems */
    uint32_t       max_terms;                   /* [budget] terms */

    storage_t          *storage;            /* who keeps the rows: storage.h */

    declared_type_t    **types;
    uint32_t       type_count;
    kept_text_t     *kept;            /* where an unloaded module's settings text was */
    uint32_t       kept_count;
    bridge_system_t **systems;        /* every module's, in registration order */
    uint32_t       system_count;
    module_t        *modules;
    uint32_t       module_count, max_modules;
};

/* bridge.c */
void say(bridge_t *b, const module_t *m, const bridge_system_t *s, uint8_t level, const char *fmt, ...);
void bridge_given_up(bridge_t *b, const module_t *m);     /* an unload of what stopped the bridge */
[[nodiscard]] bool abi_marks(module_t *m, uint32_t more);    /* false if there is no memory for it */
void abi_park(bridge_t *b, bridge_system_t *s);         /* out of storage, for a reload */
void abi_unpark(bridge_t *b, bridge_system_t *s);       /* and back, as it was */          /* room for a mark from every call it may yet take */
[[nodiscard]] char *copy(const char *s, size_t n);
/* 1, with a record, if nothing may run. `repairing` names a module being loaded or reloaded:
 * the one call a stopped bridge accepts is one that brings back the module that stopped it. */
[[nodiscard]] int bridge_stopped(bridge_t *b, const char *repairing);
void bridge_stop(bridge_t *b, const char *identity, const char *what);
void bridge_resume(bridge_t *b, const module_t *m);     /* if `m` is what it was waiting for */

/* lifecycle.c */
void module_discard(bridge_t *b, module_t *m);
void module_shut_down(bridge_t *b, module_t *m);    /* its shutdown, what it asked for, what it said */

/* config.c */
[[nodiscard]] int config_seed(bridge_t *b, module_t *m, const bridge_module_config_t *cfg, char *why, size_t len);
void config_keep(bridge_t *b, module_t *m);
void config_forget(bridge_t *b, const module_t *m);

/* schedule.c */
[[nodiscard]] int schedule_order(bridge_t *b, const bridge_order_t *order, uint32_t count);  /* at create */
[[nodiscard]] int schedule_build(bridge_t *b, char *why, size_t why_len);     /* 0, or why it cannot be */
[[nodiscard]] int schedule_room(bridge_t *b);     /* at create: what a dispatch works in */
/* failure.md, *Out of memory*: 1, with a record that says what for, if `got` is nullptr */
[[nodiscard]] int bridge_short(bridge_t *b, const void *got, const char *what);
void schedule_run(bridge_t *b, uint32_t phase);
void schedule_account(bridge_t *b);       /* at each barrier: the failure budgets, and the stops */

/* pool.c */
typedef void (*pool_fn_t)(void *job);
[[nodiscard]] pool_t *pool_create(uint32_t workers, uint32_t most_jobs);
void  pool_destroy(pool_t *p);
/* fn(jobs[i]) for every i, on the workers and the caller; a job marked is the caller's alone */
void  pool_run(pool_t *p, pool_fn_t fn, void **jobs, const uint8_t *caller_only, uint32_t count);

/* block.c */
[[nodiscard]] int block_create(bridge_t *b, const bridge_budget_t *budget);    /* 0, or a record was emitted */
void     block_destroy(bridge_t *b);
int      block_refill(bridge_t *b, module_t *m);                      /* 1 if the ceiling cut its ids short */
void     block_drain(bridge_t *b, module_t *m);                       /* its log ring, to the sink */
void     block_barrier(bridge_t *b);
void     block_header(bridge_t *b, module_t *m);                      /* what a module reads at offset 0 */
uint32_t block_offset(bridge_t *b, const void *p);                  /* into the shared memory, 0 for null */
/* a piece of a region: 0 if it has none that long; and a piece given back */
[[nodiscard]] int region_take(region_t *r, uint32_t len, uint32_t *at);
void     region_give(region_t *r, uint32_t at, uint32_t len);
[[nodiscard]] bool region_room(region_t *r, uint32_t ranges);     /* false if there is no memory */
/* the piece at `at` made longer where it stands: false, and nothing changed, if what follows it
 * is not free */
[[nodiscard]] bool region_grow(region_t *r, uint32_t at, uint32_t had, uint32_t want);
uint32_t region_most(const region_t *r);          /* the longest piece it could give now */
/* the column region, carved: what storage asks for is what it gets, to sixteen bytes.
 * `new_size` bytes holding what the `old_size` at `ptr` held, or fresh if `ptr` is null. Null,
 * and nothing changed, if the region cannot give them: nothing comes from outside it */
[[nodiscard]] void *block_column_regrow(bridge_t *b, void *ptr, uint32_t old_size, uint32_t new_size);
void     block_column_free(bridge_t *b, void *ptr, uint32_t size);

/* abi.c */
[[nodiscard]] int abi_define(module_t *m, backend_err_t *err);                      /* the eight imports */

#endif
