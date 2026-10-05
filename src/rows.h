/* What Bridge-ASM700 asks of the thing that keeps its rows. docs/plan/PHASE10.md.
 *
 * Each entry is named for the request, as the backend seam's are, and not for how any storage
 * goes about it: at load a type is declared and a system's terms are handed over, at a barrier
 * the queues are applied and what a module reads is written, at unload a component is dropped.
 * How a barrier is worked is storage's own: src/rows.c says how, and why.
 *
 * Nothing here is reachable from a module or from a host. */
#ifndef BRIDGE_ROWS_H
#define BRIDGE_ROWS_H

#include "bridge-abi.h"

typedef struct bridge bridge;
typedef struct module module;

typedef struct rows rows;               /* one bridge's storage */
typedef struct rows_type rows_type;     /* a declared type */
typedef struct rows_terms rows_terms;   /* a system's terms, or an observer's, matched to tables */

/* nullptr if it cannot be made, and a record says why */
[[nodiscard]] rows *rows_create(bridge *b);
void rows_destroy(rows *r);

/* ---- at load ---- */

/* A type whose rows are `size` bytes. What it is made of is its declarer's to know: schema.h.
 * There is room for [budget] types of them, and whoever declares counts. */
[[nodiscard]] rows_type *rows_declare(rows *r, uint32_t size);
void rows_undeclare(rows *r, rows_type *type);

typedef struct {
    rows_type       *type;
    enum bridge_oper oper;      /* how it is touched is the scheduler's, and not storage's */
} rows_term;

/* who an observer is owed for, and how: one call a change, in the order they were applied */
typedef void (*rows_heard)(void *ctx, uint64_t id, enum bridge_event event);

/* There is room for [budget] systems of the two together, and whoever registers counts. */
[[nodiscard]] rows_terms *rows_system(rows *r, const rows_term *terms, uint32_t count);
[[nodiscard]] rows_terms *rows_observer(rows *r, const rows_term *terms, uint32_t count,
                                        enum bridge_event events, rows_heard heard, void *ctx);
void rows_forget(rows *r, rows_terms *terms);

/* ---- at a barrier ---- */

/* every module's queue, module by module in the order they loaded and within one in the order
 * it asked - threads.md, *Threads* */
void rows_apply(rows *r);
/* up to `want` ids with nothing on them, for a module to hand out: how many there were */
uint32_t rows_reserve(rows *r, uint64_t *ids, uint32_t want);
/* what a module reads: its spans, its column_base, its observers' lists. 1 if they do not fit */
[[nodiscard]] int rows_publish(rows *r, module *m);
/* once every module has been written for: what changed since the last time is forgotten */
void rows_written(rows *r);
/* how many table ids there are, counting 0, which is no table */
uint32_t rows_tables(rows *r);

/* ---- at unload, and for settings ---- */

/* the rows of a component nobody declares any more, by the policy its last declarer stated -
 * lifetime.md, *What unload leaves behind* */
void rows_drop(rows *r, rows_type *type, enum bridge_on_unload policy);
/* ids that were set aside and never handed out, or a settings row: gone, with what is on them */
void rows_release(rows *r, const uint64_t *ids, uint32_t count);
/* one entity carrying one row of `type`: its id, or 0 if the entity ceiling is reached */
[[nodiscard]] uint64_t rows_row(rows *r, rows_type *type, const void *bytes, uint32_t size);
bool rows_alive(rows *r, uint64_t id);

#endif
