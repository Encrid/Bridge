# src/os/windows/lock.c

`os_lock` takes the run lock for a Windows process.

The call creates or opens the path, then locks one byte with `_locking` under `_LK_NBLCK`.
A value of 0 means the lock is held, 1 means a different process owns it, and -1 means the open did not work.
The handle closes after `_locking` reports a conflict.

## Functions

| Function | Result |
| --- | --- |
| `os_lock` | Locks one byte with `_locking`. Answers 0, 1 or -1. |
