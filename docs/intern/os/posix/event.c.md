# src/os/posix/event.c

This file gives the event of `include/os/os.h` on Linux and macOS: a flag, its mutex, and the condition that a wait sleeps on.
Each function keeps the contract of `docs/extern/os/os.h.md`, *Functions*.

## The wait

`os_event_wait` takes the mutex. With `OS_FOREVER` it waits on the condition while the flag is not set,
and a wake with the flag not set waits again. Else it waits on the condition one time, until the flag is
set or a deadline of `nanos` nanoseconds passes. At the end, the function clears the flag: the wait
consumes the event.

## Limits

`os_event_create` checks the results of `pthread_mutex_init` and `pthread_cond_init`, and gives `nullptr` if one fails.

## Functions

| Function | Result |
| --- | --- |
| `os_event_create` | Allocates the event with `malloc`, starts its mutex and its condition with the default attributes, and clears its flag. Gives `nullptr` if there is no memory. |
| `os_event_free` | Destroys the condition and the mutex of the event, and frees it. A null event does nothing. |
| `os_event_set` | Sets the flag under the mutex, and wakes a waiting thread with `pthread_cond_signal`. |
| `os_event_wait` | With `OS_FOREVER`, waits with `pthread_cond_wait` until the flag is set. Else waits once with `pthread_cond_timedwait` until the flag is set or `nanos` pass. Clears the flag. |
