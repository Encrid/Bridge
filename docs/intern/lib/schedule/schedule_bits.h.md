# src/lib/schedule/schedule_bits.h

This header gives how many words a matrix of the schedule has.
`src/lib/schedule/schedule.h` includes it.
`src/lib/bits.h` gives the operations on the bits of a row.

## The matrix

A matrix has one word more than its rows need, so its size is never 0.

## Functions

| Function | Result |
| --- | --- |
| `schedule_get_matrix_words` | Gives how many words a matrix of `n` rows of `n` bits has: one more than its rows need. |
