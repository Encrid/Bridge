# src/block/block_sort.c

This file sorts the work list by the order of the modules: `docs/intern/block/block_work.c.md`.

## The order

A barrier writes the log lines and the reasons of the modules in the order in which the world lists them.
So each step walks the work list in that order too.
`place` of a module gives that order: `block_join_work` gives each loaded module the next number of `places` of the world.
A reload gives the new module the place of the old one.

`block_sort_work` sorts the work list by `place` with a merge sort of linked records.
`block_sort_list` splits a list in 2 halves, sorts each half, and `merge_work` merges the two.
`block_sort_work` sorts only when a module came on the list since the last sort.
The sort needs no memory, and the depth of its calls is the base-2 logarithm of the length of the list.

## Functions

| Function | Result |
| --- | --- |
| `merge_work` | Merges two sorted lists into one, by `place`. |
| `block_sort_list` | Sorts a list by `place`. |
| `block_sort_work` | Sorts the work list if a module came on it since the last sort. |
