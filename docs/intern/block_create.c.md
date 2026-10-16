# src/block_create.c

This file makes the shared block of a world, with storage in it, and has each backend adopt the block.

## The steps

1. Check the budget: `docs/intern/block_budget.c.md`, *The checks*.
2. Find where each part of the block begins: `src/block_layout.c`.
3. Reserve the address space of the block, and commit the block.
4. Keep the amounts of the budget in the world.
5. Make the lists of free gaps of the two regions, and make storage.
6. Have each backend adopt the block.

If the operating system does not give the block, the budget cannot be satisfied, and the record tells how many bytes were asked for.
Each other step that fails writes its own record, and the world is not made.

## The shape of a block

A backend states the shape of a block that it needs, and the library allocates the block to that shape.
`memory_get_shape` gives three numbers: the reservation, the guard and the granule.
A backend that keeps a module in bounds with faults needs address space after the block, reserved and not mapped: that is the guard.
A stray access then reaches nothing.
The granule is the number of bytes that the length of a block must be a whole number of.
A backend with no need gives 0 for each.

A block has the shape of the backends that map it.
Each backend maps the shared block, so it has the largest shape that any backend stated: `src/bridge_backends.c` finds it.
A private block has the shape of the backend of its module only: `src/module_call.c`.

## The adoption

`memory_adopt` takes pages that the library owns already, and asks the backend to make them addressable by a module.
It does not ask the backend to allocate.
So the same region reaches a native module as a pointer, and a wasm module as an import.
A backend that refuses to adopt the block refuses the world, with a record that names the backend and its reason.

## The lists of gaps

The list of the column region has room for `[budget] tables`, plus `[budget] sparse_sets`, plus `[budget] types`, plus 1 gaps.
A gap lies between two blocks of the region.
A table has one block, a sparse set has one, and each type that is direct-indexed or a singleton has one.
There are no more such types than types, so there are never more gaps than these blocks, plus one.
The list of the persistent arena starts empty, and it grows when a module takes a range: `docs/intern/region.c.md`, *The room of the list*.

## Functions

| Function | Result |
| --- | --- |
| `map_block` | Reserves the address space of the shared block, commits the block, and keeps where each part begins. 1 with a record if the operating system does not give it. |
| `set_limits` | Keeps the amounts of the budget in the world. |
| `make_storage` | Makes the lists of free gaps of the column region and of the arena, and makes storage. 1 if there is no memory. |
| `adopt_shared` | Has each backend adopt the shared block, so that each can hand it to the modules it runs. 1 with a record if a backend refuses. |
| `block_create` | Makes the shared block that the budget states, with storage in it, and has each backend adopt it. 0, or 1 with a record. |
