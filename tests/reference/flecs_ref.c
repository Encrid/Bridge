/* The work `storage measure` and `storage weigh` time, done by flecs: the reference each
 * storage layout is measured beside. docs/plan/STAGE11.md, *Beside the best there is*.
 *
 * Built and run by tools/reference.py against flecs at a pinned tag, which it fetches. Not
 * part of the library, and not built by build.py.
 *
 * A change is timed both ways flecs has: at once, and deferred between ecs_defer_begin and
 * ecs_defer_end, which is the one that queues as a barrier does. */
#include "flecs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct { uint32_t v; } A;
typedef struct { uint32_t v; } B;
typedef struct { uint32_t v; } S;
typedef struct { uint32_t v; } T;
typedef struct { uint32_t v; uint8_t rest[4092]; } Fat;
typedef struct { uint32_t gravity, wind; } G;

constexpr int32_t ROWS = 204'800;
constexpr uint32_t ROUNDS = 8;

static uint64_t now()
{
    struct timespec t;
    timespec_get(&t, TIME_UTC);
    return (uint64_t)t.tv_sec * 1'000'000'000u + (uint64_t)t.tv_nsec;
}

static uint32_t state = 2463534242u;
static uint32_t random32()
{
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}

#ifdef _WIN32
typedef struct {
    uint32_t cb, faults;
    size_t peak, now, paged_peak, paged, nonpaged_peak, nonpaged, pagefile, pagefile_peak;
} held_counters;
[[gnu::dllimport]] void *GetCurrentProcess();
[[gnu::dllimport]] int K32GetProcessMemoryInfo(void *process, held_counters *counters, uint32_t size);
static size_t held()
{
    held_counters c = { .cb = sizeof c };
    return K32GetProcessMemoryInfo(GetCurrentProcess(), &c, sizeof c) ? c.now >> 10 : 0;
}
#else
static size_t held() { return 0; }
#endif

static ecs_entity_t *made(ecs_world_t *w, ecs_entity_t with, int32_t count)
{
    ecs_entity_t *ids = malloc((size_t)count * sizeof *ids);
    memcpy(ids, ecs_bulk_new_w_id(w, with, count), (size_t)count * sizeof *ids);
    return ids;
}

/* a four-byte component given to, and taken from, an entity that carries `bytes` */
static void change(bool fat, bool sparse, bool deferred)
{
    ecs_world_t *w = ecs_mini();
    ECS_COMPONENT(w, A);
    ECS_COMPONENT(w, Fat);
    ECS_COMPONENT(w, S);
    int32_t count = fat ? 10'000 : ROWS;
    ecs_entity_t *ids;
    uint64_t on = 0, off = 0, t;
    if (sparse) {
        ecs_add_id(w, ecs_id(S), EcsSparse);
        ecs_add_id(w, ecs_id(S), EcsDontFragment);
    }
    ids = made(w, fat ? ecs_id(Fat) : ecs_id(A), count);
    for (uint32_t round = 0; round < ROUNDS; round ++) {
        t = now();
        if (deferred) ecs_defer_begin(w);
        for (int32_t i = 0; i < count; i ++) {
            S value = { 100 + (uint32_t)i };
            ecs_set_id(w, ids[i], ecs_id(S), sizeof value, &value);
        }
        if (deferred) ecs_defer_end(w);
        t = now() - t;
        if (!round || t < on) on = t;
        t = now();
        if (deferred) ecs_defer_begin(w);
        for (int32_t i = 0; i < count; i ++) ecs_remove_id(w, ids[i], ecs_id(S));
        if (deferred) ecs_defer_end(w);
        t = now() - t;
        if (!round || t < off) off = t;
    }
    printf("flecs, %s, %s, an entity of %4u bytes: a component given %6.1f ns, taken %6.1f ns\n",
        sparse ? "sparse and not fragmenting" : "in tables", deferred ? "deferred" : "at once",
        fat ? 4096u : 4u, (double)on / count, (double)off / count);
    free(ids);
    ecs_fini(w);
}

/* the lines of `storage measure`: an entity's first component, one it has, and its end */
static void basics(bool deferred)
{
    ecs_world_t *w = ecs_mini();
    ECS_COMPONENT(w, A);
    constexpr int32_t COUNT = 100'000;
    ecs_entity_t *ids = made(w, 0, COUNT);
    uint64_t first, again, gone, t;
    A value = { 1 };
    t = now();
    if (deferred) ecs_defer_begin(w);
    for (int32_t i = 0; i < COUNT; i ++) ecs_set_id(w, ids[i], ecs_id(A), sizeof value, &value);
    if (deferred) ecs_defer_end(w);
    first = now() - t;
    t = now();
    if (deferred) ecs_defer_begin(w);
    for (int32_t i = 0; i < COUNT; i ++) ecs_set_id(w, ids[i], ecs_id(A), sizeof value, &value);
    if (deferred) ecs_defer_end(w);
    again = now() - t;
    t = now();
    if (deferred) ecs_defer_begin(w);
    for (int32_t i = 0; i < COUNT; i ++) ecs_delete(w, ids[i]);
    if (deferred) ecs_defer_end(w);
    gone = now() - t;
    printf("flecs, in tables, %s: an entity's first component %6.1f ns, one it has %6.1f ns, "
        "destroyed %6.1f ns\n", deferred ? "deferred" : "at once", (double)first / COUNT,
        (double)again / COUNT, (double)gone / COUNT);
    free(ids);
    ecs_fini(w);
}

/* read: a table's column; a sparse component alone; and the sparse one beside the column */
static void reads()
{
    ecs_world_t *w = ecs_mini();
    ECS_COMPONENT(w, A);
    ECS_COMPONENT(w, S);
    ecs_entity_t *ids;
    ecs_query_t *column, *alone, *beside;
    uint64_t best[3] = {}, t;
    uint32_t total = 0;
    ecs_add_id(w, ecs_id(S), EcsSparse);
    ecs_add_id(w, ecs_id(S), EcsDontFragment);
    ids = made(w, ecs_id(A), ROWS);
    for (int32_t i = 0; i < ROWS; i ++) {
        S value = { 1 };
        ecs_set_id(w, ids[i], ecs_id(S), sizeof value, &value);
    }
    column = ecs_query(w, { .terms = {{ ecs_id(A) }} });
    alone = ecs_query(w, { .terms = {{ ecs_id(S) }} });
    beside = ecs_query(w, { .terms = {{ ecs_id(A) }, { ecs_id(S) }} });
    for (uint32_t round = 0; round < ROUNDS * 4; round ++) {
        ecs_iter_t it = ecs_query_iter(w, column);
        t = now();
        while (ecs_query_next(&it)) {
            const A *a = ecs_field(&it, A, 0);
            for (int32_t i = 0; i < it.count; i ++) total += a[i].v;
        }
        t = now() - t;
        if (!round || t < best[0]) best[0] = t;
        it = ecs_query_iter(w, alone);
        t = now();
        while (ecs_query_next(&it)) {
            for (int32_t i = 0; i < it.count; i ++) total += ecs_field_at(&it, S, 0, i)->v;
        }
        t = now() - t;
        if (!round || t < best[1]) best[1] = t;
        it = ecs_query_iter(w, beside);
        t = now();
        while (ecs_query_next(&it)) {
            const A *a = ecs_field(&it, A, 0);
            for (int32_t i = 0; i < it.count; i ++) total += a[i].v + ecs_field_at(&it, S, 1, i)->v;
        }
        t = now() - t;
        if (!round || t < best[2]) best[2] = t;
    }
    printf("flecs, %d rows: a table's column read %6.1f us; a sparse component walked alone %6.1f us; "
        "read beside the column %6.1f us, %.3f ns a row (%u)\n", ROWS, (double)best[0] / 1000.0,
        (double)best[1] / 1000.0, (double)best[2] / 1000.0, (double)best[2] / ROWS, total & 1);
    free(ids);
    ecs_fini(w);
}

/* a value reached by an id that is held: kept in a table, and sparse */
static void reach(bool sparse)
{
    ecs_world_t *w = ecs_mini();
    ECS_COMPONENT(w, A);
    ecs_entity_t *ids, *mixed = malloc((size_t)ROWS * sizeof *mixed);
    uint64_t near = 0, far = 0, t;
    uint32_t total = 0;
    if (sparse) {
        ecs_add_id(w, ecs_id(A), EcsSparse);
        ecs_add_id(w, ecs_id(A), EcsDontFragment);
    }
    ids = made(w, 0, ROWS);
    for (int32_t i = 0; i < ROWS; i ++) {
        A value = { 1 };
        ecs_set_id(w, ids[i], ecs_id(A), sizeof value, &value);
    }
    for (int32_t i = 0; i < ROWS; i ++) mixed[i] = ids[random32() % ROWS];
    for (uint32_t round = 0; round < ROUNDS * 4; round ++) {
        t = now();
        for (int32_t i = 0; i < ROWS; i ++) total += ((const A *)ecs_get_id(w, ids[i], ecs_id(A)))->v;
        t = now() - t;
        if (!round || t < near) near = t;
        t = now();
        for (int32_t i = 0; i < ROWS; i ++) total += ((const A *)ecs_get_id(w, mixed[i], ecs_id(A)))->v;
        t = now() - t;
        if (!round || t < far) far = t;
    }
    printf("flecs, %s: %d values reached by id, in the order they lie %6.3f ns each, in no order "
        "%6.3f ns each (%u)\n", sparse ? "sparse" : "in tables", ROWS, (double)near / ROWS,
        (double)far / ROWS, total & 1);
    free(ids);
    free(mixed);
    ecs_fini(w);
}

/* a toggled component: a row switched, and the rows that are on read */
static void toggle(uint32_t one_in)
{
    ecs_world_t *w = ecs_mini();
    ECS_COMPONENT(w, T);
    ecs_entity_t *ids;
    ecs_query_t *q;
    uint64_t flip = 0, read = 0, t;
    uint32_t total = 0, on = 0;
    ecs_add_id(w, ecs_id(T), EcsCanToggle);
    ids = made(w, 0, ROWS);
    for (int32_t i = 0; i < ROWS; i ++) {
        T value = { 1 };
        ecs_set_id(w, ids[i], ecs_id(T), sizeof value, &value);
    }
    q = ecs_query(w, { .terms = {{ ecs_id(T) }} });
    for (uint32_t round = 0; round < ROUNDS; round ++) {
        t = now();
        for (int32_t i = 0; i < ROWS; i ++) ecs_enable_id(w, ids[i], ecs_id(T), random32() % one_in == 0);
        t = now() - t;
        if (!round || t < flip) flip = t;
    }
    for (uint32_t round = 0; round < ROUNDS * 4; round ++) {
        ecs_iter_t it = ecs_query_iter(w, q);
        on = 0;
        t = now();
        while (ecs_query_next(&it)) {
            const T *v = ecs_field(&it, T, 0);
            for (int32_t i = 0; i < it.count; i ++) total += v[i].v;
            on += (uint32_t)it.count;
        }
        t = now() - t;
        if (!round || t < read) read = t;
    }
    printf("flecs, a toggle, one row in %2u on (%u of %d): the rows that are on read %6.1f us; a row "
        "switched %6.1f ns, with the draw (%u)\n", one_in, on, ROWS, (double)read / 1000.0,
        (double)flip / ROWS, total & 1);
    free(ids);
    ecs_fini(w);
}

static void singleton()
{
    ecs_world_t *w = ecs_mini();
    ECS_COMPONENT(w, G);
    uint64_t best = 0, t;
    uint32_t total = 0;
    ecs_add_id(w, ecs_id(G), EcsSingleton);
    ecs_singleton_set(w, G, { 10, 1 });
    for (uint32_t round = 0; round < ROUNDS * 4; round ++) {
        t = now();
        for (uint32_t i = 0; i < 1'000'000; i ++) total += ecs_singleton_get(w, G)->gravity;
        t = now() - t;
        if (!round || t < best) best = t;
    }
    printf("flecs, a singleton read: %6.3f ns each time (%u)\n", (double)best / 1'000'000, total & 1);
    ecs_fini(w);
}

/* 204,800 entities with a four-byte value and six markers each of twelve, over 4,096 tables */
static void markers(bool tags)
{
    ecs_world_t *w;
    ecs_entity_t value, marker[12];
    size_t before = held();
    w = ecs_mini();
    value = ecs_component_init(w, &(ecs_component_desc_t){ .type = { .size = 4, .alignment = 4 } });
    for (uint32_t k = 0; k < 12; k ++) {
        marker[k] = tags ? ecs_new(w)
            : ecs_component_init(w, &(ecs_component_desc_t){ .type = { .size = 4, .alignment = 4 } });
    }
    for (int32_t i = 0; i < ROWS; i ++) {
        ecs_entity_t e = ecs_new(w);
        ecs_add_id(w, e, value);
        for (uint32_t k = 0; k < 12; k ++) if ((uint32_t)i % 4096 >> k & 1) ecs_add_id(w, e, marker[k]);
    }
    printf("flecs, 204800 entities with six markers each, as %s: the process holds %zu KB more\n",
        tags ? "tags" : "components of four bytes", held() - before);
    ecs_fini(w);
}

int main()
{
    setvbuf(stdout, nullptr, _IONBF, 0);
    basics(false);
    basics(true);
    for (uint32_t k = 0; k < 8; k ++) change(k & 1, k >> 1 & 1, k >> 2 & 1);
    reads();
    reach(false);
    reach(true);
    toggle(1);
    toggle(2);
    toggle(16);
    singleton();
    markers(false);
    markers(true);
    return 0;
}
