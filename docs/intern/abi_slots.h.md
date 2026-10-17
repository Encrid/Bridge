# src/abi_slots.h

This header reads a list of slots that a module put in its scratch arena: the changes of a system, or the systems before it.

## A list of slots

A list is `count` slots, each a `u32`, at an offset that the module chose.
`get_slots` gives the place of the list in the scratch arena, or null if the list does not lie in it.
A list of 0 slots needs no place: `get_slots` gives an empty text, and looks at nothing.
`get_slot` copies slot `i` out of the list, since the module can put the list at an offset that is not aligned.

## Functions

| Function | Result |
| --- | --- |
| `get_slots` | Gives the place of a list of `count` slots in the scratch arena, an empty text for 0 slots, or null. |
| `get_slot` | Gives slot `i` of a list. |
