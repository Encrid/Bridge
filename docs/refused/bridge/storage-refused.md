# What was refused: Storage

The record of `storage.md`: `README.md`, *Three layers, three seams*.

## A table

Of `storage.md`, *A table*.

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