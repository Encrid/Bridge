/* A table's id goes when flecs frees the table, by whichever path it frees it.
 *
 * From inside: this is the one test that reads Bridge-ASM700's own structures, because what
 * it checks cannot be seen from a host. An id that outlived its table would be a pointer to
 * freed memory that the next publish hands to flecs, and nothing would say so.
 * docs/bridge/flecs.md, *flecs will not tell you which*. */
#include "inside.h"

#include <stdio.h>

static void sink(void *ud, const bridge_record *r)
{
    (void)ud;
    printf("record[%d]: %.*s\n", r->level, (int)r->len, r->text);
}

static int check(const char *what, int ok)
{
    printf("%s: %s\n", what, ok ? "retired" : "FAIL: its id still names the freed table");
    return !ok;
}

int main()
{
    const char *phases[1] = { "update" };
    bridge_config cfg = { nullptr, 0, phases, 1, nullptr, 0, 1, { 4096, 4096, 1 << 20, 8 } };
    bridge *b = bridge_create(&cfg, sink, nullptr);
    ecs_world_t *w;
    ecs_entity_t kind, other, e, parent, child;
    uint32_t swept, orphaned, reused, before;
    int failed = 0, k;

    if (!b) { printf("FAIL: bridge_create\n"); return 1; }
    w = b->world;

    /* 1. flecs' own sweep of empty tables: no component is deleted, so nothing Bridge-ASM700
     * does itself could have told it */
    kind = ecs_component(w, { .entity = ecs_new(w), .type = { 4, 4 } });
    e = ecs_new(w);
    ecs_add_id(w, e, kind);
    store_touch(b, e);
    store_barrier(b);
    swept = b->index[(uint32_t)e].table;
    if (!swept || !b->tables[swept].table) { printf("FAIL: the entity's table has no id\n"); return 1; }
    ecs_remove_all(w, kind);
    for (k = 0; k < 3; k ++) {
        ecs_delete_empty_tables(w, &(ecs_delete_empty_tables_desc_t){ .delete_generation = 1 });
    }
    failed += check("a table flecs swept as empty", b->tables[swept].table == nullptr);

    /* 2. a table whose type is only a pair: the children of something deleted. A wildcard
     * observer is not told of this one. */
    parent = ecs_new(w);
    child = ecs_new(w);
    ecs_add_pair(w, child, EcsChildOf, parent);
    store_touch(b, child);
    store_barrier(b);
    orphaned = b->index[(uint32_t)child].table;
    if (!orphaned || !b->tables[orphaned].table) { printf("FAIL: the child's table has no id\n"); return 1; }
    ecs_delete(w, parent);
    failed += check("the table of a deleted parent's children", b->tables[orphaned].table == nullptr);

    /* 3. and a retired id is free once the index has let go of it: the next table takes one,
     * and the count of ids does not grow */
    store_barrier(b);
    before = b->table_count;
    other = ecs_component(w, { .entity = ecs_new(w), .type = { 8, 4 } });
    e = ecs_new(w);
    ecs_add_id(w, e, other);
    store_touch(b, e);
    store_barrier(b);
    reused = b->index[(uint32_t)e].table;
    printf("a new table: id %u of %u, %s\n", reused, b->table_count,
        b->table_count == before ? "reused" : "FAIL: a new id though retired ones were free");
    failed += b->table_count != before;

    bridge_destroy(b);
    printf("%s\n", failed ? "FAIL" : "alive");
    return failed;
}
