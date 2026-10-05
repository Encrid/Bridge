# Phase 10: storage of our own

In progress.

`flecs.md`, *Why flecs rather than our own*, says that writing our own would have produced a
worse flecs. This phase does it anyway, and this document is why that is no longer the same bet.

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

## The gate

1. flecs measured as it is, and the untested behaviours pinned on it.
2. One internal header, `src/ecs.h`, with flecs behind it and nothing changed.
3. Ours beside it, chosen at build, the same cases run against both.
4. Every case on ours, on three platforms, alone and under load; every figure measured again.
5. flecs removed.

**What stops it**: ours slower on any line and not to be made otherwise.

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

## What flecs costs

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

The second figure of the lower three is the one that was not expected: with ten thousand tables
standing and nothing changed, a barrier is eight hundred times dearer than with one. Every
barrier walks every query's matched tables to publish them again, changed or not.
