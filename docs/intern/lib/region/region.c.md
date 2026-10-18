# src/lib/region/region.c

This file keeps the list of gaps of a region, and gives a piece back to the region.
`docs/intern/lib/region/region.h.md`, *The gaps*, tells what a gap is.

## The room of the list

Each range handed out can leave one gap after it, and the first range can leave one gap before it.
So a list of gaps for `ranges` ranges has room for `ranges + 1` gaps.
The caller makes this room when it takes a range, so a later give never asks for memory.
`docs/intern/lib/block/block_create.c.md`, *The lists of gaps*, gives the room of the list of the column region.
The list of the persistent arena has room for each range that the modules hold, plus one.
So the list always has room for a freed piece, and `add_gap` does not test it.

## A piece given back

1. Find the place of the piece in offset order.
2. If a gap ends where the piece begins, add the piece to that gap.
3. If not, put a new gap for the piece at its place.
4. If the gap now ends where the next gap begins, join the two gaps.
5. If the gap now ends at `used`, set `used` to the start of the gap and take the gap out.

A gap at the top is not a gap: it becomes part of the space that was never used.

## Functions

| Function | Result |
| --- | --- |
| `region_make_room` | Makes room in the list for the gaps that `ranges` ranges can leave. False if there is not the memory. |
| `region_drop_gap` | Takes gap `i` out of the list and moves the gaps after it down. |
| `add_gap` | Records a freed piece in the gap before `i`, or in a new gap at `i`. Gives the index of its gap. |
| `region_give` | Gives a piece back to the region, with the steps of *A piece given back*. |
