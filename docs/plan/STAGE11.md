# Stage 11: storage layouts

In progress. `PLAN.md`, stage 11, is what it is for and when it is done. This document is the
order it is done in, what was measured, and each question while it is open.

## The order

    1  the baseline      what the table costs at the work each other storage layout is for
    2  the gate          sparse set, toggle, direct-indexed, singleton: each weighed on what
                         it answers, measured in scratch, and on every named need, before it
                         is built. One that fails is not built
    3  the questions     who states a component's storage layout; what a module is handed
                         for each, and what a query may ask across two; what each costs of
                         which budget; whether a tag exists; what moving a component from one
                         storage layout to another takes
    4  each that passed  built, with its cases and its measurement beside the table's
    5  the move          a component from one storage layout to another, with its values
    6  a changed         its rows moved field by field, in every storage layout, and how a
       component layout  reload states that it accepts a loss
    7  close             by the done-when

The gate is before the questions because a storage layout that fails takes its share of every
question with it. The sparse set is first in the gate: it alone answers something that was
measured before this stage, and direct-indexed is weighed against it. A changed component layout
is last because it is written once for every storage layout there is.

**A storage layout is named for what keeps the component.** "Table" is both the storage of one
archetype and the name of the storage layout that keeps a component in it, as "sparse set" is
both. It was asked whether the first should be "archetype" or "archetype table". An archetype is
a set of components, which is another thing; and there is one kind of table, so the longer name
would tell it from nothing.

## The baseline

What the table costs at work it was not chosen for, done the way a module does it today.
`storage weigh` has the first two and `storage baseline` the rest, on Windows, the least of
thirty-two runs, three times over.

**A component that comes and goes, and one few entities carry**, which is what a sparse set is
for. A four-byte component given to and taken from an entity moves everything the entity
carries, and rows spread over more tables are read more slowly:

    the entity carries     attached     detached          204,800 rows read from
        4 bytes              40 ns        37 ns              1 table       6.0 us
     1024 bytes             150 ns       145 ns            256 tables     12.3 us
     4096 bytes             505 ns       500 ns           4096 tables     19.3 us

**A component an entity always has and sometimes does not use**, which is what a toggle is for.
Today it is a field that says whether, tested at every row, or a detach and an attach. 204,800
rows:

    read with no test                       5.7 us     0.028 ns a row
    tested, every row on                   55 us       0.27 ns a row
    tested, one row in sixteen on          87 us       0.42 ns a row
    tested, half of them on, in no order  515 us       2.5 ns a row
    switched by a detach and an attach     77 ns a switch, and 1,005 ns for 4 KB carried

The test stops the compiler from reading sixteen bytes at a time, which is the first factor of
ten; and where the processor cannot guess the answer, that is the second.

**A value reached by entity id and not walked**, which is what direct-indexed is for. Today it
is `reference.md`, *Reference following*: the entity's record, then the table's cell of
`column_base`, then the value. 204,800 of them:

    read in order, for comparison           5.7 us     0.028 ns each
    by id, in the order they lie          226 us       1.1 ns each
    by id, in no order                    510 us       2.5 ns each

All of it fits the processor's cache here, so this is the arithmetic and not the misses that
`reference.md` counts.

**One value for the whole deployment**, which is what a singleton is for. Today it is a
component on one entity, found through a query of one span:

    found and read                          0.6 ns each time

So a singleton has nothing to gain in time, and is to be weighed on the other five needs alone.
