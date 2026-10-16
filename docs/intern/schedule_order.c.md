# src/schedule_order.c

This file takes the pairs that a deployment states in `[order]`: two systems that conflict in one phase, in the order they must run.
`src/bridge_create.c` calls `schedule_order` once, at create.

## The pairs

`schedule_order` reads the pairs in their order, with these steps:

1. It allocates the list of names, and `before` and `after` of the world.
2. It allocates a matrix for twice as many names as pairs.
3. For each pair, `add_pair` refuses a pair that does not name two different systems.
4. `add_pair` copies the two names into `before` and `after`.
5. `add_pair` gives each name its index among the names met so far, and adds the edge.
6. `check_pairs` closes the matrix, and refuses the pairs if a name reaches itself.

A pair counts in `order_count` once it has its names, so a refused world frees them.
The function frees the list of names and the matrix before it returns.
A pair need not name a loaded system: `schedule_build` reads `before` and `after` each time it builds the schedule.

## Refusals

Each refusal writes a record and gives 1:

- A pair that does not name two systems: the record gives the index of the pair.
- Pairs that contradict each other: the record names a system that would run before itself.
- No memory for the order: `bridge_lacks` writes the record.

## Functions

| Function | Result |
| --- | --- |
| `is_pair` | Gives whether two names make a pair: neither is null or empty, and they differ. |
| `name_system` | Gives the index of a name among the names met so far, and adds the name if it is new. |
| `add_pair` | Takes pair `i`: keeps its two names and adds the edge from the first to the second. Gives 1, with a record, if refused. |
| `check_pairs` | Refuses pairs that contradict each other: a system that would run before itself. Gives 1 with a record. |
| `schedule_order` | Takes the pairs a deployment states. Gives 0, or 1 with a record. |
