# Storage

Who keeps the rows, and how. The model a module sees is `data.md`; what it reads is `query.md` and
`reference.md`; which block each thing is in is `memory.md`.

Storage is Bridge-ASM700's own: `src/storage.c`, behind the one internal header `src/storage.h`. Until
stage 10 it was flecs, patched. Why that changed, and the figures it changed on, are `STAGE10.md`;
what flecs was while it was carried is kept as `flecs.md`, a record and no longer a description.

## Made for what the bridge promises

A general ECS serves any program, so it must be right at every call: a row may move between any
two of them, a query may be made at any time, anyone may iterate. Bridge-ASM700 promises much more
than any program does, and storage is made of what each promise allows:

    rows are touched only at a barrier,     no lock, no stage, no thread-local: one thread has
    by one thread                           the world - `threads.md`, *Threads*
    every change of a barrier is queued     an entity's whole fate is known before a row of it
    before the first is applied             moves - `data.md`, *Structural change*
    systems are fixed at load               a table is matched against them once, when it is made
    nobody iterates through storage         no iterator, no cache to keep warm: a module reads
                                            spans - `query.md`, *The interface is a query*
    no order of rows or spans is a          a hole is filled by the last row, and a system's
    module's to rely on                     tables are listed in whatever order is cheapest
    a closed set of types, no pointer       a row is bytes, moved by `memcpy`: no constructor,
    among them                              no hook - `schema.md`, *The type set*
    no pairs, no wildcards, no or-terms     a term is one type, so a table matches or it does not

## A table

An **archetype** is a set of components. A **table** is the storage of one archetype: the entities
that carry exactly that set, and their values. A table is found **by its archetype**: a
bit for each declared type, hashed. There is no graph of which table an added or removed
component leads to, because nothing walks from table to table: an entity's final set is known
before it is placed.

Its rows are **one block of the column region**: the entity ids, then a column for each type, in the
order the types were declared, each at the next multiple of its own type's alignment - a column of
`uint8_t` follows the one before it with no gap, and one of `double` begins at a multiple of eight.
A table's first room is the rows it was asked for, and no more. A table that needs more rows asks
for half as much again as it had, in place when what follows it is free, and otherwise elsewhere
with its rows copied; and for what it needs and no more where that is not to be had. Half as much
again was measured against twice and against only what is needed, on a table of 64-byte rows grown a
thousand at a time to a hundred thousand: 55 to 61 ns a row and 15% of its block idle; 54 to 62 ns
and 28% idle; 97 to 105 ns and none idle. So growing by a share is worth it, and the smaller share
costs no time. A row that leaves is filled by the table's last row.

**One table for each set, with one exception**: a strip at unload that had no room to join two
tables leaves both - `lifetime.md`, *What unload leaves behind*. The set finds the one that was
there first, and the other lasts until its rows have left.

**A table lives while it has rows.** It is made when the first entity is placed in it and released,
block and id, at the barrier its last one leaves. Table ids are dense, from 1; 0 is no table, which
is where an entity with nothing on it is. A released id is used again by the next table made.

Refused: keeping an emptied table for the next entity of that set, as flecs does. It would save
making the table again, and it costs what makes the count unbounded: every set ever seen stays,
and a quiet barrier pays for all of them. `STAGE10.md`, *What it costs*, measures the case this
could lose - ten thousand sets filled at one barrier and emptied at the next - and it does not.
The condition that brings it back is a workload where that line does lose.

## A barrier is worked an entity at a time

The queues are read in their stated order - module by module as they loaded, and within one as it
asked - and nothing in that order is changed. What is changed is what the order is applied *to*:

1. **The fold.** Each queued change is applied to a set of bits, the entity's own, starting from
   the set of the table it is in. No row moves. At the end each changed entity has the set it
   will carry, and whether it is destroyed.
2. **Room.** The table of each final set is found, or made; and room for every row about to
   arrive is asked of the column region, a table at a time. What cannot be had is known here,
   while every entity is still where it was - `budgets.md`, *What running out does*.
3. **Observers.** For each changed entity, the set it carried is compared with the set it will
   carry: that difference is what an observer is owed - `data.md`, *Observers*.
4. **Placement.** Each changed entity moves once, from the table it was in to the one it will be
   in, with what both carry copied and what is new zeroed. Or it is destroyed.
5. **Values.** The queues are read a last time, and each `attach` writes its bytes where the
   entity now is. The last one written is the one that stays, which is the order they were asked.
6. **Release.** A table nothing is left in is given back.

An entity given five components in one barrier is placed once, in the table of all five, and the
four tables between never exist. Applied a change at a time it moves five times and leaves four
tables behind it.

Refused: applying each change as it is read, which is what a storage that may be called at any
time must do. Changes reach storage only at a barrier; outside one it is asked for two things, a
module's settings row at load and its components at unload.

## What a barrier writes for a module

A module reads three things storage writes into its private memory: its spans, its `column_base`,
and its observers' lists - `memory.md`, *Where the seven regions live*. **Only what changed is
written.** Storage keeps a list of the tables touched since it last wrote: grown, moved, gained or
lost a row, made or released. For each module:

    column_base    the cells of the touched tables
    a span         rewritten where it is, if its table was touched
    a query's      rewritten whole, with every query's after it, when the set of tables the
    spans          query matches changed: they sit one after another
    an observer's  when it has something, or had something last time
    list

A barrier at which nothing changed writes nothing, however many tables stand. And **a span exists
for each table that has rows**, since a table without any does not exist.

All of it is written again for a module the first time, and for every module when one loads,
unloads or reloads.

## Nothing in the shared block is read back

Storage keeps, in memory of its own that no module can address:

    where each entity is      table, row and generation, by index
    which entity is in        for every table, in step with its block
    which row
    every set, every          the tables themselves
    column's offset

What is in the shared block of any of these - the entity index, each table's array of ids - is a
**copy**, written when a row is placed and never read. A module that writes over them misleads
itself and whoever reads them after; it does not change what storage does next.
`tests/storage.c`, case `scribble`, has a module overwrite every index record and every entity
array, and then moves and destroys rows.

That was not so under flecs. Its table read the id of the row it had moved from the array the
patch had put in the block, and acted on it - `STAGE10.md`, *Not a freestanding flecs*.

Component bytes are different, and stay so: a column in the block *is* the data, and a module
that writes a term it has is doing what it is for - `data.md`, *Access mode*.

## The column region

Carved to what a table asks for, to eight bytes: the widest alignment a field has, and an
entity id's, so that every block begins where its ids may. A freed range is a gap, kept in order and
joined to its neighbours, and a request takes the smallest gap that holds it before fresh space.
What says which ranges are free is kept outside the block: a list as long as there may be
tables, made with the bridge, which never grows.

**Nothing is ever taken from outside it.** A request the region cannot meet is refused, and the
barrier's second step is where that is found out, before anything has moved.

## What bounds it

    [budget] entities    the index, and everything storage keeps an entity
    [budget] tables      every table's record, its archetype, the hash that finds it
    [budget] columns     the rows, and the list of which entity is in which
    [budget] types       how wide a set is, and with `tables`, an offset for each type in
                         each table
    [budget] systems     with `terms`, each one's terms; and with `tables`, its list of the
                         tables it matches
    [budget] terms       how long a system's record of its terms is

**All of it is allocated when the bridge is created, and nothing after**: not at a barrier, not
when a module loads. What a module declares takes a place that is already there, and a `declare`
or a `system` past its key is refused - `budgets.md`, *Budgets*. That is storage. What the rest
of the library allocates, and when, is `budgets.md`, *Nothing is allocated in a phase*.

A type is a size to storage, and nothing else. Its fields, their names and an enum's constants
are read from its text by `src/schema.c` and kept with the component's record, by the part of the
library that checks a load - `schema.md`, *There is no schema format*.

## What is asked of it

`src/storage.h` is the whole of it, and each entry is named for the request, as the backend seam's
are: declare a type of a size, hand over a system's terms or an observer's, apply every queue,
set ids aside, write what a module reads, drop a component by its policy. Nothing there says how.

It is internal: one implementation, and no host or backend sees it. Refused: making storage
swappable as a backend is. A backend abstracts a behaviour, and storage would have to abstract a
memory layout - a span's `base[]` *are* offsets into its columns - so a second implementation
would be shaped by the first.
