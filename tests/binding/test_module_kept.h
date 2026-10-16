/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_MODULE_KEPT_H
    #define TEST_MODULE_KEPT_H

    #include "test_module_api.h"

static inline void *test_get_single(const test_api_t *api,
    uint32_t slot)
{
    const uint8_t *kept = api->private_base
        + test_get_header(api)->kept;
    uint32_t at = ((const uint32_t *)kept)[slot];

    return (at ? api->shared_base + at : nullptr);
}

static inline uint32_t test_get_cells(const test_api_t *api,
    uint32_t slot)
{
    const uint8_t *kept = api->private_base
        + test_get_header(api)->kept;

    return (((const uint32_t *)kept)[slot]);
}

static inline const uint32_t *test_get_sparse_index(
    const test_api_t *api, uint32_t slot)
{
    const uint8_t *kept = api->private_base
        + test_get_header(api)->kept;

    return ((const uint32_t *)(api->shared_base
        + ((const uint32_t *)kept)[slot]));
}

static inline uint64_t *test_get_bits(const test_api_t *api,
    uint32_t base)
{
    uint32_t at;

    memcpy(&at, api->shared_base + base - 16, 4);
    return ((uint64_t *)(api->shared_base + at));
}

static inline const bridge_list_t *test_get_list(
    const test_api_t *api, uint32_t observer)
{
    const uint8_t *lists = api->private_base
        + test_get_header(api)->observers;

    return ((const bridge_list_t *)lists + observer);
}

#endif
