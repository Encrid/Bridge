# What was refused: Storage

The record of `storage.md`: `README.md`, *Three layers, three seams*.

## Storage layouts

Of `storage.md`, *Storage layouts*. Who states a component's storage layout, and what else was
weighed for each of the five, is in the record of `STAGE11.md`, *The questions*.

## A table

Of `storage.md`, *A table*.

Refused: a column beginning at its own type's alignment, and a region carved by eight bytes, which
is what this was for a day. It is the least padding there can be, and it was decided on two
measurements, of a change applied and of the room held, that showed nothing; a third, of a column
read, was not made, and shows a read that takes half as long again whenever a column lands eight
past a multiple of sixteen. The condition that brings it back is columns read by code that does not
load sixteen bytes at a time.

Refused: a multiple of sixty-four, a whole cache line. Measured no faster than sixteen, for up to
sixty-three bytes a column a table. The condition that brings it back is a loop that loads more than
sixteen bytes at once, measured as losing by it.

Refused: keeping an emptied table for the next entity of that set, as flecs does. It would save
making the table again, and it costs what makes the count unbounded: every set ever seen stays, and
a quiet barrier pays for all of them. `STAGE10.md`, *What it costs*, measures the case this could
lose - ten thousand sets filled at one barrier and emptied at the next - and it does not. The
condition that brings it back is a workload where that line does lose.

## A barrier is worked an entity at a time

Of `storage.md`, *A barrier is worked an entity at a time*.

Refused: applying each change as it is read, which is what a storage that may be called at any time
must do. Changes reach storage only at a barrier; outside one it is asked for two things, a module's
settings row at load and its components at unload. The condition that brings it back is storage
having to be called outside a barrier for more than those two things.

## What is asked of it

Of `storage.md`, *What is asked of it*.

Refused: making storage swappable as a backend is. A backend abstracts a behaviour, and storage
would have to abstract a memory layout - a span's `base[]` *are* offsets into its columns - so a
second implementation would be shaped by the first. The condition that brings it back is the storage
layouts of `PLAN.md`, stage 11, not fitting inside one implementation.