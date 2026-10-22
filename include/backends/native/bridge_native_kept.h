/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_NATIVE_KEPT_H
    #define BRIDGE_NATIVE_KEPT_H

    #include "bridge_native_api.h"

static inline void *bridge_native_get_single(const bridge_api_t *api,
    uint32_t slot)
{
    const uint8_t *kept = api->private_base
        + bridge_native_get_header(api)->kept;
    uint32_t at = ((const uint32_t *)kept)[slot];

    return (at ? api->shared_base + at : nullptr);
}

static inline uint32_t bridge_native_get_cells(const bridge_api_t *api,
    uint32_t slot)
{
    const uint8_t *kept = api->private_base
        + bridge_native_get_header(api)->kept;

    return (((const uint32_t *)kept)[slot]);
}

static inline const uint32_t *bridge_native_get_sparse_index(
    const bridge_api_t *api, uint32_t slot)
{
    const uint8_t *kept = api->private_base
        + bridge_native_get_header(api)->kept;

    return ((const uint32_t *)(api->shared_base
        + ((const uint32_t *)kept)[slot]));
}

static inline uint64_t *bridge_native_get_bits(const bridge_api_t *api,
    uint32_t base)
{
    uint32_t at;

    memcpy(&at, api->shared_base + base - 16, 4);
    return ((uint64_t *)(api->shared_base + at));
}

static inline const bridge_list_t *bridge_native_get_list(
    const bridge_api_t *api, uint32_t observer)
{
    const uint8_t *lists = api->private_base
        + bridge_native_get_header(api)->observers;

    return ((const bridge_list_t *)lists + observer);
}

#endif
