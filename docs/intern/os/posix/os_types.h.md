# src/os/posix/os_types.h

This header gives the thread, the mutex and the event of `include/os/os.h` where there are POSIX threads: on Linux and macOS.
It includes `<pthread.h>` and `include/os/os.h`, and the other files of `src/os/posix` include it.

## Types

### struct os_thread

A thread that `os_thread_start` makes.

| Field | Content |
| --- | --- |
| `handle` | The `pthread_t` of the thread. |
| `fn` | The function that the thread runs. |
| `arg` | The argument of `fn`. |

### struct os_mutex

A mutex.

| Field | Content |
| --- | --- |
| `mutex` | The `pthread_mutex_t`. |

### struct os_event

An event: a flag, the mutex that guards the flag, and a condition for a wait.

| Field | Content |
| --- | --- |
| `lock` | The `pthread_mutex_t` that guards `set`. |
| `cond` | The `pthread_cond_t` that a wait sleeps on. |
| `set` | 1 while the event is set, and 0 when it is not. |

## Functions

The header declares no function.
