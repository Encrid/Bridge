# src/module/module_call.c

This file makes the private memory of a module, and calls a lifecycle export of the module.

## The private memory

`docs/extern/bridge_abi.h.md`, *The memories*, tells what the private memory of a module holds.
Each module has a private memory of its own, sized from its own config, and not a share of a block.
A reload makes a new instance with a new private memory, so a reload can state larger sizes.
`module_take_private` makes the private memory in these steps:

1. Find where the regions sit, with `module_lay_private`. `module_check_fit` laid out the same config with the same granule, so it fits.
2. Round the length up to the granule of the backend.
3. Reserve the larger of that length and the reservation of the backend, plus its guard.
4. Record the regions: the scratch arena at `MODULE_SCRATCH_AT`, and the log ring as a count of records.
5. Write the log level of the module in the header, and `UINT32_MAX` as the system: no system runs.
6. Hand the block to the backend with `memory_adopt`.

Only the backend of the module maps the block, so the block has the shape of that backend.
The checks of the config refused a config that cannot be laid out, so the first step cannot fail.
If `os_reserve` fails, the load fails and says how many bytes it asked for.

## A lifecycle call

`module_call` calls `init`, `reload` or `shutdown` once, in these steps:

1. If the timeout of the call is above 0, set it on the instance, in nanoseconds.
2. Clear the refusal of the module.
3. Give the call its turn: `TURN_LIFECYCLE`, and the count of lifecycle calls since the last barrier.
4. Make room for its marks: `docs/intern/abi/abi_alloc.c.md`, *The marks of the queue*.
5. Call the export.

The barrier applies what a lifecycle call changed after the observers of the last barrier, and before the systems of the next phase.
The lifecycle calls are applied in the order they were made: `src/storage/storage_runs.h`.
If there is no memory for the marks, the call fails as "out of memory", and the export is not called.

An entry that refused during the call is the reason of the failure: Bridge knows why better than the trap does.
If no entry refused, a trap is the reason, with the text of the backend.
If the export did not trap, a status that is not 0 is the reason, with the status.

## Functions

| Function | Result |
| --- | --- |
| `place_regions` | Records where the regions of the private memory are, and writes the log level and no system in the header. |
| `module_take_private` | Reserves the private memory of a module and hands it to its backend. Gives 1 with `err`. |
| `say_trap` | Writes into `err` that the export `name` trapped, with the text of the backend, cut at 200 characters. Gives 1. |
| `module_call` | Calls a lifecycle export once, under its timeout. Gives 0 if the export gave 0, and 1 with the reason in `err` if not. |
