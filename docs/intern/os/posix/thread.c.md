# src/os/posix/thread.c

This file gives the threads and the clock of `include/os/os.h` on Linux and macOS.
Each function keeps the contract of `docs/extern/os/os.h.md`, *Functions*.

## Limits

`os_thread_start` checks the result of `pthread_create`. If that call fails, it frees the record and gives `nullptr`.

## Functions

| Function | Result |
| --- | --- |
| `run_thread` | The start function that `pthread_create` gets. It calls `fn(arg)` of the record, and gives `nullptr`. |
| `os_thread_start` | Allocates the record of the thread with `malloc`, keeps `fn` and `arg` in it, and starts the thread with the default attributes. Gives `nullptr` if there is no memory. |
| `os_thread_join` | Waits for the thread with `pthread_join`, and frees its record. |
| `os_get_thread_id` | Gives the `pthread_t` of the calling thread as a number, which is never 0 for a thread that runs. |
| `os_get_now` | Gives the nanoseconds of `CLOCK_MONOTONIC`: its seconds times `OS_SECOND`, plus its nanoseconds. |
