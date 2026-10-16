# src/block_barrier.c

This file runs the barrier: what happens between two phases, in an order that never changes.
`docs/intern/block.h.md`, *A phase and its barrier*, tells what a barrier is for.

## The steps of a barrier

1. Storage applies the queued changes of each module: `src/storage_apply.c`.
2. Each module gets its ids topped up: `docs/intern/block_module.c.md`, *The ids set aside*.
3. The spans of each running module are published again, until publishing makes no more table ids.
4. Storage ends the publish, and the layout counts as not moved.
5. The observers of what changed run. Their phase is the one past the last phase of the deployment.
6. No system or observer is pending any more, and the count of lifecycle calls goes back to 0.
7. The failure budgets are settled: `src/schedule_account.c`.
8. The log ring of each module is drained to the sink: `docs/intern/block_log.c.md`, *The log ring*.
9. The count of barriers goes up by 1.

The header of a module tells how many table ids there are.
So if publishing makes more table ids, every module is published again, until the number stays the same.

A module whose spans no longer hold what its queries match is stopped, with a record that names the size of its spans.
If it ran on, its systems would run over tables that they were not told about.
A stopped module keeps its components, and its spans are published all over again if it comes back.

## An observer at the barrier

An observer subscribes to a change, and not to a phase.
So it runs at the barrier that applied the change, after the layout changed and the spans were published again.
It reads the state that the change made.

What an observer queues is applied at the next barrier, and not at this one: one hop for each barrier.
A chain of three observers, each one reacting to the one before, settles in three barriers.
A barrier that applied changes until nothing changes would have no bound: one cycle, and the barrier never ends.
With one hop for each barrier, a cycle becomes a stream of work and not a hang.
A cycle is legal, so nothing refuses it.

## The count of a module

Storage that runs out does not stop a world.
A barrier that cannot place an entity, for want of a table or of columns, leaves the entity as it was.
The barrier then counts as one that the module which asked failed in, beside what its systems did.
A destroy that the module may not do counts the same way: `src/storage_settle.h` marks both.
`src/schedule.h` gives the rules of the counts of a module and of its systems, and `src/schedule_account.c` settles them in step 7.

## Functions

| Function | Result |
| --- | --- |
| `refill_all` | Tops up the ids of each module. |
| `publish_all` | Publishes the spans of each running module, and stops a module whose spans no longer hold what its queries match. Marks a stopped module as not published. |
| `republish` | Publishes until publishing makes no more table ids. |
| `finish_barrier` | Ends a barrier once its observers ran: no system pending, the failure budgets settled, each log drained, and the barrier counted. |
| `block_barrier` | Runs the barrier, with the steps of *The steps of a barrier*. |
