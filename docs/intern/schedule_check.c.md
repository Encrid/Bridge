# src/schedule_check.c

This file gives the refusal of a schedule: two systems that conflict while nothing orders them.

## The refusal

`schedule_check_phase` looks at the systems of the phase in their order, and stops at the first one that gives a reason.
For system `i`, `check_system` looks only at the systems after `i`, so it checks each pair once.

| Refusal | Condition | Reason in `why` |
| --- | --- | --- |
| Two systems are not ordered | Two systems of two modules conflict, and the pairs put neither before the other. | The two systems, the component they both touch or both attach or detach, and the phase or the barrier. |

No system of a phase runs before itself, so this file does not test it: `docs/intern/schedule_build.c.md`, *The steps of a phase*.
Two systems of one module never cause the refusal, because their module orders them: `docs/intern/schedule.h.md`, *What orders two systems*.

## Limits

A reason cuts each name to a length: a system name to 80 characters.
A component name is cut to 60 characters, and a phase name to 40.

## Functions

| Function | Result |
| --- | --- |
| `is_unordered` | Gives whether two systems of two modules conflict and the pairs put neither before the other: `schedule_orders`. |
| `say_unordered` | Writes in `why` the two systems, what they both touch or both attach or detach, and where: in the phase or at the barrier. |
| `check_system` | Refuses system `i` if a later system conflicts with it and nothing orders them. Gives 1 with `why`. |
| `schedule_check_phase` | Refuses a phase at the first system that gives a reason. Gives 1 if it refuses. |
