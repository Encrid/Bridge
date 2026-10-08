/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** backends/native/sdk/bridge_native.h -- what a native module compiles
** against, beside bridge_abi.h: the table its backend hands it, and the
** same ABI layouts read through real pointers. Only a native module and the
** native backend compile this header.
**
** What is compiled with this file is a work based on Bridge and is under
** the same licence. Other terms: docs/licence.md.
**
** native.md, *What the backend provides*
*/

#ifndef BRIDGE_NATIVE_H
    #define BRIDGE_NATIVE_H

    #include "bridge_native_api.h"
    #include "bridge_native_kept.h"
    #include "bridge_native_log.h"

/*
** An attach with its value, which is as long as the type was declared.
**
** abi.md, *The ABI*
*/
    #define BRIDGE_NATIVE_ATTACH(api, id, slot, value) \
        (api)->attach(api, id, slot, \
            bridge_native_lend(api, &(value), sizeof(value)))

/*
** Registration, with names as literals. A type another declaration's fields
** use is declared before it, in the same namespace.
**
** schema.md, *What a module compiles against*
*/
    #define BRIDGE_NATIVE_DECLARE(api, kind, name, T, on_unload, storage) \
        (api)->declare(api, BRIDGE_NATIVE_STATE(api, bridge_declaration_t, \
            bridge_native_put(api, name, sizeof(name) - 1), sizeof(name) - 1, \
            kind, sizeof(T), alignof(T), \
            bridge_native_put(api, T##_desc, sizeof(T##_desc) - 1), \
            sizeof(T##_desc) - 1, on_unload, storage))
    #define BRIDGE_NATIVE_COMPONENT(api, name, T, on_unload, storage) \
        BRIDGE_NATIVE_DECLARE(api, BRIDGE_STRUCT, name, T, on_unload, storage)

/*
** A tag: a component with no bytes, so no type to take them from.
**
** storage.md, *A tag*
*/
    #define BRIDGE_NATIVE_DECLARE_TAG(api, name, on_unload, storage) \
        (api)->declare(api, BRIDGE_NATIVE_STATE(api, bridge_declaration_t, \
            bridge_native_put(api, name, sizeof(name) - 1), sizeof(name) - 1, \
            BRIDGE_TAG, 0, 1, 0, 0, on_unload, storage))

/*
** A system, and an observer. `changes` is the slots of the components it
** attaches or detaches, `change_count` of them: one that does neither hands
** nullptr and 0.
**
** abi.md, *The ABI*
*/
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

/*
** bridge_native_get_query
**
** Answers a query's spans, as the barrier published them.
**
** native.md, *Reaching a component*
*/
static inline const bridge_query_t *bridge_native_get_query(
    const bridge_api_t *api, uint32_t query)
{
    const uint8_t *queries = api->private_base
        + bridge_native_get_header(api)->queries;

    return ((const bridge_query_t *)queries + query);
}

/*
** bridge_native_get_span
**
** Answers span `i` of a query that declared `terms` terms: u32 count, u32
** entities, u32 base[terms].
**
** native.md, *Reaching a component*
*/
static inline const uint32_t *bridge_native_get_span(const bridge_api_t *api,
    const bridge_query_t *q, uint32_t i, uint32_t terms)
{
    return ((const uint32_t *)(api->private_base + q->at
        + i * BRIDGE_SPAN_SIZE(terms)));
}

#endif
