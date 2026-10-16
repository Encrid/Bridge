# src/storage_terms.c

This file takes the terms of a system or of an observer at a load, and forgets them at an unload.

## A record of terms

Storage has `[budget] systems` records, for the systems and the observers together.
The module that registers counted first, so a free record exists.
A record has its own part of each list: `[budget] terms` terms, and a `list` and a `pos` of `table_room` plus 2 places each.
A new system gets each live table that it reads, and storage marks it `changed`, so its module gets its spans.
A forgotten system gives its place in `systems` to the last system.
A forgotten observer gives its place to the observers after it, in their order, since storage tells the observers in the order they came.

## Functions

| Function | Result |
| --- | --- |
| `take_terms` | Takes a free record of terms for a system or an observer, with its own part of each list, and copies the terms into it. |
| `storage_add_system` | Takes the terms of a system, and lists each live table that it reads. |
| `storage_add_observer` | Takes the terms of an observer, and who gets what it is owed. |
| `storage_forget` | Forgets the terms of a system or of an observer, and frees its record. |
