/* What Bridge-ASM700 asks of the thing that keeps its rows. docs/plan/STAGE10.md.
 *
 * Each entry is named for the request, as the backend seam's are, and not for how any storage
 * goes about it: at load a type is declared and a system's terms are handed over, at a barrier
 * the queues are applied and what a module reads is written, at unload a component is dropped.
 * How a barrier is worked is storage's own: src/storage.c says how, and why.
 *
 * Nothing here is reachable from a module or from a host. */
#ifndef BRIDGE_STORAGE_H
#define BRIDGE_STORAGE_H

#include "bridge-abi.h"

typedef struct bridge bridge;
typedef struct module module;

typedef struct storage storage;               /* one bridge's storage */
typedef struct storage_type storage_type;     /* a declared type */
typedef struct storage_terms storage_terms;   /* a system's terms, or an observer's, matched to tables */

/* nullptr if it cannot be made, and a record says why */
[[nodiscard]] storage *storage_create(bridge *b);
void storage_destroy(storage *r);

/* ---- at load ---- */

/* A type whose rows are `size` bytes. What it is made of is its declarer's to know: schema.h.
 * There is room for [budget] types of them, and whoever declares counts. */
[[nodiscard]] storage_type *storage_declare(storage *r, uint32_t size, uint32_t align);
void storage_undeclare(storage *r, storage_type *type);

typedef struct {
    storage_type       *type;
    enum bridge_oper oper;      /* how it is touched is the scheduler's, and not storage's */
} storage_term;

/* who an observer is owed for, and how: one call a change, in the order they were applied */
typedef void (*storage_heard)(void *ctx, uint64_t id, enum bridge_event event);

/* There is room for [budget] systems of the two together, and whoever registers counts. */
[[nodiscard]] storage_terms *storage_system(storage *r, const storage_term *terms, uint32_t count);
[[nodiscard]] storage_terms *storage_observer(storage *r, const storage_term *terms, uint32_t count,
                                        enum bridge_event events, storage_heard heard, void *ctx);
void storage_forget(storage *r, storage_terms *terms);

/* ---- at a barrier ---- */

/* every module's queue, module by module in the order they loaded and within one in the order
 * it asked - threads.md, *Threads* */
void storage_apply(storage *r);
/* `m` may write `type`: what gives it a stake in an entity that carries it - grants.md */
[[nodiscard]] bool storage_writable(storage *r, module *m);    /* its set, made: false if it cannot be */
void storage_writes(storage *r, module *m, storage_type *type);
/* up to `want` ids with nothing on them, for a module to hand out: how many there were */
uint32_t storage_reserve(storage *r, uint64_t *ids, uint32_t want);
/* what a module reads: its spans, its column_base, its observers' lists. 1 if they do not fit */
[[nodiscard]] int storage_publish(storage *r, module *m);
/* once every module has been written for: what changed since the last time is forgotten */
void storage_written(storage *r);
/* how many table ids there are, counting 0, which is no table */
uint32_t storage_tables(storage *r);

/* ---- at unload, and for settings ---- */

/* the rows of a component nobody declares any more, by the policy its last declarer stated -
 * lifetime.md, *What unload leaves behind* */
void storage_drop(storage *r, storage_type *type, enum bridge_on_unload policy);
/* ids that were set aside and never handed out, or a settings row: gone, with what is on them */
void storage_release(storage *r, const uint64_t *ids, uint32_t count);
/* one entity carrying one row of `type`: its id, or 0 if the entity ceiling is reached */
[[nodiscard]] uint64_t storage_row(storage *r, storage_type *type, const void *bytes, uint32_t size);
bool storage_alive(storage *r, uint64_t id);
/* whether `is` answers true for any row of `type`, handed the row's bytes */
bool storage_any(storage *r, storage_type *type, bool (*is)(void *ctx, const uint8_t *row), void *ctx);

#endif
