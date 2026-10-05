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
