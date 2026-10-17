# src/module_check_fit.c

This file refuses the keys of the config of a module that do not fit: in what there is, and with one another.
It also finds where the regions of a module sit in its private memory.

## The budget of a module

A module states the sizes of its private memory: `scratch`, `log_ring` and `spans`, in bytes.
It also states what the shared block sets aside for it alone, so that what it makes does not depend on who ran beside it:

| Key | What is set aside |
| --- | --- |
| `spawns` | The entity ids that the module may take between two barriers. |
| `persistent` | Its share of the persistent arena, in bytes, for as long as it is loaded. |
| `changes` | The bytes of `attach`, `detach` and `destroy` that it may queue between two barriers. |

The ids of `spawns` are set aside at a barrier and held: real entities with nothing on them, which no query matches.
They count against `[budget] entities`.
A change takes 24 bytes in the queue, and an attach takes its value too, to a multiple of 8.
A module that spawns nothing, keeps no string or queues no change states `BRIDGE_NONE` for that key.

This file refuses these amounts:

- A `log_level` that is unset, or past `BRIDGE_LEVEL_MOST`, which is 7.
- A `config` that is null or empty. A module with no settings component states `BRIDGE_NO_CONFIG`.
- More `spawns` than `[budget] entities`. Whether they fit in what is left is found when they are set aside: `src/bridge_load.c`.
- Regions of the private memory that an offset of 32 bits cannot reach.
- A `log_ring` that cannot hold one record of 32 bytes.

A log level is a severity of syslog: 0 is the worst, and 7 is the most detail.
Zero is unset for `log_level`, as for every key, so the least it can be is 1.
The log ring has the one floor with a reason.
A module that can tell nothing is the one legal number that makes a deployment incoherent.

## The layout of the private memory

`module_lay_private` puts the regions one after another:

| Offset | Region |
| --- | --- |
| 0 | The header. |
| `MODULE_SCRATCH_AT`, 128 | The scratch arena, `scratch` bytes. |
| The next multiple of 8 | The log ring, `log_ring` bytes. |
| The next multiple of 8 | The spans, `spans` bytes. |

Every offset into the private memory is an offset of 32 bits.
So each sum must fit in 32 bits, and the end, rounded up to the granule of the backend, must too.

## Functions

| Function | Result |
| --- | --- |
| `module_lay_private` | Finds where the regions of a module sit in its private memory. Gives 1 if they cannot be laid out in 32 bits. |
| `check_log_level` | Refuses a log level that is unset, or that is not one there is. |
| `module_check_fit` | Refuses a config whose amounts do not fit. |
