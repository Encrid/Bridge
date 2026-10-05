/* The module seam: what Bridge-ASM700 and every module both compile.
 * docs/modules/impl/wasm.md, *The published layouts*.
 *
 * It says how to find data and never what data there is: no component, no field. Everything
 * here is read by a module with plain loads, so every member is a fixed width and the structs
 * have no padding a compiler could place differently. */
#ifndef BRIDGE_ABI_H
#define BRIDGE_ABI_H

#include <stdint.h>

/* A module states the version it was built against; one built against another is refused. */
#define BRIDGE_ABI 1

/* Everything a module imports comes from one namespace; the two memories go by name. */
#define BRIDGE_NAMESPACE "bridge"
#define BRIDGE_SHARED    "shared"
#define BRIDGE_PRIVATE   "private"

/* declare's on_unload - docs/bridge/data/lifetime.md. Zero is unset, and refused. */
enum { BRIDGE_STRIP = 1, BRIDGE_SWEEP = 2, BRIDGE_DELETE = 3 };

/* A query term: which component, how the system touches it, and how it must match.
 * `slot` is what declare answered. Zero is unset for `access` and `oper`, and refused. */
enum { BRIDGE_IN = 1, BRIDGE_OUT = 2, BRIDGE_INOUT = 3 };
enum { BRIDGE_AND = 1, BRIDGE_NOT = 2, BRIDGE_OPTIONAL = 3, BRIDGE_OR = 4 };
#define BRIDGE_WILDCARD  0xffffffffu
#define BRIDGE_TERMS_MAX 32

typedef struct {
    uint32_t slot;
    uint8_t  access, oper;
    uint16_t reserved;
} bridge_term;

/* observe's events, as a mask, and what an observer reads: who triggered it, and how. */
enum { BRIDGE_ON_ADD = 1, BRIDGE_ON_SET = 2, BRIDGE_ON_REMOVE = 4 };

typedef struct {
    uint64_t id;
    uint32_t events;
    uint32_t reserved;
} bridge_trigger;

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
    uint32_t column_base;               /* u32 [tables][terms] of shared-memory offsets */
    uint32_t terms, tables;
    uint32_t entity_index;              /* in the shared memory: bridge_entity [entities] */
    uint32_t entities;
    uint32_t when;                      /* set before every dispatch */
    uint32_t system;                    /* the slot of the system running */
    int32_t  log_level;                 /* a line above it is not written */
    uint32_t observers;                 /* bridge_list, one per slot `observe` answered */
} bridge_header;

typedef struct { uint32_t spans, at; } bridge_query;

/* An observer's slot: how many bridge_trigger records this barrier, and where they begin. */
typedef struct { uint32_t count, at; } bridge_list;

/* A span, one per matched table - docs/bridge/data/query.md:
 *     u32 count, u32 entities, u32 base[terms], then at the next multiple of 8, u64 source[terms] */
#define BRIDGE_SPAN_BASE       8u
#define BRIDGE_SPAN_SOURCE(n)  ((8u + 4u * (n) + 7u) & ~7u)
#define BRIDGE_SPAN_SIZE(n)    (BRIDGE_SPAN_SOURCE(n) + 8u * (n))

/* The entity index, by the low 32 bits of an id. Table 0 is no table. */
typedef struct { uint32_t table, row, generation; } bridge_entity;

/* A log record. `text` is an offset into the writer's scratch arena, always. */
typedef struct {
    uint32_t when;
    uint8_t  reserved;
    int8_t   level;
    uint16_t system;
    uint32_t text, len;
    uint32_t arg[2];
} bridge_log_record;

#endif
