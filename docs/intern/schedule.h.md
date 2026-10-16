# src/schedule.h

This header gives the schedule: which systems conflict, what orders them, and which systems run at the same time.
It also gives the failure budget that each barrier settles.

## Phases and the barrier

The host asks for the phases one at a time, and a barrier follows each phase.
The schedule has an entry for each phase of the deployment, and one more entry for the barrier, at the index `phase_count`.
A system has the index of its phase, and an observer has `phase_count`: the observers run at the barrier.
`schedule_build` makes the schedule again when a module loads, reloads or unloads.

## Conflicts

Two systems conflict in one of two ways:

| Conflict | Condition |
| --- | --- |
| Through a column | One system writes a component that the other reads or writes. Their terms show it. |
| Through a change | Both systems attach or detach the same component. The list of changes that each system states beside its terms shows it. |

A change is in the conflict graph because a barrier applies the changes in the order the systems ran.
Two modules that attach one component to one entity in one phase leave the value of the module that ran last.
So the deployment must state which one runs last.
A change does not conflict with a term: the barrier applies a change after each system of the phase read its terms.
Every attach and every detach is stated, so two systems that attach one component only to their own entities conflict too.

## What orders two systems

A pair of `[order]` states that one system runs before another.
A pair names a system by the identity of its module without the version, a colon, and the name of the system: `game:actors:movement`.
Without the version in the name, an upgrade of a module does not change the pairs of a deployment.
A pair need not name a loaded system, because a deployment can order modules that it does not load.
`schedule_order` refuses a pair that does not name two systems, and pairs that contradict each other.

A module orders its own systems that conflict by the order in which it registers them.
One author wrote both registrations, so that order stands without a pair.
A pair can state the reverse order, and then the pair stands.

Two systems of two modules that conflict in one phase, with nothing to order them, refuse the load that brings the second one.
The refusal names both systems and the component.
The order of declaration and the order of load never decide, so nothing depends on which module the host found first.
Two observers at the barrier follow the same rule.

## Levels and jobs

A level holds each system whose predecessors are all in earlier levels, and a phase runs its levels one after the other.
In one level, the systems of one module make one job, in the order the module registered them.
One thread runs a job from its start to its end.
The jobs of different modules run at the same time, on the threads of `src/pool.h`.
So the world runs modules in parallel, and the systems of one module one after the other.
The systems of one module never overlap: they share the private memory of the module.

## Cost

For each phase, the schedule keeps a matrix of who must run before whom: a row of bits for each system.
The closure of the matrix joins rows with an or: `n^3 / 64` word operations at a load, for `n` systems in the phase.
The order of registration of the modules keeps the matrix closed, with at most `3 n^3 / 128` more.
For a phase of a thousand systems, that is at most some forty million operations.
None of this runs in a frame.

`schedule_room` makes the room for the schedule and for a dispatch once, at create, `[budget] systems` wide.
So a load, a reload and an unload ask for no memory to build the schedule, and a phase allocates nothing.

## The failure budget

Each system and each observer has its own count of failures.
The limit is `max_system_failures` of its module, and `-1`, `BRIDGE_NO_LIMIT`, means no limit.

- A failed invocation adds 1 to the count of its system. A clean invocation removes 1, and the count does not go below 0.
- When the count of a system reaches the limit, the barrier that follows stops its module. The record names the system.
- The module has its own count for what is no system's. A barrier raises it by 1 if an entity that the module changed could not be placed.
- A destroy that the module may not do raises the same count by 1.
- A barrier that placed each entity the module changed lowers that count by 1. A barrier where the module changed nothing does neither.
- A stopped module keeps its components. Its systems and observers stop.
- If the module states `BRIDGE_FAIL` for `on_error`, its stop stops the world too: the deployment does not run without it.

A system is counted alone, so a clean system of a module does not forgive a system of it that always fails.
A count that falls as fast as it rises stops a system that fails more often than not.
The counts use invocations and barriers, and never time.
So the same history stops the same module at the same place on every run.
`src/schedule_run.c` counts the systems, `src/storage_settle.h` marks the barrier of each module, and `src/schedule_account.c` settles the modules.

## Types

### schedule_matrix_t

Who reaches whom among `n` systems. Bit `j` of row `i` says that system `i` runs before system `j`.

| Field | Content |
| --- | --- |
| `bits` | The rows, one after the other. |
| `n` | How many systems. |
| `words` | How many 64-bit words a row has: `(n + 63) / 64`. |

### schedule_pairs_t

The pairs of `[order]`, as `schedule_order` reads them.

| Field | Content |
| --- | --- |
| `names` | Every name met so far, in the order met. |
| `n` | How many names. |
| `reach` | Who runs before whom among the names. |

### schedule_phase_t

One phase that `schedule_build` schedules, or the barrier after all the phases.

| Field | Content |
| --- | --- |
| `in` | The systems of the phase, in the room of `build_in`. |
| `n` | How many systems. |
| `reach` | Who reaches whom among them. |
| `phase` | The index of the phase, and `phase_count` for the barrier. |
| `where` | The name of the phase, and `nullptr` for the barrier. |
| `why`, `why_len` | Where a refusal is written, and its size. |

### schedule_job_t

The systems of one module in one level, in the order the module registered them: what one thread runs.

| Field | Content |
| --- | --- |
| `m` | The module. |
| `list` | Its systems, in the room of `run_lists`. |
| `count` | How many systems. |

### schedule_pass_t

One phase that `schedule_run` runs.

| Field | Content |
| --- | --- |
| `b` | The world. |
| `phase` | The index of the phase. |
| `level` | The level that runs. |
| `count` | How many jobs the level has. |
| `listed` | How many systems the jobs of the level list. |
| `ordinal` | How many systems have a number in this pass. |

## Functions

| Function | Result |
| --- | --- |
| `schedule_order` | Takes the pairs of `[order]` at create. Gives 1, with a record, if it refuses them. In `src/schedule_order.c`. |
| `schedule_build` | Schedules every phase and the barrier: the edges, the refusal, then the levels. Gives 0, or 1 with `why`. In `src/schedule_build.c`. |
| `schedule_room` | Makes, once, the room of a dispatch and of a schedule. Gives 1, with a record, if there is no memory. In `src/schedule_account.c`. |
| `schedule_run` | Runs the systems of one phase, a level at a time, one job for each module. In `src/schedule_run.c`. |
| `schedule_account` | At a barrier, stops each module that reached its failure budget. In `src/schedule_account.c`. |
| `schedule_init_matrix` | Makes an empty matrix for `n` systems. Gives `false` if there is no memory. In `src/schedule_matrix.c`. |
| `schedule_reaches` | Gives whether `i` runs before `j`. In `src/schedule_matrix.c`. |
| `schedule_add_edge` | Records that `i` runs before `j`, and before all that `j` runs before. In `src/schedule_matrix.c`. |
| `schedule_join_through` | Makes each system that reaches `k` reach all that `k` reaches. In `src/schedule_matrix.c`. |
| `schedule_close_matrix` | Makes each system that reaches a system reach all that it reaches. In `src/schedule_matrix.c`. |
| `schedule_find_conflict` | Gives the name of the component two systems conflict over, or `nullptr`. `changing` tells the kind. In `src/schedule_conflict.c`. |
| `schedule_gather_phase` | Lists the systems of a phase that are not parked, with an empty matrix. In `src/schedule_edges.c`. |
| `schedule_add_stated` | Adds the edges that the pairs of the deployment state. In `src/schedule_edges.c`. |
| `schedule_add_own` | Adds the edges that a module states by the order of its registrations. In `src/schedule_edges.c`. |
| `schedule_check_phase` | Refuses a phase with two unordered conflicting systems of two modules. Gives 1 with `why`. In `src/schedule_check.c`. |
| `schedule_run_job` | What a thread runs: the systems of one job, one after the other. In `src/schedule_dispatch.c`. |
| `schedule_gather_jobs` | Makes the jobs of one level, and gives a number to each system that runs. In `src/schedule_jobs.c`. |
