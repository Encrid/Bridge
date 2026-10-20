/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_WASM_ENTITY_H
    #define BRIDGE_WASM_ENTITY_H

    #include "bridge_wasm_acc.h"

static inline uint64_t bridge_get_entity(uint32_t table, uint32_t row)
{
    return (acc_who(table, row));
}

static inline uint64_t bridge_find(uint64_t id)
{
    return (acc_entity(id));
}

static inline int bridge_is_alive(uint64_t id)
{
    return (id != 0 && acc_entity(id) != UINT64_MAX);
}

static inline uint32_t bridge_entity_table(uint64_t at)
{
    return ((uint32_t)(at >> 32));
}

static inline uint32_t bridge_entity_row(uint64_t at)
{
    return ((uint32_t)at);
}

#endif
