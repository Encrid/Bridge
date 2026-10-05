/* What a wasm module written in C compiles against: the eight imports, the accessors of
 * bridge-wasm.wat, the schema shim, and the layouts of bridge-abi.h read through them.
 * docs/modules/impl/wasm.md and docs/modules/schema.md.
 *
 * Nothing here is generated: sizeof, offsetof and _Generic compute at a module's build what a
 * generator would have had to be told. Every function is inline, so a module pays for what it
 * uses and the merge leaves plain loads behind. */
#ifndef BRIDGE_WASM_H
#define BRIDGE_WASM_H

#include "bridge-abi.h"
#include <stddef.h>

#define BRIDGE_IMPORT(name) [[clang::import_module(BRIDGE_NAMESPACE), clang::import_name(#name)]]
#define BRIDGE_ACC(name)    [[clang::import_module("acc"), clang::import_name(#name)]]
#define BRIDGE_EXPORT(name) [[clang::export_name(#name)]]

/* ---- the ABI: docs/modules/abi.md. Every pointer is an offset into the scratch arena ---- */

BRIDGE_IMPORT(declare) uint32_t bridge_declare(uint32_t name, uint32_t name_len,
    enum bridge_kind kind, uint32_t size, uint32_t align, uint32_t desc, uint32_t desc_len,
    enum bridge_on_unload on_unload);
BRIDGE_IMPORT(system)  uint32_t bridge_system(uint32_t name, uint32_t name_len, uint32_t phase,
    uint32_t phase_len, uint32_t terms, uint32_t term_count);
BRIDGE_IMPORT(observe) uint32_t bridge_observe(uint32_t name, uint32_t name_len,
    enum bridge_event events, uint32_t terms, uint32_t term_count);
BRIDGE_IMPORT(spawn)   uint64_t bridge_spawn();
BRIDGE_IMPORT(destroy) void     bridge_destroy(uint64_t id);
BRIDGE_IMPORT(attach)  void     bridge_attach(uint64_t id, uint32_t slot, uint32_t ptr, uint32_t len);
BRIDGE_IMPORT(detach)  void     bridge_detach(uint64_t id, uint32_t slot);
BRIDGE_IMPORT(alloc)   uint32_t bridge_alloc(uint32_t len);

/* ---- the accessors: bridge-wasm.wat ---- */

BRIDGE_ACC(acc_row)    uint32_t acc_row(uint32_t base, uint32_t row, uint32_t count, uint32_t stride);
BRIDGE_ACC(acc_ld_i8)  int32_t  acc_ld_i8(uint32_t at);
BRIDGE_ACC(acc_ld_u8)  uint32_t acc_ld_u8(uint32_t at);
BRIDGE_ACC(acc_ld_i16) int32_t  acc_ld_i16(uint32_t at);
BRIDGE_ACC(acc_ld_u16) uint32_t acc_ld_u16(uint32_t at);
BRIDGE_ACC(acc_ld_32)  uint32_t acc_ld_32(uint32_t at);
BRIDGE_ACC(acc_ld_64)  uint64_t acc_ld_64(uint32_t at);
BRIDGE_ACC(acc_ld_f32) float    acc_ld_f32(uint32_t at);
BRIDGE_ACC(acc_ld_f64) double   acc_ld_f64(uint32_t at);
BRIDGE_ACC(acc_st_8)   void     acc_st_8(uint32_t at, uint32_t v);
BRIDGE_ACC(acc_st_16)  void     acc_st_16(uint32_t at, uint32_t v);
BRIDGE_ACC(acc_st_32)  void     acc_st_32(uint32_t at, uint32_t v);
BRIDGE_ACC(acc_st_64)  void     acc_st_64(uint32_t at, uint64_t v);
BRIDGE_ACC(acc_st_f32) void     acc_st_f32(uint32_t at, float v);
BRIDGE_ACC(acc_st_f64) void     acc_st_f64(uint32_t at, double v);
BRIDGE_ACC(acc_own_32) uint32_t acc_own_32(uint32_t at);
BRIDGE_ACC(acc_own_64) uint64_t acc_own_64(uint32_t at);
BRIDGE_ACC(acc_own_st) void     acc_own_st(uint32_t at, uint32_t v);
BRIDGE_ACC(acc_to_private)  void acc_to_private(uint32_t to, const void *from, uint32_t len);
BRIDGE_ACC(acc_to_shared)   void acc_to_shared(uint32_t to, const void *from, uint32_t len);
BRIDGE_ACC(acc_from_shared) void acc_from_shared(void *to, uint32_t from, uint32_t len);

/* ---- the schema: a struct, and the string both sides hash ---- */

#define BRIDGE_SCHEMA_STRUCT(name, ...) \
    typedef struct name __VA_ARGS__ name; \
    static const char name##_desc[] = #__VA_ARGS__
/* an enum is four bytes signed and a bitmask four unsigned, and the type says so: it is what
 * Bridge-ASM700 lays the field out as, whatever a compiler would have chosen */
#define BRIDGE_SCHEMA_ENUM(name, ...) \
    typedef enum name : int32_t __VA_ARGS__ name; \
    static const char name##_desc[] = #__VA_ARGS__
#define BRIDGE_SCHEMA_BITMASK(name, ...) \
    typedef enum name : uint32_t __VA_ARGS__ name; \
    static const char name##_desc[] = #__VA_ARGS__

/* ---- the header and the scratch arena ---- */

#define BRIDGE_HEADER(field) acc_own_32(offsetof(bridge_header, field))

/* `len` bytes of the scratch arena, good until the next barrier. A module that asks for more
 * than it was given stops there. */
static inline uint32_t bridge_scratch(uint32_t len)
{
    uint32_t used = (BRIDGE_HEADER(scratch_used) + 7u) & ~7u;
    uint32_t room = BRIDGE_HEADER(scratch_len);
    if (used > room || len > room - used) __builtin_trap();
    acc_own_st(offsetof(bridge_header, scratch_used), used + len);
    return BRIDGE_HEADER(scratch) + used;
}

static inline uint32_t bridge_put(const void *bytes, uint32_t len)
{
    uint32_t at = bridge_scratch(len);
    acc_to_private(at, bytes, len);
    return at;
}

/* ---- registration, with names as literals ---- */

/* a type another declaration's fields use is declared before it, in the same namespace */
#define BRIDGE_DECLARE(kind, name, T, on_unload) \
    bridge_declare(bridge_put(name, sizeof name - 1), sizeof name - 1, kind, sizeof(T), \
                   alignof(T), bridge_put(T##_desc, sizeof T##_desc - 1), sizeof T##_desc - 1, \
                   on_unload)
#define BRIDGE_COMPONENT(name, T, on_unload) BRIDGE_DECLARE(BRIDGE_STRUCT, name, T, on_unload)

#define BRIDGE_SYSTEM(name, phase, terms, count) \
    bridge_system(bridge_put(name, sizeof name - 1), sizeof name - 1, \
                  bridge_put(phase, sizeof phase - 1), sizeof phase - 1, \
                  bridge_put(terms, (count) * sizeof(bridge_term)), count)

#define BRIDGE_OBSERVER(name, events, terms, count) \
    bridge_observe(bridge_put(name, sizeof name - 1), sizeof name - 1, events, \
                   bridge_put(terms, (count) * sizeof(bridge_term)), count)

/* an initial value travels with the attach - abi.md, *The ABI* */
#define BRIDGE_ATTACH(id, slot, value) \
    bridge_attach(id, slot, bridge_put(&(value), sizeof(value)), sizeof(value))

/* ---- a query's spans: two loops and no call. docs/bridge/data/query.md ---- */

static inline uint32_t bridge_spans(uint32_t query)
{
    return acc_own_32(BRIDGE_HEADER(queries) + query * sizeof(bridge_query));
}

/* span `i` of a query that declared `terms` terms */
static inline uint32_t bridge_span(uint32_t query, uint32_t i, uint32_t terms)
{
    uint32_t at = acc_own_32(BRIDGE_HEADER(queries) + query * sizeof(bridge_query) + 4u);
    return at + i * BRIDGE_SPAN_SIZE(terms);
}

static inline uint32_t bridge_rows(uint32_t span)     { return acc_own_32(span); }
static inline uint32_t bridge_entities(uint32_t span) { return acc_own_32(span + 4u); }

/* zero: the term is absent in this table */
static inline uint32_t bridge_base(uint32_t span, uint32_t term)
{
    return acc_own_32(span + BRIDGE_SPAN_BASE + 4u * term);
}

static inline uint64_t bridge_entity_at(uint32_t entities, uint32_t row, uint32_t count)
{
    return acc_ld_64(acc_row(entities, row, count, 8u));
}

/* ---- an observer's list: who triggered it at this barrier, and how ---- */

static inline uint32_t bridge_triggers(uint32_t observer)
{
    return acc_own_32(BRIDGE_HEADER(observers) + observer * sizeof(bridge_list));
}

static inline uint32_t bridge_trigger_at(uint32_t observer, uint32_t i)
{
    uint32_t at = acc_own_32(BRIDGE_HEADER(observers) + observer * sizeof(bridge_list) + 4u);
    return at + i * (uint32_t)sizeof(bridge_trigger);
}

static inline uint64_t bridge_trigger_id(uint32_t trigger)     { return acc_own_64(trigger); }
static inline uint32_t bridge_trigger_events(uint32_t trigger) { return acc_own_32(trigger + 8u); }

/* ---- a component's fields ---- */

/* where row `row` of a column of T is: carries the row < count test */
#define BRIDGE_ROW(T, base, row, count) acc_row(base, row, count, sizeof(T))

#define BRIDGE_LD_(v) _Generic((v), \
    bool: acc_ld_u8, char: acc_ld_i8, signed char: acc_ld_i8, unsigned char: acc_ld_u8, \
    short: acc_ld_i16, unsigned short: acc_ld_u16, \
    int: acc_ld_32, unsigned: acc_ld_32, long: acc_ld_32, unsigned long: acc_ld_32, \
    long long: acc_ld_64, unsigned long long: acc_ld_64, float: acc_ld_f32, double: acc_ld_f64)
#define BRIDGE_ST_(v) _Generic((v), \
    bool: acc_st_8, char: acc_st_8, signed char: acc_st_8, unsigned char: acc_st_8, \
    short: acc_st_16, unsigned short: acc_st_16, \
    int: acc_st_32, unsigned: acc_st_32, long: acc_st_32, unsigned long: acc_st_32, \
    long long: acc_st_64, unsigned long long: acc_st_64, float: acc_st_f32, double: acc_st_f64)

/* `at` is what BRIDGE_ROW or bridge_ref answered */
#define BRIDGE_GET(T, field, at) \
    BRIDGE_LD_(((T *)0)->field)((at) + (uint32_t)offsetof(T, field))
#define BRIDGE_SET(T, field, at, value) \
    BRIDGE_ST_(((T *)0)->field)((at) + (uint32_t)offsetof(T, field), (value))

/* ---- following a reference: arithmetic, not a call. docs/bridge/data/reference.md ---- */

/* Where component `slot` of entity `id` is, or zero: a stale id, or an entity without it. */
static inline uint32_t bridge_ref(uint64_t id, uint32_t slot, uint32_t stride)
{
    uint32_t index = (uint32_t)id, record, base;
    if (index >= BRIDGE_HEADER(entities)) return 0;
    record = BRIDGE_HEADER(entity_index) + index * (uint32_t)sizeof(bridge_entity);
    if (acc_ld_32(record + 8u) != (uint32_t)(id >> 32)) return 0;
    base = acc_own_32(BRIDGE_HEADER(column_base)
                      + (acc_ld_32(record) * BRIDGE_HEADER(terms) + slot) * 4u);
    return base ? base + acc_ld_32(record + 4u) * stride : 0;
}

/* Whether `id` still names an entity: one that was destroyed, or never spawned, does not. */
static inline int bridge_alive(uint64_t id)
{
    uint32_t index = (uint32_t)id;
    if (!id || index >= BRIDGE_HEADER(entities)) return 0;
    return acc_ld_32(BRIDGE_HEADER(entity_index) + index * (uint32_t)sizeof(bridge_entity) + 8u)
           == (uint32_t)(id >> 32);
}

/* ---- the log: stores, and no call. docs/bridge/diagnostics/log.md ---- */

enum bridge_log : int8_t { BRIDGE_LOG_ERR = -3, BRIDGE_LOG_WARN = -2, BRIDGE_LOG_INFO = 0 };

static inline void bridge_log(int level, const char *text, uint32_t len, uint32_t a0, uint32_t a1)
{
    uint32_t head, at;
    if (level > (int32_t)BRIDGE_HEADER(log_level)) return;
    head = BRIDGE_HEADER(ring_head);
    acc_own_st(offsetof(bridge_header, ring_head), head + 1u);
    /* full: the line is dropped, and counted by the head having moved */
    if (head - BRIDGE_HEADER(ring_tail) >= BRIDGE_HEADER(ring_len)) return;
    at = BRIDGE_HEADER(ring) + (head % BRIDGE_HEADER(ring_len)) * (uint32_t)sizeof(bridge_log_record);
    acc_own_st(at, BRIDGE_HEADER(when));
    acc_own_st(at + 4u, ((uint32_t)level & 0xffu) << 8 | BRIDGE_HEADER(system) << 16);
    acc_own_st(at + 8u, bridge_put(text, len));
    acc_own_st(at + 12u, len);
    acc_own_st(at + 16u, a0);
    acc_own_st(at + 20u, a1);
}

#define BRIDGE_LOG(level, text, a0, a1) bridge_log(level, text, sizeof text - 1, a0, a1)

#endif
