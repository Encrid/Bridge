# src/reload_share.c

This file grows the persistent share of a module at a reload, and gives the growth back if the reload fails.

## A share that a reload grows

A reload can change a budget of a module, and `persistent` only upwards.
A reload that states less is refused: what the module allocated is in its share, and stays there while the module is loaded.
A reload whose `persistent` rounds up to the same share changes nothing: the same `persistent`, or one more within the same 8 bytes.
A reload that states more adds bytes, so that the share becomes the new `persistent` rounded up to a multiple of 8.

The added bytes come from what is left of the arena:

1. The last range grows where it is, if the module allocates from it and the bytes after it are free.
2. If not, the module gets one range more.

`alloc` moves to the new range as `docs/intern/abi_alloc.c.md`, *The persistent arena*, tells.
The offsets that the module has stay good.
Before `take_range` takes a range, it gets room in the list of gaps, as a load does: `docs/intern/bridge_load.c.md`, *The share of the arena*.
So to give a range back never asks for memory.

If no free piece of the arena holds the added bytes, the reload is refused, and the record tells what is left.
The added bytes are computed in 64 bits, so a `persistent` past 32 bits is refused this way, and is never cut.

## Functions

| Function | Result |
| --- | --- |
| `grow_in_place` | Grows the last range of the module where it is, if the module allocates from it and the bytes after it are free. |
| `take_range` | Takes one more range of the persistent arena for the added bytes. 1, with a record, if it gets no memory, or the arena has no room. |
| `reload_grow_share` | Grows the share of the module to what the reload states: the last range where it is, or a range more. 1, with a record, if refused. |
| `reload_give_back` | Gives back what `reload_grow_share` added, if it added anything. |
