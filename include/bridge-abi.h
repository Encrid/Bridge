/* The module seam: what Bridge-ASM700 and every module both compile.
 * docs/backends/wasm/wasm.md, *The ABI layouts*.
 *
 * It says how to find data and never what data there is: no component, no field. Everything
 * here is read by a module with plain loads, so every member is a fixed width and the structs
 * have no padding a compiler could place differently: each layout is asserted where it is
 * stated, and a module and Bridge-ASM700 that disagreed about one would not both compile.
 *
 * A member named `pad` is that padding, written out so that every offset is stated. It means
 * nothing: whoever writes the struct writes zero there, and nobody reads it.
 *
 * An enum has the width of where it goes. One that is an argument of an entry is four bytes,
 * as every argument that is not an id is. One that is a member of a struct is one byte, which
 * holds every value these have. `bridge_event` is both, and is four. */
#ifndef BRIDGE_ABI_H
#define BRIDGE_ABI_H

#include <stddef.h>
#include <stdint.h>

/* To a C++ compiler these are C: the names are not mangled, and a host in C++ links them. */
#ifdef __cplusplus
extern "C" {
#endif

/* A module states the version it was built against; one built against another is refused. */
constexpr uint32_t BRIDGE_ABI = 1;

/* Everything a module imports comes from one namespace; the two memories go by name. */
#define BRIDGE_NAMESPACE "bridge"
#define BRIDGE_SHARED    "shared"
#define BRIDGE_PRIVATE   "private"

/* declare's kind: what the descriptor describes - docs/modules/schema.md, *The type set*.
 * Zero is unset, and refused. A tag is a component with no bytes: its size is 0 and it has no
 * descriptor - docs/bridge/storage.md, *A tag*. */
enum bridge_kind : uint32_t { BRIDGE_STRUCT = 1, BRIDGE_ENUM = 2, BRIDGE_BITMASK = 3, BRIDGE_TAG = 4 };

/* What a module is handed where it would be handed where a value is, for a tag, which has
 * none: that it is carried. Offset 0 of the shared memory is reserved and its first eight
 * bytes hold nothing, so this is not zero, which says absent, and is no value's place. */
constexpr uint32_t BRIDGE_CARRIED = 1;

/* declare's on_unload - docs/bridge/data/lifetime.md. Zero is unset, and refused. */
enum bridge_on_unload : uint32_t { BRIDGE_STRIP = 1, BRIDGE_SWEEP = 2, BRIDGE_DELETE = 3 };

/* declare's storage: the storage layout the component is kept in - docs/bridge/storage.md,
 * *Storage layouts*. Zero is unset, and refused. Every module that declares a component states
 * the same one. */
enum bridge_storage : uint32_t {
    BRIDGE_TABLE = 1, BRIDGE_SPARSE_SET = 2, BRIDGE_TOGGLE = 3, BRIDGE_DIRECT_INDEXED = 4,
    BRIDGE_SINGLETON = 5
};

/* A query term: which component, whether the system writes it, and how it must match. `slot`
 * is what declare answered. Zero is unset for `access` and `oper`, and refused; so is any value
 * that is not one of these.
 *
 * `access` is what the scheduler reads to know what may run at once, and it reads one thing:
 * whether the term is written. `oper`: the entity must carry it, must not, or may. */
enum bridge_access : uint8_t { BRIDGE_READ = 1, BRIDGE_WRITE = 2 };
enum bridge_oper : uint8_t { BRIDGE_AND = 1, BRIDGE_NOT = 2, BRIDGE_OPTIONAL = 3 };

typedef struct {
    uint32_t           slot;
    enum bridge_access access;
    enum bridge_oper   oper;
    uint16_t           pad;
} bridge_term;
static_assert(sizeof(bridge_term) == 8 && alignof(bridge_term) == 4);
static_assert(offsetof(bridge_term, slot) == 0 && offsetof(bridge_term, access) == 4);
static_assert(offsetof(bridge_term, oper) == 5 && offsetof(bridge_term, pad) == 6);

/* observe's events, as a mask of these, and what an observer reads: who triggered it, and how.
 * Each is what a barrier made of an entity, against the observer's terms. ENTER: it did not
 * match them and now does. LEAVE: it matched and no longer does, or was destroyed. ATTACH: it
 * matches, and an attach of one of them was applied. A write in place raises none. */
enum bridge_event : uint32_t { BRIDGE_ON_ENTER = 1, BRIDGE_ON_ATTACH = 2, BRIDGE_ON_LEAVE = 4 };

typedef struct {
    uint64_t          id;
    enum bridge_event events;
    uint32_t          pad;
} bridge_trigger;
static_assert(sizeof(bridge_trigger) == 16 && alignof(bridge_trigger) == 8);
static_assert(offsetof(bridge_trigger, id) == 0 && offsetof(bridge_trigger, events) == 8);
static_assert(offsetof(bridge_trigger, pad) == 12);

/* Offset 0 of the private memory: where each of this module's regions begins. Bridge-ASM700
 * writes all of it; a module writes `scratch_used` and `ring_head` and nothing else. */
typedef struct {
    uint32_t abi;
    uint32_t scratch, scratch_len;      /* the scratch arena, in bytes */
    uint32_t scratch_used;              /* the module bumps it; a barrier resets it */
    uint32_t ring, ring_len;            /* the log ring, in records */
    uint32_t ring_head;                 /* records ever written, by the module */
    uint32_t ring_tail;                 /* records ever drained, by Bridge-ASM700 */
    uint32_t queries;                   /* bridge_query, one per slot `system` answered */
    uint32_t column_base;               /* u32 [tables][types] of shared-memory offsets */
    uint32_t types, tables;             /* how many types this module declared, and tables */
    uint32_t entity_index;              /* in the shared memory: bridge_entity [entities] */
    uint32_t entities;
    uint32_t when;                      /* set before every dispatch */
    uint32_t system;                    /* the slot of the system running */
    uint32_t log_level;                 /* a line above it is not written */
    uint32_t observers;                 /* bridge_list, one per slot `observe` answered */
    uint32_t kept;                      /* u32 [types]: for a type that is not kept in a table,
                                         * where in the shared memory it is reached. A sparse
                                         * set: its index, a u32 for each entity */
} bridge_header;
static_assert(sizeof(bridge_header) == 76 && alignof(bridge_header) == 4);
static_assert(offsetof(bridge_header, abi) == 0 && offsetof(bridge_header, scratch) == 4);
static_assert(offsetof(bridge_header, scratch_len) == 8 && offsetof(bridge_header, scratch_used) == 12);
static_assert(offsetof(bridge_header, ring) == 16 && offsetof(bridge_header, ring_len) == 20);
static_assert(offsetof(bridge_header, ring_head) == 24 && offsetof(bridge_header, ring_tail) == 28);
static_assert(offsetof(bridge_header, queries) == 32 && offsetof(bridge_header, column_base) == 36);
static_assert(offsetof(bridge_header, types) == 40 && offsetof(bridge_header, tables) == 44);
static_assert(offsetof(bridge_header, entity_index) == 48 && offsetof(bridge_header, entities) == 52);
static_assert(offsetof(bridge_header, when) == 56 && offsetof(bridge_header, system) == 60);
static_assert(offsetof(bridge_header, log_level) == 64 && offsetof(bridge_header, observers) == 68);
static_assert(offsetof(bridge_header, kept) == 72);

typedef struct { uint32_t spans, at; } bridge_query;
static_assert(sizeof(bridge_query) == 8 && alignof(bridge_query) == 4);
static_assert(offsetof(bridge_query, spans) == 0 && offsetof(bridge_query, at) == 4);

/* Text a component keeps: where its bytes are in the persistent arena, and how many. A field
 * may be of this type under any namespace - docs/modules/schema.md, *The type set*. */
typedef struct { uint32_t at, len; } bridge_text;
static_assert(sizeof(bridge_text) == 8 && alignof(bridge_text) == 4);
static_assert(offsetof(bridge_text, at) == 0 && offsetof(bridge_text, len) == 4);

/* An observer's slot: how many bridge_trigger records this barrier, and where they begin. */
typedef struct { uint32_t count, at; } bridge_list;
static_assert(sizeof(bridge_list) == 8 && alignof(bridge_list) == 4);
static_assert(offsetof(bridge_list, count) == 0 && offsetof(bridge_list, at) == 4);

/* A span, one per matched table - docs/bridge/data/query.md:
 *     u32 count, u32 entities, u32 base[terms] */
constexpr uint32_t BRIDGE_SPAN_BASE = 8;
#define BRIDGE_SPAN_SIZE(n)    (8u + 4u * (n))

/* The entity index, by the low 32 bits of an id. Table 0 is no table. */
typedef struct { uint32_t table, row, generation; } bridge_entity;
static_assert(sizeof(bridge_entity) == 12 && alignof(bridge_entity) == 4);
static_assert(offsetof(bridge_entity, table) == 0 && offsetof(bridge_entity, row) == 4);
static_assert(offsetof(bridge_entity, generation) == 8);

/* What a log line is: the severities of syslog, RFC 5424, by its numbers. 0 is the worst and 7
 * the most detail; these are the four the bindings name. docs/bridge/diagnostics/log.md. */
enum bridge_log : uint8_t {
    BRIDGE_LOG_ERR = 3, BRIDGE_LOG_WARN = 4, BRIDGE_LOG_INFO = 6, BRIDGE_LOG_DEBUG = 7
};

/* A log record. `text` is an offset into the writer's scratch arena, always. */
typedef struct {
    uint32_t when;
    uint8_t  pad;
    uint8_t  level;                     /* enum bridge_log: lower is worse */
    uint16_t system;
    uint32_t text, len;
    uint32_t arg[2];
} bridge_log_record;
static_assert(sizeof(bridge_log_record) == 24 && alignof(bridge_log_record) == 4);
static_assert(offsetof(bridge_log_record, when) == 0 && offsetof(bridge_log_record, pad) == 4);
static_assert(offsetof(bridge_log_record, level) == 5 && offsetof(bridge_log_record, system) == 6);
static_assert(offsetof(bridge_log_record, text) == 8 && offsetof(bridge_log_record, len) == 12);
static_assert(offsetof(bridge_log_record, arg) == 16);

#ifdef __cplusplus
}
#endif

#endif
