# src/block/block_destroy.c

This file gives the shared block back, with what the world kept about it.

## The order

1. Destroy storage.
2. Have each backend that adopted the block release it.
3. Give the address space of the block back to the operating system.
4. Free the list of gaps of each region: the column region and the persistent arena.
5. Free each declared type: its fields, the types its fields name, its name and its zero value.
6. Free the list of where the settings text of unloaded modules was.

A backend frees nothing of the block when it releases it, because the block was never the backend's.
The library gives the block back only after each backend released it.

## Functions

| Function | Result |
| --- | --- |
| `release_shared` | Has each backend that adopted the shared block release it, and then gives the block back to the operating system. |
| `free_types` | Frees each declared type of the world with `module_free_type`, and the list of them. |
| `block_destroy` | Gives the shared block back: storage first, then the block once each backend released it, then each declared type. |
