# src/lib/bridge/bridge_profile.c

This file answers `bridge_profile`: it reads the counters that the dispatch keeps per system and observer.

## What it reads

The dispatch adds to `calls` and `nanos` on each `bridge_system_t` as it runs: `docs/intern/lib/schedule/schedule_dispatch.c.md`, *The time*.
This file only copies those two numbers, with the name, out to the caller.
Nothing here is timed or reset; the reader is cheap and may run between phases.

`profile_rows` walks `w->systems`, the array of every system and observer, and fills one `bridge_profile_row_t` per entry.
It writes an entry only while the caller gave room, so a null array or a small `max` is safe.
`bridge_profile` wraps it: it claims the world to read a steady array, but needs no ownership, as `bridge_list_modules` does not.

## Functions

| Function | Result |
| --- | --- |
| `profile_rows` | Fills up to `max` rows from `w->systems`, each a name and its two counts, and returns the total. |
| `bridge_profile` | Claims the world, calls `profile_rows`, and gives -1 when another thread holds a call. |
