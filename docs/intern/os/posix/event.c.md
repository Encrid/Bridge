# src/os/posix/event.c

This file gives the event of `src/os/os.h` on Linux and macOS: a flag, its mutex, and the condition that a wait sleeps on.
Each function keeps the contract of `docs/intern/os/os.h.md`, *Functions*.

## The wait

`os_event_wait` takes the mutex, and waits on the condition while the flag is not set.
A wait that wakes with the flag not set waits again.
At the end, the function reads the flag, clears it, and gives it: the wait consumes the event.
It reads the flag after the last wait, so a set that comes at the end of the time still gives 1.

For a time other than `OS_FOREVER`, the wait ends at a deadline.
`get_deadline` makes the deadline before the wait, from `CLOCK_REALTIME`.
That is the clock of a condition with the default attributes, so a change of the system time moves the end of a wait.

## Limits

`os_event_create` checks the results of `pthread_mutex_init` and `pthread_cond_init`, and gives `nullptr` if one fails.

## Functions

| Function | Result |
| --- | --- |
| `os_event_create` | Allocates the event with `malloc`, starts its mutex and its condition with the default attributes, and clears its flag. Gives `nullptr` if there is no memory. |
| `os_event_free` | Destroys the condition and the mutex of the event, and frees it. A null event does nothing. |
| `os_event_set` | Sets the flag under the mutex, and wakes a waiting thread with `pthread_cond_signal`. |
| `get_deadline` | Gives the time at the end of a wait of `nanos` from now, by `CLOCK_REALTIME`. It carries a second when the nanoseconds reach `OS_SECOND`. |
| `os_event_wait` | Waits with `pthread_cond_timedwait` until the deadline, or with `pthread_cond_wait` for `OS_FOREVER`. Gives 1 if the flag was set, and clears it, or 0 if the time ran out. |
