/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_FLECS_REF_H
    #define TEST_FLECS_REF_H

    #include "flecs.h"

    #include "reference.h"

constexpr int32_t ROWS = 204'800;
constexpr uint32_t ROUNDS = 8;
constexpr uint32_t MARKERS = 12;

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

typedef struct {
    ecs_world_t *w;
    ecs_entity_t *ids;
    ecs_entity_t id;
    int32_t count;
    bool deferred;
} flecs_ref_t;

    #ifdef _WIN32

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

static inline void flecs_ref_begin(const flecs_ref_t *r)
{
    if (r->deferred)
        ecs_defer_begin(r->w);
}

static inline void flecs_ref_end(const flecs_ref_t *r)
{
    if (r->deferred)
        ecs_defer_end(r->w);
}

ecs_entity_t *flecs_ref_make_ids(ecs_world_t *w, ecs_entity_t with,
    int32_t count);

void flecs_ref_set_ones(const flecs_ref_t *r);

ecs_query_t *flecs_ref_make_query(ecs_world_t *w, ecs_entity_t first,
    ecs_entity_t second);

size_t flecs_ref_read_held(void);

void flecs_ref_open_reads(flecs_ref_t *r, ecs_query_t **q);

void flecs_ref_measure_basics(bool deferred);
void flecs_ref_measure_change(bool fat, bool sparse, bool deferred);
void flecs_ref_measure_reads(void);
void flecs_ref_measure_reach(uint32_t *state, bool sparse);
void flecs_ref_measure_toggle(uint32_t *state, uint32_t one_in);
void flecs_ref_measure_singleton(void);
void flecs_ref_measure_markers(bool tags);

#endif
