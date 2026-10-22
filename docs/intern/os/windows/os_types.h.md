# src/os/windows/os_types.h

This header gives `include/os/os.h` its thread, its mutex and its event on Windows.
It defines `WIN32_LEAN_AND_MEAN`, includes `<windows.h>` and `include/os/os.h`, and the other files of `src/os/windows` include it.

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

An event: an event object of Windows, which resets itself as a wait returns for it.

| Field | Content |
| --- | --- |
| `handle` | The `HANDLE` of the event object. |

## Functions

The header declares no function.
