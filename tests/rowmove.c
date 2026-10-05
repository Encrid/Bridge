/* No row moves without something saying so.
 *
 * A shadow index of entity to table and row is kept from two sources only:
 * flecs' events, which name an entity whose components changed, and the
 * row-moved hook. After the workload it must equal flecs' own record for every
 * live entity. Without the hook it cannot: a swap-remove moves an entity that
 * no event names. */
#include "workload.h"

typedef struct { ecs_table_t *table; int32_t row; } where_t;

static where_t *shadow;         /* by entity index */
static ecs_entity_t *dirty;     /* entities an event named since the last sync */
static uint8_t *is_dirty;
static int32_t dirty_count, capacity, hook_calls, event_calls;

static uint32_t index_of(ecs_entity_t e) { return (uint32_t)e; }

static void on_change(ecs_iter_t *it)
{
    int32_t i;
    for (i = 0; i < it->count; i ++) {
        uint32_t idx = index_of(it->entities[i]);
        event_calls ++;
        if (idx < (uint32_t)capacity && !is_dirty[idx]) {
            is_dirty[idx] = 1;
            dirty[dirty_count ++] = it->entities[i];
        }
    }
}

static void on_row_moved(ecs_world_t *world, ecs_entity_t entity, ecs_table_t *table, int32_t row)
{
    uint32_t idx = index_of(entity);
    (void)world;
    hook_calls ++;
    if (idx < (uint32_t)capacity) {
        shadow[idx].table = table;
        shadow[idx].row = row;
    }
}

/* an event says who changed, not where they ended up: read that once the batch is done */
static void sync(workload_t *w)
{
    int32_t i;
    for (i = 0; i < dirty_count; i ++) {
        ecs_entity_t e = dirty[i];
        uint32_t idx = index_of(e);
        const ecs_record_t *r = ecs_is_alive(w->world, e) ? ecs_record_find(w->world, e) : nullptr;
        shadow[idx].table = r ? r->table : nullptr;
        shadow[idx].row = r ? ECS_RECORD_TO_ROW(r->row) : -1;
        is_dirty[idx] = 0;
    }
    dirty_count = 0;
}

static void* os_alloc(ecs_size_t size) { return malloc((size_t)size); }
static void* os_realloc(void *p, ecs_size_t o, ecs_size_t n) { (void)o; return realloc(p, (size_t)n); }
static void os_free(void *p, ecs_size_t size) { (void)size; free(p); }

int main()
{
    int32_t count = 20000, i, wrong = 0, live = 0;
    capacity = count * 2 + 100000;
    shadow = calloc((size_t)capacity, sizeof(where_t));
    dirty = calloc((size_t)capacity, sizeof(ecs_entity_t));
    is_dirty = calloc((size_t)capacity, 1);

    flecs_confine = (flecs_confine_t){ os_alloc, os_realloc, os_free, on_row_moved };

    workload_t w = { .world = ecs_init() };
    ecs_observer(w.world, {
        .query.terms = {{ .id = EcsWildcard }},
        .events = { EcsOnAdd, EcsOnRemove },
        .callback = on_change
    });
    workload_run(&w, count, sync);

    for (i = 0; i < count; i ++) {
        if (!w.e[i]) continue;
        const ecs_record_t *r = ecs_record_find(w.world, w.e[i]);
        where_t *s = &shadow[index_of(w.e[i])];
        live ++;
        if (s->table != r->table || s->row != ECS_RECORD_TO_ROW(r->row)) {
            if (!wrong) {
                printf("row mismatch: entity %u, shadow row %d, real row %d%s\n",
                    index_of(w.e[i]), s->row, ECS_RECORD_TO_ROW(r->row),
                    s->table != r->table ? ", and another table" : "");
            }
            wrong ++;
        }
    }

    printf("%d operations, %d live entities, %d named by an event, %d by the hook, %d wrong\n",
        w.ops, live, event_calls, hook_calls, wrong);
    ecs_fini(w.world);
    if (wrong) {
        printf("FAIL: %d row mismatches\n", wrong);
        return 1;
    }
    return 0;
}
