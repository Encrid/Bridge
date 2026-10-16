# src/os/windows/thread.c

This file gives the threads and the clock of `src/os/os.h` on Windows.
Each function keeps the contract of `docs/intern/os/os.h.md`, *Functions*.

## The clock

`os_get_now` reads the frequency and the count of the performance counter at each call.
It divides the count by the frequency into whole seconds and a rest, and scales each part to nanoseconds apart.
So the product stays inside 64 bits: `whole * OS_SECOND + rest * OS_SECOND / ticks`, where `ticks` is the frequency.

## Limits

`os_thread_start` checks the result of `CreateThread`. If that call fails, it frees the record and gives `nullptr`.

## Functions

| Function | Result |
| --- | --- |
| `run_thread` | The start function that `CreateThread` gets. It calls `fn(arg)` of the record, and gives 0. |
| `os_thread_start` | Allocates the record of the thread with `malloc`, keeps `fn` and `arg` in it, and starts the thread at once with the default stack size. Gives `nullptr` if there is no memory. |
| `os_thread_join` | Waits for the thread with `WaitForSingleObject` and `INFINITE`, closes its handle, and frees its record. |
| `os_get_thread_id` | Gives the result of `GetCurrentThreadId`, which Windows never makes 0. |
| `os_get_now` | Gives nanoseconds from the performance counter, as *The clock* tells. |
