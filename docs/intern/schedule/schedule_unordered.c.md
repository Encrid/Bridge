# src/schedule/schedule_unordered.c

This file finds the first pair of systems that conflict while nothing orders them, through the index of the components.
`docs/intern/schedule/schedule_uses.c.md` keeps the index.

## The first unordered pair

Only a fresh system can conflict while nothing orders it: `docs/intern/schedule/schedule_marks.c.md`, *The marks*.
`schedule_find_unordered` takes each fresh system of the group, and each system on the lists of its components.
It skips a system of another phase, a parked system, and the fresh system itself.
`schedule_is_unordered` tells which pairs conflict while nothing orders them: `docs/intern/schedule/schedule_check.c.md`.
Of these pairs, it keeps the first: the one whose first system comes first in `systems`, then the one whose second system comes first.
A check of each pair of the phase finds that pair first, so the reason is the same.

A conflict needs a component that both systems touch or change, so the lists hold each system that can conflict.
`at` gives the place of each system in `systems`: `docs/intern/schedule/schedule_build.c.md`, *The loop*.

## Functions

| Function | Result |
| --- | --- |
| `weigh` | Keeps the pair of `s` and `t` if it comes before the kept pair. |
| `check_list` | Weighs each system of a list of uses that conflicts with fresh system `s` while nothing orders them. |
| `schedule_find_unordered` | Gives the first pair of the group that conflicts while nothing orders it, or gives no pair. |
