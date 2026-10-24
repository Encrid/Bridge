# src/reload/reload_group.c

This file reloads a list of modules as one, between phases.
`bridge_reload` sends a list of more than one here: `docs/intern/bridge/bridge_calls.c.md`, *Functions*.
It includes `src/bridge/bridge_inside.h` and `src/reload/reload.h`.

## As one

The list is replaced at one barrier, or not at all.
Every member must run on a backend that holds both instances at once, or Bridge refuses the list and names the member.
Bridge marks each old as regrouping before any new instance starts.
So a member that supersedes a shared type does not read another member of the list as a rival declarer.

## The order of the steps

1. It validates every member, and marks each old as regrouping.
2. It grows, records and starts each new instance beside its old.
3. It checks the schedule once, for the whole list.
4. It moves the rows once, for every changed type of the list.
5. It ends each old instance, and each new one takes over, at a single barrier.

A failure before the barrier undoes every new instance, clears the marks, and keeps every old.
`docs/extern/bridge.h.md`, *bridge_reload*, tells what a caller sees.

## Functions

| Function | Result |
| --- | --- |
| `undo_member` | Drops one member's new instance, and unparks the systems of its old. |
| `validate_group` | Checks each member. Refuses an empty list, a non-side-by-side module, or a repeated identity. Marks each old as regrouping. |
| `start_members` | Grows, records and starts each new instance beside its old. Gives how many started. |
| `group_fail` | Undoes the started members, clears every mark, frees the list, and gives 1. |
| `bridge_reload_group_now` | Validates, starts, checks the schedule, moves the rows, then commits at one barrier. |
