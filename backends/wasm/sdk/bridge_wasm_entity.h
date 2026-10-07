/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** backends/wasm/sdk/bridge_wasm_entity.h -- an entity: its id in a row, and
** a reference to it followed, which is arithmetic and not a call.
**
** What is compiled with this file is a work based on Bridge and is under
** the same licence. Other terms: docs/licence.md.
**
** reference.md, *Reference following*
*/

#ifndef BRIDGE_WASM_ENTITY_H
    #define BRIDGE_WASM_ENTITY_H

    #include "bridge_wasm_acc.h"

/*
** bridge_get_entity
**
** Answers the id of row `row` of `count`, of the ids at `entities`.
**
** query.md, *The interface is a query*
*/
static inline uint64_t bridge_get_entity(uint32_t entities, uint32_t row,
    uint32_t count)
{
    return (acc_ld_64(acc_row(entities, row, count, 8u)));
}

/*
** bridge_get_ref
**
** Answers where component `slot` of entity `id` is, or zero: a stale id, or
** an entity without it.
**
** reference.md, *Reference following*
*/
static inline uint32_t bridge_get_ref(uint64_t id, uint32_t slot,
    uint32_t stride)
{
    constexpr uint32_t size = sizeof(bridge_entity_t);
    uint32_t index = (uint32_t)id;
    uint32_t record;
    uint32_t base;

    if (index >= BRIDGE_HEADER(entities))
        return (0);
    record = BRIDGE_HEADER(entity_index) + index * size;
    if (acc_ld_32(record + 8u) != (uint32_t)(id >> 32))
        return (0);
    base = acc_ld_32(record) * BRIDGE_HEADER(types) + slot;
    base = acc_own_32(BRIDGE_HEADER(column_base) + base * 4u);
    return (base ? base + acc_ld_32(record + 4u) * stride : 0);
}

/*
** bridge_is_alive
**
** Answers whether `id` still names an entity: one that was destroyed, or
** never spawned, does not.
**
** reference.md, *Reference following*
*/
static inline int bridge_is_alive(uint64_t id)
{
    constexpr uint32_t size = sizeof(bridge_entity_t);
    uint32_t index = (uint32_t)id;
    uint32_t record;

    if (!id || index >= BRIDGE_HEADER(entities))
        return (0);
    record = BRIDGE_HEADER(entity_index) + index * size;
    return (acc_ld_32(record + 8u) == (uint32_t)(id >> 32));
}

#endif
