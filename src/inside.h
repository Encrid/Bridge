/* Bridge-ASM700's inside: what bridge.c, store.c and abi.c share. Nothing here crosses a seam. */
#ifndef BRIDGE_INSIDE_H
#define BRIDGE_INSIDE_H

#include "bridge.h"
#include "bridge-abi.h"
#include "backend.h"
#include "os.h"

#include "flecs.h"
#include "flecs_confine.h"

#define PAGE 65536u

typedef struct module module;

typedef struct {
    char            *name;
    void            *lib;
    backend_api      api;
    backend         *be;
    uint32_t         capabilities;
    backend_memory  *shared;        /* the shared block, as this backend sees it */
} open_backend;

/* A component, by its namespaced name. flecs holds the storage; this is what load checks. */
typedef struct {
    char        *name;
    ecs_entity_t id;
    uint32_t     size, align;
    uint64_t     fingerprint;       /* of the descriptor string - schema.md, *Fingerprints* */
    void        *zero;              /* `size` zero bytes: what an attach with no value copies */
} component;

typedef struct {
    module       *m;
    char         *name;
    uint32_t      phase;
    ecs_query_t  *query;            /* NULL for a system with no terms */
    uint32_t      terms;
    uint32_t      sizes[BRIDGE_TERMS_MAX];
    backend_func *fn;
} bridge_system;

struct module {
    bridge           *b;
    char             *identity, *ns;
    open_backend     *on;
    backend_module   *image;
    backend_instance *instance;
    backend_memory   *memory;       /* its private block, as its backend sees it */

    /* The private block. A module can write all of it, so every offset Bridge-ASM700 acts on
     * is kept here and never read back from the header. */
    uint8_t  *priv;
    size_t    priv_len, priv_reserve;
    uint32_t  scratch, scratch_len;
    uint32_t  ring, ring_len, ring_tail;
    uint32_t  spans, spans_len;     /* the span region: query slots, column_base, spans */

    component    **declared;        /* by slot */
    uint32_t       declared_count;
    bridge_system **systems;        /* by slot */
    uint32_t       system_count;

    int64_t   fuel;
    char    **read, **write;        /* grants */
    uint32_t  read_count, write_count;

    int   in_init, stopped;
    int   refused;                  /* an entry refused during the call in progress */
    char  why[256];                 /* and this is why */
    module *next;
};

typedef struct { ecs_table_t *table; int32_t rows; } table_slot;
typedef struct { uint32_t *at; uint32_t count, room; } free_list;

struct bridge {
    bridge_sink_fn sink;
    void          *ud;
    open_backend  *backends;
    uint32_t       backend_count;
    /* the address space every block is allocated with: the largest any backend stated */
    uint64_t       reservation, guard;
    char          *stopped_by;      /* why nothing runs any more, until bridge_destroy */

    char   **phases;
    uint32_t phase_count;
    uint32_t barriers;

    /* the shared block: a reserved page, the entity index, the persistent arena, the columns */
    uint8_t       *shared;
    size_t         shared_len, shared_reserve;
    bridge_entity *index;
    uint32_t       entities;
    uint32_t       arena, arena_len, arena_used;
    uint32_t       columns, columns_len, columns_used;
    free_list      classes[32];     /* freed column space by power of two, kept outside the block */
    int            columns_out;     /* the column budget ran out during this barrier */
    int            layout_moved;    /* a column, a table or a declaration changed */

    ecs_world_t   *world;
    /* table ids are ours: dense, and 0 is no table - flecs.md, *Table ids are Bridge-ASM700's own* */
    table_slot    *tables;
    uint32_t       table_count, table_room;
    uint32_t      *table_hash;      /* open addressing over `tables`, by pointer */
    uint32_t       table_hash_room;
    /* entities something named since the last barrier: where they are is read once, then */
    ecs_entity_t  *dirty;
    uint32_t       dirty_count, dirty_room;
    uint8_t       *is_dirty;

    component    **components;
    uint32_t       component_count;
    bridge_system **systems;        /* every module's, in registration order */
    uint32_t       system_count;
    module        *modules;
    uint32_t       module_count, max_modules;
};

/* bridge.c */
void say(bridge *b, const module *m, const bridge_system *s, int8_t level, const char *fmt, ...);
char *copy(const char *s, size_t n);

/* store.c */
int      store_create(bridge *b, const bridge_budget *budget);    /* 0, or a record was emitted */
void     store_destroy(bridge *b);
void     store_touch(bridge *b, ecs_entity_t e);                  /* e may have moved: look at the barrier */
void     store_barrier(bridge *b);
void     store_header(bridge *b, module *m);                      /* what a module reads at offset 0 */
uint32_t store_offset(bridge *b, const void *p);                  /* into the shared memory, 0 for NULL */

/* abi.c */
int abi_define(module *m, backend_err *err);                      /* the eight imports */

#endif
