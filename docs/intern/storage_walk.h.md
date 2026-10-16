# src/storage_walk.h

This header walks the types of a set, in the order of their numbers or against it.

## A walk

A walk keeps the word that it is in and what is left of that word.
So the next type is one count of zeros and one bit cleared.
Measured, to ask a set again for the next type costs 20 ns more for each table made, with 9 types to a table.
At the end, a walk gives `type_room`, the count of types there may be.

## Types

### walk_t

A walk of a set.

| Field | Content |
| --- | --- |
| `set` | The set that it walks. |
| `bits` | What is left of the word that it is in. |
| `w` | The word that it is in. |

## Functions

| Function | Result |
| --- | --- |
| `walk_set` | Starts a walk of a set from its lowest type. |
| `walk_next` | Gives the next type of the walk, upward, or `type_room` when none is left. |
| `walk_back` | Starts a walk of a set from its highest type. |
| `walk_prev` | Gives the next type of a walk downward, or `type_room` when none is left. |
