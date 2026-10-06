/* What reaching a value by entity id costs with nothing in the way: a C array with a cell for
 * every entity, the id's generation tested and a bit that says the entity has a value. No ECS
 * ships a direct-indexed storage layout to measure ours beside, so this is the least it could
 * cost. docs/plan/STAGE11.md, *Beside the best there is*.
 *
 * Built and run by tools/reference.py. Not part of the library, and not built by build.py. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct { uint32_t table, row, generation; } entity;

constexpr uint32_t ROWS = 204'800, ROUNDS = 32;

static uint64_t now()
{
    struct timespec t;
    timespec_get(&t, TIME_UTC);
    return (uint64_t)t.tv_sec * 1'000'000'000u + (uint64_t)t.tv_nsec;
}

static entity *index_;
static uint32_t *cells;
static uint64_t *given;

[[gnu::noinline]] static uint32_t reached(const uint64_t *ids)
{
    uint32_t total = 0;
    for (uint32_t i = 0; i < ROWS; i ++) {
        uint32_t e = (uint32_t)ids[i];
        if (index_[e].generation != (uint32_t)(ids[i] >> 32)) continue;
        if (given[e >> 6] >> (e & 63) & 1) total += cells[e];
    }
    return total;
}

int main()
{
    uint64_t *near = malloc(ROWS * 8), *far = malloc(ROWS * 8), a = 0, b = 0, t;
    uint32_t state = 2463534242u;
    volatile uint32_t sink = 0;
    index_ = malloc((ROWS + 1) * sizeof *index_);
    cells = malloc((ROWS + 1) * 4);
    given = malloc((ROWS / 64 + 2) * 8);
    memset(given, 0xff, (ROWS / 64 + 2) * 8);
    for (uint32_t i = 0; i < ROWS; i ++) {
        index_[i + 1] = (entity){ 1, i, 1 };
        cells[i + 1] = 1;
        near[i] = (uint64_t)1 << 32 | (i + 1);
    }
    for (uint32_t i = 0; i < ROWS; i ++) {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
        far[i] = (uint64_t)1 << 32 | (state % ROWS + 1);
    }
    for (uint32_t round = 0; round < ROUNDS; round ++) {
        t = now();
        sink = reached(near);
        t = now() - t;
        if (!round || t < a) a = t;
        t = now();
        sink = reached(far);
        t = now() - t;
        if (!round || t < b) b = t;
    }
    printf("a C array by entity: %u values reached by id, in the order they lie %6.3f ns each, in no "
        "order %6.3f ns each\n", ROWS, (double)a / ROWS, (double)b / ROWS);
    (void)sink;
    return 0;
}
