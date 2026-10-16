# src/os/windows/os_types.h

This header gives the thread, the mutex and the event of `src/os/os.h` on Windows.
It defines `WIN32_LEAN_AND_MEAN`, includes `<windows.h>` and `src/os/os.h`, and the other files of `src/os/windows` include it.

## Types

### struct os_thread

A thread that `os_thread_start` makes.

| Field | Content |
| --- | --- |
| `handle` | The `HANDLE` of the thread. |
| `fn` | The function that the thread runs. |
| `arg` | The argument of `fn`. |

### struct os_mutex

A mutex: a critical section.

| Field | Content |
| --- | --- |
| `section` | The `CRITICAL_SECTION`. |

### struct os_event

An event: an event object of Windows that resets itself when a wait returns for it.

| Field | Content |
| --- | --- |
| `handle` | The `HANDLE` of the event object. |

## Functions

The header declares no function.
