# src/block/block_barrier.c

This file runs the barrier: what happens between two phases, in an order that never changes.
`docs/intern/block/block.h.md`, *A phase and its barrier*, tells what a barrier is for.

## The steps of a barrier

1. Storage ends the hold on each id that a module handed out: `docs/intern/storage/storage_held.c.md`.
2. Storage applies the queued changes of each module: `src/storage/storage_apply.c`.
3. Each module gets its ids topped up: `docs/intern/block/block_module.c.md`, *The ids set aside*.
4. The spans of each running module are published again.
5. Storage ends the publish, and the layout counts as not moved. When `validate` is on, the columns become the baseline of the next check: `docs/intern/storage/storage_validate.c.md`.
6. The observers of what changed run in their phase: `docs/intern/abi/abi_system.c.md`, *observe*.
7. No system or observer is pending any more, and the count of lifecycle calls goes back to 0.
8. The failure budgets are settled: `src/schedule/schedule_account.c`.
9. The log ring of each module is drained to the sink: `docs/intern/block/block_log.c.md`, *The log ring*.
10. The count of barriers goes up by 1.

The header of a module tells how many table ids there are.
That number is `[budget] tables` and `[budget] sparse_sets`, and 1. It is fixed when the world is made, so one publish is enough.

The barrier stops a module whose span region cannot hold its spans, or the list of an observer: `docs/extern/bridge.h.md`, *The private memory*, and `docs/extern/bridge_abi.h.md`, *observe*.
Its record names the size of its spans, and the observer whose list did not fit.
If its `on_error` is `BRIDGE_FAIL`, the world stops too: `docs/intern/bridge/bridge_stop.c.md`.
If it ran on, its systems would run over tables that they were not told about.
A stopped module keeps its components, and storage writes nothing more for it.
It comes back only as a new record, by a load or a reload, which storage writes as a whole.

## Passes

`block_barrier` runs the steps above, and then runs them again at once while both of these hold:

1. A module has a change in its queue, which the observers of the last barrier queued.
2. Fewer than `observer_passes` barriers ran in this call.

Each pass is a whole barrier with the ten steps, so each rule of a barrier holds for it: `docs/extern/bridge.h.md`, *Observer passes*.
A barrier whose observers queued nothing ends the passes, since the next barrier would apply nothing.

## An observer at the barrier

An observer subscribes to a change, and not to a phase.
So it runs at the barrier that applied the change, after the layout changed and the spans were published again.
It reads the state that the change made.

What an observer queues is applied at the next barrier, and not at this one: one hop for each barrier.
A chain of three observers, each one reacting to the one before, settles in three barriers.
A barrier that applied changes until nothing changes would have no bound: one cycle, and the barrier never ends.
With one hop for each barrier, a cycle becomes a stream of work and not a hang.
A cycle is legal, so nothing refuses it. `observer_passes` keeps the bound: one call runs that many barriers at most.

## The count of a module

Storage that runs out does not stop a world.
A barrier that has no table or no columns for an entity leaves the entity as it was: `docs/extern/bridge.h.md`, *Running out*.
The barrier then counts as one that the module which asked failed in, beside what its systems did.
A destroy that the module may not do counts the same way: `src/storage/storage_settle.h` marks both.
`src/schedule/schedule.h` gives the rules of the counts of a module and of its systems, and `src/schedule/schedule_account.c` settles them in step 8.

## Functions

| Function | Result |
| --- | --- |
| `refill_all` | Tops up the ids of each module. |
| `publish_all` | Publishes the spans of each running module, and stops a module whose spans do not fit. |
| `finish_barrier` | Ends a barrier once its observers ran: no system pending, the failure budgets settled, each log drained, and the barrier counted. |
| `block_barrier` | Runs the barrier, with the steps of *The steps of a barrier*, and runs it again by the rules of *Passes*. |
