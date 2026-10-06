# What was refused: The data model

The record of `data.md`: `readme.md`, *Three layers, three seams*.

## Access mode

Of `data.md`, *Access mode*.

Refused: three, `in`, `out` and `inout`, which is what flecs has and what this had. flecs tells a
write that reads from one that does not because it tracks what was read; nothing here does, and the
scheduler asked one thing of the three, whether the term is written. The condition that brings a
third back is something that treats a term written and never read differently.

## Observers

Of `data.md`, *Observers*.

Refused: the names other ECS use, `ON_ADD`, `ON_SET` and `ON_REMOVE`, which is what these were
called. In flecs each is a fact about one component: it was added, `ecs_set` was called, it was
removed. Here each is a fact about an observer's terms, so an "add" could arrive at a detach, and a
"set" never arrived for a value written in place. The condition that brings the names back is events
that are raised for each component, as flecs raises them.

Refused: two events, enter and leave, and no third. A module could then announce a new value only by
a detach and an attach across two barriers. The condition that brings it back is that no module asks
for `ON_ATTACH`.

Refused: an event for every change as it was asked, which is what this did while flecs kept the rows
and what storage of our own first copied. It told an observer of all three events at once, for an
entity it could not then find, and it cost a second reading of every queue. It was never flecs'
design either: when flecs folds an entity's queued commands itself it commits once, with the
difference (`flecs.c:6262`), and what we had was the way we drove it, a call for each change. The
condition that brings it back is a module that must hear of a change that left nothing behind, and
cannot say it with an entity that carries the event.
