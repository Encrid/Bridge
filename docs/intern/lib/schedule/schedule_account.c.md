# src/lib/schedule/schedule_account.c

This file gives the room in which a dispatch and a schedule work, and the settlement of the failure budgets at a barrier.
It also notes each system that gave `BRIDGE_END`.
The rules of the budget are in `docs/intern/lib/schedule/schedule.h.md`, *The failure budget*.

## The room

`schedule_room` runs once, at create, from `src/lib/bridge/bridge_create.c`.
It allocates these arrays, where `n` is `max_systems`, which is `[budget] systems`.
`block_check_budget` refuses a budget of 0 systems before, so `n` is never 0:

| Array | Content |
| --- | --- |
| `run_jobs` | The jobs of a level: `n` of `schedule_job_t`. |
| `run_handed` | A pointer to each job, as `pool_run` gets them. |
| `run_lists` | The systems that the jobs list, one run of entries for each job. |
| `run_callers` | A byte for each job: 1 if the job must run on the calling thread. |
| `build_in` | The systems of the phase that `schedule_build` works on. |
| `build_bits` | The matrix of a phase: as many words of 64 bits as `schedule_get_matrix_words` gives for `n`. |

If an allocation fails, `bridge_lacks` writes a record and the function gives 1.

## The settlement at a barrier

`src/lib/block/block_barrier.c` calls `schedule_account` at the end of each barrier, after the observers ran.
`schedule_account` settles each module, in the order of the list of modules.
For one module, `settle_module` does these steps:

1. If `fault_now` is set, it adds 1 to `failures`, the count of the module.
2. If not, and `changed_now` is set and `failures` is above 0, it removes 1.
3. It clears `changed_now`, `fault_now` and `reached` for the next barrier.
4. It stops nothing if the module is stopped already, or if `max_failures` is not above 0.
5. It stops the module if a system reached the limit, in `reached`, or if `failures` reached it.

A stopped module gets `stopped`, and `say_stopped` writes a record at the level `BRIDGE_ERR`.
The record names the system, or ends with the words of the fault of this barrier.
A count of the module reaches the limit only at a barrier that raised it, so that barrier had a fault.
If the module states `BRIDGE_FAIL`, `bridge_stop` stops the world too.

## The end of the run

`account_job` calls `schedule_note_end` for each system after its level, on the thread that asked for the phase.
If `ended` of the system is set, `schedule_note_end` sets `end_asked` of the world.
Its record, at the level `BRIDGE_INFO`, names the system or the observer.
`bridge_run_phase` takes `end_asked` after its barrier: `docs/intern/lib/bridge/bridge_run.c.md`, *A phase run*.
An observer at the barrier of a load, an unload or a reload sets `end_asked` too, and the next phase that runs takes it.

## Functions

| Function | Result |
| --- | --- |
| `schedule_room` | Allocates the room of a dispatch and of a schedule, `[budget] systems` wide. Gives 0, or 1 with a record if there is no memory. |
| `say_stopped` | Writes the record that a module is stopped: by the system that reached the limit, or by its own count. |
| `settle_module` | Counts the barrier of one module against its budget, and stops the module if a count reached the limit. |
| `schedule_account` | Settles each module at a barrier. |
| `schedule_note_end` | Sets `end_asked` of the world, with a record, if `ended` of the system is set. |
