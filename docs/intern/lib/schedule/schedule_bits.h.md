# src/lib/schedule/schedule_bits.h

This header gives the operations on the bits of the schedule: the rows of a matrix, in words of 64 bits.
`src/lib/schedule/schedule.h` includes it.

## The bits

Bit `j` of a row is bit `j % 64` of word `j / 64`.
A row of `n` bits has `(n + 63) / 64` words.
`schedule_get_words` computes this in 64 bits, so a count of bits near 2^32 does not give 0 words.
A matrix has one word more than its rows need, so its size is never 0.

## Functions

| Function | Result |
| --- | --- |
| `schedule_get_words` | Gives how many words a row of `n` bits has. |
| `schedule_get_matrix_words` | Gives how many words a matrix of `n` rows of `n` bits has: one more than its rows need. |
| `schedule_has_bit` | Gives whether bit `j` is set. |
| `schedule_put_bit` | Sets bit `j`. |
| `schedule_join_bits` | Sets in `to` each bit that is set in `from`, a word at a time, for `words` words. |
