# src/schedule/schedule_conflict.c

This file tells whether two systems conflict: whether they must not run at the same time.
The two kinds of conflict are in `docs/intern/schedule/schedule.h.md`, *Conflicts*.

## The search

`schedule_find_conflict` looks for a change first, then for a term:

1. `find_changed` looks for a component that is in the change lists of both systems.
2. `find_touched` looks for a component that a term of each system touches, with one of the two terms writing it.

The change list of a system holds each component that the system states it attaches or detaches.
A term `BRIDGE_NOT` must not match, so it touches nothing: its `touches` is `nullptr`, and it conflicts with nothing.
The result is the name of the first component found, in the order of the lists of the first system.
`changing` is `true` if the component comes from the change lists.

## Functions

| Function | Result |
| --- | --- |
| `changes_it` | Gives whether the change list of a system holds a component. |
| `find_changed` | Gives the name of a component that both systems attach or detach, or `nullptr`. |
| `touches_it` | Gives whether a system touches the component of term `i` of another system, with one of the two writing it. |
| `find_touched` | Gives the name of a component that both systems touch with one of them writing it, or `nullptr`. |
| `schedule_find_conflict` | Gives the name of the component two systems conflict over, or `nullptr`. Sets `changing` to tell the kind. |
