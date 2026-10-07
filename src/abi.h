/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** src/abi.h -- the ABI: the eight functions a module imports.
**
** Every pointer argument is an offset into the caller's scratch arena. An
** entry that cannot be honoured says why in the module's record and answers
** non-zero, which abandons the invocation that made it.
**
** abi.md, *The ABI*
** budgets.md, *What running out does*
*/

#ifndef BRIDGE_SRC_ABI_H
    #define BRIDGE_SRC_ABI_H

    #include "inside.h"

/*
** One import of the ABI: its name, what answers it, and its signature.
*/
typedef struct {
    const char *name;
    backend_fn_t fn;
    enum backend_type params[3];
    uint32_t param_count;
    enum backend_type results[1];
    uint32_t result_count;
} abi_entry_t;

/*
** abi_refuse
**
** Refuses the call in progress, with why, written as printf would. The
** first refusal of a call is the reason: a backend that cannot unwind lets
** the module run on, and what it meets afterwards is consequence. Answers 1.
*/
[[gnu::format(printf, 2, 3)]] int abi_refuse(module_t *m, const char *format,
    ...);

/*
** abi_define
**
** Defines the eight imports of the ABI for a module's instance. 1, with
** `err`, if its backend refuses one.
*/
[[nodiscard]] int abi_define(module_t *m, backend_err_t *err);

/*
** abi_marks
**
** Makes room for a mark from each of a module's systems and observers, and
** from the lifecycle call in progress, and `more` beyond: asked for where a
** module gains a system and before a lifecycle call, so that queueing a
** change never has to. false if there is no memory for it.
*/
[[nodiscard]] bool abi_marks(module_t *m, uint32_t more);

/*
** abi_park
**
** Takes a system out of storage, for a reload. A reload does not need room
** for a module's systems twice: the old instance's leave storage before the
** new one's are registered, and count for nothing until they are back or
** gone.
**
** budgets.md, *Budgets*
*/
void abi_park(bridge_t *b, bridge_system_t *s);

/*
** abi_unpark
**
** Puts a parked system back in storage, as it was: the places it had are
** free again, since whatever took them has been dropped.
*/
void abi_unpark(bridge_t *b, bridge_system_t *s);

/*
** abi_hear
**
** What storage calls for each change an observer is owed: it is added to
** who triggered the observer since the last barrier.
*/
void abi_hear(void *ctx, uint64_t id, enum bridge_event event);

/*
** The entries a module calls at load: declare(at) -> slot, add_system(at) ->
** slot, observe(at) -> slot. Each is handed where one struct is in the
** scratch arena.
*/
int abi_declare(void *ud, const uint64_t *at, uint64_t *r);
int abi_add_system(void *ud, const uint64_t *at, uint64_t *r);
int abi_observe(void *ud, const uint64_t *at, uint64_t *r);

/*
** The entries a module calls in a phase: spawn() -> id, destroy(id),
** attach(id, slot, ptr), detach(id, slot), alloc(len) -> offset. Nothing in
** them touches storage. Modules run at once, and what one asks for must not
** depend on who ran beside it: an id comes from a block set aside for the
** module, arena space from its own share, and a change goes on its own
** queue, which the barrier applies in an order that never varies. No lock:
** a module's systems are serialised, so each of the three has one writer.
**
** schedule.md, *What a pass depends on*
*/
int abi_spawn(void *ud, const uint64_t *a, uint64_t *r);
int abi_destroy(void *ud, const uint64_t *a, uint64_t *r);
int abi_attach(void *ud, const uint64_t *a, uint64_t *r);
int abi_detach(void *ud, const uint64_t *a, uint64_t *r);
int abi_alloc(void *ud, const uint64_t *a, uint64_t *r);

#endif
