# src/bridge_backends.c

This file opens the backends of a deployment, one after another.
It works out what all of them together need of a block that they map.

## At least one backend

A backend runs each module, so a world with no backend could load nothing.
A deployment that configures no backend is refused.
The backends open in the order that the deployment states them.

## The shape of the shared block

After a backend starts, Bridge asks it for its capabilities and for the shape of a block that it maps.
The shape is a reservation, a guard and a granule. A backend with no granule gets a granule of 1.
Every backend maps the shared block, so the world keeps the largest reservation and the largest guard of the backends.
The length of the shared block is a whole number of each granule, so the world keeps the least common multiple of the granules.
`src/block_create.c` makes the shared block to this shape.

## Functions

| Function | Result |
| --- | --- |
| `get_common_multiple` | Gives the least length that is a whole number of both `a` and `b`. |
| `take_shape` | Asks a backend for its capabilities and its shape, and makes the shape of the world hold it. |
| `bridge_open_backends` | Opens and starts every backend of the deployment, in its order. 1, with a record, if there is none or one fails. |
