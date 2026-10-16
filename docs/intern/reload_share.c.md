# src/reload_share.c

This file grows the share of a module of the persistent arena at a reload, and gives the growth back if the reload fails.

## A share that a reload grows

A reload can change a budget of a module, and `persistent` only upwards.
A reload that states less is refused: what the module allocated is in its share, and stays there while the module is loaded.
A reload whose `persistent` rounds up to the same share changes nothing: the same `persistent`, or one more within the same 8 bytes.
A reload that states more adds bytes, so that the share becomes the new `persistent` rounded up to a multiple of 8.

The added bytes come from what is left of the arena:

1. The last range grows where it is, if the module allocates from it and the bytes after it are free.
2. If not, the module gets one range more.

`alloc` moves to the new range when what is asked no longer fits in the range before.
The offsets that the module has stay good. What was left of the range before is dead space.
Before it takes a range, the list of gaps of the arena gets room for one more range.
So to give a range back never asks for memory.

If no free piece of the arena holds the added bytes, the reload is refused, and the record tells what is left.
The added bytes are computed in 64 bits, so a `persistent` past 32 bits is refused this way, and is never cut.

## Functions

| Function | Result |
| --- | --- |
| `grow_in_place` | Grows the last range of the module where it is, if the module allocates from it and the bytes after it are free. |
| `take_range` | Takes one more range of the persistent arena for the added bytes. 1, with a record, if there is no memory or no room. |
| `reload_grow_share` | Grows the share of the module to what the reload states: the last range where it is, or a range more. 1, with a record, if refused. |
| `reload_give_back` | Gives back what `reload_grow_share` added, if it added anything. |
