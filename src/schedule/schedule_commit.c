/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "schedule.h"

static uint32_t gather_phase(bridge_world_t *w, uint32_t phase)
{
    bridge_system_t **in = (bridge_system_t **)w->build_in;
    uint32_t n = 0;

    for (uint32_t i = 0; i < w->system_count; i++) {
        if (w->systems[i]->phase == phase && !w->systems[i]->parked) {
            in[n] = w->systems[i];
            n++;
        }
    }
    return (n);
}

static uint32_t count_levels(bridge_world_t *w, uint32_t n)
{
    bridge_system_t *const *in = (bridge_system_t *const *)w->build_in;
    uint64_t *counts = w->build_bits;
    uint32_t levels = 0;

    for (uint32_t i = 0; i < n; i++)
        if (in[i]->level >= levels)
            levels = in[i]->level + 1;
    for (uint32_t l = 0; l < levels; l++)
        counts[l] = 0;
    for (uint32_t i = 0; i < n; i++)
        counts[in[i]->level]++;
    return (levels);
}

static void place_levels(bridge_world_t *w, uint32_t n, uint32_t at,
    uint32_t levels)
{
    bridge_system_t *const *in = (bridge_system_t *const *)w->build_in;
    bridge_system_t **out = (bridge_system_t **)w->run_lists + at;
    uint64_t *counts = w->build_bits;
    uint64_t start = 0;
    uint64_t size;

    for (uint32_t l = 0; l < levels; l++) {
        size = counts[l];
        counts[l] = start;
        start += size;
    }
    for (uint32_t i = 0; i < n; i++) {
        out[counts[in[i]->level]] = in[i];
        counts[in[i]->level]++;
    }
}

static void make_jobs(bridge_world_t *w, uint32_t at, uint32_t n)
{
    bridge_system_t **list = (bridge_system_t **)w->run_lists + at;
    schedule_job_t *jobs = w->run_jobs;
    schedule_job_t *job = nullptr;

    for (uint32_t i = 0; i < n; i++) {
        if (!job || job->m != list[i]->m || job->level != list[i]->level) {
            job = &jobs[w->job_count];
            *job = (schedule_job_t){ .m = list[i]->m, .list = &list[i],
                .phase = list[i]->phase, .level = list[i]->level };
            w->job_count++;
        }
        job->count++;
        list[i]->job = (uint32_t)(job - jobs);
    }
}

void schedule_commit_jobs(bridge_world_t *w)
{
    uint32_t at = 0;
    uint32_t n;

    w->job_count = 0;
    for (uint32_t phase = 0; phase <= w->phase_count; phase++) {
        n = gather_phase(w, phase);
        place_levels(w, n, at, count_levels(w, n));
        make_jobs(w, at, n);
        at += n;
    }
}
