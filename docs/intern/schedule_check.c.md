# src/schedule_check.c

This file gives the refusals of a schedule: a system that runs before itself, and two systems that conflict while nothing orders them.

## The refusals

`schedule_check_phase` looks at the systems of the phase in their order, and stops at the first one that gives a reason.
For system `i`, `check_system` looks only at the systems after `i`, so it checks each pair once.

| Refusal | Condition | Reason in `why` |
| --- | --- | --- |
| A system runs before itself | The closed matrix has the system reach itself. | `[order]` has the system run before itself. |
| Two systems are not ordered | Two systems of two modules conflict, and neither runs before the other. | The two systems, the component they both touch or both attach or detach, and the phase or the barrier. |

The first reason names `[order]`: the edges of a module alone make no loop, since each goes from an earlier system to a later one.
Two systems of one module never cause the second refusal, because their module orders them: `docs/intern/schedule.h.md`, *What orders two systems*.

## Limits

A reason cuts each name to a length: a system name to 80 characters, or 100 in the first reason.
A component name is cut to 60 characters, and a phase name to 40.

## Functions

| Function | Result |
| --- | --- |
| `is_unordered` | Gives whether two systems of two modules conflict and neither runs before the other. |
| `say_unordered` | Writes in `why` the two systems, what they both touch or both attach or detach, and where: in the phase or at the barrier. |
| `check_system` | Refuses system `i` if it runs before itself, or if a later system conflicts with it and nothing orders them. Gives 1 with `why`. |
| `schedule_check_phase` | Refuses a phase at the first system that gives a reason. Gives 1 if it refuses. |
