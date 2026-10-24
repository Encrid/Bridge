# src/reload/reload_group_rows.c

This file moves the rows of a group reload as one, and commits the group.
`docs/intern/reload/reload_rows.c.md`, *All the rows or none*, tells how one move works.
It includes `src/bridge/bridge_inside.h` and `src/reload/reload.h`.

## One move for the list

A group reload gathers the moves of every member into one array, and hands it to `storage_convert` once.
So every changed type of the list moves together, all of them or none.
A type that more than one member declares is superseded once, by the first member that declares its new layout.
The others join that new type, so the array holds one move for it.

## The commit

The commit swaps each old for its new, and resumes each new instance.
It runs only after the schedule and the move have both passed, so it does not fail.

## Functions

| Function | Result |
| --- | --- |
| `count_replaced` | Gives how many types of a member replace another. |
| `fill_from` | Writes the move of each replacing type of a member into the array. |
| `do_moves` | Takes room for the moves of the list, and hands them to `storage_convert`. Gives `STORAGE_NO_MEMORY` if there is no room. |
| `reload_move_group` | Gathers the moves of the list, moves them once, then settles each member. 1 and why if not. |
| `reload_commit_group` | Swaps each old for its new, and resumes each new instance. |
