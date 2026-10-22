/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_NATIVE_H
    #define BRIDGE_NATIVE_H

    #include "bridge_native_api.h"
    #include "bridge_native_kept.h"
    #include "bridge_native_log.h"

    #define BRIDGE_NATIVE_ATTACH(api, id, slot, value) \
        (api)->attach(api, id, slot, \
            bridge_native_lend(api, &(value), sizeof(value)))

    #define BRIDGE_NATIVE_DECLARE(api, kind, name, T, on_unload, storage) \
        (api)->declare(api, BRIDGE_NATIVE_STATE(api, bridge_declaration_t, \
            bridge_native_put(api, name, sizeof(name) - 1), sizeof(name) - 1, \
            kind, sizeof(T), alignof(T), \
            bridge_native_put(api, T##_desc, sizeof(T##_desc) - 1), \
            sizeof(T##_desc) - 1, on_unload, storage))
    #define BRIDGE_NATIVE_COMPONENT(api, name, T, on_unload, storage) \
        BRIDGE_NATIVE_DECLARE(api, BRIDGE_STRUCT, name, T, on_unload, storage)

    #define BRIDGE_NATIVE_DECLARE_TAG(api, name, on_unload, storage) \
        (api)->declare(api, BRIDGE_NATIVE_STATE(api, bridge_declaration_t, \
            bridge_native_put(api, name, sizeof(name) - 1), sizeof(name) - 1, \
            BRIDGE_TAG, 0, 1, 0, 0, on_unload, storage))

    #define BRIDGE_NATIVE_ADD_SYSTEM(api, name, phase, terms, count, changes, \
        change_count) \
        (api)->add_system(api, BRIDGE_NATIVE_STATE(api, \
            bridge_system_statement_t, \
            bridge_native_put(api, name, sizeof(name) - 1), sizeof(name) - 1, \
            bridge_native_put(api, phase, sizeof(phase) - 1), \
            sizeof(phase) - 1, \
            bridge_native_put(api, terms, (count) * sizeof(bridge_term_t)), \
            count, BRIDGE_NATIVE_PUT_CHANGES(api, changes, change_count), \
            change_count))
    #define BRIDGE_NATIVE_OBSERVER(api, name, events, terms, count, changes, \
        change_count) \
        (api)->observe(api, BRIDGE_NATIVE_STATE(api, \
            bridge_observer_statement_t, \
            bridge_native_put(api, name, sizeof(name) - 1), sizeof(name) - 1, \
            events, \
            bridge_native_put(api, terms, (count) * sizeof(bridge_term_t)), \
            count, BRIDGE_NATIVE_PUT_CHANGES(api, changes, change_count), \
            change_count))
    #define BRIDGE_NATIVE_PUT_CHANGES(api, changes, count) \
        ((count) ? bridge_native_put(api, changes, \
            (count) * sizeof(uint32_t)) : 0)

static inline const bridge_query_t *bridge_native_get_query(
    const bridge_api_t *api, uint32_t query)
{
    const uint8_t *queries = api->private_base
        + bridge_native_get_header(api)->queries;

    return ((const bridge_query_t *)queries + query);
}

static inline const uint32_t *bridge_native_get_span(const bridge_api_t *api,
    const bridge_query_t *q, uint32_t i, [[maybe_unused]] uint32_t terms)
{
    return ((const uint32_t *)(api->private_base + q->at
        + i * BRIDGE_SPAN_SIZE));
}

#endif
