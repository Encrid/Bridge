# src/storage_apply.c

This file works a barrier an entity at a time.
It folds the queues, finds room, tells the observers, places each entity once, and writes the values.

## The steps of a barrier

Storage reads the queues in their stated order: `docs/intern/storage_runs.h.md`, *The order of the queues*.
Nothing changes that order.
What changes is the thing that storage applies the order to:

1. The fold. Each change goes into a set of bits for its entity, which starts as the set it carries. No row moves.
2. Room. Storage finds or makes the table of each final set, and finds room in it for each row that arrives.
3. Observers. For each changed entity, storage compares what it carried with what it will carry. The difference is what an observer is owed.
4. Placement. Each changed entity moves once, to the table it will be in, or storage destroys it.
5. Values. Storage reads the queues again, and each attach writes its bytes where its entity now is.
6. Release. Storage empties the queues, forgets the fold, and gives back each table that nothing is left in.

At the end of the fold, storage knows the set that each changed entity will carry, and whether it is destroyed.
At the second step, storage finds what it cannot have, while each entity is still where it was: `docs/intern/storage_room.h.md`, *Running out*.
An entity that storage cannot place stays as it was, and storage tells its module.
At the fourth step, what both tables hold goes with the entity, and what is new is zero.
At the fifth step, the last value written stays, because storage writes them in the order they were asked.

An entity that gets 5 components in one barrier moves once, into the table of all 5.
The 4 tables between never exist.
A change at a time would move the entity 5 times, and leave 4 tables behind.

## Functions

| Function | Result |
| --- | --- |
| `fold_all` | The first step. Takes each run of the queues in order, and folds it with `fold_run`. |
| `tell_all` | The third step, when there is an observer. For each changed entity that is not left as it was, gives each observer what it is owed. A destroyed entity has no set after. |
| `place_all` | The fourth step. Places each changed entity that is not left as it was, with `place_one`. |
| `write_all` | The fifth step. Takes each run of the queues in order again, and writes the value of each change with `write_value`. |
| `storage_apply` | Does the 6 steps: `fold_all`, `reserve_all`, `tell_all`, `place_all`, `write_all` and `settle_all`. |
