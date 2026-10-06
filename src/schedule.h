/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/schedule.h -- the schedule: which systems conflict, what orders them,
** and who runs at once.
**
** Who must run before whom is a reachability matrix per phase, a row of bits
** per system, closed by or-ing rows: n^3 / 64 word operations at a load,
** which for a phase of a thousand systems is some sixteen million. Nothing
** of that runs per frame.
**
** schedule.md, *Order within a phase is never implicit*
*/

#ifndef BRIDGE_SRC_SCHEDULE_H
    #define BRIDGE_SRC_SCHEDULE_H

    #include "inside.h"

/*
** Who reaches whom among `n` systems: a row of `words` words for each.
*/
typedef struct {
    uint64_t *bits;
    uint32_t n;
    uint32_t words;
} schedule_matrix_t;

/*
** The pairs a deployment states, as they are read: every name met so far,
** and who runs before whom among them.
*/
typedef struct {
    const char **names;
    uint32_t n;
    schedule_matrix_t reach;
} schedule_pairs_t;

/*
** One phase being scheduled, or the barrier after them all, which is one
** more: its systems, who reaches whom, and where a refusal is written.
** `where` is the phase's name, and nullptr for the barrier.
*/
typedef struct {
    bridge_system_t **in;
    uint32_t n;
    schedule_matrix_t reach;
    uint32_t phase;
    const char *where;
    char *why;
    size_t why_len;
} schedule_phase_t;

/*
** A module's systems of one level, in the order it registered them: what one
** thread runs.
*/
typedef struct {
    module_t *m;
    bridge_system_t **list;
    uint32_t count;
} schedule_job_t;

/*
** One phase being run: the level it is at, how many jobs that level has and
** how many systems they list, and how many systems have been numbered.
*/
typedef struct {
    bridge_t *b;
    uint32_t phase;
    uint32_t level;
    uint32_t count;
    uint32_t listed;
    uint32_t ordinal;
} schedule_pass_t;

/*
** schedule_order
**
** Takes the pairs a deployment states. A pair need not name anything loaded;
** what is refused is a pair that says nothing, and pairs that contradict
** each other. 1, with a record, if refused.
*/
[[nodiscard]] int schedule_order(bridge_t *b, const bridge_order_t *order,
    uint32_t count);

/*
** schedule_build
**
** Schedules every phase, and the barrier after them as one more: edges, the
** refusal, then levels. 0, or 1 with `why` saying why it cannot be.
*/
[[nodiscard]] int schedule_build(bridge_t *b, char *why, size_t why_len);

/*
** schedule_room
**
** Makes, once, what a dispatch and a schedule are worked out in. 1, with a
** record, if there is no memory for it.
**
** budgets.md, *Nothing is allocated in a phase*
*/
[[nodiscard]] int schedule_room(bridge_t *b);

/*
** schedule_run
**
** Runs the systems of one phase, a level at a time, one job per module.
*/
void schedule_run(bridge_t *b, uint32_t phase);

/*
** schedule_account
**
** Stops, at a barrier, each module that reached its failure budget.
**
** failure.md, *The budget*
*/
void schedule_account(bridge_t *b);

/*
** schedule_init_matrix
**
** Makes an empty matrix for `n` systems. false if there was not the memory.
*/
bool schedule_init_matrix(schedule_matrix_t *m, uint32_t n);

/*
** schedule_reaches
**
** Answers whether `i` runs before `j`.
*/
bool schedule_reaches(const schedule_matrix_t *m, uint32_t i, uint32_t j);

/*
** schedule_add_edge
**
** States that `i` runs before `j`.
*/
void schedule_add_edge(schedule_matrix_t *m, uint32_t i, uint32_t j);

/*
** schedule_close_matrix
**
** Makes whatever reaches a system reach all that it does.
*/
void schedule_close_matrix(schedule_matrix_t *m);

/*
** schedule_find_conflict
**
** Answers the name of the component two systems both touch with one of them
** writing it, or that both attach or detach, or nullptr. `changing` says
** which.
*/
const char *schedule_find_conflict(const bridge_system_t *a,
    const bridge_system_t *b, bool *changing);

/*
** schedule_gather_phase
**
** Lists the systems of a phase that are not parked, with an empty matrix
** for them, in room made at create.
*/
void schedule_gather_phase(bridge_t *b, schedule_phase_t *p);

/*
** schedule_add_stated
**
** Adds the edges the deployment's pairs state between systems of the phase.
*/
void schedule_add_stated(const bridge_t *b, schedule_phase_t *p);

/*
** schedule_add_own
**
** Adds the edges a module states by registering: it orders its own systems
** that conflict, unless the deployment's pairs say otherwise.
*/
void schedule_add_own(schedule_phase_t *p);

/*
** schedule_check_phase
**
** Refuses a phase where a system would run before itself, or where two
** systems of two modules conflict and nothing orders them. 1 with `why`.
*/
int schedule_check_phase(schedule_phase_t *p);

/*
** schedule_run_job
**
** What a thread runs: a module's systems of one level, one after another.
*/
void schedule_run_job(void *arg);

/*
** schedule_gather_jobs
**
** Makes the jobs of one level of a phase: a module's systems are gathered
** together, so that its job's list is one run. Each system that runs is
** numbered.
*/
void schedule_gather_jobs(schedule_pass_t *pass);

#endif
