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

## The questions

Each is decided here and built in step 4, where it goes into the document it belongs to.

**1. The author states a component's storage layout, at `declare`.** One more argument beside
`on_unload`, an `enum bridge_storage`, where zero is unset and refused. Every module that
declares a shared component states the same one, or its load is refused naming the others and
both storage layouts, as a component layout that differs is. The reason is what decides it: a
table's column, a sparse set's array and a cell by entity are read by different code, and that
code is fixed when the module is compiled, as `sizeof(T)` is - `query.md`, *The interface is a
query*.

    need          at declare           in the manifest         the deployment, or both
    fast          serves               serves                  fails: a module carries code
                                                               for all five, or reads by a call
    scalable      does not apply       does not apply          serves: it knows the population
    modular       serves: checked      serves                  fails: it must know each
                  between declarers                            module's reading code
    extensible    serves               serves                  serves
    minimal       serves: one more     fails: a second place   fails: a table of
                  argument             that names components   configuration, or two places
    explicit      serves: unset is     fails: code and         fails: what runs is not what
                  refused              manifest can disagree   the code reads as

**2.1 A query walks one thing, and what is kept elsewhere is reached from its row.** A table's
rows are in the table's order and a sparse set's entries in the set's, so row 0 of one and entry
0 of the other are different entities, and one loop cannot step through both side by side. So
the terms that select, `BRIDGE_AND` and `BRIDGE_NOT`, are all of components kept in tables, or
are one sparse set alone; that is what is walked, as its storage layout is walked. A term of a
component kept elsewhere is stated `BRIDGE_OPTIONAL` and reached by entity id through an index
storage publishes, as `column_base` is; whether this row has one is tested by the module's own
code. A query that states anything else is refused at `system`, naming the term. Which side is
walked is the author's, by how the query is written.

Measured in scratch, the walk alone: a table's rows with a sparse value reached through its
index, 0.59 to 0.76 ns a row of the table; a set's entries with a table's value followed by id,
1.1 to 2.5 ns a carrier; a table's columns alone, 0.028 ns a row.

    need          one walked, the rest     the matching rows listed      an archetype counts
                  reached                  at each barrier               every component
    fast          serves: a query of       fails: barrier work for       fails: a query that
                  tables is untouched      every change of such a        names one leaves the
                                           component, in each query      column loop wholly
    scalable      serves                   fails: a list a query         fails: archetypes
                                                                         multiply again
    modular       serves                   serves                        serves
    extensible    serves: direct-indexed   does not apply                fails
                  and singleton are
                  reached the same way
    minimal       serves: an index a       fails                         fails: a second
                  component, a refusal                                   structure
    explicit      serves: the test is in   serves                        serves
                  the reader's code

What it gives up: a query cannot say "must carry" of a component in a table and of one in a
sparse set, and be handed those rows alone.

**2.2 For a sparse set a module is handed a span when it walks it, and a cell for each entity
when it reaches into it.** Walked, a query on the component alone has one span: the count, the
ids of the entities that carry it, and the values, which is what a table's span is, read by the
same loop. Reached from a table's row, the module reads the component's index, in the shared
memory: **one cell an entity, holding where that entity's value is, and zero where it has
none**. So `at = index[e]`, and the value is at `at` if `at` is not zero, which is the rule
every base of a span already follows - `query.md`, *The interface is a query*. A module finds
each index through a cell for each type it declared, in its private memory beside
`column_base`.

Measured in scratch, a table's 204,800 rows walked and the value reached at each:

                                      every row carries it     one in sixteen
    the cell holds where it is        0.76 ns a row            0.58 ns
    the cell holds its place          0.77 ns                  0.60 ns
    the index in pages of 4,096       1.17 ns                  0.95 ns

It is four bytes for every entity there may be, for each such component, and as much again for
the copy storage keeps - `storage.md`, *Nothing in the shared block is read back*: 3.2 MB at
four hundred thousand. That is where it was scored as failing scalable, by an amount the
deployment states. When the set's arrays move to grow, each carrier's cell is written again,
once for a value that is copied then anyway.

**2.3 A toggled component is a column and a bit a row, and a module is handed the bits.** A
query that states the component matches its table, since every row carries it, on or off;
whether a row is on is tested by the module's code, as 2.1 has it. The bits are 64-bit words:
row r is bit `r & 63` of word `r >> 6`. Testing every row, finding the rows that are on a word
at a time, and taking runs of them are three loops over those words, and none is chosen for the
module.

**A row is switched by a write in place.** The system states the term `BRIDGE_WRITE` and sets
or clears the bit, as it writes a value in the column. The scheduler already keeps a system
that writes a component from running beside another that reads or writes it - `data.md`,
*Access mode* - so two never write one word at once. No entry is added, and no observer is
told: a write in place raises none. One store, where a queued change that moves nothing is 18.9
ns and the detach and attach it stands in for are 77.

    need          a write in place          a queued change            both
    fast          serves                    fails beside it            serves
    scalable      does not apply            does not apply             does not apply
    modular       fails in part: another    serves                     serves
                  module is not told
    extensible    does not apply            does not apply             does not apply
    minimal       serves                    fails: two entries in      fails most
                                            each backend and SDK
    explicit      serves: it is a write     serves                     fails: two ways, and
                                                                       one of them heard

A module that must be told of a change has the sparse set, whose changes are queued and heard.

**The bits are found ahead of the column.** The sixteen bytes before a toggled column's first
value hold where its bits begin, so a reader that has the column, from a span or from
`column_base`, has the bits by one more load, and neither published shape changes. Measured in
scratch, a toggled value reached by entity id with its bit tested, 256 tables:

                                      204,800           204,800          8,000,000
                                      in order          in no order      in no order
    no bit, as today                  1.47 ns           2.29 ns          16.5 to 16.7 ns
    a second cell for the bits        2.17 ns           3.07 ns          18.2 to 18.6 ns
    ahead of the column               2.17 ns           3.10 ns          18.6 to 18.8 ns

Testing the bit costs 0.7 ns, and where the bits are found costs nothing that shows. Not
measured: many thousands of tables reached in no order, where the cell ahead of each column
may miss the cache and a denser array of cells might not.

**2.4 A direct-indexed component is a cell for every entity and a bit for every entity that
says it has one.** The cells are found through the same cell for each declared type as a sparse
set's index, and entity i's value is cell i. A cell exists for an entity nobody gave the
component to, so the bit is what tells a value from bytes: set by `attach` and cleared by
`detach`, queued and applied at the barrier and told to observers, as for a component kept any
other way. The bits are found ahead of the cells, as a toggle's are ahead of its column. A
reader that needs to know tests the bit, and one that knows by its own design does not.

Measured in scratch, a value reached by id with its generation tested:

                                      204,800           204,800          8,000,000
                                      in order          in no order      in no order
    as today, through the table       1.45 ns           2.22 ns          16.5 ns
    direct-indexed, no bit tested     0.62 ns           1.19 ns          10.9 ns
    direct-indexed, the bit tested    0.98 ns           1.61 ns          13.3 ns

    need          a bit an entity                      every entity has it, zero until written
    fast          serves: under the table tested       serves
                  or not
    scalable      does not apply                       does not apply
    modular       serves: heard as any component is    fails: a given value cannot be told
                                                       from none, nor heard
    extensible    serves: one kind of component        fails in part: a component nobody
                                                       attaches is a second kind
    minimal       fails in part: bits, and attach,     serves
                  detach and strip for it
    explicit      serves: has or has not               serves only where zero may mean none

**2.5 A singleton is one value at one place, given by `attach` with the id 0.** A module finds
the place through the cell for each declared type, zero there meaning no value, and reads it by
one load. Every entry that gives or takes a component names an entity, and a singleton is on
none: so the id is 0, which is no entity - `reference.md`, *Reference following* - and any other
is refused by name, as 0 is for a component kept any other way. It is queued, applied at the
barrier and told to observers, whose record carries the id 0; it can be without a value; and
its fields are changed afterwards by a write in place, as any value's are. Nothing was
measured: it happens a few times in a run.

    need          attach with id 0          in place only                 two new entries
    fast          does not apply            does not apply                does not apply
    scalable      does not apply            does not apply                does not apply
    modular       serves: heard             fails: given cannot be told   serves
                                            from never given
    extensible    serves: one kind of       fails in part: a second kind  serves
                  component
    minimal       serves: no entry added    serves                        fails
    explicit      serves: a wrong id is     serves only where zero may    serves
                  refused by name           mean none

**2.6 The component a module names as `config` is a singleton, or the load is refused by
name.** Bridge-ASM700 gives it its value with the id 0 at init, where it spawned an entity of
its own for it; that entity and the code that keeps it go. It closes what the gate found: a
module could attach its settings component to a second entity, and `config.md` called it a
singleton with nothing holding it to one. Allowing either was scored as failing minimal, two
paths for one thing; leaving it as it is as failing explicit.

**3. What grows with rows is paid from `[budget] columns`, and what is kept for every entity is
bounded by two keys.** A toggle's bits and the sixteen bytes ahead of its column are part of its
table's block. A sparse set's values and the ids beside them are a block of the column region,
grown by half as a table's is. A singleton's value is a block of its size, taken at `declare`.
Running out of `columns` is what it is today: the entity is left as it was and the module that
asked has failed.

A sparse set's index and a direct-indexed component's cells and bits are kept for every entity
there may be, carried or not, and how many such components there will be is not known when the
bridge is created. So the deployment states it:

    [budget] sparse_sets       how many components may be kept in a sparse set. Storage
                               reserves their indexes whole at create: eight bytes an entity
                               for each, the published one and its own
    [budget] direct_indexed    how many may be direct-indexed. Storage reserves their bits at
                               create. Their cells are the component's size an entity, which
                               is not known at create, so they are taken from `columns` at
                               `declare`

A `declare` past either key is refused, and one whose cells do not fit `columns`. So storage
still allocates nothing after `bridge_create` - `storage.md`, *What bounds it*.

    need          two keys                    from `columns` at declare     the most there
                                                                            may be
    fast          does not apply              does not apply                does not apply
    scalable      serves                      serves                        fails: by `types`
                                                                            times `entities`
    modular       serves                      serves                        serves
    extensible    fails in part: a key a      serves                        serves
                  storage layout
    minimal       fails: two keys, and        serves                        serves
                  cells from `columns` still
    explicit      serves: stated by who       fails in part: `columns`      fails: a number
                  knows the modules           pays for three kinds of       nobody stated
                                              thing, and storage
                                              allocates at load

Scored as failing minimal, and chosen: a load cannot fail for an index that create would have
reserved, and storage's rule holds as written.

