# src/schedule/schedule_triggered.c

This file keeps the list of the observers that something triggered since the last barrier, and runs their jobs at the barrier.

## The list

`abi_hear` calls `schedule_trigger` when an observer gets its first change since the last barrier.
The observer goes at the head of `triggered` of the world, through its field `next_triggered`.
Each call of `abi_hear` is on the thread that calls Bridge, so the list needs no lock.

`schedule_untrigger` takes the observers of a module off the list before the module frees them.
`schedule_clear_triggered` sets the count of changes of each listed observer to 0 at the end of a barrier, and empties the list.
No other observer has a change, so the end of a barrier visits no other system.

## A run at the barrier

`schedule_run_triggered` sets one bit for the job of each listed observer, in `build_bits`. No build runs during a barrier, so that room is free.
Then `hand_word` walks the bits in their order. A build sorts the jobs by level and by module, so the walk keeps that order.
It hands each job once, a level at a time, as a phase does.
So a barrier with no triggered observer runs no job, and visits no observer.

## Functions

| Function | Result |
| --- | --- |
| `hand_word(pass, first, word, level)` | Hands the job of each bit of one word, and runs the jobs of a level when the next level starts. Gives the level of the last job. |
| `schedule_trigger(s)` | Puts an observer at the head of the list of its world. |
| `schedule_untrigger(w, m)` | Takes each observer of the module `m` off the list. |
| `schedule_clear_triggered(w)` | Sets the count of changes of each listed observer to 0, and empties the list. |
| `schedule_run_triggered(pass)` | Runs the jobs of the listed observers, as *A run at the barrier* tells. |
