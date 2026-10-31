# src/schedule/schedule_levels.c

This file gives each system of a group its level.

## Levels

A level holds each system whose predecessors are all in earlier levels.
`schedule_set_levels` puts each system of the group at level 0, then calls `raise_all` until a call raises no system.
A group that the check accepts has no cycle, so the levels stop growing, and the loop ends.
`raise_all` makes each system put every system that it runs before at a level after its own.

The levels of a group depend only on its edges: `docs/intern/schedule/schedule_group.c.md`, *A group*.

## Functions

| Function | Result |
| --- | --- |
| `raise_after` | Puts each system that system `i` runs before at a level after the level of `i`. Gives whether it raised one. |
| `raise_all` | Calls `raise_after` for each system of the group. Gives whether one call raised a system. |
| `schedule_set_levels` | Gives each system of the group its level. |
