# src/lib/schedule/schedule_before.c

This file gives the order that a module states between its own systems: the bits of `stated_before`.

## The bits

`stated_before` of a system has a bit for each earlier slot of its module, in words of 64 bits.
A system of slot `k` has `schedule_get_words(k)` words, after its changes, in the block of the system.
The bit of slot `j` tells that the module states the system of slot `j` before this one, also through a chain.
The system of slot 0 has no word, since no system of its module comes before it.

The bits of a system have only the words of its own slot, so they are not a row of a `schedule_matrix_t`.
A row of a matrix has the words of all the systems of a phase.

## Functions

| Function | Result |
| --- | --- |
| `schedule_join_before` | Puts system `t`, and each system before `t`, among the systems before `s`. `t` must be an earlier system of the module of `s`. |
| `schedule_is_stated_before` | Gives whether the module states that system `s` runs before system `t`. Gives `false` if the slot of `s` is not below the slot of `t`, since no bit can hold it then. |
