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

typedef struct {
    uint32_t loud;
    uint32_t outside;
    uint32_t first;
} log_fault_t;

static inline uint32_t block_get_offset(bridge_world_t *w, const void *p)
{
    const uint8_t *c = p;

    if (!c || c < w->shared || c >= w->shared + w->shared_len)
        return (0);
    return ((uint32_t)(c - w->shared));
}

static inline void block_write_room(const module_t *m)
{
    bridge_header_t *h = (bridge_header_t *)m->priv;

    h->changes_left = m->queue_room - m->queue_len;
}

[[nodiscard]] int block_create(bridge_world_t *w,
    const bridge_budget_t *budget);

void block_destroy(bridge_world_t *w);

[[nodiscard]] int block_check_budget(bridge_world_t *w,
    const bridge_budget_t *budget);

[[nodiscard]] int block_lay_out(bridge_world_t *w,
    const bridge_budget_t *budget, block_layout_t *layout);

int block_refill(bridge_world_t *w, module_t *m);

int block_write_header(bridge_world_t *w, module_t *m);

void block_drain(bridge_world_t *w, module_t *m);

void block_hand_out(bridge_world_t *w);

void block_barrier(bridge_world_t *w);

[[nodiscard]] void *block_column_regrow(bridge_world_t *w, void *ptr,
    uint32_t old_size, uint32_t new_size);

void block_column_free(bridge_world_t *w, void *ptr, uint32_t size);

#endif
