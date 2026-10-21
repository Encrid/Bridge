# src/lib/bridge/bridge_usage.c

This file answers `bridge_usage`: it gathers the use of every budget key, at the world level, into a `bridge_budget_t`.

## What it gathers

`usage_fill` reads one `bridge_budget_t` from the world. It takes the counts, the arena and column bytes, each tier counted against its cap, and `most_terms` for the term key.
The entities and the tables come from storage: `docs/intern/lib/storage/storage_usage.c.md`, *What it counts*.
With its `peak` flag set it takes the high-water of the entities, tables and the two byte regions; without it, the figure now. The rest of the keys carry no mark, so both reads land on the same number.

`bridge_usage_now` fills `used`, `most`, or both, by whichever the caller passed.
`bridge_usage` wraps it; it holds the world to read a settled state, yet asks for no ownership.

## Functions

| Function | Result |
| --- | --- |
| `most_terms` | Gives the most terms that any one system of the world has. |
| `usage_fill` | Fills one budget struct, with the figure now or, when `peak` is set, the high-water. |
| `bridge_usage_now` | Fills the use now, the high-water, or both. |
| `bridge_usage` | Holds the world, gathers the use, and gives -1 to a call from another thread. |
