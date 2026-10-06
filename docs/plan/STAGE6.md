# Stage 6: failure and lifecycle

Done, on Windows and on Linux.

A module could load, run, and fail one invocation at a time. Now it can be stopped, unloaded,
reloaded and configured, and a host can ask what is running. `PLAN.md` stage 6 is the claim. This
document is what the stage built and what it found on the way.

## What was settled first

Thirteen questions, each by every step of `STAGE0.md`, and each applied to the document it belongs
to.

    what a reload runs            the new instance's reload, then the old one's shutdown:
                                  `lifecycle.md`, *The runtime lifecycle*
    what the budget counts        raised by a failed barrier, lowered by a clean one:
                                  `failure.md`, *The budget*
    fail, on a stop               it covers one, as it covers a load:
                                  `failure.md`, *When something fails*
    which component is settings   the one the author names:
                                  `config.md`, *A module's own settings are a component*
    where a text setting goes     where its field's type says: the same section
    how an author says "cannot"   with a key, `reloadable`, and never by leaving `reload` out:
                                  `wasm.md`, *Exports*
    how a module has no settings  it says `config = none`; left out, `config` is unset:
                                  `config.md`, *A module's own settings are a component*
    an artifact with no bytes,    a fact, not a statement; the backend that needs the missing
    or no path                    one refuses by name: `backend.md`, *What a backend is asked for*
    a reload that fails with no   the module stays, stopped, with what it declared:
    old instance left             `lifecycle.md`, *The runtime lifecycle*
    a share, across a reload      it may grow and never shrink: `budgets.md`, *Budgets*
    a stopped bridge              accepts the call that brings back what stopped it:
                                  `failure.md`, *When something fails*
    when a table's id is retired  when flecs says it freed the table:
                                  `flecs.md`, *flecs will not tell you which*
    destroy, with a pinned        refused from any thread but the module's, with nothing torn
    module loaded                 down: `create.md`, *Destroy*

Three of them were first put with steps missing, and were put again. Reading the original Bridge's
source for the second pass gave reload a fourth option and changed the recommendation for the
budget. The sixth was asked after the stage was reported: its report called `reload` optional, which
is how these documents had it from the start, and a statement made by leaving something out is what
this project refuses everywhere else. The seventh is the same defect found by looking for it: a
null `config` meant "no settings". The eighth looked like it and was not: an artifact's bytes or
path may be null, and the answer was to keep that and make the two refusals exact, `nopath` and
`nobytes` among stage 4's cases.

The last four were decided while building and reported afterwards as "forced by the documents".
Put through the method, every one came out differently from what had been built. A failed native
reload had been removing the module and dropping its data, on a call that means "keep running".
A share could not change at all, where every other budget changes by reload. A stopped bridge was
terminal, which made reload useless for the modules a deployment calls essential. And table ids
were retired on Bridge-ASM700's decision to delete a component rather than on flecs freeing the
table, which missed two ways a table is freed. "Forced by the documents" was the same mistake four
times: a choice with a justification bolted on, which is what `STAGE0.md` step 4 warns of.

The thirteenth came out of reading for one of those. `bridge_destroy` had been discarding modules
without calling their `shutdown`, against what `wasm.md` says of that export; it calls every one
now, and answers a status, because a pinned module's is code that only its own thread may run.

## What it produced

    include/bridge.h              bridge_unload, bridge_reload, bridge_modules; hot_reload,
                                  max_system_failures, two more timeouts, config and settings
    include/bridge-abi.h          bridge_text
    include/bridge-backend.h      a sixth capability: two instances of one module at once
    src/lifecycle.c               load, unload and reload, and what becomes of components
    src/config.c                  one row of settings, written by what each field is
    src/schedule.c                the budget, counted at each barrier, and the stop
    src/store.c                   table ids retired and reused; a ring drained at shutdown
    src/bridge.c                  bridge_modules
    tests/stage6.c                a host with both backends
    tests/modules/                ten modules, most built several ways

`src/bridge.c` lost what `lifecycle.c` gained: a load is now the same steps a reload takes, with
another first call.

## The tests

Fifty-seven new ones, one hundred and twenty-eight in all, in a release build and a debug one. Most
are cases of `tests/stage6.c`; two are among stage 4's, one among stage 2's, and one reads the
library from inside.

    skipinit         a failed init, skipped: the other module runs on and is the one listed
    refusedload      a refused load leaves nothing: its component's name takes another layout
    budget           failing every pass: stopped at the third barrier, STOPPED in the pull,
                     and the other module runs on
    forgiven         failing one pass in three: never stopped
    notforgiven      failing two in three: stopped
    neverstop        -1: fails every pass, never stopped
    failstop         a fail module stopped: the bridge stops, a phase and an unload are
                     refused, and the pull says which module
    strip            unload: 9 rows of the shared component left, the 5 emptied entities alive
    sweep            the same, and the 5 emptied entities destroyed
    delete           every entity that carried it destroyed: 0 rows, 0 alive
    nested-unload    a type outlives its declarer while another module's field names it, and
                     goes when that module does
    nounload         an identity that is not loaded: refused by name
    destroy          a bridge destroyed with two modules loaded: one's shutdown is heard, the
                     other's never returns and is ended by its timeout, and the bridge goes
    timeouts         a shutdown that never returns: ended by its timeout, and the unload completes
    slots            max_modules: refused when full, and a record is given back by an unload
    reload           a row's bytes kept: 102, then 112 and 122 from the new code
    reloadlayout     a changed struct body: refused with both fingerprints, the old instance runs
    reloadfail       the new reload answers 5: the old instance runs on
    sharegrow        a larger persistent: granted, and the new code's allocation fits in it
    sharechain       the same with another module's share carved after: a second range
    sharesame        the same persistent: the new code's allocation fails, the old runs on
    shareless        a smaller persistent: refused by name
    sharebig         more than the arena has left: refused by name
    reloadmove       another backend: refused by name
    reloaddrop       a component the new instance no longer declares is gone
    maynot           hot_reload false: refused
    revive           a stopped module reloaded into one that works: RUNNING again
    notreloadable    hot_reload stated, and the author says not reloadable: refused at load
    noreload         a module without one of the three exports: refused by its name
    reloadnothing    nothing of that name loaded: refused
    nreload          native: the old shutdown, then the new library; the row's bytes kept
    nreloadfail      native, the new reload fails: the module is STOPPED with its rows, the
                     other runs on, and a library that works picks the count up at 112
    nreloadfailunload  the same, then unloaded: a module with no code left can still leave
    nreloadfailstop  the same with fail: the bridge stops, and that reload starts it again
    repair           an essential module stopped at its budget: the bridge stops, refuses
                     another module's load, and runs again when the module is reloaded
    repairload       the same when a load was what failed: loading it again lifts the stop
    churn            three modules of enums and nested types loaded and unloaded 200 times:
                     the index never out of step
    tables           from inside: a table flecs swept as empty and the table of a deleted
                     parent's children both lose their ids, and a retired id is reused
    ntimeout         a timeout on the native backend: refused for deadline, before any library
    config           a bool, two integers, a float, an enum by name, text inline and text in the
                     arena, a bitmask, an integer into a double, 64 bits: all read back
    configkey        a key the struct lacks: refused naming it
    configkind       text into a float: refused naming it
    configlong       text longer than its array: refused naming it
    configrange      300 into a byte: refused naming it
    configenum       a name the enum lacks: refused naming it
    confignone       settings handed to a module that stated it has none: refused
    configunset      `config` left out: refused as unset, by name
    configmissing    a config init never declared: refused
    configarena      more text than the share holds: refused
    configceiling    no room under the entity ceiling for the row: refused
    configtwice      one component, two modules' settings: the second refused
    configreload     a reload handing over other settings: the row keeps what it had
    configdrop       a reload that stops declaring the settings struct: the row goes with it
    reseed           unload and load: the settings' text is written where it was
    image            a module whose own memory begins with data loads, on a system where
                     wasmtime would have mapped that data

## What the stage found

**flecs does say when a table is gone, to an observer that asks the right way.** The stage first
retired ids itself, just before deleting a component, and called the plan's `EcsOnTableDelete`
unnecessary. It was the other way round: flecs frees tables that hold no component of ours, and
keeps ones a query still holds, so only flecs knows. An observer on `EcsAny` hears all of them; one
on a wildcard misses a table whose type is only pairs - `flecs.md`, *flecs will not tell you
which*. The first stop condition did not happen: every policy leaves the mirror agreeing with
flecs.

**Two wasm instances of one module coexist over the shared block.** The second stop condition did
not happen either, so a reload on that backend keeps the old instance until the new one has
succeeded.

**A load and a reload are one path.** The same checks, image, instance, private memory and export
resolution, with `init` or `reload` as the first call. What differs is before it, what the new
record inherits, and after it, the swap.

**What a module says in its `shutdown` was being lost.** Its ring is in its private memory, which
goes with it, and the barrier that would have drained it comes after. It is drained where the
`shutdown` returns.

**All three lifecycle exports are required, and the documents had not said so.** `wasm.md` listed
`shutdown` beside `init` without a word, and called `reload` optional. Every test module has all
three now, and what an absent `reload` used to say is a key - `wasm.md`, *Exports*.

**The wasm backend's watchdog could hang, since stage 2.** It waited for a call to start and then
for it to end, one event each, and two timed calls back to back could set an event twice before it
was waited for once. The thread then waited for an end already said, and destroying the backend
waited for the thread. Nothing before this stage made two timed calls in a row; a reload is exactly
that, and one test run in some hundreds stuck. It reads what is running under a lock now, and the
event only says to look - `wasmtime.md`, *Deadlines*. Twenty thousand timed calls back to back are
part of `tests/adopt.c`, and every test has a timeout, so one that hangs fails.

**On Linux the wasm backend could abort the process, since stage 2.** wasmtime initialises a
memory from a copy-on-write image there, and asserts the memory is one it mapped; every memory of
ours comes from the host memory creator. Whether a module tripped it depended on the shape of its
data segments, and for five stages none did. The Linux job caught the first that does. That
initialisation is off now - `wasmtime.md`, *Memory* - and a module built to trip it is part of the
suite.

**A failed native reload costs no data.** With no old instance to keep, the module stays, stopped,
with what it declared, until an artifact that works is reloaded over it or it is unloaded -
`lifecycle.md`, *The runtime lifecycle*.

## Nothing is left behind it

No shortcut is marked in the code. Every refusal and every path the stage added has a case above.
The `linux` preset ran it.

## Open questions

None for the next stage.
