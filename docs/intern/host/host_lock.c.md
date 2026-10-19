# src/host/host_lock.c

This file takes the run lock of a configuration.

`take_lock` makes the `<name>.d` folder, then locks `<name>.d/lock` through `os_lock`.
It prints `refused: <name>.d is no folder` when the folder cannot be made.
It prints `refused: <name> is being run` when another process holds the lock, and `cannot lock '<path>'` when the open fails.

## Functions

| Function | Result |
| --- | --- |
| `take_lock` | Takes the run lock. Gives true when the host holds it. |
