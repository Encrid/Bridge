# src/os/windows/event.c

This file gives `include/os/os.h` its event on Windows: an event object that resets itself when a wait returns for it.
Each function keeps the contract of `docs/extern/os/os.h.md`, *Functions*.

## The wait

`os_event_wait` calls `WaitForSingleObject` with `INFINITE` for `OS_FOREVER`, else with `nanos` as
milliseconds. The event object resets itself when the wait returns for it, so the wait consumes the event.

## Limits

`os_event_create` checks the result of `CreateEventA`, and gives `nullptr` if it fails.

## Functions

| Function | Result |
| --- | --- |
| `os_event_create` | Allocates the event with `malloc`, and makes an event object with `CreateEventA`. The object resets itself, starts not set, and has no name. Gives `nullptr` if there is no memory, or no event object. |
| `os_event_free` | Closes the handle of the event object, and frees the event. A null event does nothing. |
| `os_event_set` | Sets the event object with `SetEvent`. |
| `os_event_wait` | Waits with `WaitForSingleObject`: with no end for `OS_FOREVER`, else `nanos` as milliseconds. |
