# src/schedule_matrix.c

This file gives the matrix of who runs before whom: a row of bits for each system.

## The matrix

Row `i` has `words` words, and bit `j` of the row says that system `i` runs before system `j`.
`src/schedule_bits.h` gives the words, the bits and the size of a matrix.

## The edges

`schedule_add_edge` gives row `i` the bit of `j` and all of row `j`.
Then `schedule_join_through` of `i` gives the new row `i` to each row that reaches `i`.
If the matrix was closed before the edges from `i`, it is closed again after them.
So `schedule_add_own` needs no closure after it.
Its cost is in `docs/intern/schedule.h.md`, *Cost*.

With a cycle, the matrix is closed too: each system on the cycle then reaches itself.
`src/schedule_closure.c` closes a full matrix.

## Functions

| Function | Result |
| --- | --- |
| `schedule_init_matrix` | Allocates an empty matrix for `n` systems. Gives `false` if there is no memory. |
| `schedule_reaches` | Gives whether `i` runs before `j`. |
| `schedule_add_edge` | Records that `i` runs before `j`, and before all that `j` runs before. |
| `schedule_join_through` | Makes each system that reaches `k` reach all that `k` reaches. |
