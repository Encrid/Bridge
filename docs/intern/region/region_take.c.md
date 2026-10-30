# src/region/region_take.c

This file takes a piece from a region, makes a piece longer, and tells the longest piece that a region can give.
`docs/intern/region/region.h.md`, *The gaps*, tells what a gap is.

## A piece taken

A piece comes from the smallest gap that holds it. Thus the larger gaps stay for the larger pieces.
The region has this one fit, so the fit is fixed, and is no key.
If no gap holds it, the piece comes from the space that was never used, after `used`.
If that space is too short too, the region refuses the request.

## A piece made longer

A piece grows where it is, and keeps its offset.
Usually the piece is the last piece handed out, and the space after it was never used.
If not, the gap that begins where the piece ends gives the bytes, if that gap is long enough.
If neither has the bytes, nothing changes.

## The high-water

`region_take` and `region_grow` add to `used` through `region_bump`, which raises `peak` when `used` passes its old top. A piece given back lowers `used`, never `peak`.

## Functions

| Function | Result |
| --- | --- |
| `find_best` | Gives the index of the smallest gap that holds `len` bytes, or `count` if no gap does. |
| `region_take` | Takes a piece of `len` bytes: 1 and its offset, or 0 if the region has no piece that long. |
| `find_after` | Gives the index of the gap that begins at `end`, or `count` if no gap does. |
| `region_grow` | Makes the piece at `at` longer, from `had` bytes to `want` bytes. False, with no change, if the bytes after it are not free. |
| `region_get_most` | Gives the length of the longest piece that the region can give now: the longest gap, or the space that was never used. |
