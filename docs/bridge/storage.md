# Storage

What was weighed here and refused is `storage_refused.md`, *What was refused: Storage*.

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

## Storage layouts

**A component is kept in the storage layout its author states for it**, at `declare` -
`abi.md`, *The ABI*. There are five, each named for what keeps the component:

    table             the table of its entity's archetype, a column beside the entity's
                      other components. What systems read together, row by row
    sparse set        the component's own packed array, and an index by entity. What comes
                      and goes, and what few entities carry
    toggle            a column of a table, and a bit a row that says whether it is on. What
                      an entity always has and sometimes does not use
    direct-indexed    a cell for every entity, the value of entity i at cell i. What is
                      reached by id far more than it is walked
    singleton         one value for the deployment, on no entity

**The author states it because the code that reads it depends on it.** A table's column, a
sparse set's array and a cell by entity are read by different code, fixed when the module is
compiled, as `sizeof(T)` is - `query.md`, *The interface is a query*. Unset is refused, like any
other value nobody stated.

**Every module that declares a component states the same storage layout.** One that states
another is refused at its load, and the refusal names both storage layouts and the modules that
declare the component already, as for a component layout that differs - `lifetime.md`, *When a
component layout changed*.

What each was weighed on, measured, and what a module is handed for each, is `STAGE11.md`, *The
gate* and *The questions*.

## A table

An **archetype** is a set of components. A **table** is the storage of one archetype: the entities
that carry exactly that set, and their values. A table is found **by its archetype**: a
bit for each declared type, hashed. There is no graph of which table an added or removed
component leads to, because nothing walks from table to table: an entity's final set is known
before it is placed.

Its rows are **one block of the column region**: the entity ids, then a column for each type, in the
order the types were declared, **each beginning at a multiple of sixteen bytes**, as the block does.
That number is measured. A system's loop over a column is compiled to loads of sixteen bytes, and
where the column begins at a multiple of sixteen none of them lies across two cache lines: 204,800
rows are read in 5.6 to 6.4 us from a column that begins there, and in 8.3 us from one that begins
eight bytes past it. It costs up to fifteen bytes a column a table. A table's first room is the rows
it was asked for, and no more. A table that needs more rows asks for half as much again as it had,
in place when what follows it is free, and otherwise elsewhere with its rows copied; and for what it
needs and no more where that is not to be had. Half as much again was measured against twice and
against only what is needed, on a table of 64-byte rows grown a thousand at a time to a hundred
thousand: 55 to 61 ns a row and 15% of its block idle; 54 to 62 ns and 28% idle; 97 to 105 ns and
none idle. So growing by a share is worth it, and the smaller share costs no time. A row that leaves
is filled by the table's last row.

**One table for each set, with one exception**: a strip at unload that had no room to join two
tables leaves both - `lifetime.md`, *What unload leaves behind*. The set finds the one that was
there first, and the other lasts until its rows have left.

**A table lives while it has rows.** It is made when the first entity is placed in it and released,
block and id, at the barrier its last one leaves. Table ids are dense, from 1; 0 is no table, which
is where an entity with nothing on it is. A released id is used again by the next table made.

## A sparse set

**A component kept in a sparse set is in no archetype.** An entity's archetype is the components
it carries that are kept in tables, and its table is that archetype's. Giving it a component kept
in a sparse set, or taking one from it, moves no row of any table. An entity that carries only
such components is in no table.

**The set is kept as a table of its one type**: a block of the column region that holds the ids
of the entities in it and then their values, packed, grown by half as a table's is, and given
back when the last entity leaves it. Where a table's rows are found by where the entity is, the
set's are found by an index, a cell for every entity there may be. Storage keeps that index for
itself, and writes another for modules, in the shared memory: **a cell an entity, holding where
that entity's value is from the shared base, and zero where it has none**. When the set's block
moves to grow, every cell of an entity in it is written again.

**It is for what comes and goes, and for what few entities carry.** Measured in the library,
`storage weigh`: a four-byte component given to an entity and taken from it again.

    the entity carries     in a table: given, taken     in a sparse set: given, taken
        4 bytes               30 ns      28 ns              29 ns      24 ns
     4096 bytes              507 ns     506 ns              28 ns      23 ns

That is the barrier. The call that asks for the change is 10 ns more for a give and 8 for a
take, whichever way the component is kept.

And the entities that carry it are not split off into a table of their own. **What it costs is
the read beside a table's column.** Its 204,800 values walked alone take 6.6 us, as a column's
do. Reached from a table's rows through the index they take 89 us, 0.44 ns a row, where two
columns of one table take 0.03. And it is eight bytes for every entity there may be, for each
such component, the two indexes: `[budget] sparse_sets` says how many there may be, and their
indexes are had when the bridge is created - `budgets.md`, *Budgets*.

**Running out, unload and observers are what they are for a table.** An entity the set has no
room for is left as it was, with everything else that barrier asked of it, and that is a failure
of the module that asked - `budgets.md`, *What running out does*. A component nobody declares any
more is stripped, swept or deleted by its policy - `lifetime.md`, *What unload leaves behind* -
and for a sweep an entity carries nothing else when it is in no table and in no other set. An
observer's terms are matched against everything an entity carries, in tables and in sets alike,
since an observer walks nothing.

## A toggle

**A toggled component is a column of its table, and a bit a row that says whether it is on.**
It is part of the entity's archetype, as a component kept in a table is, and a query that
states it matches its table: every row carries it, on or off. Whether a row is on is tested by
the module's own code, and the value of a row that is off stays where it is.

**The bits are in the table's block, ahead of the column**: a 64-bit word for each 64 rows, row
r being bit `r & 63` of word `r >> 6`; then sixteen bytes that hold where those bits begin, from
the shared base; then the values. So a module that has the column, from a span or from
`column_base`, has the bits by one load, at the column less sixteen, and neither of those two
has another shape for it. Bits past the table's last row are nobody's.

**A row is turned on or off by writing its bit**, by a system that states the term
`BRIDGE_WRITE`, as it would write a value. The scheduler keeps a system that writes a component
from running beside one that reads or writes it - `data.md`, *Access mode* - so two never write
one word at once. It is no call and waits for no barrier, and no observer is told: a write in
place raises none. A component is on when it is given; an attach of one an entity has writes
its value and leaves the bit; and a row that moves, to another table or within one, takes its
bit with it.

**It is for what an entity always has and sometimes does not use.** Measured in the library,
`storage weigh`, 204,800 rows with a share of them on, in no order:

                                                all on      half on     1 in 16 on
    the rows that are on, found a word of
    bits at a time                              13 us       67 us        8.5 us
    the bit tested at every row                 78 us       78 us       78 us
    a field of the struct tested instead,
    as `storage baseline` has it                55 us      515 us       87 us
    the column read with no test                 5.7 us

Sixty-four rows that are all on are read as a column is, which is the first figure. And a row
is switched by one store: 3 ns a row measured, with the draw that says which way, where a detach
and an attach to say the same are 58 ns at the barrier and 18 more in the two calls, and 1,013
at the barrier for an entity that carries 4 KB. What it does
not beat is a field tested by a mask, which a compiler makes of a small enough loop: 12 us at
any share, measured in scratch - `STAGE11.md`, *The gate*.

## Direct-indexed

**A direct-indexed component has a cell for every entity there may be**, and the value of the
entity whose id's low 32 bits are e is cell e. It is in no archetype, as a component in a sparse
set is in none, and giving or taking it moves no row. The cells are one block of the column
region, the component's size for each of `[budget] entities`, taken when the component is
declared: a `declare` they do not fit is refused, by the arithmetic, and that is the one way it
can be short of room. After that an entity given one always has its cell.

**A cell exists for an entity that was given nothing, so a bit an entity says whether it
has a value.** It is set by `attach` and cleared by `detach`, queued and applied at the barrier
and told to observers, as for a component kept any other way; and cleared when the entity is
destroyed, so that the entity which next has its place has none of what was there. Storage
keeps those bits for itself and writes another copy for modules, found as a toggle's are: the
sixteen bytes ahead of the cells hold where the bits begin. A module finds the cells through
the cell its header has for each type it declared, tests the bit where it needs to know, and
does not where it knows by its own design. `[budget] direct_indexed` says how many such
components there may be, and their bits are had when the bridge is created.

**It is for what is reached by id far more than it is walked.** A system cannot walk it: a
term of it is stated `BRIDGE_OPTIONAL`, and one that selects is refused at `add_system`. Measured
in the library, `storage weigh` and `storage baseline`, 204,800 values each reached by an id
that is held, its generation tested:

                                       in the order they lie     in no order
    through the table, as              1.1 ns                    2.5 ns
    `reference.md` has it
    direct-indexed, the bit tested     0.74 ns                   1.4 ns

And it is given in 17 ns and taken in 17 at the barrier, whatever the entity carries. **What it costs is its
size for every entity, carried or not**: sixty-four bytes at four hundred thousand entities is
25.6 MB of `[budget] columns`.

## A singleton

**A singleton is one value for the deployment, on no entity.** It is a block of the column
region of the component's size, taken when it is declared, which a `declare` that finds no room
for it is refused for. It has a value or it has none; there cannot be two.

**It is given and taken with the id 0.** Every entry that gives or takes a component names an
entity, and 0 is no entity - `reference.md`, *Reference following*. So `attach` and `detach` of
a singleton take the id 0 and are refused any other, by name; and are refused 0 for a component
that is not one. The change is queued and applied at the barrier with the rest, in the order it
was asked, and told to observers, whose record carries the id 0. A singleton that is given a
value is zero but for what the attach wrote. After that a field of it is changed by a write
where it is, by a system that states the term `BRIDGE_WRITE`, and nothing is told.

**A module reads it where it is**: the cell its header has for each type it declared holds
where the value is from the shared base, and zero while it has none. A system cannot walk it,
since it is on no entity: a term of it is stated `BRIDGE_OPTIONAL`, which is how the system
says what it reads and writes, and one that selects is refused at `add_system`. Unloaded by its
last declarer it has no value, whichever policy was stated, and that is told as a leave.

**It is what a module's settings are** - `config.md`, *A module's own settings are a
component*. It was not passed for its speed: read through a query of one span, a value on one
entity cost 0.6 ns. Read where it is, it is 0.33 ns, measured in the library; and what it gives
is that there is one.

## A tag

**A tag is a component with no bytes.** It says that an entity is one of something, and carries
no value. Standard C has no struct without a member, so it is not declared as one: it is a
fourth kind at `declare`, `BRIDGE_TAG`, whose size is 0 and which has no descriptor, and an
attach that hands it a value is refused. It may be kept in any of the five storage layouts,
and is given, taken, matched and told as a component that has bytes is.

**It has no column, no values and no cell.** In a table it adds nothing to the block; in a
sparse set the set is the ids of the entities in it; direct-indexed, it is the bits alone; a
singleton, it is that it is set. Measured in the library, `storage weigh`: 204,800 entities
that carry a four-byte value and six markers each, over 4,096 tables, use 7,424 KB of columns
with the markers as components of four bytes, 3,968 KB as components of one, and 2,432 KB as
tags.

**Where a module would be handed where a value is, it is handed `BRIDGE_CARRIED`**, which is 1.
Offset 0 of the shared memory is reserved and its first eight bytes hold nothing - `query.md`,
*The interface is a query* - so 1 is not zero, which says absent, and is no value's place. It
is the base of a tag's term in a span, its cell of `column_base`, what a sparse set's index
holds for an entity in it, and what a singleton's cell holds when it is set. Two are handed an
offset all the same, because their bits are found from it: a toggled tag, where its column
would begin, and a direct-indexed one, where its cells would.

## What a query walks

**A system walks tables, or one sparse set alone.** A table's rows are in the table's order and
a set's entries in the set's, so row 0 of one and entry 0 of the other are different entities,
and one loop cannot step through both side by side. So the terms that select, `BRIDGE_AND` and
`BRIDGE_NOT`, are all of components kept in tables; or there is one, `BRIDGE_AND`, of a component
kept in a sparse set. Anything else is refused at `add_system`, naming the term. Which is walked is
the author's, by how the query is written.

**What is kept elsewhere is stated optional, and reached from the row.** In a query that walks
tables, a term of a component kept in a sparse set has a base of zero in every span, since it is
in no table. The module reads the set's index, which it finds through the cell its header has
for each type it declared - `memory.md`, *Where the regions live*:

    for each span, for each row r:
        at = index[the low 32 bits of entities[r]]
        if at == 0      this entity is not in the set
        else            its value is at `at`, from the shared base

**A query whose one selecting term is of a sparse set walks the set**: one span, whose rows are
the entities in the set, whose `entities` are their ids and whose base is their values - the
three things a table's span is, read by the same loop. Its other terms are optional, and are
reached by id - `reference.md`, *Reference following*.

What was weighed against this is the record of `STAGE11.md`, *The questions*.

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

## What a barrier writes for a module

A module reads three things storage writes into its private memory: its spans, its `column_base`,
and its observers' lists - `memory.md`, *Where the regions live*. **Only what changed is
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

Carved to what a table asks for, to sixteen bytes, so that every block begins where a column may -
*A table*. A freed range is a gap, kept in order and joined to its neighbours, and a request takes
the smallest gap that holds it before fresh space. What says which ranges are free is kept outside
the block: a list as long as there may be tables, made with the bridge, which never grows.

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
    [budget] sparse_sets how many sparse sets: an index for each, a cell an entity, twice; and
                         a table's record for each, beside `tables`
    [budget] direct_indexed
                         how many direct-indexed: a bit an entity for each, twice

**All of it is allocated when the bridge is created, and nothing after**: not at a barrier, not
when a module loads. What a module declares takes a place that is already there, and a `declare`
or an `add_system` past its key is refused - `budgets.md`, *Budgets*. A direct-indexed component's
cells are the one thing taken at a `declare`, and they are taken from `[budget] columns`, which
is there already. That is storage. What the rest
of the library allocates, and when, is `budgets.md`, *Nothing is allocated in a phase*.

A type is a size to storage, and nothing else. Its fields, their names and an enum's constants
are read from its text by `src/schema.c` and kept with the component's record, by the part of the
library that checks a load - `schema.md`, *There is no schema format*.

## What is asked of it

`src/storage.h` is the whole of it, and each entry is named for the request, as the backend seam's
are: declare a type of a size, hand over a system's terms or an observer's, apply every queue,
set ids aside, write what a module reads, drop a component by its policy. Nothing there says how.

It is internal: one implementation, and no host or backend sees it.