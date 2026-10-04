# Phase 1: vendor and patch flecs

**Draft.** Nothing here is settled until the done-when holds.

This phase writes no Bridge-ASM700. It produces **a flecs that allocates component columns and entity
arrays from a block we hand it, and says so when a row moves**; a script that produces that flecs
from a pristine copy; two tests that prove both rather than asserting them; and the build they run
in.

What is already settled and not to be re-argued: the version, the site count, what the sites are,
what confinement is worth, and how the patch is carried - `flecs.md`, *It can be confined, and the
patch is thirteen sites*, *How it is carried*, and the fourteenth site in *flecs will not tell you
which*. Why this is phase 1 rather than part of storage is `PLAN.md`.

## What it produces

    third_party/flecs/flecs.c     pristine, as published. the 4.1.6 amalgamation
    third_party/flecs/flecs.h     pristine
    third_party/flecs/README.md   version, licence, date, and that we patch it -
                                  which breaks the rule Bridge-Ironwail's own
                                  copy states, with the reason it cannot hold here
    tools/patch-flecs.py          thirteen allocator sites and one hook, gated on
                                  FLECS_VERSION
    tests/confined.c              no pointer a module reads lands outside the block
    tests/rowmove.c               no row moves without the hook saying so
    CMakeLists.txt                patch, compile, test
    CMakePresets.json             Ninja, one preset per platform
    build/flecs.c                 generated. never committed

Bridge-Ironwail's copy under `src/ecs/third_party/flecs/` is the same amalgamation at the same
version, unedited, so it is the source to vendor from.

## The sites

Fourteen, of two kinds, and the difference decides what each is patched *to*.

    thirteen allocator sites   column malloc, realloc and free, and the entity arrays.
                               ten are direct calls; three go through `ecs_vec_*`
                               with a NULL allocator
    one row-move hook          `flecs_table_delete`. not an allocation: flecs moves
                               the last row into the hole, and the patch reports
                               the moved entity and its new row

`flecs.md` names the sites by their upstream files, `src/storage/table.c` and `src/bootstrap.c`. The
vendored artefact is one amalgamated `flecs.c`, so the script works on that file and finds each site
by its **exact source text**, never by line number.

Both kinds reach Bridge-ASM700 the same way: **a global set of function pointers, set once before
`ecs_init`**, mirroring `ecs_os_api`'s own design. Three for the allocator - allocate, reallocate,
free - and one for the row move. Two constraints rule out carrying them on the world:

- `flecs_table_resize_column(column, count, old_size, new_size)` has **no world in scope**, so an
  allocator cannot ride on the world without changing a signature.
- the three vec sites go through `ecs_vec_*` with a `NULL` allocator, and `ecs_vec_grow` delegates
  to `ecs_vec_set_count(allocator, ...)` - a path other `NULL`-allocator callers share, so "NULL
  means ours" would capture more than columns.

## Open questions

**How the three vec sites are routed.** They probably need the vec call replaced with explicit calls to
the allocator pointers rather than a crafted allocator.

**Settled by trying it rather than by arguing.** The spike that counted the sites is not on disk, so
the two tests below are written fresh from the numbers `flecs.md` records, and they are the harness
this question is tried in. `PHASE0.md` applies only in part to a phase like this: steps 3 and 4 for
that question, and no scan, because no document is changing.

## The tests

**Confinement** - `tests/confined.c`. Hand flecs a block, run a workload with table moves, column
resizes, swap-removes and deletes, then check that every pointer `ecs_get_id` answers and every
`it.entities` an iterator exposes lands inside the block. At two world sizes, 4,000 and 200,000
entities, as the spike ran it. A non-zero exit names the component and the row, or the table whose
entity array escaped.

**Row moves** - `tests/rowmove.c`. Keep a shadow index of entity to table and row, updated from
flecs' events and the hook and from nothing else. Run a workload of deferred moves, swap-removes and
deletes, then compare the shadow with flecs' own record for every live entity. A non-zero exit names
the entity and both rows. Without the hook this test must fail: the spike found 303 of 503 record
changes named by no event.

## Steps

1. **Vendor**, and write the README that records what it is and that we edit it.
2. **The build.** `CMakeLists.txt` and `CMakePresets.json`; the script runs as a build step that
   writes `build/flecs.c`, and the tests run through ctest.
3. **The gate.** The script reads `FLECS_VERSION_MAJOR/MINOR/PATCH` and refuses anything else by
   name, so a version bump fails loudly instead of patching the wrong text.
4. **The ten direct sites**, routed to the allocator pointers. Each replacement asserts on its exact
   source text, so a moved site fails naming the string it could not find.
5. **The three vec sites** - the open question above.
6. **The hook**, in `flecs_table_delete`.
7. **The two tests.**

## Done when

`PLAN.md` states the done-when. In practice: both tests are **wired into the build** rather than
run once by hand; the row-move test fails with the hook removed and passes with it; and pointing the
script at a different flecs makes it decline rather than proceed.

## What would make us stop

If a site cannot be routed without changing a signature flecs calls from many places, the patch
stops being fourteen sites and the fork gets expensive to carry. That is where the condition in
`flecs.md`, *How it is carried*, applies - ask upstream for the hook instead.
