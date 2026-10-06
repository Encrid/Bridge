/* What a wasm module written in C compiles against: the eight imports, the accessors of
 * bridge_wasm.wat, the schema shim, and the layouts of bridge_abi.h read through them.
 * docs/backends/wasm/wasm.md and docs/modules/schema.md.
 *
 * Nothing here is generated: sizeof, offsetof and _Generic compute at a module's build what a
 * generator would have had to be told. Every function is inline, so a module pays for what it
 * uses and the merge leaves plain loads behind. */
#ifndef BRIDGE_WASM_H
#define BRIDGE_WASM_H

#include "bridge_abi.h"
#include "bridge_schema.h"       /* how a type is stated: the same on every backend */
#include <stddef.h>

#define BRIDGE_IMPORT(name) [[clang::import_module(BRIDGE_NAMESPACE), clang::import_name(#name)]]
#define BRIDGE_ACC(name)    [[clang::import_module("acc"), clang::import_name(#name)]]
#define BRIDGE_EXPORT(name) [[clang::export_name(#name)]]

/* ---- the ABI: docs/modules/abi.md. Every pointer is an offset into the scratch arena ---- */

BRIDGE_IMPORT(declare) uint32_t bridge_declare(uint32_t at);
BRIDGE_IMPORT(add_system) uint32_t bridge_add_system(uint32_t at);
BRIDGE_IMPORT(observe) uint32_t bridge_observe(uint32_t at);
BRIDGE_IMPORT(spawn)   uint64_t bridge_spawn();
BRIDGE_IMPORT(destroy) void     bridge_destroy(uint64_t id);
BRIDGE_IMPORT(attach)  void     bridge_attach(uint64_t id, uint32_t slot, uint32_t ptr);
BRIDGE_IMPORT(detach)  void     bridge_detach(uint64_t id, uint32_t slot);
BRIDGE_IMPORT(alloc)   uint32_t bridge_alloc(uint32_t len);

/* ---- the accessors: bridge_wasm.wat ---- */

/* Where a row is. Whether it is tested to be one of the span's rows, each time, is stated
 * by whoever builds the module: docs/bridge/diagnostics/validation.md, *The row test*. */
#if defined(BRIDGE_ROWS_CHECKED) == defined(BRIDGE_ROWS_UNCHECKED)
#error "a module is built with BRIDGE_ROWS_CHECKED or with BRIDGE_ROWS_UNCHECKED: one of them, stated"
#endif
#ifdef BRIDGE_ROWS_CHECKED
BRIDGE_ACC(acc_row)    uint32_t acc_row(uint32_t base, uint32_t row, uint32_t count, uint32_t stride);
#else
BRIDGE_ACC(acc_row_unchecked) uint32_t acc_row_unchecked(uint32_t base, uint32_t row, uint32_t count,
                                                         uint32_t stride);
#define acc_row acc_row_unchecked
#endif
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

/* ---- the header and the scratch arena ---- */

#define BRIDGE_HEADER(field) acc_own_32(offsetof(bridge_header_t, field))

/* `len` bytes of the scratch arena, good until the next barrier. A module that asks for more
 * than it was given stops there. */
static inline uint32_t bridge_take_scratch(uint32_t len)
{
    uint32_t used = (BRIDGE_HEADER(scratch_used) + 7u) & ~7u;
    uint32_t room = BRIDGE_HEADER(scratch_len);
    if (used > room || len > room - used) __builtin_trap();
    acc_own_st(offsetof(bridge_header_t, scratch_used), used + len);
    return BRIDGE_HEADER(scratch) + used;
}

static inline uint32_t bridge_put(const void *bytes, uint32_t len)
{
    uint32_t at = bridge_take_scratch(len);
    acc_to_private(at, bytes, len);
    return at;
}

/* `len` bytes copied above what the scratch arena has in use, and not counted as used. For a
 * value an entry copies during the call, as attach does, so that a module's scratch holds one
 * such value at a time and not a phase's worth. */
static inline uint32_t bridge_lend(const void *bytes, uint32_t len)
{
    uint32_t used = (BRIDGE_HEADER(scratch_used) + 7u) & ~7u;
    uint32_t room = BRIDGE_HEADER(scratch_len);
    if (used > room || len > room - used) __builtin_trap();
    acc_to_private(BRIDGE_HEADER(scratch) + used, bytes, len);
    return BRIDGE_HEADER(scratch) + used;
}

/* ---- registration, with names as literals ---- */

#define BRIDGE_STATE(T, ...) bridge_put(&(T){ __VA_ARGS__ }, sizeof(T))

/* a type another declaration's fields use is declared before it, in the same namespace */
#define BRIDGE_DECLARE(kind, name, T, on_unload, storage) \
    bridge_declare(BRIDGE_STATE(bridge_declaration_t, \
        bridge_put(name, sizeof name - 1), sizeof name - 1, kind, sizeof(T), alignof(T), \
        bridge_put(T##_desc, sizeof T##_desc - 1), sizeof T##_desc - 1, on_unload, storage))
#define BRIDGE_COMPONENT(name, T, on_unload, storage) \
    BRIDGE_DECLARE(BRIDGE_STRUCT, name, T, on_unload, storage)

/* a tag: a component with no bytes, so no type to take them from - storage.md, *A tag* */
#define BRIDGE_DECLARE_TAG(name, on_unload, storage) \
    bridge_declare(BRIDGE_STATE(bridge_declaration_t, \
        bridge_put(name, sizeof name - 1), sizeof name - 1, BRIDGE_TAG, 0, 1, 0, 0, \
        on_unload, storage))

/* `changes` is the slots of the components it attaches or detaches, `change_count` of them:
 * a system that does neither hands nullptr and 0 */
#define BRIDGE_ADD_SYSTEM(name, phase, terms, count, changes, change_count) \
    bridge_add_system(BRIDGE_STATE(bridge_system_statement_t, \
        bridge_put(name, sizeof name - 1), sizeof name - 1, \
        bridge_put(phase, sizeof phase - 1), sizeof phase - 1, \
        bridge_put(terms, (count) * sizeof(bridge_term_t)), count, \
        (change_count) ? bridge_put(changes, (change_count) * sizeof(uint32_t)) : 0, \
        change_count))

#define BRIDGE_OBSERVER(name, events, terms, count, changes, change_count) \
    bridge_observe(BRIDGE_STATE(bridge_observer_statement_t, \
        bridge_put(name, sizeof name - 1), sizeof name - 1, events, \
        bridge_put(terms, (count) * sizeof(bridge_term_t)), count, \
        (change_count) ? bridge_put(changes, (change_count) * sizeof(uint32_t)) : 0, \
        change_count))

/* An initial value travels with the attach - abi.md, *The ABI* - and is as long as the type
 * was declared. Bridge-ASM700 takes its copy during the call. */
#define BRIDGE_ATTACH(id, slot, value) bridge_attach(id, slot, bridge_lend(&(value), sizeof(value)))

/* ---- a query's spans: two loops and no call. docs/bridge/data/query.md ---- */

static inline uint32_t bridge_count_spans(uint32_t query)
{
    return acc_own_32(BRIDGE_HEADER(queries) + query * sizeof(bridge_query_t));
}

/* span `i` of a query that declared `terms` terms */
static inline uint32_t bridge_get_span(uint32_t query, uint32_t i, uint32_t terms)
{
    uint32_t at = acc_own_32(BRIDGE_HEADER(queries) + query * sizeof(bridge_query_t) + 4u);
    return at + i * BRIDGE_SPAN_SIZE(terms);
}

static inline uint32_t bridge_count_rows(uint32_t span)     { return acc_own_32(span); }
static inline uint32_t bridge_get_entities(uint32_t span) { return acc_own_32(span + 4u); }

/* zero: the term is absent in this table */
static inline uint32_t bridge_get_base(uint32_t span, uint32_t term)
{
    return acc_own_32(span + BRIDGE_SPAN_BASE + 4u * term);
}

static inline uint64_t bridge_get_entity(uint32_t entities, uint32_t row, uint32_t count)
{
    return acc_ld_64(acc_row(entities, row, count, 8u));
}

/* ---- a sparse set, reached from a row. docs/bridge/storage.md, *A sparse set* ---- */

/* where the sparse set that keeps component `slot` has its index: a cell for each entity */
static inline uint32_t bridge_get_sparse_index(uint32_t slot)
{
    return acc_own_32(BRIDGE_HEADER(kept) + 4u * slot);
}

/* where entity `id` has its value in that set, from the shared base: zero if it has none */
static inline uint32_t bridge_get_sparse_value(uint32_t index, uint64_t id)
{
    return acc_ld_32(acc_row(index, (uint32_t)id, BRIDGE_HEADER(entities), 4u));
}

/* ---- a singleton: one value, on no entity. docs/bridge/storage.md, *A singleton* ---- */

/* where the singleton `slot` has its value, from the shared base: zero while it has none */
static inline uint32_t bridge_get_single(uint32_t slot)
{
    return acc_own_32(BRIDGE_HEADER(kept) + 4u * slot);
}

/* ---- direct-indexed: a cell for every entity. docs/bridge/storage.md, *Direct-indexed* ---- */

/* where the cells of the direct-indexed component `slot` begin */
static inline uint32_t bridge_get_cells(uint32_t slot)
{
    return acc_own_32(BRIDGE_HEADER(kept) + 4u * slot);
}

/* where entity `id` has its value of `stride` bytes among them, or zero: it was given none */
static inline uint32_t bridge_get_cell(uint32_t cells, uint64_t id, uint32_t stride)
{
    uint32_t entities = BRIDGE_HEADER(entities), e = (uint32_t)id;
    uint32_t word = acc_row(acc_ld_32(cells - 16u), e >> 6, (entities + 63u) >> 6, 8u);
    return acc_ld_64(word) >> (e & 63u) & 1u ? acc_row(cells, e, entities, stride) : 0;
}

/* ---- a toggled component: a bit a row. docs/bridge/storage.md, *A toggle* ---- */

/* where the bits of the toggled column at `base` are: row r is bit r & 63 of word r >> 6 */
static inline uint32_t bridge_get_bits(uint32_t base) { return acc_ld_32(base - 16u); }

/* whether row `row` of `count` is on */
static inline bool bridge_is_on(uint32_t bits, uint32_t row, uint32_t count)
{
    return acc_ld_64(acc_row(bits, row >> 6, (count + 63u) >> 6, 8u)) >> (row & 63u) & 1u;
}

/* a row turned on or off, by a system that stated the term BRIDGE_WRITE */
static inline void bridge_turn(uint32_t bits, uint32_t row, uint32_t count, bool on)
{
    uint32_t at = acc_row(bits, row >> 6, (count + 63u) >> 6, 8u);
    uint64_t word = (uint64_t)acc_ld_64(at), bit = (uint64_t)1 << (row & 63u);
    acc_st_64(at, on ? word | bit : word & ~bit);
}

/* ---- an observer's list: who triggered it at this barrier, and how ---- */

static inline uint32_t bridge_count_triggers(uint32_t observer)
{
    return acc_own_32(BRIDGE_HEADER(observers) + observer * sizeof(bridge_list_t));
}

static inline uint32_t bridge_get_trigger(uint32_t observer, uint32_t i)
{
    uint32_t at = acc_own_32(BRIDGE_HEADER(observers) + observer * sizeof(bridge_list_t) + 4u);
    return at + i * (uint32_t)sizeof(bridge_trigger_t);
}

static inline uint64_t bridge_get_trigger_id(uint32_t trigger)     { return acc_own_64(trigger); }
static inline uint32_t bridge_get_trigger_events(uint32_t trigger) { return acc_own_32(trigger + 8u); }

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

/* `at` is what BRIDGE_ROW or bridge_get_ref answered */
#define BRIDGE_GET(T, field, at) \
    BRIDGE_LD_(((T *)0)->field)((at) + (uint32_t)offsetof(T, field))
#define BRIDGE_SET(T, field, at, value) \
    BRIDGE_ST_(((T *)0)->field)((at) + (uint32_t)offsetof(T, field), (value))

/* ---- following a reference: arithmetic, not a call. docs/bridge/data/reference.md ---- */

/* Where component `slot` of entity `id` is, or zero: a stale id, or an entity without it. */
static inline uint32_t bridge_get_ref(uint64_t id, uint32_t slot, uint32_t stride)
{
    uint32_t index = (uint32_t)id, record, base;
    if (index >= BRIDGE_HEADER(entities)) return 0;
    record = BRIDGE_HEADER(entity_index) + index * (uint32_t)sizeof(bridge_entity_t);
    if (acc_ld_32(record + 8u) != (uint32_t)(id >> 32)) return 0;
    base = acc_own_32(BRIDGE_HEADER(column_base)
                      + (acc_ld_32(record) * BRIDGE_HEADER(types) + slot) * 4u);
    return base ? base + acc_ld_32(record + 4u) * stride : 0;
}

/* Whether `id` still names an entity: one that was destroyed, or never spawned, does not. */
static inline int bridge_is_alive(uint64_t id)
{
    uint32_t index = (uint32_t)id;
    if (!id || index >= BRIDGE_HEADER(entities)) return 0;
    return acc_ld_32(BRIDGE_HEADER(entity_index) + index * (uint32_t)sizeof(bridge_entity_t) + 8u)
           == (uint32_t)(id >> 32);
}

/* ---- the log: stores, and no call. docs/bridge/diagnostics/log.md ---- */

/* What a line of the log holds: its text, how long that is, and two numbers. */
typedef struct {
    const char *text;
    uint32_t    len;
    uint32_t    a0;
    uint32_t    a1;
} bridge_line_t;

static inline void bridge_log(uint32_t level, const bridge_line_t *line)
{
    uint32_t head, at;
    if (level > BRIDGE_HEADER(log_level)) return;
    head = BRIDGE_HEADER(ring_head);
    acc_own_st(offsetof(bridge_header_t, ring_head), head + 1u);
    /* full: the line is dropped, and counted by the head having moved */
    if (head - BRIDGE_HEADER(ring_tail) >= BRIDGE_HEADER(ring_len)) return;
    at = BRIDGE_HEADER(ring) + (head % BRIDGE_HEADER(ring_len)) * (uint32_t)sizeof(bridge_log_record_t);
    acc_own_st(at, BRIDGE_HEADER(when));
    acc_own_st(at + 4u, (level & 0xffu) << 8 | BRIDGE_HEADER(system) << 16);
    acc_own_st(at + 8u, bridge_put(line->text, line->len));
    acc_own_st(at + 12u, line->len);
    acc_own_st(at + 16u, line->a0);
    acc_own_st(at + 20u, line->a1);
}

#define BRIDGE_LOG(level, text, a0, a1) \
    bridge_log(level, &(bridge_line_t){ text, sizeof text - 1, a0, a1 })

#endif
