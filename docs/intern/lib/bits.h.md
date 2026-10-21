# src/lib/bits.h

This header gives the operations on a set of bits, in words of 64 bits.
Storage, the schedule and the block share it.

## The bits

Bit `i` of a set is bit `i % 64` of word `i / 64`.
A set of `n` bits has `(n + 63) / 64` words.
`bits_words` computes this in 64 bits, so a count of bits near 2^32 does not give 0 words.

## Functions

| Function | Result |
| --- | --- |
| `bits_words` | Gives how many words a set of `n` bits has. |
| `bits_has` | Gives whether bit `i` is set. |
| `bits_set` | Sets bit `i`. |
| `bits_clear` | Clears bit `i`. |
| `bits_or` | Sets in `to` each bit that is set in `from`, a word at a time, for `words` words. |
