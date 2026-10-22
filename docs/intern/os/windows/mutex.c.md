# src/os/windows/mutex.c

This file gives `include/os/os.h` its mutex on Windows: a critical section in memory from `malloc`.
Each function keeps the contract of `docs/extern/os/os.h.md`, *Functions*.

## Functions

| Function | Result |
| --- | --- |
| `os_mutex_create` | Allocates the mutex with `malloc`, and starts its critical section with `InitializeCriticalSection`. Gives `nullptr` if there is no memory. |
| `os_mutex_free` | Deletes the critical section with `DeleteCriticalSection`, and frees the mutex. A null mutex does nothing. |
| `os_mutex_lock` | Enters the critical section with `EnterCriticalSection`. |
| `os_mutex_unlock` | Leaves the critical section with `LeaveCriticalSection`. |
