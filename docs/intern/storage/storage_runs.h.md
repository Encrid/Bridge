# src/storage/storage_runs.h

This header takes the queues a run at a time, in the order the calls that filled them ran.

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

Two runs have the same turn only if two passes ran with no barrier between them, since each pass starts its ordinals at 0.
Then the run of the module with the lower `place` goes first, as in the order of the work list: `docs/intern/block/block_sort.c.md`, *The order*.

## The heap of the runs

A binary heap is a list in which each entry comes before the two entries below it.
The entries below the entry `i` are the entries `2 i + 1` and `2 i + 2`.
So entry 0 is the module whose next run comes first.

`reset_marks` puts each module of the work list that has marks into the list `heap` of storage, and makes the list a heap.
`take_next_run` takes the next run of entry 0.
If that module has no mark left, the last entry of the heap moves to entry 0.
Then `sink_run` moves entry 0 down until no entry below it comes before it.
So the cost of a run grows with the base-2 logarithm of the number of modules with marks.

`heap` has room for `[budget] modules` entries.
Only loaded modules are on the work list, and no load goes past that number: `docs/intern/module/module_check.c.md`, *Room for a load*.

## Functions

| Function | Result |
| --- | --- |
| `is_before` | Gives whether the next run of one module comes before the next run of another. Its turn is lower, or the turns are the same and its module has the lower `place`. |
| `sink_run` | Moves an entry of the heap down, past each entry below it that comes first. |
| `reset_marks` | Starts each module of the work list again at its first mark, and makes a heap of the modules that have marks. |
| `take_next_run` | Takes the next run of changes: one module, one turn. Gives the module, or `nullptr` when storage took each queue whole. Writes where the run begins and ends in its queue. |
