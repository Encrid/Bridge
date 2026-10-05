/* rows.h, kept by flecs: what that storage is made of. Only rows-flecs.c and the one test that
 * reads it from inside include this. It goes when flecs does - docs/plan/PHASE10.md. */
#ifndef BRIDGE_ROWS_FLECS_H
#define BRIDGE_ROWS_FLECS_H

#include "inside.h"

#include "flecs.h"
#include "flecs_confine.h"

typedef struct { ecs_table_t *table; int32_t rows; } table_slot;

struct rows_type {
    ecs_entity_t id;
    uint32_t     size;
    rows_field  *fields;            /* a struct's, built when it was declared */
    char       **names;
    uint32_t     field_count;
};

struct rows_terms {
    ecs_query_t *query;             /* nullptr for an observer */
    ecs_entity_t observing;         /* flecs' observer, or 0 */
    rows_heard   heard;
    void        *ctx;
    uint32_t     count;
    uint32_t     sizes[BRIDGE_TERMS_MAX];
};

struct rows {
    bridge      *b;
    ecs_world_t *world;
    /* table ids are ours: dense, and 0 is no table - flecs.md, *Table ids are Bridge-ASM700's own* */
    table_slot  *tables;
    uint32_t     table_count, table_room;
    uint32_t    *table_hash;        /* open addressing over `tables`, by pointer */
    uint32_t     table_hash_room;
    /* ids of tables flecs has deleted: dead until the index has let go of them, then free */
    uint32_t    *dead_tables, *free_tables;
    uint32_t     dead_count, free_count, dead_room, free_room;
    /* entities something named since the last barrier: where they are is read once, then */
    ecs_entity_t *dirty;
    uint32_t     dirty_count, dirty_room;
    uint8_t     *is_dirty;
    ecs_entity_t names;             /* under it, a scope per namespace: where a type name resolves */
    ecs_entity_t text_type;         /* bridge_text, as flecs knows it */
    rows_type  **types;             /* every declared type, to find one by flecs' id */
    uint32_t     type_count;
};

/* e may have moved: where it is now is read at the next barrier */
void rows_flecs_touch(rows *r, ecs_entity_t e);

#endif
