# src/lib/storage/storage_sets.h

This header compares and hashes sets of types, and gives the set of a table.
A set is in `docs/intern/lib/storage/storage_set.h.md`, *A set of types*.

## The hash

`hash_set` gives where the hash of tables looks for a set first.
It starts from `1469598103934665603`, and for each word `w` of the set it does `h = (h ^ w) * 1099511628211`.
It then folds the high 32 bits into the low 32 bits, and keeps the bits below `hash_room`, which is a power of two.

## Functions

| Function | Result |
| --- | --- |
| `set_is_same` | Gives whether two sets hold the same types. It compares a word at a time, since a `memcmp` of a length the compiler cannot see is a call. |
| `set_is_empty` | Gives whether a set holds no type. |
| `hash_set` | Gives the first place of a set in the hash of tables. |
| `get_set` | Gives which types the entities of table `t` carry. |
