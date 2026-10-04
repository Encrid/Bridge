# Phase 1: vendor and patch flecs

**Draft.** Nothing here is settled until the done-when holds.

This phase writes no BridgeC. It produces **a flecs that allocates component columns and entity
arrays from a block we hand it**, a script that produces that flecs from a pristine copy, and a test
that proves the confinement rather than asserting it.

What is already settled and not to be re-argued: the version, the site count, what the sites are,
what confinement is worth, and how the patch is carried - `flecs.md`, *It can be confined, and the
patch is thirteen sites*, *How it is carried*, and the fourteenth site in *flecs will not tell you
which*. Why this is phase 1 rather than part of storage is `PLAN.md`.

## What it produces

    third_party/flecs/flecs.c     pristine, as published
    third_party/flecs/flecs.h     pristine
    third_party/flecs/README.md   version, licence, date, and that we patch it -
                                  which breaks the rule Bridge-Ironwail's own
                                  copy states, with the reason it cannot hold here
    tools/patch-flecs.py          the fourteen sites, gated on FLECS_VERSION
    tests/confined.c              the spike's assertion, as a test
    build/flecs.c                 generated. never committed

## The one open question

Where the fourteen sites route *to*. The spike **tagged** them to prove coverage; routing is a
different edit at the same places, and two constraints are already checked:

- `flecs_table_resize_column(column, count, old_size, new_size)` has **no world in scope**, so an
  allocator cannot ride on the world without changing a signature.
- the three entity-array sites go through `ecs_vec_*` with a `NULL` allocator, and `ecs_vec_grow`
  delegates to `ecs_vec_set_count(allocator, ...)` - a path other `NULL`-allocator callers share, so
  "NULL means ours" would capture more than columns.

So the shape is **a global trio of function pointers, set once before `ecs_init`**, mirroring
`ecs_os_api`'s own design: idiomatic for flecs, and no signature changes. The three vec sites
probably need the vec call replaced with explicit trio calls rather than a crafted allocator.

**Settled by trying it rather than by arguing** - the spike's harness exists in shape, so being
wrong about the vec sites costs an afternoon. `PHASE0.md` applies only in part to a phase like this:
steps 3 and 4 for that question, and no scan, because no document is changing.

## Steps

1. **Vendor**, and write the README that records what it is and that we edit it.
2. **The gate.** The script reads `FLECS_VERSION_MAJOR/MINOR/PATCH` and refuses anything else by
   name, so a version bump fails loudly instead of patching the wrong lines.
3. **The eleven direct sites**, routed to the trio. Each replacement asserts on its exact source
   text, so a moved site fails naming the string it could not find.
4. **The three vec sites** - the open question above.
5. **The test**, as the spike ran it: two world sizes, and a non-zero exit if one pointer lands
   outside the block, naming the component and the row.
6. **One build step** that patches, compiles, and runs it.

## Done when

`PLAN.md` states the done-when. What it means in practice is that the test is **wired into the
build** rather than run once by hand, and that pointing the script at a different flecs makes it
decline rather than proceed.

## What would make us stop

If a site cannot be routed without changing a signature flecs calls from many places, the patch
stops being fourteen lines and the fork gets expensive to carry. That is where the condition in
`flecs.md`, *How it is carried*, applies - ask upstream for the hook instead.
