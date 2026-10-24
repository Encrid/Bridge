# src/schedule/schedule_order.c

This file takes the pairs that a deployment states in `[order]`: two systems, in the order they must run.
`src/bridge/bridge_create.c` calls `schedule_order` once, at create.

## The pairs

`schedule_order` reads the pairs in their order, with these steps:

1. It allocates the list of names in `order` of the world.
2. It allocates there a matrix for twice as many names as pairs.
3. For each pair, `add_pair` refuses a pair unless `is_full_name` accepts its two names, and they differ.
4. `add_pair` gives each name its index among the names met so far, and copies a new name into the list.
5. `add_pair` adds the edge from the first name to the second.
6. `check_pairs` closes the matrix, and refuses the pairs if a name reaches itself: `schedule_get_cycle`.

The matrix has a row for twice as many names as pairs, and a row past the last name is empty.
So `check_pairs` compares the result of `schedule_get_cycle` with the count of rows, and not with the count of names.

A name counts in `order` once it is copied, so a refused world frees it.
The world keeps the names, the pairs and the closed matrix until `bridge_destroy`.
A pair need not name a loaded system: `schedule_build` reads the matrix each time it builds the schedule.
The world keeps each pair by index so that a load refuses a pair of two phases: `docs/intern/schedule/schedule_cross.c.md`.

The name of a system has no `@` and no `:`: `docs/intern/abi/abi_subscriber.c.md`.
So a full name has no `@`, and the name of the system follows its last colon.
`is_full_name` refuses a name that no system can have. So Bridge accepts no pair that can never order a system.

## Refusals

Each refusal writes a record and gives 1:

- A pair that does not name two systems: the record gives the index of the pair.
- Pairs that contradict each other: the record names a system that would run before itself.
- No memory for the order: `bridge_lacks` writes the record.

## Functions

| Function | Result |
| --- | --- |
| `is_full_name` | Gives whether a system can have the name: it has a colon, no `@`, and 1 to `NAME_MOST` bytes after its last colon. |
| `name_system` | Gives the index of a name among the names met so far, and copies the name into the list if it is new. Gives the count of names if there is no memory for the copy. |
| `add_pair` | Takes pair `i`: keeps its two names and adds the edge from the first to the second. Gives 1, with a record, if refused. |
| `check_pairs` | Refuses pairs that contradict each other: a system that would run before itself. Gives 1 with a record. |
| `schedule_order` | Takes the pairs a deployment states. Gives 0, or 1 with a record. |
