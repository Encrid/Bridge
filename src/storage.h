/* What Bridge-ASM700 asks of the thing that keeps its rows. docs/plan/stage10.md.
 *
 * Each entry is named for the request, as the backend seam's are, and not for how any storage
 * goes about it: at load a type is declared and a system's terms are handed over, at a barrier
 * the queues are applied and what a module reads is written, at unload a component is dropped.
 * How a barrier is worked is storage's own: src/storage.c says how, and why.
 *
 * Nothing here is reachable from a module or from a host. */
#ifndef BRIDGE_STORAGE_H
#define BRIDGE_STORAGE_H

#include "bridge_abi.h"

typedef struct bridge bridge_t;
typedef struct module module_t;

typedef struct storage storage_t;               /* one bridge's storage */
typedef struct storage_type storage_type_t;     /* a declared type */
typedef struct storage_terms storage_terms_t;   /* a system's terms, or an observer's, matched to tables */

/* nullptr if it cannot be made, and a record says why */
[[nodiscard]] storage_t *storage_create(bridge_t *b);
void storage_destroy(storage_t *r);

/* ---- at load ---- */

/* A type whose rows are `size` bytes, kept in the storage layout `kept`. What it is made of
 * is its declarer's to know: schema.h. There is room for [budget] types of them, and of those
 * for [budget] sparse_sets kept in a sparse set and [budget] direct_indexed kept that way:
 * whoever declares counts, and asks first how many of those are left. nullptr for one thing:
 * a direct-indexed type whose cells, or a singleton whose value, [budget] columns has no room
 * for. */
[[nodiscard]] storage_type_t *storage_declare(storage_t *r, uint32_t size, uint32_t align,
                                            enum bridge_storage kept);
uint32_t storage_sets_free(const storage_t *r);
uint32_t storage_direct_free(const storage_t *r);
void storage_undeclare(storage_t *r, storage_type_t *type);

typedef struct {
    storage_type_t       *type;
    enum bridge_oper oper;      /* how it is touched is the scheduler's, and not storage's */
} storage_term_t;

/* who an observer is owed for, and how: one call a change, in the order they were applied */
typedef void (*storage_heard_t)(void *ctx, uint64_t id, enum bridge_event event);

/* There is room for [budget] systems of the two together, and whoever registers counts. */
[[nodiscard]] storage_terms_t *storage_system(storage_t *r, const storage_term_t *terms, uint32_t count);
[[nodiscard]] storage_terms_t *storage_observer(storage_t *r, const storage_term_t *terms, uint32_t count,
                                        enum bridge_event events, storage_heard_t heard, void *ctx);
void storage_forget(storage_t *r, storage_terms_t *terms);

/* ---- at a barrier ---- */

/* every module's queue, module by module in the order they loaded and within one in the order
 * it asked - threads.md, *Threads* */
void storage_apply(storage_t *r);
/* `m` may write `type`: what gives it a stake in an entity that carries it - grants.md */
[[nodiscard]] bool storage_writable(storage_t *r, module_t *m);    /* its set, made: false if it cannot be */
void storage_writes(storage_t *r, module_t *m, storage_type_t *type);
/* up to `want` ids with nothing on them, for a module to hand out: how many there were */
uint32_t storage_reserve(storage_t *r, uint64_t *ids, uint32_t want);
/* what a module reads: its spans, its column_base, its observers' lists. 1 if they do not fit */
[[nodiscard]] int storage_publish(storage_t *r, module_t *m);
/* once every module has been written for: what changed since the last time is forgotten */
void storage_written(storage_t *r);
/* how many table ids there are, counting 0, which is no table */
uint32_t storage_tables(storage_t *r);

/* ---- at a reload: a component's rows written again, as another type ---- */

/* bytes `from` of a value as it was, which are bytes `to` of the value as it will be */
typedef struct { uint32_t from, to, len; } storage_run_t;
/* every value of `from` becomes a value of `to`: zero, but for the runs copied across */
typedef struct {
    storage_type_t      *from, *to;
    const storage_run_t *runs;
    uint32_t           run_count;
} storage_move_t;

/* how many entities carry `type`; and of a toggled one, how many of those are off */
uint32_t storage_count(storage_t *r, const storage_type_t *type);
uint32_t storage_off(storage_t *r, const storage_type_t *type);
/* lifetime.md, *When a component is moved*. All of them or none: every table and every row
 * the new types need is had before a value moves, and false, with nothing changed, if one
 * cannot be, or if there is not the memory to carry the values across. Observers are told
 * nothing, since no entity gains or loses a component by it. */
[[nodiscard]] bool storage_convert(storage_t *r, const storage_move_t *moves, uint32_t count);

/* ---- at unload, and for settings ---- */

/* the rows of a component nobody declares any more, by the policy its last declarer stated -
 * lifetime.md, *What unload leaves behind* */
void storage_drop(storage_t *r, storage_type_t *type, enum bridge_on_unload policy);
/* ids that were set aside and never handed out: gone, with what is on them */
void storage_release(storage_t *r, const uint64_t *ids, uint32_t count);
/* a singleton given its value, outside any barrier, and told to whoever observes it */
void storage_single(storage_t *r, storage_type_t *type, const void *bytes, uint32_t size);
bool storage_alive(storage_t *r, uint64_t id);
/* whether `is` answers true for any row of `type`, handed the row's bytes */
bool storage_any(storage_t *r, storage_type_t *type, bool (*is)(void *ctx, const uint8_t *row), void *ctx);

#endif
