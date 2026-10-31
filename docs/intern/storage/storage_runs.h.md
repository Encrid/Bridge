# src/storage/storage_runs.h

This header takes the queues a run at a time, in the order the calls that filled them ran.
It also writes the values that the changes carry.

## The order of the queues

While a phase runs, each module puts its destroy, attach and detach calls on a queue of its own, so no entry takes a lock.
The barrier, where one thread has the storage, applies the queues in the order the calls that filled them ran:

1. The observers of the barrier before.
2. Then each lifecycle call made since that barrier.
3. Then the systems of the phase, a level at a time.

Within one call, storage applies the changes in the order the call asked them.
Two systems of one level are in no order, and need none.
Had they changed the same component, the schedule would order them: `docs/intern/schedule/schedule.h.md`, *What orders two systems*.
That order is the same on each run, on one thread or on many.
So the ids that a module gets, and the rows that its changes land in, are the same on each run too.

The queue of a module is in that order already.
So each module keeps a mark for each call that queued something: the turn of the call, and where its changes begin in the queue.
The barrier takes next the run of the module whose next mark has the lowest turn.
It looks only at the modules of the work list, since each module that queued something ran code: `docs/intern/block/block_work.c.md`.
The turn of a system is in `docs/intern/schedule/schedule_jobs.c.md`, *The turn of a system*, and a lifecycle call has the kind `TURN_LIFECYCLE`.

## Functions

| Function | Result |
| --- | --- |
| `reset_marks` | Starts each module of the work list again at its first mark. |
| `find_next_run` | Gives the module whose next run comes first, or `nullptr` when storage took each queue whole. |
| `take_next_run` | Takes the next run of changes: one module, one turn. Gives the module, and writes where the run begins and ends in its queue. |
| `is_written` | Gives whether a change is an attach whose value storage writes. The barrier changed its entity, did not destroy it or leave it as it was, and the id has its generation. |
| `write_value` | Passes over a change of `CHANGE_SKIPPED`. Writes the value of each other change where its entity has the type. For an entity left as it was, it counts the change for the module. `left[0]` counts for want of a table, and `left[1]` for want of columns. |
