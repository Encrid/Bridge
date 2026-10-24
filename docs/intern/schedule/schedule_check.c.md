# src/schedule/schedule_check.c

This file gives the refusals of a schedule: a cycle, and two systems that conflict while nothing orders them.

## The refusal

`check_cycle` comes first: in the closed matrix of the phase, a system on a cycle reaches itself, and `schedule_get_cycle` finds it.
Then `schedule_check_phase` looks at the systems of the phase in their order, and stops at the first one that gives a reason.
For system `i`, `check_system` looks only at the systems after `i`, so it checks each pair once.

| Refusal | Condition | Reason in `why` |
| --- | --- | --- |
| A system runs before itself | The pairs and `before` make a cycle together. | The first system of the phase on the cycle, and the phase or the barrier. |
| Two systems are not ordered | Two systems conflict. For one module, `before` puts neither before the other. For two modules, the pairs put neither before the other. | The two systems, the component, and the phase or the barrier. |

`set_levels` must not meet a cycle, since its loop would never end: `docs/intern/schedule/schedule_build.c.md`, *Levels*.
For two systems of one module, only `before` counts, and not a pair: `docs/intern/schedule/schedule.h.md`, *What orders two systems*.
`docs/extern/bridge.h.md`, *A load that failed*, gives how each reason ends.

## Limits

A reason cuts each name to a length: a system name to 80 characters.
A component name is cut to 60 characters, and a phase name to 40.
The whole reason is cut as `docs/extern/bridge.h.md`, *A load that failed*, tells.

## Functions

| Function | Result |
| --- | --- |
| `check_cycle` | Refuses the phase if a system reaches itself in its closed matrix. Gives 1 with `why`. |
| `is_unordered` | Gives whether two systems conflict and nothing orders them: `before` in either direction for one module, the pairs for two modules. |
| `say_unordered` | Writes in `why` the two systems, what they both touch or both attach or detach, and where: in the phase or at the barrier. For one module, it adds that their module states no order of them. |
| `check_system` | Refuses system `i` if a later system conflicts with it and nothing orders them. Gives 1 with `why`. |
| `schedule_check_phase` | Refuses a phase at the first system that gives a reason. Gives 1 if it refuses. |
