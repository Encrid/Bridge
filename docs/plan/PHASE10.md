# Phase 10: storage of our own

In progress: flecs is out and the gate is passed, and *What was not in the plan* lists what was
built without being decided and is being decided now, one at a time.

`flecs.md`, *Why flecs rather than our own*, said that writing our own would have produced a
worse flecs. This phase did it anyway, and this document is why that was no longer the same bet,
and what came of it. What keeps the rows now is `storage.md`.

## What changed

**flecs is not where the time goes.** What an ECS is tuned for is iteration: matching a query and
walking its rows. Bridge-ASM700 does that itself. It publishes spans, and a system reads its
columns through them with no call into anything. flecs runs at a barrier, where it applies each
queued change, and at load. The parts that made it a framework - its systems, its pipeline, its
deferral, its threads - were never used, because each had to be ours for a reason of its own.

**What it costs was measured in phase 9's wake.** A hundred thousand lines, of which forty-one
calls are used. About fifty C library functions even built with one addon, file input and number
parsing among them, which is what stood in the way of `build.md`, *Self contained*. Between
three quarters and nine tenths of its memory outside the shared block. A patch at twenty-eight
sites that the next release of flecs has already invalidated, since it rewrote the storage the
patch touches. A thread-local variable, because its allocator callbacks carry no context.

**And the fear has a test.** That ours would be slower is a claim, so it is checked like one.

## What was settled

    the ECS            Bridge-ASM700's own, modelled on flecs
    when flecs goes    only when ours passes every case and is not slower on any measurement
    if it does not     flecs stays, and a fork or the patch is chosen with the numbers in hand

Refused: extending the patch to cut flecs down, which is a fork kept by regular expression.
Refused for now: a fork, a trimmed copy that is ours. It is what this falls back to.

## Not a freestanding flecs

The first plan for this phase drew the line between the library and its storage at flecs'
primitives under other names: make an id, attach, detach, match, watch. Behind a line of that
shape the best ours could be is a smaller flecs, and it was put back to be done again, from what
the documents say and by every step of `PHASE0.md`.

flecs has to serve any program. Bridge-ASM700 promises much more than any program does, and each
promise is something a design made for it can spend:

    storage is touched only at a barrier, by one thread    `threads.md`, *Threads*
    every change of a barrier exists before the first      `data.md`, *Structural change*
    is applied
    queries are fixed at load                              a system registers in init only
    nobody iterates through storage                        `query.md`, *The interface is a query*
    no order of rows or spans is a module's to rely on     `module.md`, *What no module may do*
    a closed set of types, with no pointer among them      `schema.md`, *The type set*
    no pairs, no wildcards, no or-terms                    refused where terms are handed over

Seven questions followed, one at a time.

    where the seam sits              at what the library does: declare this, apply these queues,
                                     write what this module reads. As the backend seam is drawn
    the unit of work at a barrier    an entity. Its changes are folded, in the order stated,
                                     into what it will be; then it is placed once. No table is
                                     made that nothing stays in
    what a barrier publishes         what changed, and a span for each table that has rows. A
                                     barrier that changed nothing writes nothing
    what storage trusts of the       nothing. Its own index and row lists are outside it; what is
    shared block                     in it is a copy, written and never read back
    what bounds storage's memory     a table lives while it has rows, and `[budget] tables` says
                                     how many may exist at once. All of it allocated at create
    out of tables                    the entity stays as it was, and it is a failure of the
                                     module that asked, toward its own budget
    out of columns                   the same, and no memory from outside the block

    what a descriptor may say        exactly the type set, in the spellings a module's own header
                                     uses, and nothing of flecs' vocabulary: `schema.md`, *The
                                     type set*

Each is in its document now: `storage.md`, `budgets.md`, *What running out does*, `schema.md`,
*The type set*.

The last of those was asked when the parser came to be written. flecs' own took `_Bool`,
`ecs_entity_t`, `const`, pointers, C++ template spellings, any symbol flecs itself knew, and
`ECS_PRIVATE`, after which the rest of a struct went undescribed. None of the tree's descriptors
used one of them. `src/schema.c` states its whole grammar in the comment at its head, and
`tests/schema.c` reads every accepted spelling against what the compiler lays the same struct out
as, and has every other refused by the word it stopped at.

**The fourth closes something that is open today.** When a row is removed, flecs reads from the
table's array of entity ids which entity filled the hole, and updates that entity's record in
its own index. The patch of phase 1 moved that array into the shared block, so that modules could
read it. A module can therefore write it, and what it writes is then an id flecs acts on. Nothing
had asked what storage believes of memory a sandboxed module can reach; the answer now is nothing.

**The last two exist because of the second.** Columns running out stopped the bridge for good,
and the documents gave two reasons: nobody could be named, and flecs could not be unwound from
the middle of a change. An entity's whole fate is known before a row of it moves, and so is who
asked last, so neither reason holds, and one module's growth no longer takes a deployment down.

## What a replacement has to reproduce

Every use of flecs was read, in the four files that reach it: `src/store.c`, `src/abi.c`,
`src/lifecycle.c` and `src/config.c`. What a module, a host or a test can see:

    an entity id            index in the low 32 bits, generation in the high 32; a record of
                            table, row and generation by index; table 0 is no table
    tables                  dense ids; a column's base by table and slot; columns and each
                            table's entity array inside the shared block
    spans                   one for each matched table: count, entities, a base for each term,
                            zero where a term is NOT or an OPTIONAL that did not match
    terms                   AND, NOT, OPTIONAL over whole tables, and every refusal's text
    changes                 applied at a barrier, in the order modules loaded and then asked;
                            a dead id passed over; ids set aside for each module
    events                  attach to an entity without the component is add and set, with it
                            set; detach and destroy are remove; one record an entity
    declare                 size and alignment from the descriptor's text; a field's type found
                            in the declarer's namespace; pointer-sized types refused
    settings                written by a field's type, with every error text
    unload                  strip, sweep, delete; a type kept while a field names it
    the column budget       refused at create, or the bridge stopped at a barrier

Four of those had no test: NOT and OPTIONAL terms, an observer with more than one term, what an
emptied table's span says, and what a destroy tells an observer. They are pinned on flecs before
anything is written to match them.

## What goes with flecs

Everything that exists because flecs decides where a row is and Bridge-ASM700 has to be told:
the patch and its script, the hook for a moved row, the two observers that feed the mirror, the
list of entities to look at again, the table ids that are dead and then free, the thread-local
variable. Ours places every row, so the index is written where the row is placed.

## What a host or a module will see change

    entity ids              start lower. flecs' own entities, and one for every scope, type,
                            member and observer, shared the id space and were not counted
    [budget] entities       counts entities and nothing else
    [budget] columns        loses its floor of about 50 KB, which was flecs' own tables
    field tables            leave the shared block: a module could address them there
    records naming flecs    are reworded, and the forwarded "flecs: ..." ones are gone
    the generation          is 32 bits in fact, where flecs gave 16 in a field of 32
    [budget] tables         a new key: how many sets of components may exist at once
    out of tables, or of    the entity is left as it was and the module that asked has failed.
    columns                 A bridge no longer stops for columns
    spans                   too small for `column_base` at the stated `tables`: refused at load,
                            where it was a stop at some later barrier
    tables                  none that nothing stays in, and none once it is emptied

## The gate

1. flecs measured as it is, and the untested behaviours pinned on it.
2. One internal header, `src/rows.h`, with flecs behind it and nothing changed. Not `ecs.h`:
   while flecs is in the tree that prefix is its own.
3. Ours beside it, chosen at build, the same cases run against both.
4. Every case on ours, on three platforms, alone and under load; every figure measured again,
   and three more on both: an entity given several components in one barrier, a barrier with
   nothing to do and ten thousand tables standing, and a set of components filled and emptied
   at every barrier.
5. flecs removed.

**What stops it**: ours slower on any line and not to be made otherwise. The last of the three
new lines is where a table that lives only while it has rows could lose, and if it does that
question is put again with the figures.

## What flecs does, pinned

`tests/storage.c` drives a native module, `tests/modules/native/nstore.c`, that makes one kind of
change in a phase of its own and says what a system and an observer then see. The host prints
it. What it prints is kept in `tests/storage-changes.txt` and `tests/storage-tables.txt`, and
the two cases pass only by saying the same, word for word. Nothing in a transcript depends on
how ids are numbered, so the same two files will judge storage of our own.

What was not known until it ran:

    an emptied table            gets no span: a system never sees a span of no rows
    a term that is NOT          takes a span's place and has no base
    an OPTIONAL term            has a base in the tables that carry it and none in the rest
    an observer of two terms    hears an entity when it comes to match both, by whichever
                                change completed the pair: add and set. A set of either, while
                                it matches, is a set. Losing either is a remove
    a destroy                   is a remove to each observer the entity matched, once, however
                                many times it was asked for
    there and gone in one       is one record with all three events: add, set, remove
    barrier
    a detach of what is not     nothing, and no event
    there; a change to a dead
    id
    a value attached            is the bytes at the call, whatever is written there after
    a freed index               comes back under another generation, some barriers later,
                                since ids are set aside a barrier ahead. Every destroyed id
                                reads as dead in the index meanwhile
    ten thousand tables         each entity of ten thousand in a table of its own: five
                                thousand spans for a term half of them carry

None of it looked accidental, so ours copies all of it. One line of the transcript was taken
out before it was kept: how many tables there are, which counted flecs' own.

## Found on the way: a sleep is a least time

`phase5-parallel` failed on macOS twice in four runs of one commit, with two systems on two
threads taking 203 ms where two sleeps of 100 ms at once should take 100. Each system was made to
say when it began, and four more runs showed both beginning in the same millisecond every time:
the pool had woken its worker at once. What was late was the sleep. `usleep` for a tenth of a
second returned after 116 to 200 ms on that runner, where Windows and Linux return within a few
of it: macOS gathers timers together to save power, and a virtual machine adds its own delay.

The case had been deciding "at once" from "took under 180 ms", which holds only where a sleep
takes what it was asked for. It decides now from what it claims: each system says when it began
and ended, and two ran at once if one began before the other ended. No length is asked of
anything. Widening the bound was refused, as `PHASE8.md` said a bound would be.

## What flecs cost

`storage measure`, on Windows, with nothing else running. A hundred thousand entities, each
change made to all of them in one phase, and the barrier that applies them timed alone. The best
of six rounds, three runs. This is the line ours has to stay under.

    an id handed out                          16 to 17 ns
    attach, the entity's first component     120 to 140 ns
    attach, a component it has                26 to 27 ns
    attach, a second component               126 to 146 ns
    detach                                    97 to 111 ns
    destroy                                   66 to  75 ns
    a barrier with nothing to do             about 100 ns

    ten thousand tables made                 3.4 us a table
    a barrier with nothing to do, then       82 to 85 us
    ten thousand more, by one attach each    2.8 us a table

    the stress run, on four threads          3.5 s
    the stress run, on one thread            5.6 s

**The stress run** is `storage stress`: four modules at once, each asking for twenty thousand
changes a pass for a hundred passes, chosen by a generator it seeds itself - an entity made with
a few components, one destroyed, a component attached, set or detached. Eight million changes.
Each module keeps its own account of what every one of its entities ought to carry and with what
value, and at every pass checks storage against it twice over: every entity through the index
and `column_base`, and every component through its spans. What it prints is how many disagreed,
which is none, and a digest of its own account, kept in `tests/storage-stress.txt`. The digest
does not depend on what keeps the rows or on how many threads ran, so the same file judges ours;
and the two times above are what ours is measured against on a mixed load, where the lines
before them each measure one kind of change alone.

The second figure of the lower three is the one that was not expected: with ten thousand tables
standing and nothing changed, a barrier is eight hundred times dearer than with one. Every
barrier walks every query's matched tables to publish them again, changed or not.

## What it costs

The same program, on the same machine, with storage of our own. One run beside one run of flecs
taken the same hour, so the first column is not quite the ranges above.

                                              flecs        ours
    an id handed out                          17.0 ns       1.5 ns
    attach, the entity's first component     123.9 ns      30.8 ns
    attach, a component it has                27.4 ns      21.0 ns
    attach, a second component               128.1 ns      38.3 ns
    detach                                   100.5 ns      34.3 ns
    destroy                                   66.1 ns      15.3 ns
    attach, two components in one barrier    241.3 ns      37.1 ns
    a barrier with nothing to do             100 ns        under 100 ns

    ten thousand tables made                   3.4 us       0.3 us a table
    a barrier with nothing to do, then        85.4 us       under 0.1 us
    ten thousand more, by one attach each      2.9 us       0.3 us a table
    ten thousand sets filled                   0.4 us       0.2 us a table
    and emptied again                          0.3 us       0.2 us a table

    the stress run, on four threads          3647 ms      1684 ms
    the stress run, on one thread            5531 ms      3343 ms

Not slower on any line, which was the gate. Three lines were added for it, each where a decision
could have lost. **Two components in one barrier** is the fold: flecs moves the entity twice and
ours once, so ours costs what one attach does. **A quiet barrier with ten thousand tables** is
publishing only what changed. **Ten thousand sets filled and emptied** is the one that was a risk:
flecs keeps the emptied tables and only moves rows into them again, where ours makes and releases
ten thousand tables at every barrier, and still it is the quicker.

**Memory is not ahead on every line, and it is a budget that decides.** What the process holds, as
the system counts it, at the end of each of the three runs:

                                    flecs        ours
    a hundred thousand entities     30.3 MB      27.8 MB
    ten thousand tables             86.6 MB      22.9 MB
    the stress run                  46.4 MB      61.1 MB, and 48.1 MB with `tables` at 2,048

The test states `[budget] tables` as 32,768 for all three, because one of them needs twenty
thousand. The stress run has about a thousand, and pays for the rest: `column_base` has a cell for
every table there may be, in each of four modules, and storage keeps a row of offsets a table.
Stated at what it uses, the run holds what flecs did. Columns in use are 9,052 KB against 8,427,
since a table asks for twice what it had.

## What the cases say

Every case passes on storage of our own on Windows, in release and in debug, on Linux and on
macOS, and four runs of the whole suite at once on one machine pass too. Before flecs was removed
both storages were built from one tree and run against the same cases by the same script, and the
three transcripts - `storage-changes.txt`, `storage-tables.txt`, `storage-stress.txt` - were the
same files for both, word for word.

Three cases are new and are of what only this storage does. Each is a transcript.

    storage notables     `tables` of 1: the entities that would have needed a second table are
                         left as they were and unheard, a record names the module, and at its
                         second failing barrier in a row it is stopped
    storage nocolumns    `columns` of 64 bytes: three of six rows fit, then a fourth, then a
                         fifth exactly, with the bridge running throughout; the sixth never
                         does, and the module is stopped at its budget
    storage scribble     a module writes over every index record and every entity array of
                         its entities, each to name the first; rows are then destroyed and
                         moved, and every entity left is where the index says, under its own id

The first two were each run with the rule they test taken out, and failed. The third was run on
flecs before it left: its observers heard the first entity removed three times over, and an entity
nobody had made.

Three cases changed their answer, as the decisions said they would. `phase3-nospans` is refused
at load where it was stopped at a barrier. `phase3-nocolumns` leaves 747 of 1,000 entities as they
were and runs on, where the bridge stopped. `phase6-configceiling` runs out at `entities` of 1,
where 440 was flecs' own entities and one more. And `phase5-nested2` is refused by the word the
reading stopped at.

Two more came with the decision on strip: `storage strip` and `storage stripfull`.

Six cases went with flecs: the two that held its patch to its claim and the two that showed each
failing without it, the one that declined another version, and the one that freed a table each of
the ways flecs could.

## What was not in the plan

Five things were built as whatever was nearest to hand and written up as settled. None had been
through the steps of `PHASE0.md`. Each is put back as a question, and this section says where
each stands.

**A strip at unload: decided.** As first built it moved an entity at a time to the table of what
it had left, needed that table to stand beside the one it was leaving, and destroyed the entity
when `tables` or `columns` could not give it. The plan had said a table at a time, and `strip`
says the entity stays. It is now worked a table at a time, in place wherever a join is not to be
had, and cannot fail: `lifetime.md`, *What unload leaves behind*. `storage strip` and `storage
stripfull` are its cases, each run with its rule taken out and failing.

**What storage may allocate after `bridge_create`: decided, and it is nothing.** As first built,
a set's width and each system's list of tables were sized by how many types and systems there
were, which no budget stated, and allocated when a module loaded; and the plan's sentence had been
reworded to fit. There are two more keys now, `[budget] types` and `[budget] systems`, and
storage allocates in `rows_create` and nowhere else: `budgets.md`, *Budgets*. A type's fields
left storage for the component's record on the way, since storage only ever needed its size.

**Open: a test that a barrier allocates nothing.** It holds by reading: `src/rows.c` calls no
allocator outside `rows_create`. Nothing checks it.

**Open: an allocation that fails at load.** Older than this phase and found in it: of the
library's allocations when a module loads, one checks what it was given.

**Open: which entities are placed when columns run short.** As built, a table that cannot take
all the rows arriving takes as many as fit, found by halving what it asks for.

**Open: `source[]` in a span, which is always zero.** Under flecs it could name the entity a term came from;
nothing a module could ask for ever made it do so, and storage of our own has no such term. The
field stays in the span, and `query.md`, *The interface is a query*, says what it holds.
