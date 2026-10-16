# src/region.h

This header gives a region: a stretch of the shared block that the library hands out in pieces and takes back.
The column region and the persistent arena are regions.

## The gaps

The list of the free ranges is outside the block, where a stray write of a module cannot reach it.
A free range is a gap, kept by its offset from the start of the region.
The gaps are in offset order, and a gap is joined to the gaps that it touches.
A request takes the smallest gap that holds it before it takes fresh space.
A region never takes memory from outside itself: it refuses a request that it cannot meet.

## Types

| Type | Content |
| --- | --- |
| `range_t` | A range of a region: `at`, its offset from the start of the region, and `len`, its length in bytes. |
| `region_t` | A region: `gaps` and `count` of them, `room` gaps that the list has memory for, `used` and `len`. |

Everything past `used` was never handed out. `len` is the length of the region.

## Functions

| Function | Source | Result |
| --- | --- | --- |
| `region_make_room` | `src/region.c` | Makes room in the list for the gaps that `ranges` ranges can leave. False if there is not the memory. |
| `region_drop_gap` | `src/region.c` | Takes gap `i` out of the list. |
| `region_give` | `src/region.c` | Gives a piece back. The piece becomes a gap, joined to the gaps that it touches. |
| `region_take` | `src/region_take.c` | Takes a piece of `len` bytes: 1 and its offset, or 0 if the region has no piece that long. |
| `region_grow` | `src/region_take.c` | Makes the piece at `at` longer where it is. False, with no change, if what follows it is not free. |
| `region_get_most` | `src/region_take.c` | Gives the length of the longest piece that the region can give now. |
