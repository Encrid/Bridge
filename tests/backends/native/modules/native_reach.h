/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_NATIVE_REACH_H
    #define TEST_NATIVE_REACH_H

    #include "bridge_native.h"

static inline bridge_entity_t *get_entity_index(const bridge_api_t *api)
{
    const bridge_header_t *h = bridge_native_get_header(api);

    return ((bridge_entity_t *)(api->shared_base + h->entity_index));
}

static inline const uint32_t *get_column_bases(const bridge_api_t *api)
{
    const bridge_header_t *h = bridge_native_get_header(api);

    return ((const uint32_t *)(api->private_base + h->column_base));
}

#endif
