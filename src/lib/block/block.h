/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_BLOCK_H
    #define BRIDGE_SRC_BLOCK_H

    #include "inside.h"

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

[[nodiscard]] int block_create(bridge_t *b, const bridge_budget_t *budget);

void block_destroy(bridge_t *b);

[[nodiscard]] int block_check_budget(bridge_t *b,
    const bridge_budget_t *budget);

[[nodiscard]] int block_lay_out(bridge_t *b, const bridge_budget_t *budget,
    block_layout_t *layout);

int block_refill(bridge_t *b, module_t *m);

void block_write_header(bridge_t *b, module_t *m);

void block_drain(bridge_t *b, module_t *m);

void block_hand_out(bridge_t *b);

void block_barrier(bridge_t *b);

[[nodiscard]] void *block_column_regrow(bridge_t *b, void *ptr,
    uint32_t old_size, uint32_t new_size);

void block_column_free(bridge_t *b, void *ptr, uint32_t size);

#endif
