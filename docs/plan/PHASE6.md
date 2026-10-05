# Phase 6: failure and lifecycle

Done, on Windows and on Linux.

A module could load, run, and fail one invocation at a time. Now it can be stopped, unloaded,
reloaded and configured, and a host can ask what is running. `PLAN.md` phase 6 is the claim. This
document is what the phase built and what it found on the way.

## What was settled first

Five questions, each by every step of `PHASE0.md`, and each applied to the document it belongs to.

    what a reload runs            the new instance's reload, then the old one's shutdown:
                                  `lifecycle.md`, *The runtime lifecycle*
    what the budget counts        raised by a failed barrier, lowered by a clean one:
                                  `failure.md`, *The budget*
    fail, on a stop               it covers one, as it covers a load:
                                  `failure.md`, *When something fails*
    which component is settings   the one the author names:
                                  `config.md`, *A module's own settings are a component*
    where a text setting goes     where its field's type says: the same section

Three of them were first put with steps missing, and were put again. Reading the original Bridge's
source for the second pass gave reload a fourth option and changed the recommendation for the
budget.

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
    tests/phase6.c                a host with both backends
    tests/modules/                ten modules, most built several ways

`src/bridge.c` lost what `lifecycle.c` gained: a load is now the same steps a reload takes, with
another first call.

## The tests

Forty-two new ones, one hundred and thirteen in all, in a release build and a debug one.

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
    timeouts         a shutdown that never returns: ended by its timeout, and the unload completes
    slots            max_modules: refused when full, and a record is given back by an unload
    reload           a row's bytes kept: 102, then 112 and 122 from the new code
    reloadlayout     a changed struct body: refused with both fingerprints, the old instance runs
    reloadfail       the new reload answers 5: the old instance runs on
    reloadshare      another persistent: refused by name
    reloadmove       another backend: refused by name
    reloaddrop       a component the new instance no longer declares is gone
    maynot           hot_reload false: refused
    revive           a stopped module reloaded into one that works: RUNNING again
    noreload         hot_reload stated, no reload export: refused at load
    reloadnothing    nothing of that name loaded: refused
    nreload          native: the old shutdown, then the new library; the row's bytes kept
    nreloadfail      native, the new reload fails: the module is gone, the other runs on
    nreloadfailstop  the same with fail: the bridge stops
    ntimeout         a timeout on the native backend: refused for deadline, before any library
    config           a bool, two integers, a float, an enum by name, text inline and text in the
                     arena, a bitmask, an integer into a double, 64 bits: all read back
    configkey        a key the struct lacks: refused naming it
    configkind       text into a float: refused naming it
    configlong       text longer than its array: refused naming it
    configrange      300 into a byte: refused naming it
    configenum       a name the enum lacks: refused naming it
    confignone       settings and no config named: refused
    configmissing    a config init never declared: refused
    configarena      more text than the share holds: refused
    configceiling    no room under the entity ceiling for the row: refused
    configtwice      one component, two modules' settings: the second refused
    configreload     a reload handing over other settings: the row keeps what it had
    configdrop       a reload that stops declaring the settings struct: the row goes with it
    reseed           unload and load: the settings' text is written where it was

## What the phase found

**Unload did not need flecs to say a table was gone.** The plan had table ids retired from
`EcsOnTableDelete`. The only thing here that deletes a table is deleting a component, which
Bridge-ASM700 does itself, so the ids are retired just before - `flecs.md`, *flecs will not tell
you which*. The first stop condition did not happen: every policy leaves the mirror agreeing with
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

**`shutdown` is required, and the documents had not said so.** `wasm.md` listed it beside `init` and
marked only `reload` optional. Every test module gained one.

**The wasm backend's watchdog could hang, since phase 2.** It waited for a call to start and then
for it to end, one event each, and two timed calls back to back could set an event twice before it
was waited for once. The thread then waited for an end already said, and destroying the backend
waited for the thread. Nothing before this phase made two timed calls in a row; a reload is exactly
that, and one test run in some hundreds stuck. It reads what is running under a lock now, and the
event only says to look - `wasmtime.md`, *Deadlines*. Twenty thousand timed calls back to back are
part of `tests/adopt.c`, and every test has a timeout, so one that hangs fails.

**A failed native reload costs data, and that had to be said.** With no old instance to keep, the
module is out as an unload leaves it, exclusive components dropped. It follows from the reason
unload drops them, and it is written beside the reload order - `lifecycle.md`, *The runtime
lifecycle* - because it is the one place a call that means "keep running" loses something.

## Nothing is left behind it

No shortcut is marked in the code. Every refusal and every path the phase added has a case above.
The `linux` preset ran it.

## Open questions

None for the next phase.
