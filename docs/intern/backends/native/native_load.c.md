# src/backends/native/native_load.c

This file fills the entry table a module reads, and gives three of its load-time entries.
`declare`, `add_system` and `observe` each take an offset into the scratch arena and reach `native_enter`.
`docs/intern/backends/native/native_inside.h.md`, *The invariant*, describes `native_enter`.

## Functions

| Function | Result |
| --- | --- |
| `declare` | Reaches the module's `declare` entry with the offset it was given. |
| `add_system` | Reaches the module's `add_system` entry with the offset. |
| `observe` | Reaches the module's `observe` entry with the offset. |
| `native_fill_api` | Writes the eight entry pointers into `bridge_api_t`. |
