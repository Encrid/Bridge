# Stage 1: vendor and patch flecs

Done. The `linux` preset was first run after stage 5, and passed as written.

This stage wrote no Bridge-ASM700. It produced **a flecs that allocates component columns and entity
arrays from a block we hand it, and says so when a row moves**; a script that produces that flecs
from a pristine copy; two tests that prove both rather than asserting them; and the build they run
in.

What the patch is and why is `flecs.md`, *It can be confined*, *How it is carried* and *flecs will
not tell you which*. This document is what the stage built and what it found on the way.

## What it produced

    third_party/flecs/flecs.c     pristine: the v4.1.6 release, as published
    third_party/flecs/flecs.h     pristine
    third_party/flecs/readme.md   version, commit, hashes, licence, and that we patch a copy
    tools/patch-flecs.py          the edits, gated on the version and on a hash
    tests/workload.h              what both tests run
    tests/confined.c              no pointer a module reads lands outside the block
    tests/rowmove.c               no row moves without something saying so
    CMakeLists.txt                patch, compile, test
    CMakePresets.json             Ninja, one preset per platform
    build/                        generated, flecs_confine.h included. never committed

    cmake --preset windows && cmake --build --preset windows && ctest --preset windows

## The interface

`flecs_confine`, declared in the generated `flecs_confine.h` and set before `ecs_init`:

    alloc(size)                           -> ptr
    realloc(ptr, old_size, new_size)      -> ptr
    free(ptr, size)
    row_moved(world, entity, table, row)      may be left NULL

Sizes are passed everywhere, because flecs knows them at every site and an allocator carving a block
should not have to store them. It is a global rather than something on the world, mirroring
`ecs_os_api`: the `ecs_vec` functions that do the allocating have no world in scope.

## The tests

Five, and two of them pass by failing:

    confined              0 escaped, at 4,000 and at 200,000 entities
    rowmove               0 wrong, over 44,845 operations
    confined-unconfined   the same test, on a flecs without the allocator edits: must report
                          every pointer escaped
    rowmove-nohook        the same test, on a flecs without the hook: must report mismatches
    patch-version         the script, on a flecs.h saying 4.1.7: must decline by name

The two middle ones are what makes the first two mean something. **A test that cannot fail proves
nothing**, and both of these were seen failing for the stated reason - 225,974 escaped, 5,374
mismatched - rather than by crashing.

## What the stage found

**The pin was wrong, and the version number could not have said so.** The copy the plan meant to
vendor, Bridge-Ironwail's, reports 4.1.6 and is byte-identical to an upstream `master` commit of
September 2026. The v4.1.6 tag is from June and its table storage is different code: no
`flecs_table_resize_column` at all. The stage 0 spike had run against the snapshot. The release was
chosen, so the script checks a SHA-256 of both files as well as the version.

**So the site list was re-derived, and it is not thirteen.** In the release a column and an entity
array are both an `ecs_vec`, allocated by calls that pass `NULL` for the allocator. The patch is:

    23 calls, in 9 functions      pass FLECS_CONFINE instead of NULL
     4 functions of ecs_vec       learn what that argument means
     4 lines of flecs_table_shrink    which allocates by hand
     1 line of flecs_table_delete     the row-move hook

The open question the plan carried - how the vec sites are routed - closed in the direction it did
not expect: not explicit calls replacing the vec, but a sentinel allocator the vec itself
understands. "NULL means ours" stays refused; other callers pass `NULL` and mean the OS.

**One hook is enough for what Bridge-ASM700 does**, and the test's workload includes the other
thing that moves rows - `flecs.md`, *flecs will not tell you which*, for which sites are silent and
why two of them are not hooked.

**What confinement is worth was not measured in this stage.** The spike's figures, from the
snapshot, stood in `flecs.md` until stage 5 closed, when `tests/confined.c` took them on the
release: `flecs.md`, *It can be confined*.

## Open questions

None.

## What would make us stop

It did not happen: no edit changes a signature. It is still the condition for the next flecs, and
what an upgrade takes is `flecs.md`, *How it is carried*.
