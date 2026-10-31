/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_REACH_H
    #define TEST_REACH_H

    #include "test_module.h"

static inline bridge_entity_t *get_entity_index(const test_api_t *api)
{
    const bridge_header_t *h = test_get_header(api);

    return ((bridge_entity_t *)(api->shared_base + h->entity_index));
}

static inline const uint32_t *get_column_bases(const test_api_t *api)
{
    const bridge_header_t *h = test_get_header(api);

    return ((const uint32_t *)(api->private_base + h->column_base));
}

static inline const uint32_t *get_blocks(const test_api_t *api)
{
    const bridge_header_t *h = test_get_header(api);

    return ((const uint32_t *)(api->shared_base + h->blocks));
}

#endif
