# src/backends/wasm/sdk/bridge_wasm.wat

This file holds the accessors a compiled module reaches Bridge's two other memories through.
A wasm instruction names the memory it loads, and no C compiler emits a load from a memory that is not the module's own.
So the accessors are written by hand in wasm text, where each load and store names its memory.

## The three memories

The module imports three memories: the shared world data, its own private data, and its own linear memory.
A module holds no shared address: it states a coordinate, and an accessor turns the coordinate into a bounded offset.
`docs/extern/bridge_abi.h.md`, *Spans* gives the capability table that the accessors read.

## resolve

`$resolve` maps a coordinate to an offset in the shared memory, or it traps.
It takes a slot, a packed place, a field, a width, and a flag that tells a store from a load.
`docs/intern/backends/wasm/sdk/bridge_wasm.h.md`, *The macros* packs the place from a table and a row, or from an id.
`$resolve` reads the capability table from the private header, and branches on the `kind` of the slot:

1. It traps if the slot is past the declared types, or a store names a read-only type.
2. It traps if the field and the width run past the stride of the type.
3. For a table or a toggle, it returns the column base plus the row by the stride.
4. For a singleton, it returns the kept cell of the slot.
5. For a sparse set, it reads the cell for the id, and traps when the cell is 0.
6. For a direct-indexed type, it tests the presence bit of the id, then takes the cell.

Each branch adds the field, and each one traps when a bound does not hold.

## The accessors

Each accessor below builds on `$resolve` or reads the header the same way:

- `acc_ld_*` and `acc_st_*` resolve by width, then load or store the shared memory. A store sets the flag, so a read-only type traps.
- `acc_entity` gives the table and the row of an id, or all ones when the id is dead. `acc_who` gives the id of a row.
- `acc_has` tells whether a sparse, direct-indexed, or singleton slot holds a value for an id.
- `acc_toggle` and `acc_toggle_set` read and write the presence bit of a row of a toggle.
- `acc_own_32`, `acc_own_64` and `acc_own_st` reach the private memory. A write traps unless it stays below the published table and leaves the header alone.
- `acc_to_private` copies into the scratch region, below the published table.
- `acc_to_shared` copies into the shared memory, and stays inside one owned range. `acc_from_shared` copies out, inside the arena bounds.

## The build

The module build merges this file into the compiled module as `acc`, inlines every accessor into its callers, and strips the exports.
`.toolchain/tools/wasm_guard.py` then refuses a module that imports a memory or a global of its own.
