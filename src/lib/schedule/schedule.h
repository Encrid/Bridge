/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_SCHEDULE_H
    #define BRIDGE_SRC_SCHEDULE_H

    #include "inside.h"
    #include "schedule_bits.h"

typedef struct {
    bridge_system_t **in;
    uint32_t n;
    schedule_matrix_t reach;
    const schedule_pairs_t *order;
    uint32_t phase;
    char where[64];
    char *why;
    size_t why_len;
} schedule_phase_t;

typedef struct {
    module_t *m;
    bridge_system_t **list;
    uint32_t count;
} schedule_job_t;

typedef struct {
    bridge_world_t *w;
    uint32_t phase;
    uint32_t level;
    uint32_t count;
    uint32_t listed;
    uint32_t ordinal;
} schedule_pass_t;

[[nodiscard]] int schedule_order(bridge_world_t *w, const bridge_order_t *order,
    uint32_t count);

[[nodiscard]] int schedule_build(bridge_world_t *w, char *why, size_t why_len);

[[nodiscard]] int schedule_cross_phase(const bridge_world_t *w, char *why,
    size_t why_len);

[[nodiscard]] int schedule_room(bridge_world_t *w);

void schedule_run(bridge_world_t *w, uint32_t phase);

void schedule_account(bridge_world_t *w);

bool schedule_init_matrix(schedule_matrix_t *m, uint32_t n);

bool schedule_reaches(const schedule_matrix_t *m, uint32_t i, uint32_t j);

void schedule_add_edge(schedule_matrix_t *m, uint32_t i, uint32_t j);

void schedule_join_through(schedule_matrix_t *m, uint32_t k);

void schedule_close_matrix(schedule_matrix_t *m);

uint32_t schedule_get_cycle(const schedule_matrix_t *m);

const char *schedule_find_conflict(const bridge_system_t *a,
    const bridge_system_t *w, bool *changing);

void schedule_gather_phase(bridge_world_t *w, schedule_phase_t *p);

void schedule_add_stated(const bridge_world_t *w, schedule_phase_t *p);

uint32_t schedule_find_name(const schedule_pairs_t *order, const char *full);

void schedule_resolve_names(bridge_world_t *w);

void schedule_free_pairs(schedule_pairs_t *o);

bool schedule_orders(const schedule_pairs_t *order, const bridge_system_t *s,
    const bridge_system_t *t);

void schedule_join_before(bridge_system_t *s, const bridge_system_t *t);

bool schedule_is_stated_before(const bridge_system_t *s,
    const bridge_system_t *t);

void schedule_add_own(schedule_phase_t *p);

int schedule_check_phase(schedule_phase_t *p);

void schedule_run_job(void *arg);

void schedule_gather_jobs(schedule_pass_t *pass);

void schedule_validate_system(bridge_system_t *s);

#endif
