# src/schedule.h

This header gives the schedule: which systems conflict, what orders them, and which systems run at the same time.
It also gives the failure budget that each barrier settles.

## Phases and the barrier

A barrier follows each phase, and the host asks for one phase at a time.
The schedule has an entry for each phase of the deployment, and one more entry for the barrier, at the index `phase_count`.
A system is in the entry of its phase, and an observer is in the entry of the barrier: `docs/intern/abi_system.c.md`, *observe*.
`schedule_build` makes the schedule again when a module loads, reloads or unloads.

## Conflicts

Two systems conflict in one of two ways:

| Conflict | Condition |
| --- | --- |
| Through a column | `docs/extern/bridge_abi.h.md`, *Conflicts*. |
| Through a change | Both systems attach or detach the same component. The list of changes that each system states beside its terms shows it. |

A change is in the conflict graph because of the order in which a barrier applies the changes: `docs/extern/bridge_abi.h.md`, *Conflicts*.
Two modules that attach one component to one entity in one phase leave the value of the module that ran last.
So the deployment must state which one runs last.
A change does not conflict with a term: the barrier applies a change after each system of the phase read its terms.
Every attach and every detach is stated, so two systems that attach one component only to their own entities conflict too.

## What orders two systems

A pair of `[order]` states that one system runs before another.
A pair names a system by its full name, for example `game:actors:movement`: `docs/extern/bridge.h.md`, *bridge_order_t*.
Without the version in the name, an upgrade of a module does not change the pairs of a deployment.
A deployment can order modules that it does not load: `docs/extern/bridge.h.md`, *bridge_order_t*.
`schedule_order` refuses the pairs that `docs/intern/schedule_order.c.md`, *Refusals*, names.

The world keeps the names of the pairs and their closed matrix from create.
So a chain of pairs orders two systems also through a name that is not loaded.

A module orders its own systems in `before`: `docs/extern/bridge_abi.h.md`, *add_system*.
One author wrote both systems, so their order is in the module.
Two systems of one module that conflict refuse the load if `before` does not order them, also if a pair orders them.
`docs/intern/schedule_build.c.md`, *The steps of a phase*, tells what refuses a pair against `before`.
For a conflict, the order that a module states counts only between the systems of that module.

Two systems of two modules that conflict in one phase refuse the load that brings the second one.
Only a pair, or a chain of pairs, counts for them.
The refusal names both systems and the component.
The order of declaration and the order of load never decide, so nothing depends on which module the host found first.
Two observers at the barrier follow the same rules.

So an unload, a failed load and a reload take away the order of no two loaded systems that conflict.
The `schedule_build` after each of them refuses nothing, and its callers drop no reason when they drop its result.

## Levels and jobs

A phase runs its levels one after the other, and `docs/intern/schedule_build.c.md`, *Levels*, tells what a level holds.
In one level, the systems of one module make one job: `docs/intern/schedule_jobs.c.md`, *The jobs of a level*.
One thread runs a job from its start to its end.
The threads of `src/pool.h` run the jobs of different modules at the same time.
So the world runs modules in parallel, and the systems of one module one after the other.
The systems of one module never overlap: they share the private memory of the module.

## Cost

For each phase, the schedule keeps the matrix of who runs before whom, `n` rows of `n` bits for `n` systems: `docs/intern/schedule_matrix.c.md`, *The matrix*.
The edges of the pairs join rows with an or: at most `n^3 / 64` word operations at a load, for `n` systems in the phase.
The order that the modules state keeps the matrix closed, with at most `3 n^3 / 128` more.
For a phase of a thousand systems, that is at most some forty million operations.
None of this runs in a frame.

`schedule_room` makes the room for the schedule and for a dispatch once, at create, `[budget] systems` wide.
So a load, a reload and an unload ask for no memory to build the schedule, and a phase allocates nothing.
The matrix of the names of `[order]` is closed once, at create: `n^3 / 64` word operations for `n` names.

## The failure budget

Each system and each observer has its own count of failures.
The limit is `max_system_failures` of its module, and `-1`, `BRIDGE_NO_LIMIT`, means no limit.

- An invocation changes the count of its system: `docs/extern/bridge.h.md`, *Failures*.
- When the count of a system reaches the limit, the barrier that follows stops its module. The record names the system.
- The module has its own count for what is no system's. A barrier raises it by 1 if an entity that the module changed could not be placed.
- So does a destroy that it may not do: `docs/intern/block_barrier.c.md`, *The count of a module*.
- A barrier that placed each entity the module changed lowers that count by 1. A barrier where the module changed nothing does neither.
- A stopped module keeps its components. Its systems and observers stop.
- If the module states `BRIDGE_FAIL` for `on_error`, its stop stops the world too: the deployment does not run without it.

A system is counted alone, so a clean system of a module does not forgive a system of it that always fails.
A count that falls as fast as it rises stops a system that fails more often than not.
The counts use invocations and barriers, and never time.
So the same history stops the same module at the same place on every run.
`src/schedule_run.c` counts the systems, `src/storage_settle.h` marks the barrier of each module, and `src/schedule_account.c` settles the modules.

## Types

`schedule_matrix_t` and `schedule_pairs_t` are in `src/inside.h`, since the world holds its pairs: `docs/intern/inside.h.md`, *Types*.

### schedule_phase_t

One phase that `schedule_build` schedules, or the barrier after all the phases.

| Field | Content |
| --- | --- |
| `in` | The systems of the phase, in the room of `build_in`. |
| `n` | How many systems. |
| `reach` | Who reaches whom among them. |
| `order` | The pairs of the world, and their closed matrix. |
| `phase` | The index of the phase, and `phase_count` for the barrier. |
| `where` | Where the phase is, as a reason names it: in the phase of its name, or at the barrier. |
| `why`, `why_len` | Where a refusal is written, and its size. |

### schedule_job_t

What one thread runs: the systems that one module has in one level, in the order of their registration.

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
| `schedule_order` | Takes the pairs of `[order]`: `docs/intern/schedule_order.c.md`, *Functions*. |
| `schedule_build` | Builds the schedule: `docs/intern/schedule_build.c.md`, *The steps of a phase*. |
| `schedule_room` | Makes the room of the schedule: `docs/intern/schedule_account.c.md`, *Functions*. |
| `schedule_run` | Runs one phase: `docs/intern/schedule_run.c.md`, *Functions*. |
| `schedule_account` | Settles the failure budgets: `docs/intern/schedule_account.c.md`, *Functions*. |
| `schedule_init_matrix` | Makes an empty matrix: `docs/intern/schedule_matrix.c.md`, *Functions*. |
| `schedule_reaches` | Tells whether `i` runs before `j`: `docs/intern/schedule_matrix.c.md`, *Functions*. |
| `schedule_add_edge` | Adds an edge: `docs/intern/schedule_matrix.c.md`, *Functions*. |
| `schedule_join_through` | Joins the rows through `k`: `docs/intern/schedule_matrix.c.md`, *Functions*. |
| `schedule_close_matrix` | Closes a matrix: `docs/intern/schedule_closure.c.md`, *Functions*. |
| `schedule_get_cycle` | Finds a system on a cycle: `docs/intern/schedule_closure.c.md`, *Functions*. |
| `schedule_find_conflict` | Finds what two systems conflict over: `docs/intern/schedule_conflict.c.md`, *Functions*. |
| `schedule_gather_phase` | Lists the systems of a phase: `docs/intern/schedule_edges.c.md`, *Functions*. |
| `schedule_add_stated` | Adds the edges of the pairs: `docs/intern/schedule_edges.c.md`, *Functions*. |
| `schedule_find_name` | Finds the index of a full name: `docs/intern/schedule_names.c.md`, *Functions*. |
| `schedule_orders` | Tells whether the pairs order two systems: `docs/intern/schedule_names.c.md`, *Functions*. |
| `schedule_join_before` | Joins `t` to the systems before `s`: `docs/intern/schedule_before.c.md`, *Functions*. |
| `schedule_is_stated_before` | Tells whether `before` orders two systems: `docs/intern/schedule_before.c.md`, *Functions*. |
| `schedule_add_own` | Adds the edges of `before`: `docs/intern/schedule_edges.c.md`, *Functions*. |
| `schedule_check_phase` | Refuses a phase that it cannot schedule: `docs/intern/schedule_check.c.md`, *Functions*. |
| `schedule_run_job` | Runs one job: `docs/intern/schedule_dispatch.c.md`, *Functions*. |
| `schedule_gather_jobs` | Makes the jobs of one level: `docs/intern/schedule_jobs.c.md`, *Functions*. |
