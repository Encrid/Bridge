/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/reference/flecs_ref.h -- the work `storage measure` and `storage
** weigh` time, done by flecs: the reference each storage layout is measured
** beside.
**
** Built and run by tools/reference.py against flecs at a pinned tag, which
** it fetches. Not part of the library, and not built by build.py.
**
** A change is timed both ways flecs has: at once, and deferred between
** ecs_defer_begin and ecs_defer_end, which is the one that queues as a
** barrier does.
**
** stage11.md, *Beside the best there is*
*/

#ifndef TEST_FLECS_REF_H
    #define TEST_FLECS_REF_H

    #include "flecs.h"

    #include "reference.h"

constexpr int32_t ROWS = 204'800;
constexpr uint32_t ROUNDS = 8;
constexpr uint32_t MARKERS = 12;

/*
** The components. Each is named for what a measure does with it: kept in a
** table's column, kept sparse, toggled, wide enough that moving its entity
** costs, and held once for the whole world.
*/
typedef struct {
    uint32_t v;
} small_t;

typedef struct {
    uint32_t v;
} sparse_t;

typedef struct {
    uint32_t v;
} toggled_t;

typedef struct {
    uint32_t v;
    uint8_t rest[4092];
} fat_t;

typedef struct {
    uint32_t gravity;
    uint32_t wind;
} globals_t;

/*
** What one measure works on: a world, the entities made in it, the
** component the measure gives, takes or reads, how many entities there
** are, and whether a change is queued between ecs_defer_begin and
** ecs_defer_end. A timed loop copies what it uses out of it before the
** clock is read, so that nothing is read back between two calls of flecs.
*/
typedef struct {
    ecs_world_t *w;
    ecs_entity_t *ids;
    ecs_entity_t id;
    int32_t count;
    bool deferred;
} flecs_ref_t;

    #ifdef _WIN32

/*
** What Windows answers about a process's memory: PROCESS_MEMORY_COUNTERS.
*/
typedef struct {
    uint32_t cb;
    uint32_t faults;
    size_t peak;
    size_t now;
    size_t paged_peak;
    size_t paged;
    size_t nonpaged_peak;
    size_t nonpaged;
    size_t pagefile;
    size_t pagefile_peak;
} held_counters_t;

[[gnu::dllimport]] void *GetCurrentProcess(void);
[[gnu::dllimport]] int K32GetProcessMemoryInfo(void *process,
    held_counters_t *counters, uint32_t size);

    #endif

/*
** flecs_ref_begin
**
** Opens flecs's queue, if the measure's changes are deferred.
*/
static inline void flecs_ref_begin(const flecs_ref_t *r)
{
    if (r->deferred)
        ecs_defer_begin(r->w);
}

/*
** flecs_ref_end
**
** Closes flecs's queue, which applies what it holds, if the measure's
** changes are deferred.
*/
static inline void flecs_ref_end(const flecs_ref_t *r)
{
    if (r->deferred)
        ecs_defer_end(r->w);
}

/*
** flecs_ref_make_ids
**
** Answers `count` new entities that carry `with`, or nothing if it is 0, in
** an array the caller frees.
*/
ecs_entity_t *flecs_ref_make_ids(ecs_world_t *w, ecs_entity_t with,
    int32_t count);

/*
** flecs_ref_set_ones
**
** Gives every entity of the measure its component, four bytes that hold 1.
*/
void flecs_ref_set_ones(const flecs_ref_t *r);

/*
** flecs_ref_make_query
**
** Answers a query for the entities that carry `first`, and `second` too
** unless it is 0.
*/
ecs_query_t *flecs_ref_make_query(ecs_world_t *w, ecs_entity_t first,
    ecs_entity_t second);

/*
** flecs_ref_read_held
**
** Answers the kilobytes the process holds, and 0 where it is not asked.
*/
size_t flecs_ref_read_held(void);

/*
** flecs_ref_open_reads
**
** Makes the entities of the measure, each with the small component in a
** table and the sparse one, which does not fragment, and the three queries
** in `q`: the column, the sparse component, and both.
*/
void flecs_ref_open_reads(flecs_ref_t *r, ecs_query_t **q);

/*
** The measures. Each makes a world, times its work in it and says a line.
** The two that draw are handed the state of the draw, which they move on.
*/
void flecs_ref_measure_basics(bool deferred);
void flecs_ref_measure_change(bool fat, bool sparse, bool deferred);
void flecs_ref_measure_reads(void);
void flecs_ref_measure_reach(uint32_t *state, bool sparse);
void flecs_ref_measure_toggle(uint32_t *state, uint32_t one_in);
void flecs_ref_measure_singleton(void);
void flecs_ref_measure_markers(bool tags);

#endif
