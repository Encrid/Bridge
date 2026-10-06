/* What a native module compiles against, beside bridge_abi.h: the table its backend hands it,
 * and the same ABI layouts read through real pointers. docs/backends/native/native.md.
 *
 * Only a native module and the native backend compile this header. */
#ifndef BRIDGE_NATIVE_H
#define BRIDGE_NATIVE_H

#include "bridge_abi.h"
#include "bridge_schema.h"       /* how a type is stated: the same on every backend */
#include <stddef.h>
#include <string.h>

#ifdef _WIN32
#define BRIDGE_NATIVE_EXPORT [[gnu::dllexport]]
#else
#define BRIDGE_NATIVE_EXPORT [[gnu::visibility("default")]]
#endif

/* What the native backend hands a native module: the eight entries of docs/modules/abi.md and
 * where the two memories begin. It is passed to every export, and every entry takes it first,
 * so nothing is found by who happens to be running. Every pointer argument of an entry is still
 * an offset into the scratch arena: that is Bridge-ASM700's, and the same on every backend.
 *
 * An entry may be called only from inside a call the backend made, on the thread it made it
 * on: init, a system, an observer. That is native.md, *The invariant*, and the backend checks
 * it. */
typedef struct bridge_api bridge_api_t;
struct bridge_api {
    uint32_t (*declare)(const bridge_api_t *api, uint32_t at);
    uint32_t (*add_system)(const bridge_api_t *api, uint32_t at);
    uint32_t (*observe)(const bridge_api_t *api, uint32_t at);
    uint64_t (*spawn)(const bridge_api_t *api);
    void     (*destroy)(const bridge_api_t *api, uint64_t id);
    void     (*attach)(const bridge_api_t *api, uint64_t id, uint32_t slot, uint32_t ptr);
    void     (*detach)(const bridge_api_t *api, uint64_t id, uint32_t slot);
    uint32_t (*alloc)(const bridge_api_t *api, uint32_t len);
    uint8_t  *shared_base;          /* the shared memory. it does not move */
    uint8_t  *private_base;         /* this module's private memory */
};

/* What a native module exports: a function of this name, handed the table as `api`. A system
 * called "move" is the export bridge_move. */
#define BRIDGE_NATIVE(name) \
    BRIDGE_NATIVE_EXPORT int bridge_##name([[maybe_unused]] const bridge_api_t *api)

/* ---- the scratch arena ---- */

static inline bridge_header_t *bridge_native_get_header(const bridge_api_t *api)
{
    return (bridge_header_t *)api->private_base;
}

/* `len` bytes copied into the scratch arena: where they are, or 0 if they do not fit, which
 * every entry refuses */
static inline uint32_t bridge_native_put(const bridge_api_t *api, const void *bytes, uint32_t len)
{
    bridge_header_t *h = bridge_native_get_header(api);
    uint32_t used = (h->scratch_used + 7u) & ~7u;
    if (used > h->scratch_len || len > h->scratch_len - used) return 0;
    memcpy(api->private_base + h->scratch + used, bytes, len);
    h->scratch_used = used + len;
    return h->scratch + used;
}

/* `len` bytes copied above what the scratch arena has in use, and not counted as used: where
 * they are, or 0 if they do not fit. For a value an entry copies during the call, as attach
 * does, so that a module's scratch holds one such value at a time and not a phase's worth. */
static inline uint32_t bridge_native_lend(const bridge_api_t *api, const void *bytes, uint32_t len)
{
    bridge_header_t *h = bridge_native_get_header(api);
    uint32_t used = (h->scratch_used + 7u) & ~7u;
    if (used > h->scratch_len || len > h->scratch_len - used) return 0;
    memcpy(api->private_base + h->scratch + used, bytes, len);
    return h->scratch + used;
}

/* An attach with its value, which is as long as the type was declared. */
#define BRIDGE_NATIVE_ATTACH(api, id, slot, value) \
    (api)->attach(api, id, slot, bridge_native_lend(api, &(value), sizeof(value)))

#define BRIDGE_NATIVE_STATE(api, T, ...) \
    bridge_native_put(api, &(T){ __VA_ARGS__ }, sizeof(T))

#define BRIDGE_NATIVE_DECLARE(api, kind, name, T, on_unload, storage) \
    (api)->declare(api, BRIDGE_NATIVE_STATE(api, bridge_declaration_t, \
        bridge_native_put(api, name, sizeof name - 1), sizeof name - 1, kind, sizeof(T), \
        alignof(T), bridge_native_put(api, T##_desc, sizeof T##_desc - 1), \
        sizeof T##_desc - 1, on_unload, storage))
#define BRIDGE_NATIVE_COMPONENT(api, name, T, on_unload, storage) \
    BRIDGE_NATIVE_DECLARE(api, BRIDGE_STRUCT, name, T, on_unload, storage)

/* a tag: a component with no bytes, so no type to take them from - storage.md, *A tag* */
#define BRIDGE_NATIVE_DECLARE_TAG(api, name, on_unload, storage) \
    (api)->declare(api, BRIDGE_NATIVE_STATE(api, bridge_declaration_t, \
        bridge_native_put(api, name, sizeof name - 1), sizeof name - 1, BRIDGE_TAG, 0, 1, 0, 0, \
        on_unload, storage))

/* `changes` is the slots of the components it attaches or detaches, `change_count` of them:
 * a system that does neither hands nullptr and 0 */
#define BRIDGE_NATIVE_ADD_SYSTEM(api, name, phase, terms, count, changes, change_count) \
    (api)->add_system(api, BRIDGE_NATIVE_STATE(api, bridge_system_statement_t, \
        bridge_native_put(api, name, sizeof name - 1), sizeof name - 1, \
        bridge_native_put(api, phase, sizeof phase - 1), sizeof phase - 1, \
        bridge_native_put(api, terms, (count) * sizeof(bridge_term_t)), count, \
        (change_count) ? bridge_native_put(api, changes, (change_count) * sizeof(uint32_t)) : 0, \
        change_count))

#define BRIDGE_NATIVE_OBSERVER(api, name, events, terms, count, changes, change_count) \
    (api)->observe(api, BRIDGE_NATIVE_STATE(api, bridge_observer_statement_t, \
        bridge_native_put(api, name, sizeof name - 1), sizeof name - 1, events, \
        bridge_native_put(api, terms, (count) * sizeof(bridge_term_t)), count, \
        (change_count) ? bridge_native_put(api, changes, (change_count) * sizeof(uint32_t)) : 0, \
        change_count))

/* ---- a query's spans, through real pointers. native.md, *Reaching a component* ---- */

static inline const bridge_query_t *bridge_native_get_query(const bridge_api_t *api, uint32_t query)
{
    return (const bridge_query_t *)(api->private_base + bridge_native_get_header(api)->queries) + query;
}

/* span `i` of a query that declared `terms` terms: u32 count, u32 entities, u32 base[terms] */
static inline const uint32_t *bridge_native_get_span(const bridge_api_t *api, const bridge_query_t *q,
                                                 uint32_t i, uint32_t terms)
{
    return (const uint32_t *)(api->private_base + q->at + i * BRIDGE_SPAN_SIZE(terms));
}

/* ---- a singleton: one value, on no entity. storage.md, *A singleton* ---- */

/* The value of the singleton `slot`, or nullptr while it has none. It is given by an attach
 * with the id 0, taken by a detach with it, and written where it is by a system that stated
 * the term BRIDGE_WRITE. */
static inline void *bridge_native_get_single(const bridge_api_t *api, uint32_t slot)
{
    uint32_t at = ((const uint32_t *)(api->private_base + bridge_native_get_header(api)->kept))[slot];
    return at ? api->shared_base + at : nullptr;
}

/* ---- direct-indexed: a cell for every entity. storage.md, *Direct-indexed* ---- */

/* The cells of the direct-indexed component `slot`, from the shared base: the value of the
 * entity whose id's low 32 bits are e is cell e. Whether that entity has been given one is its
 * bit, bit e & 63 of word e >> 6 of bridge_native_get_bits of the same offset. */
static inline uint32_t bridge_native_get_cells(const bridge_api_t *api, uint32_t slot)
{
    return ((const uint32_t *)(api->private_base + bridge_native_get_header(api)->kept))[slot];
}

/* ---- a sparse set, reached from a row. storage.md, *A sparse set* ---- */

/* The index of the sparse set that keeps component `slot`: a cell for each entity, by the low
 * 32 bits of its id, holding where that entity's value is from the shared base, and zero
 * where it has none. */
static inline const uint32_t *bridge_native_get_sparse_index(const bridge_api_t *api, uint32_t slot)
{
    auto kept = (const uint32_t *)(api->private_base + bridge_native_get_header(api)->kept);
    return (const uint32_t *)(api->shared_base + kept[slot]);
}

/* ---- a toggled component: a bit a row. storage.md, *A toggle* ---- */

/* The bits of the toggled column that begins at `base`, which a span or column_base gave: row
 * r is bit r & 63 of word r >> 6, and a row whose bit is 0 is off. A system that stated the
 * term BRIDGE_WRITE turns a row on or off by writing its bit. */
static inline uint64_t *bridge_native_get_bits(const bridge_api_t *api, uint32_t base)
{
    uint32_t at;
    memcpy(&at, api->shared_base + base - 16, 4);
    return (uint64_t *)(api->shared_base + at);
}

/* an observer's list: `count` bridge_trigger_t records, at private_base + `at` */
static inline const bridge_list_t *bridge_native_get_list(const bridge_api_t *api, uint32_t observer)
{
    return (const bridge_list_t *)(api->private_base + bridge_native_get_header(api)->observers) + observer;
}

/* ---- the log ---- */

/* What a line of the log holds: its text, how long that is, and two numbers. */
typedef struct {
    const char *text;
    uint32_t    len;
    uint32_t    a0;
    uint32_t    a1;
} bridge_line_t;

static inline void bridge_native_log(const bridge_api_t *api, uint32_t level,
                                     const bridge_line_t *line)
{
    bridge_header_t *h = bridge_native_get_header(api);
    bridge_log_record_t *r;
    uint32_t head = h->ring_head;
    if (level > h->log_level) return;
    h->ring_head = head + 1u;
    if (head - h->ring_tail >= h->ring_len) return;     /* full: dropped, and counted */
    r = (bridge_log_record_t *)(api->private_base + h->ring) + head % h->ring_len;
    r->when = h->when;
    r->pad = 0;
    r->level = (uint8_t)level;
    r->system = (uint16_t)h->system;
    r->text = bridge_native_put(api, line->text, line->len);
    r->len = line->len;
    r->arg[0] = line->a0;
    r->arg[1] = line->a1;
}

#endif
