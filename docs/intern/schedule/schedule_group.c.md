# src/schedule/schedule_group.c

This file gives the group of one phase: the systems whose levels a build gives again.

## A group

The group of a phase starts with each dirty system of the phase that is not parked.
It then takes each neighbor of a system of the group, until no neighbor is left out.
`docs/intern/schedule/schedule_marks.c.md`, *Neighbors*, tells what a neighbor is.

No edge joins a system of the group to a system out of it.
So the levels of the systems out of the group cannot change, and the build keeps them.
A group can hold more than one set of joined systems, and no edge joins two of them.

## The steps

1. Take a new stamp from `build_stamp` of the world.
2. Put each dirty system of the phase that is not parked into `build_in`, with the stamp in `grouped`.
3. For each system of `build_in`, in turn, add each neighbor that has another stamp, with the stamp.
4. Put the systems with the stamp into `build_in` again, in the order of `systems` of the world.
5. Make the matrix of the group empty, in the room of `build_bits`.

Step 3 adds to the list that it walks, so it stops when no neighbor is left.
Step 4 gives the order of registration, so a reason names the same system as a check of the whole phase.
The stamp rises at each group, so no step takes the stamps away.
`build_in` and `build_bits` are `[budget] systems` wide, and `schedule_room` makes them at create.

## Cost

Step 3 tests each system of the world for each system of the group: `g` times `S` tests for `g` systems in the group.
A load of a module whose systems no pair names makes groups of only its own systems.

## Functions

| Function | Result |
| --- | --- |
| `seed_group` | Puts each dirty system of the phase that is not parked into the group. |
| `add_neighbors` | Puts each neighbor of system `q` of the group into the group. |
| `order_group` | Puts the systems of the group into `build_in`, in the order of `systems`. |
| `schedule_gather_group` | Gives the group of the phase in `build_in`, with an empty matrix for it. |
