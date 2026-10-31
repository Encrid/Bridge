# src/bridge/bridge_order.c

This file sets the whole `[order]` of a running world again, between two phases.

## The order set again

`bridge_set_order` takes the whole new set of pairs, as `bridge_create` took the first set.
It builds them both again from the whole set, and keeps nothing of the old set: `docs/extern/bridge.h.md`, *bridge_set_order*.

The call is all or nothing. `bridge_set_order_now` keeps the old `schedule_pairs_t` by value, zeroes the world's own, and builds the new set into it.
`schedule_order` refuses a pair that does not fit, and leaves the old set to go back to.
`schedule_mark_all` marks each system, since the new pairs can order any two: `docs/intern/schedule/schedule_marks.c.md`, *The whole schedule*.
`schedule_build` then refuses a schedule that cannot be made, and `roll_back` puts the old set back and marks each system again.
The build of the old set cannot fail, since it is the set that built before, and a build takes no memory.

A refusal keeps the old order and the old schedule. The world is the same as before the call.

## Functions

| Function | Result |
| --- | --- |
| `roll_back` | Records the reason, frees the new set, restores the old one, and rebuilds the schedule. Gives 1. |
| `bridge_set_order_now` | Builds the new set, or restores the old one on a refusal. Its result is in `docs/extern/bridge.h.md`, *bridge_set_order*. |
| `bridge_set_order` | Takes the single-caller lock, then calls `bridge_set_order_now`. |
