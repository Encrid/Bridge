# Stage 4: the native backend

Done. The `linux` preset was first run after stage 5, and passed as written.

The second backend: a shared library that opens a module's library and calls it directly. What it
is and why is `native.md` and `capabilities.md`. This document is what the stage built and what it
found on the way.

## What was settled first

Nothing needed the method. Two things `native.md` had left unsaid were forced by what it does say,
and are written there now, each with what it refuses: how an entry knows which module called it,
and what a refused entry does when nothing can unwind its caller.

## What it produced

    include/bridge-native.h           bridge_api, the export macros, and the layouts of
                                      bridge-abi.h read through real pointers
    backends/native/native.c          the seventeen entries, over a shared library
    tests/modules/native/nmover.c     stage 3's mover, as a library
    tests/modules/native/nflood.c     a system that spawns past the ceiling
    tests/stage4.c                    a host with both backends configured

Above the backend seam one line moved: the first refusal of a call is kept as its reason, since on
this backend a module runs on after one.

## The tests

Six new ones, forty-three in all, in a release build and a debug one. A seventh, `setting`, came
after stage 5.

    raw          a native system adds to every row through a `Position *`, and another counts
                 two tables and 1,000 rows
    mixed        the wasm reader and a native system in one phase, over one column: the
                 reader's sum moves by what the native module wrote
    untrusted    `trusted = false` on the native backend: refused for `sandbox`
    limit        a stated `fuel`: refused for `meter`
    nolib        a library that will not open: the module's failure, by name
    refuse       a native loop of `spawn` past the ceiling: one failed invocation, by name

`untrusted` and `limit` name a library that does not exist, and must not say so: that is "before
its library is opened".

## What the stage found

**The seam held.** Bridge-ASM700 gained no native case. The scheduler, the barrier, the entries and
the load path are the code stage 3 wrote; the native backend is under 300 lines beside them. That is the
claim `backend.md` makes for the surface, and this is the first time it was tested by a second
backend.

**The capability refusal ran false for the first time.** Stage 2 wrote the check and could not
test it, having only a backend that answers yes to everything.

**A refusal has to be sticky on a backend that cannot unwind.** The first version let a native loop
call `spawn` 96,000 times past the ceiling; each call minted an id and queued its deletion, and the
column budget ran out behind it. So after a refusal the backend answers zero to every entry until
the call returns - `native.md`, *What the backend provides*.

**A macro parameter shadowed the variable it was to set.** `bridge_bind` assigned its argument to
itself, and every native module crashed in `init`. Recorded because the fix is one renamed
parameter in a header every native module compiles.

## What it left

**Nothing catches a native fault.** A native module that dereferences a bad pointer takes the
process with it, which is what `sandbox = no` says.

**`bridge_reload` and `bridge_shutdown` are not called yet.** Neither is a wasm module's; both
arrive with unload and reload in `PLAN.md` stage 6.

**The manifest section is not read.** `bridge_manifest_parse` is the host stage's, beside the library.

## Open questions

None for the next stage.
