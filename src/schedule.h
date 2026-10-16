/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_SCHEDULE_H
    #define BRIDGE_SRC_SCHEDULE_H

    #include "inside.h"

typedef struct {
    uint64_t *bits;
    uint32_t n;
    uint32_t words;
} schedule_matrix_t;

typedef struct {
    const char **names;
    uint32_t n;
    schedule_matrix_t reach;
} schedule_pairs_t;

typedef struct {
    bridge_system_t **in;
    uint32_t n;
    schedule_matrix_t reach;
    uint32_t phase;
    const char *where;
    char *why;
    size_t why_len;
} schedule_phase_t;

typedef struct {
    module_t *m;
    bridge_system_t **list;
    uint32_t count;
} schedule_job_t;

typedef struct {
    bridge_t *b;
    uint32_t phase;
    uint32_t level;
    uint32_t count;
    uint32_t listed;
    uint32_t ordinal;
} schedule_pass_t;

[[nodiscard]] int schedule_order(bridge_t *b, const bridge_order_t *order,
    uint32_t count);

[[nodiscard]] int schedule_build(bridge_t *b, char *why, size_t why_len);

[[nodiscard]] int schedule_room(bridge_t *b);

void schedule_run(bridge_t *b, uint32_t phase);

void schedule_account(bridge_t *b);

bool schedule_init_matrix(schedule_matrix_t *m, uint32_t n);

bool schedule_reaches(const schedule_matrix_t *m, uint32_t i, uint32_t j);

void schedule_add_edge(schedule_matrix_t *m, uint32_t i, uint32_t j);

void schedule_join_through(schedule_matrix_t *m, uint32_t k);

void schedule_close_matrix(schedule_matrix_t *m);

const char *schedule_find_conflict(const bridge_system_t *a,
    const bridge_system_t *b, bool *changing);

void schedule_gather_phase(bridge_t *b, schedule_phase_t *p);

void schedule_add_stated(const bridge_t *b, schedule_phase_t *p);

void schedule_add_own(schedule_phase_t *p);

int schedule_check_phase(schedule_phase_t *p);

void schedule_run_job(void *arg);

void schedule_gather_jobs(schedule_pass_t *pass);

#endif
