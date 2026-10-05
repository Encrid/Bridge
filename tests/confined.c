/* No pointer a module reads lands outside the block.
 *
 * flecs is handed one block through flecs_confine. After a workload that moves,
 * resizes, swap-removes, shrinks and merges, every pointer ecs_get_id answers
 * and every entity array an iterator exposes must lie inside it. */
#include "workload.h"

static char *block;
static size_t block_size, block_used;

/* What confinement keeps out of a module's reach: everything flecs allocates that is not a
 * column. Counted by giving flecs an allocator that remembers each size, beside the bytes of
 * column in use, so the share can be said with a number and not an argument. */
static size_t own_live, column_live;

static void *own_malloc(ecs_size_t size)
{
    size_t *p = malloc((size_t)size + 16);
    p[0] = (size_t)size;
    own_live += (size_t)size;
    return (char *)p + 16;
}

static void *own_calloc(ecs_size_t size)
{
    void *p = own_malloc(size);
    memset(p, 0, (size_t)size);
    return p;
}

static void own_free(void *ptr)
{
    if (!ptr) return;
    own_live -= ((size_t *)((char *)ptr - 16))[0];
    free((char *)ptr - 16);
}

static void *own_realloc(void *ptr, ecs_size_t size)
{
    size_t *p;
    if (!ptr) return own_malloc(size);
    own_live -= ((size_t *)((char *)ptr - 16))[0];
    p = realloc((char *)ptr - 16, (size_t)size + 16);
    p[0] = (size_t)size;
    own_live += (size_t)size;
    return (char *)p + 16;
}

static void* block_alloc(ecs_size_t size)
{
    size_t n = ((size_t)size + 15) & ~(size_t)15;
    if (block_used + n > block_size) {
        fprintf(stderr, "FAIL: the test's block is too small\n");
        exit(2);
    }
    void *p = block + block_used;
    block_used += n;
    column_live += (size_t)size;
    return p;
}

/* a bump allocator: nothing is reused, which is all a coverage test needs */
static void* block_realloc(void *ptr, ecs_size_t old_size, ecs_size_t new_size)
{
    void *p = block_alloc(new_size);
    if (ptr) {
        memcpy(p, ptr, (size_t)(old_size < new_size ? old_size : new_size));
        column_live -= (size_t)old_size;
    }
    return p;
}

static void block_free(void *ptr, ecs_size_t size)
{
    if (ptr) column_live -= (size_t)size;
}

static int inside(const void *p)
{
    return (const char*)p >= block && (const char*)p < block + block_size;
}

static int check(workload_t *w)
{
    ecs_world_t *world = w->world;
    ecs_entity_t ids[2] = { w->position, w->velocity };
    const char *names[2] = { "Position", "Velocity" };
    int escaped = 0, rows = 0, arrays = 0, t, i;

    for (i = 0; i < w->count; i ++) {
        if (!w->e[i]) continue;
        for (t = 0; t < 2; t ++) {
            if (!ecs_has_id(world, w->e[i], ids[t])) continue;
            rows ++;
            if (!inside(ecs_get_id(world, w->e[i], ids[t]))) {
                if (!escaped) {
                    const ecs_record_t *r = ecs_record_find(world, w->e[i]);
                    printf("escaped: %s, row %d\n", names[t], ECS_RECORD_TO_ROW(r->row));
                }
                escaped ++;
            }
        }
    }

    ecs_query_t *q = ecs_query(world, { .terms = {{ .id = w->position }} });
    ecs_iter_t it = ecs_query_iter(world, q);
    while (ecs_query_next(&it)) {
        arrays ++;
        if (!inside(it.entities)) {
            if (!escaped) printf("escaped: the entity array of table %d\n", arrays);
            escaped ++;
        }
    }
    ecs_query_fini(q);

    printf("%8d entities %8d rows checked, %4d entity arrays, %d escaped, block %zu KB\n",
        w->count, rows, arrays, escaped, block_used / 1024);
    printf("%8d entities: flecs' own memory %zu KB, columns %zu KB: %.0f%% is its own\n",
        w->count, own_live / 1024, column_live / 1024,
        100.0 * (double)own_live / (double)(own_live + column_live));
    return escaped;
}

static int run(int32_t count)
{
    block_used = 0;
    own_live = column_live = 0;
    workload_t w = { .world = ecs_init() };
    workload_run(&w, count, nullptr);
    int escaped = check(&w);
    ecs_fini(w.world);
    free(w.e);
    return escaped;
}

int main()
{
    block_size = (size_t)512 << 20;
    block = malloc(block_size);
    flecs_confine = (flecs_confine_t){ block_alloc, block_realloc, block_free, nullptr };
    ecs_os_set_api_defaults();
    ecs_os_api_t os = ecs_os_get_api();
    os.malloc_ = own_malloc;
    os.calloc_ = own_calloc;
    os.realloc_ = own_realloc;
    os.free_ = own_free;
    ecs_os_set_api(&os);

    int escaped = run(4000) + run(200000);
    if (escaped) {
        printf("FAIL: %d pointers escaped the block\n", escaped);
        return 1;
    }
    return 0;
}
