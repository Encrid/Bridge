# src/lib/block/block_column.c

This file carves the column region: storage gets the bytes that it asks for, rounded up to sixteen bytes.
The column region is a region of the shared block: `docs/intern/lib/region/region.h.md`, *The gaps*.

## Sixteen bytes

A column must begin at a multiple of sixteen bytes to be read at full speed.
The loop of a system over a column reads sixteen bytes at a time, and then no read lies across two cache lines.
A block of a table begins with its entity ids, and the ids begin where a column may.
So each piece of the region is rounded up to `GRAIN`, which is 16, and each piece begins where a column may.

## A piece of a new length

`block_column_regrow` gives a piece of `new_size` bytes that holds what the piece at `ptr` held:

1. Round the old length and the new length up to sixteen bytes.
2. If there is a piece, try to change its length where it is.
3. If it cannot stay, take a new piece from the region.
4. If the region cannot give the piece, give null, with no change.
5. Copy the old piece into the new piece, and give the old piece back.

A shorter piece stays where it is, and gives back the bytes that it no longer needs.
A longer piece stays where it is if the bytes after it are free, which is usually so.
Nothing comes from outside the region.

## Functions

| Function | Result |
| --- | --- |
| `round_up` | Gives a size rounded up to a multiple of sixteen bytes. |
| `block_column_free` | Gives `size` bytes of the column region back, rounded up to sixteen bytes. |
| `resize_in_place` | Makes a piece `want` bytes long where it is. False if the piece must move. |
| `block_column_regrow` | Gives `new_size` bytes of the column region that hold what the `old_size` bytes at `ptr` held, or fresh bytes if `ptr` is null. Null, with no change, if the region cannot give them. |
