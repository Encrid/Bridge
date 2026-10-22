# src/os/posix/mutex.c

This file gives the mutex of `include/os/os.h` on Linux and macOS: a `pthread_mutex_t` in memory from `malloc`.
Each function keeps the contract of `docs/extern/os/os.h.md`, *Functions*.

## Limits

`os_mutex_create` checks the result of `pthread_mutex_init`, and gives `nullptr` if it fails.

## Functions

| Function | Result |
| --- | --- |
| `os_mutex_create` | Allocates the mutex with `malloc`, and starts it with the default attributes. Gives `nullptr` if there is no memory. |
| `os_mutex_free` | Destroys the mutex and frees it. A null mutex does nothing. |
| `os_mutex_lock` | Locks the mutex with `pthread_mutex_lock`. |
| `os_mutex_unlock` | Unlocks the mutex with `pthread_mutex_unlock`. |
