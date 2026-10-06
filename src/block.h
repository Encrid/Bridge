/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/block.h -- the shared block, the column region carved out of it, the
** log ring, and the barrier. Who keeps the rows is behind storage.h.
**
** memory.md, *Memory*
*/

#ifndef BRIDGE_SRC_BLOCK_H
    #define BRIDGE_SRC_BLOCK_H

    #include "inside.h"

/*
** Where each part of the shared block begins, how long the block is, and how
** much address space is reserved for it. Offset 0 is none, so nothing is
** there: eight bytes keep the entity index aligned. `sets` and `cells` are
** how many sparse sets and direct-indexed components there may be.
*/
typedef struct {
    uint64_t index;
    uint64_t sparse;
    uint64_t sets;
    uint64_t direct;
    uint64_t cells;
    uint64_t arena;
    uint64_t columns;
    uint64_t len;
    uint64_t reserve;
} block_layout_t;

/*
** block_create
**
** Makes the shared block a deployment's budget states, with storage in it,
** and has every backend adopt it. 0, or 1 and a record was emitted.
*/
[[nodiscard]] int block_create(bridge_t *b, const bridge_budget_t *budget);

/*
** block_destroy
**
** Gives the shared block back, with storage and every declared type.
*/
void block_destroy(bridge_t *b);

/*
** block_check_budget
**
** Refuses a budget with a key unset, a size an offset of 32 bits cannot
** reach, or a count that is neither a count nor none. 1 with a record.
*/
[[nodiscard]] int block_check_budget(bridge_t *b,
    const bridge_budget_t *budget);

/*
** block_lay_out
**
** Works out where each part of the shared block begins. 1, with a record, if
** together they are more than an offset of 32 bits can reach.
*/
[[nodiscard]] int block_lay_out(bridge_t *b, const bridge_budget_t *budget,
    block_layout_t *layout);

/*
** block_refill
**
** Tops up a module's ids for the time until the next barrier: what it did
** not use, up to what its config stated. They are real entities with nothing
** on them, which no query matches. 1 if the entity ceiling cut them short.
*/
int block_refill(bridge_t *b, module_t *m);

/*
** block_write_header
**
** Writes what a module reads at offset 0 of its private memory.
*/
void block_write_header(bridge_t *b, module_t *m);

/*
** block_get_offset
**
** Answers where a pointer is in the shared memory, or 0 for one that is
** null or not in it.
*/
uint32_t block_get_offset(bridge_t *b, const void *p);

/*
** block_drain
**
** Hands a module's log ring to the sink, and gives it its scratch arena
** back: a line's text lives there, so that comes after.
**
** log.md, *The log*
*/
void block_drain(bridge_t *b, module_t *m);

/*
** block_barrier
**
** The barrier: queued changes applied, spans republished, the observers of
** what changed run, the failure budgets settled, the log rings drained.
**
** schedule.md, *Phases*
*/
void block_barrier(bridge_t *b);

/*
** block_column_regrow
**
** Answers `new_size` bytes of the column region holding what the `old_size`
** at `ptr` held, or fresh if `ptr` is null. What storage asks for is what it
** gets, to sixteen bytes. Null, and nothing changed, if the region cannot
** give them: nothing comes from outside it.
*/
[[nodiscard]] void *block_column_regrow(bridge_t *b, void *ptr,
    uint32_t old_size, uint32_t new_size);

/*
** block_column_free
**
** Gives `size` bytes of the column region back.
*/
void block_column_free(bridge_t *b, void *ptr, uint32_t size);

#endif
