/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/binding/test_module.h -- what a module of the tests is written
** with, beside bridge_abi.h: the table the tests' backend hands it, and the
** ABI layouts read through real pointers. It is the tests' own, and is
** handed to nobody.
**
** abi.md, *The ABI*
*/

#ifndef TEST_MODULE_H
    #define TEST_MODULE_H

    #include "test_module_api.h"
    #include "test_module_kept.h"
    #include "test_module_log.h"

/*
** An attach with its value, which is as long as the type was declared.
**
** abi.md, *The ABI*
*/
    #define TEST_ATTACH(api, id, slot, value) \
        (api)->attach(api, id, slot, \
            test_lend(api, &(value), sizeof(value)))

/*
** Registration, with names as literals. A type another declaration's fields
** use is declared before it, in the same namespace.
**
** schema.md, *What a module compiles against*
*/
    #define TEST_DECLARE(api, kind, name, T, on_unload, storage) \
        (api)->declare(api, TEST_STATE(api, bridge_declaration_t, \
            test_put(api, name, sizeof(name) - 1), sizeof(name) - 1, \
            kind, sizeof(T), alignof(T), \
            test_put(api, T##_desc, sizeof(T##_desc) - 1), \
            sizeof(T##_desc) - 1, on_unload, storage))
    #define TEST_COMPONENT(api, name, T, on_unload, storage) \
        TEST_DECLARE(api, BRIDGE_STRUCT, name, T, on_unload, storage)

/*
** A tag: a component with no bytes, so no type to take them from.
**
** storage.md, *A tag*
*/
    #define TEST_DECLARE_TAG(api, name, on_unload, storage) \
        (api)->declare(api, TEST_STATE(api, bridge_declaration_t, \
            test_put(api, name, sizeof(name) - 1), sizeof(name) - 1, \
            BRIDGE_TAG, 0, 1, 0, 0, on_unload, storage))

/*
** A system, and an observer. `changes` is the slots of the components it
** attaches or detaches, `change_count` of them: one that does neither hands
** nullptr and 0.
**
** abi.md, *The ABI*
*/
    #define TEST_ADD_SYSTEM(api, name, phase, terms, count, changes, \
        change_count) \
        (api)->add_system(api, TEST_STATE(api, \
            bridge_system_statement_t, \
            test_put(api, name, sizeof(name) - 1), sizeof(name) - 1, \
            test_put(api, phase, sizeof(phase) - 1), \
            sizeof(phase) - 1, \
            test_put(api, terms, (count) * sizeof(bridge_term_t)), \
            count, TEST_PUT_CHANGES(api, changes, change_count), \
            change_count))
    #define TEST_OBSERVER(api, name, events, terms, count, changes, \
        change_count) \
        (api)->observe(api, TEST_STATE(api, \
            bridge_observer_statement_t, \
            test_put(api, name, sizeof(name) - 1), sizeof(name) - 1, \
            events, \
            test_put(api, terms, (count) * sizeof(bridge_term_t)), \
            count, TEST_PUT_CHANGES(api, changes, change_count), \
            change_count))
    #define TEST_PUT_CHANGES(api, changes, count) \
        ((count) ? test_put(api, changes, \
            (count) * sizeof(uint32_t)) : 0)

/*
** test_get_query
**
** Answers a query's spans, as the barrier published them.
**
** abi.md, *The ABI*
*/
static inline const bridge_query_t *test_get_query(
    const test_api_t *api, uint32_t query)
{
    const uint8_t *queries = api->private_base
        + test_get_header(api)->queries;

    return ((const bridge_query_t *)queries + query);
}

/*
** test_get_span
**
** Answers span `i` of a query that declared `terms` terms: u32 count, u32
** entities, u32 base[terms].
**
** abi.md, *The ABI*
*/
static inline const uint32_t *test_get_span(const test_api_t *api,
    const bridge_query_t *q, uint32_t i, uint32_t terms)
{
    return ((const uint32_t *)(api->private_base + q->at
        + i * BRIDGE_SPAN_SIZE(terms)));
}

#endif
