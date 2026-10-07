/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/storage.h -- what Bridge-ASM700 asks of the thing that keeps its rows.
**
** Each entry is named for the request, as the backend seam's are, and not
** for how any storage goes about it: at load a type is declared and a
** system's terms are handed over, at a barrier the queues are applied and
** what a module reads is written, at unload a component is dropped. How a
** barrier is worked is storage's own.
**
** Nothing here is reachable from a module or from a host.
**
** storage.md, *What is asked of it*
*/

#ifndef BRIDGE_SRC_STORAGE_H
    #define BRIDGE_SRC_STORAGE_H

    #include "bridge_abi.h"

typedef struct bridge bridge_t;
typedef struct module module_t;
typedef struct storage storage_t;
typedef struct storage_type storage_type_t;
typedef struct storage_terms storage_terms_t;

/*
** A term, as storage is handed it. How it is touched is the scheduler's,
** and not storage's.
*/
typedef struct {
    storage_type_t *type;
    enum bridge_oper oper;
} storage_term_t;

/*
** Who an observer is owed for, and how: one call a change, in the order they
** were applied.
*/
typedef void (*storage_heard_t)(void *ctx, uint64_t id,
    enum bridge_event event);

/*
** Who is told what an observer is owed: the events it asked for, and what
** is called with them.
*/
typedef struct {
    enum bridge_event events;
    storage_heard_t heard;
    void *ctx;
} storage_listener_t;

/*
** Bytes `from` of a value as it was, which are bytes `to` of the value as
** it will be.
*/
typedef struct {
    uint32_t from;
    uint32_t to;
    uint32_t len;
} storage_run_t;

/*
** Every value of `from` becomes a value of `to`: zero, but for the runs
** copied across.
*/
typedef struct {
    storage_type_t *from;
    storage_type_t *to;
    const storage_run_t *runs;
    uint32_t run_count;
} storage_move_t;

typedef bool (*storage_is_fn_t)(void *ctx, const uint8_t *row);
typedef void (*storage_visit_fn_t)(storage_t *r, uint32_t index, void *ctx);

/*
** storage_create
**
** Makes a bridge's storage, sized by its budget. nullptr if it cannot be
** made, and a record says why.
*/
[[nodiscard]] storage_t *storage_create(bridge_t *b);

/*
** storage_destroy
**
** Frees a bridge's storage.
*/
void storage_destroy(storage_t *r);

/*
** storage_declare
**
** Declares a type whose rows are `size` bytes, kept in the storage layout
** `kept`. What it is made of is its declarer's to know. There is room for
** [budget] types of them, and of those for [budget] sparse_sets kept in a
** sparse set and [budget] direct_indexed kept that way: whoever declares
** counts, and asks first how many of those are left. nullptr for one thing:
** a direct-indexed type whose cells, or a singleton whose value, [budget]
** columns has no room for.
*/
[[nodiscard]] storage_type_t *storage_declare(storage_t *r, uint32_t size,
    uint32_t align, enum bridge_storage kept);

/*
** storage_count_free_sets
**
** Answers how many more types may be kept in a sparse set.
*/
uint32_t storage_count_free_sets(const storage_t *r);

/*
** storage_count_free_direct
**
** Answers how many more types may be kept direct-indexed.
*/
uint32_t storage_count_free_direct(const storage_t *r);

/*
** storage_undeclare
**
** Forgets a type nobody declares any more.
*/
void storage_undeclare(storage_t *r, storage_type_t *type);

/*
** storage_add_system
**
** Hands over a system's terms. There is room for [budget] systems of systems
** and observers together, and whoever registers counts.
*/
[[nodiscard]] storage_terms_t *storage_add_system(storage_t *r,
    const storage_term_t *terms, uint32_t count);

/*
** storage_add_observer
**
** Hands over an observer's terms, and who is told what it is owed.
*/
[[nodiscard]] storage_terms_t *storage_add_observer(storage_t *r,
    const storage_term_t *terms, uint32_t count,
    const storage_listener_t *listener);

/*
** storage_forget
**
** Forgets a system's terms, or an observer's.
*/
void storage_forget(storage_t *r, storage_terms_t *terms);

/*
** storage_apply
**
** Applies every module's queue, in the order the calls that filled them
** ran, and within one in the order it asked.
**
** threads.md, *Threads*
*/
void storage_apply(storage_t *r);

/*
** storage_make_writable
**
** Makes the set of the types a module may write. false if there was not the
** memory for it: asked before a type is made, so that storage_let_write
** cannot fail.
*/
[[nodiscard]] bool storage_make_writable(storage_t *r, module_t *m);

/*
** storage_let_write
**
** Records that a module may write a type: what gives it a stake in an
** entity that carries it.
**
** grants.md, *What a module may destroy*
*/
void storage_let_write(storage_t *r, module_t *m, storage_type_t *type);

/*
** storage_reserve
**
** Takes up to `want` ids with nothing on them, for a module to hand out.
** Answers how many there were.
*/
uint32_t storage_reserve(storage_t *r, uint64_t *ids, uint32_t want);

/*
** storage_publish
**
** Writes what a module reads: its spans, its column_base, its observers'
** lists. 1 if they do not fit.
*/
[[nodiscard]] int storage_publish(storage_t *r, module_t *m);

/*
** storage_end_publish
**
** Forgets what changed since the last time, once every module has been
** written for.
*/
void storage_end_publish(storage_t *r);

/*
** storage_count_tables
**
** Answers how many table ids there are, counting 0, which is no table.
*/
uint32_t storage_count_tables(storage_t *r);

/*
** storage_count_carriers
**
** Answers how many entities carry a type.
*/
uint32_t storage_count_carriers(storage_t *r, const storage_type_t *type);

/*
** storage_count_off
**
** Answers how many of the entities that carry a toggled type have it off.
*/
uint32_t storage_count_off(storage_t *r, const storage_type_t *type);

/*
** storage_visit_carriers
**
** Calls `visit` for every entity that carries a type, whichever way it is
** kept.
*/
void storage_visit_carriers(storage_t *r, const storage_type_t *type,
    storage_visit_fn_t visit, void *ctx);

/*
** storage_convert
**
** Writes the rows of components again, as other types. All of them or none:
** every table and every row the new types need is had before a value moves,
** and false, with nothing changed, if one cannot be, or if there is not the
** memory to carry the values across. Observers are told nothing, since no
** entity gains or loses a component by it.
**
** lifetime.md, *When a component is moved*
*/
[[nodiscard]] bool storage_convert(storage_t *r, const storage_move_t *moves,
    uint32_t count);

/*
** storage_drop
**
** Drops the rows of a component nobody declares any more, by the policy its
** last declarer stated.
**
** lifetime.md, *What unload leaves behind*
*/
void storage_drop(storage_t *r, storage_type_t *type,
    enum bridge_on_unload policy);

/*
** storage_release
**
** Destroys ids that were set aside and never handed out, with what is on
** them.
*/
void storage_release(storage_t *r, const uint64_t *ids, uint32_t count);

/*
** storage_give_single
**
** Gives a singleton its value, outside any barrier, and tells whoever
** observes it.
*/
void storage_give_single(storage_t *r, storage_type_t *type,
    const void *bytes, uint32_t size);

/*
** storage_is_alive
**
** Answers whether an id names an entity there is.
*/
bool storage_is_alive(storage_t *r, uint64_t id);

/*
** storage_has_any
**
** Answers whether `is` answers true for any row of a type, handed the row's
** bytes.
*/
bool storage_has_any(storage_t *r, storage_type_t *type, storage_is_fn_t is,
    void *ctx);

#endif
