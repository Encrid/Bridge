# src/schedule_matrix.c

This file gives the matrix of who runs before whom: a row of bits for each system.

## The matrix

Row `i` has `words` words of 64 bits, and bit `j` of the row says that system `i` runs before system `j`.
Bridge computes `words` in 64 bits, so a count of systems near 2^32 does not give 0 words.
Word `j / 64` of the row holds that bit, at the place `j % 64`.
A matrix has one word more than its rows need, so its size is never 0.

## The closure

`schedule_close_matrix` is the closure of Warshall, one system at a time.
For each system `k`, `schedule_join_through` gives each row that reaches `k` the row of `k`, with an or, a word at a time.
After it, each system that reaches a system reaches all that system reaches: the matrix is closed.

`schedule_add_edge` gives row `i` the bit of `j` and all of row `j`.
Then `schedule_join_through` of `i` gives the new row `i` to each row that reaches `i`.
If the matrix was closed before the edges from `i`, it is closed again after them.
So `schedule_add_own` needs no closure after it.
Its cost is in `docs/intern/schedule.h.md`, *Cost*.

## Functions

| Function | Result |
| --- | --- |
| `schedule_init_matrix` | Allocates an empty matrix for `n` systems. Gives `false` if there is no memory. |
| `schedule_reaches` | Gives whether `i` runs before `j`. |
| `schedule_add_edge` | Records that `i` runs before `j`, and before all that `j` runs before. |
| `schedule_join_through` | Makes each system that reaches `k` reach all that `k` reaches. |
| `schedule_close_matrix` | Calls `schedule_join_through` for each system: the closure. |
