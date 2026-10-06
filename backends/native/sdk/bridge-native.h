/* What a native module compiles against, beside bridge-abi.h: the table its backend hands it,
 * and the same ABI layouts read through real pointers. docs/backends/native/native.md.
 *
 * Only a native module and the native backend compile this header. */
#ifndef BRIDGE_NATIVE_H
#define BRIDGE_NATIVE_H

#include "bridge-abi.h"
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
typedef struct bridge_api bridge_api;
struct bridge_api {
    uint32_t (*declare)(const bridge_api *api, uint32_t name, uint32_t name_len, enum bridge_kind kind,
                        uint32_t size, uint32_t align, uint32_t desc, uint32_t desc_len,
                        enum bridge_on_unload on_unload);
    uint32_t (*system)(const bridge_api *api, uint32_t name, uint32_t name_len, uint32_t phase,
                       uint32_t phase_len, uint32_t terms, uint32_t term_count, uint32_t changes,
                       uint32_t change_count);
    uint32_t (*observe)(const bridge_api *api, uint32_t name, uint32_t name_len,
                        enum bridge_event events, uint32_t terms, uint32_t term_count,
                        uint32_t changes, uint32_t change_count);
    uint64_t (*spawn)(const bridge_api *api);
    void     (*destroy)(const bridge_api *api, uint64_t id);
    void     (*attach)(const bridge_api *api, uint64_t id, uint32_t slot, uint32_t ptr, uint32_t len);
    void     (*detach)(const bridge_api *api, uint64_t id, uint32_t slot);
    uint32_t (*alloc)(const bridge_api *api, uint32_t len);
    uint8_t  *shared_base;          /* the shared memory. it does not move */
    uint8_t  *private_base;         /* this module's private memory */
};

/* What a native module exports: a function of this name, handed the table as `api`. A system
 * called "move" is the export bridge_move. */
#define BRIDGE_NATIVE(name) \
    BRIDGE_NATIVE_EXPORT int bridge_##name([[maybe_unused]] const bridge_api *api)

/* ---- the schema: a struct, and the string both sides hash ---- */

#define BRIDGE_SCHEMA_STRUCT(name, ...) \
    typedef struct name __VA_ARGS__ name; \
    static const char name##_desc[] = #__VA_ARGS__
/* An enum and a bitmask are the integer their author states: the width a row gives one. It
 * goes into the descriptor ahead of the brace, where Bridge-ASM700 reads it and checks that
 * every constant fits - docs/modules/schema.md, *The type set* */
#define BRIDGE_SCHEMA_ENUM(name, type, ...) \
    typedef enum name : type __VA_ARGS__ name; \
    static const char name##_desc[] = #type " " #__VA_ARGS__
#define BRIDGE_SCHEMA_BITMASK(name, type, ...) \
    typedef enum name : type __VA_ARGS__ name; \
    static const char name##_desc[] = #type " " #__VA_ARGS__

/* ---- the scratch arena ---- */

static inline bridge_header *bridge_native_header(const bridge_api *api)
{
    return (bridge_header *)api->private_base;
}

/* `len` bytes copied into the scratch arena: where they are, or 0 if they do not fit, which
 * every entry refuses */
static inline uint32_t bridge_native_put(const bridge_api *api, const void *bytes, uint32_t len)
{
    bridge_header *h = bridge_native_header(api);
    uint32_t used = (h->scratch_used + 7u) & ~7u;
    if (used > h->scratch_len || len > h->scratch_len - used) return 0;
    memcpy(api->private_base + h->scratch + used, bytes, len);
    h->scratch_used = used + len;
    return h->scratch + used;
}

/* An attach with its value. Bridge-ASM700 takes its copy during the call, so the scratch the
 * value was put in is given back when the call returns: a module's scratch holds one such
 * value at a time, and not a phase's worth. */
static inline void bridge_native_attach(const bridge_api *api, uint64_t id, uint32_t slot,
                                        const void *value, uint32_t len)
{
    bridge_header *h = bridge_native_header(api);
    uint32_t was = h->scratch_used;
    api->attach(api, id, slot, bridge_native_put(api, value, len), len);
    h->scratch_used = was;
}

#define BRIDGE_NATIVE_DECLARE(api, kind, name, T, on_unload) \
    (api)->declare(api, bridge_native_put(api, name, sizeof name - 1), sizeof name - 1, kind, \
                   sizeof(T), alignof(T), bridge_native_put(api, T##_desc, sizeof T##_desc - 1), \
                   sizeof T##_desc - 1, on_unload)
#define BRIDGE_NATIVE_COMPONENT(api, name, T, on_unload) \
    BRIDGE_NATIVE_DECLARE(api, BRIDGE_STRUCT, name, T, on_unload)

/* `changes` is the slots of the components it attaches or detaches, `change_count` of them:
 * a system that does neither hands nullptr and 0 */
#define BRIDGE_NATIVE_SYSTEM(api, name, phase, terms, count, changes, change_count) \
    (api)->system(api, bridge_native_put(api, name, sizeof name - 1), sizeof name - 1, \
                  bridge_native_put(api, phase, sizeof phase - 1), sizeof phase - 1, \
                  bridge_native_put(api, terms, (count) * sizeof(bridge_term)), count, \
                  (change_count) ? bridge_native_put(api, changes, \
                      (change_count) * sizeof(uint32_t)) : 0, change_count)

#define BRIDGE_NATIVE_OBSERVER(api, name, events, terms, count, changes, change_count) \
    (api)->observe(api, bridge_native_put(api, name, sizeof name - 1), sizeof name - 1, events, \
                   bridge_native_put(api, terms, (count) * sizeof(bridge_term)), count, \
                   (change_count) ? bridge_native_put(api, changes, \
                       (change_count) * sizeof(uint32_t)) : 0, change_count)

/* ---- a query's spans, through real pointers. native.md, *Reaching a component* ---- */

static inline const bridge_query *bridge_native_query(const bridge_api *api, uint32_t query)
{
    return (const bridge_query *)(api->private_base + bridge_native_header(api)->queries) + query;
}

/* span `i` of a query that declared `terms` terms: u32 count, u32 entities, u32 base[terms] */
static inline const uint32_t *bridge_native_span(const bridge_api *api, const bridge_query *q,
                                                 uint32_t i, uint32_t terms)
{
    return (const uint32_t *)(api->private_base + q->at + i * BRIDGE_SPAN_SIZE(terms));
}

/* an observer's list: `count` bridge_trigger records, at private_base + `at` */
static inline const bridge_list *bridge_native_list(const bridge_api *api, uint32_t observer)
{
    return (const bridge_list *)(api->private_base + bridge_native_header(api)->observers) + observer;
}

/* ---- the log ---- */

static inline void bridge_native_log(const bridge_api *api, uint32_t level, const char *text,
                                     uint32_t len, uint32_t a0, uint32_t a1)
{
    bridge_header *h = bridge_native_header(api);
    bridge_log_record *r;
    uint32_t head = h->ring_head;
    if (level > h->log_level) return;
    h->ring_head = head + 1u;
    if (head - h->ring_tail >= h->ring_len) return;     /* full: dropped, and counted */
    r = (bridge_log_record *)(api->private_base + h->ring) + head % h->ring_len;
    r->when = h->when;
    r->pad = 0;
    r->level = (uint8_t)level;
    r->system = (uint16_t)h->system;
    r->text = bridge_native_put(api, text, len);
    r->len = len;
    r->arg[0] = a0;
    r->arg[1] = a1;
}

#endif
