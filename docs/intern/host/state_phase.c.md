# src/host/state_phase.c

This file takes the phase map of a module.

`add_phase_names` lists the phase names under one table of the module.
`state_take_phases` gathers the names from the three tables, then resolves each name.
`take_phase` finds the value of a phase name and pairs the name with it.

## Functions

| Function | Result |
| --- | --- |
| `state_take_phases` | Fills the phase map. Gives true when every phase resolves. |
