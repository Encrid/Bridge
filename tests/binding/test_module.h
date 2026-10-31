/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_MODULE_H
    #define TEST_MODULE_H

    #include "test_module_api.h"
    #include "test_module_kept.h"
    #include "test_module_log.h"

    #define TEST_ATTACH(api, id, slot, value) \
        (api)->attach(api, id, slot, \
            test_lend(api, &(value), sizeof(value)))

    #define TEST_DECLARE_AS(api, kind, name, T, on_unload, storage, \
        on_destroy) \
        (api)->declare(api, TEST_STATE(api, bridge_declaration_t, \
            test_put(api, name, sizeof(name) - 1), sizeof(name) - 1, \
            kind, sizeof(T), alignof(T), \
            test_put(api, T##_desc, sizeof(T##_desc) - 1), \
            sizeof(T##_desc) - 1, on_unload, storage, on_destroy))
    #define TEST_DECLARE(api, kind, name, T, on_unload, storage) \
        TEST_DECLARE_AS(api, kind, name, T, on_unload, storage, \
            BRIDGE_ON_DESTROY_REMOVE)
    #define TEST_COMPONENT(api, name, T, on_unload, storage) \
        TEST_DECLARE(api, BRIDGE_STRUCT, name, T, on_unload, storage)

    #define TEST_DECLARE_TAG(api, name, on_unload, storage) \
        (api)->declare(api, TEST_STATE(api, bridge_declaration_t, \
            test_put(api, name, sizeof(name) - 1), sizeof(name) - 1, \
            BRIDGE_TAG, 0, 1, 0, 0, on_unload, storage, \
            BRIDGE_ON_DESTROY_REMOVE))

    #define TEST_ADD_SYSTEM(api, name, phase, terms, count, changes, \
        change_count, before, before_count) \
        (api)->add_system(api, TEST_STATE(api, \
            bridge_system_statement_t, \
            test_put(api, name, sizeof(name) - 1), sizeof(name) - 1, \
            test_put(api, phase, sizeof(phase) - 1), \
            sizeof(phase) - 1, \
            test_put(api, terms, (count) * sizeof(bridge_term_t)), \
            count, TEST_PUT_SLOTS(api, changes, change_count), \
            change_count, TEST_PUT_SLOTS(api, before, before_count), \
            before_count))
    #define TEST_OBSERVER(api, name, events, terms, count, changes, \
        change_count, before, before_count) \
        (api)->observe(api, TEST_STATE(api, \
            bridge_observer_statement_t, \
            test_put(api, name, sizeof(name) - 1), sizeof(name) - 1, \
            events, \
            test_put(api, terms, (count) * sizeof(bridge_term_t)), \
            count, TEST_PUT_SLOTS(api, changes, change_count), \
            change_count, TEST_PUT_SLOTS(api, before, before_count), \
            before_count))
    #define TEST_PUT_SLOTS(api, slots, count) \
        ((count) ? test_put(api, slots, \
            (count) * sizeof(uint32_t)) : 0)

static inline const bridge_query_t *test_get_query(
    const test_api_t *api, uint32_t query)
{
    const uint8_t *queries = api->private_base
        + test_get_header(api)->queries;

    return ((const bridge_query_t *)queries + query);
}

static inline uint32_t test_count_rows(const test_api_t *api,
    const uint32_t *span)
{
    const uint32_t *rows = (const uint32_t *)(api->shared_base
        + test_get_header(api)->rows);

    return (rows[span[0]]);
}

static inline const uint32_t *test_get_span(const test_api_t *api,
    const bridge_query_t *q, uint32_t i, [[maybe_unused]] uint32_t terms)
{
    return ((const uint32_t *)(api->private_base + q->at
        + i * BRIDGE_SPAN_SIZE));
}

#endif
