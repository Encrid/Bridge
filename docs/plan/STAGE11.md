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

**Those three are the test as the compiler happened to write it, a branch, and not what a field
costs.** The same test written as a mask, `v & -(on != 0)`, was measured in scratch at 12 us
whatever share of the rows is on. That is the figure a toggle is weighed beside. A loop that
does too much at a row to be masked pays the branch, with a field or with anything else.

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

## The gate

Each measured in scratch, outside the library, beside what exists; and scored on every need.

**The sparse set passes.** The set alone, 204,800 entities: a component given in 2.3 to 2.6 ns
and taken in 2.6 to 3.3 ns, on top of a queued change that moves nothing, which the library does
in 18.9 ns. So about 22 ns whatever the entity carries, where the table takes 40 ns for four
bytes carried and 505 ns for 4 KB. Its values read alone cost what a column does, 5.6 to 6.6 us.
Read beside a table's column, a row of the table at a time through the set's index, they cost
0.59 to 0.76 ns a row, where the table costs 0.028 ns from one table and 0.094 ns from 4,096.
And the index is four bytes for every entity there may be, for each such component.

    need          built, stated for a component          the table alone
    fast          serves: a change costs the same        fails: 505 ns at 4 KB carried, three
                  whatever is carried. A joined read     times as long to read 4,096 tables
                  is dearer, for that component only
    scalable      serves: such a component no longer     fails: a table for every combination
                  doubles the tables. An index of
                  `entities` for each
    modular       serves: one module's marker no         fails: it splits their spans and
                  longer splits another's spans          moves their rows
    extensible    serves: storage has a second layout    does not apply
    minimal       fails: placing, strip, running out,    serves
                  what is heard and what is written,
                  each a second time
    explicit      serves, stated by the author and       serves
                  never picked by the library

Each fails one need, so the conflict was the question, and it was decided for the three that
are measured.

**The toggle passes.** A bit a row, 204,800 rows, a share of them on in no order:

                                              all on     half on    1 in 16 on
    a field tested by a mask, as today        12 us      12 us      12 us
    the bit tested at every row               16.5 us    16.5 us    16.6 us
    the rows that are on, found a word of
    bits at a time                            47 us      32 us       5.5 us
    runs of rows that are on, each read as
    a column                                  13.4 us    432 us     60 us

It is quicker than a field where fewer than about one row in eight is on, read a word of bits at
a time, and slower elsewhere. It was scored as failing three needs: fast, since every reader of
the component leaves the plain loop over a column; minimal, since it is a new shape of span in
`bridge-abi.h` and in each SDK, and two things more for an observer to be told; explicit, since a
row that is off is still in the span and a reader that forgets the bits reads it. It was passed
all the same, by the decision of whoever the design is for. Which of the three ways above a
reader is handed is one of this stage's questions, and what it must answer is in those three
failures.

**Direct-indexed passes.** A value reached by entity id, the id's generation tested each time:

                                    204,800           204,800          8,000,000
                                    in order          in no order      in no order
    as today, through the table     1.5 ns            2.2 to 4.7 ns    17 ns
    a sparse set                    1.0 ns            2.0 to 4.0 ns    17 ns
    direct-indexed                  0.63 ns           1.2 ns           11 ns

One and a half to four times as quick as what exists, at the one thing it is for, and a sparse
set gives nothing there. It costs the component's size for every entity there may be, carried or
not: 25.6 MB for sixty-four bytes at four hundred thousand. Scored as failing scalable for that,
and minimal as the least of the three, having no index, no row to move and nothing to run out
of; where not building it fails fast. The need is argued and not measured in a deployment:
`reference.md` calls a reach by id something done a few times. Whether a cell nobody gave a
value to can be told from one that has one is a question that follows from it.

**The singleton passes.** Found through its query and read, a value on one entity costs 0.6 ns
each time, so there is no time to gain. What was found is that nothing holds there to be one: a
module that declares its settings component can attach it to a second entity, and its query
then answers two rows. As a storage layout, one place and no entity, it holds that by being what
it is; and it is a fourth way to keep a component and something a module is handed that is not
a query, which is how it was scored as failing minimal. Passed by the decision of whoever the
design is for, over a rule on a component kept in a table.

**So all five stand**, and every question below is asked of five.

