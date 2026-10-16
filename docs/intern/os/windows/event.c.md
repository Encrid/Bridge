# src/os/windows/event.c

This file gives the event of `src/os/os.h` on Windows: an event object that resets itself when a wait returns for it.
Each function keeps the contract of `docs/intern/os/os.h.md`, *Functions*.

## The wait

`os_event_wait` rounds `nanos` up to whole milliseconds, which is the unit of Windows, and calls `WaitForSingleObject`.
`OS_FOREVER` becomes `INFINITE`.
The cast to `DWORD` keeps only the low 32 bits of the count of milliseconds.
A result other than `WAIT_OBJECT_0`, a failure too, gives 0.

## Limits

`os_event_create` checks the result of `CreateEventA`, and gives `nullptr` if it fails.

## Functions

| Function | Result |
| --- | --- |
| `os_event_create` | Allocates the event with `malloc`, and makes an event object with `CreateEventA`. The object resets itself, starts not set, and has no name. Gives `nullptr` if there is no memory, or no event object. |
| `os_event_free` | Closes the handle of the event object, and frees the event. A null event does nothing. |
| `os_event_set` | Sets the event object with `SetEvent`. |
| `os_event_wait` | Waits for at most `nanos`, rounded up to a millisecond, or with no end for `OS_FOREVER`. Gives 1 if the event was set, and 0 if it was not. |
