# src/bridge/bridge_module_usage.c

This file answers `bridge_module_usage`: it writes one row for each loaded module, of the four budget keys a module owns.

## What it writes

A row carries the four numbers one module owns alone: its systems and observers, its reserved arena bytes, and the entity ids it holds.
Each is a field of the module, so the file reads it straight and copies it to the row, beside the module identity.
It fills a row only when `out` has space for that row, so a null `out` or a short `max` never overflows. It returns the count of modules either way.
`bridge_module_usage` wraps the walk; it holds the world yet asks for no ownership, as the other readers do.

## Functions

| Function | Result |
| --- | --- |
| `bridge_module_usage_now` | Walks the loaded modules and fills up to `max` rows, each the four owned keys of one module. |
| `bridge_module_usage` | Holds the world, runs the walk, and returns -1 when a second thread is mid-call. |
