# src/lib/bridge/bridge_backends.c

This file opens the backends of a deployment, one after another.
It works out what all of them together need of a block that they map.

## At least one backend

A backend runs each module, so a world with no backend could load nothing.
A deployment that configures no backend is refused.
The backends open in the order that the deployment states them.

## The shape of the shared block

After a backend starts, Bridge asks it for its capabilities and for the shape of a block that it maps.
The shape is a reservation, a guard and a granule. A backend with no granule gets a granule of 1.
`check_shape` refuses a granule that is not 0 and not a power of two up to 2 to the power 30.
It also refuses a reservation or a guard past 2 to the power 62 bytes.
So no length and no reservation that Bridge adds from them passes 64 bits.

Every backend maps the shared block, so the world keeps the largest reservation and the largest guard of the backends.
Each granule is a power of two, so a multiple of the largest one is a multiple of each.
The world keeps the largest granule.
`src/lib/block/block_create.c` makes the shared block to this shape.

## Functions

| Function | Result |
| --- | --- |
| `refuse_span` | Refuses a reservation or a guard past 2 to the power 62 bytes, with a record. Gives 1. |
| `check_shape` | Refuses a granule that is not 0 and not a power of two up to 2 to the power 30, and calls `refuse_span`. Gives 1 with a record. |
| `take_shape` | Asks a backend for its capabilities and its shape, and makes the shape of the world hold it. Gives 1, with a record, if refused. |
| `bridge_open_backends` | Opens and starts every backend of the deployment, in its order. 1, with a record, if there is none or one fails. |
