# The element model

An **element** is a module or a plugin. There is one model for both, and BridgeC has no opinion
about which you use for what - choosing is the deployment's, because the trade is about guarantees
rather than about where code belongs.

This file is everything they share. `module-abi.md` and `plugin-abi.md` describe only how each kind
binds to it.

## Identity

`namespace:name@version` plus the **ABI version**, and **four things use it**: it is the config key,
the grant key, the namespace components are declared in, and how `[order]` names a system. An
element without one is refused.

BridgeC is handed it as data, so **a host that cannot supply an identity cannot load an element**.
The convention is a named section in the artifact - a wasm custom section for one kind, an
object-file section for the other - and a helper beside the library reads it. A host sourcing it
elsewhere is not doing anything wrong. Where the bytes sit and how they are read is the only part
that differs: `module-abi.md` and `plugin-abi.md`, *Identity*.

## The ABI

At init, and again whenever an element is loaded while the program runs - `host-api.md`, *The
runtime lifecycle*. All three may take names, because registration is not a hot path.

    declare(name, name_len, size, align, desc, desc_len)        -> slot
    system (name, name_len, phase, phase_len, terms, term_count) -> slot
    observe(name, name_len, events, terms, term_count)           -> slot

Any time, queued and applied at the next barrier:

    spawn()                        -> id
    destroy(id)
    attach(id, slot, ptr, len)
    detach(id, slot)
    alloc(len)                     -> offset into the persistent arena

`desc` is the descriptor string `ECS_STRUCT` already emits beside the typedef - `schema.md`, *There
is no schema format*, for what BridgeC does with it and what it does not.

**`attach` carries the initial value.** `ptr` and `len` point at the element's own memory - an
offset for a module, a pointer for a plugin - and BridgeC copies the bytes into the row when it
applies the change, bounds-checking them the way it checks a log record's text. Null means zeroed.

That is there because **nothing applies immediately.** Every structural change is queued, so a row
exists at the next barrier and not before, and the only thing an element could have wanted sooner
was to fill a row it just made. Carrying the value does that without reaching the row at all.

`spawn` still answers a usable id at once - the id is BridgeC's to mint - so a parent can be spawned
and referenced by its children in the same system.

**There are no immediate forms.** They could not deliver acting at once: a span is a snapshot from
the last barrier, so a row made immediately would be unreachable anyway. Worse, flecs swap-removes -
`src/storage/table.c:1465`, *"move last element to deleted element"* - so an immediate destroy would
move **another** entity's row, silently invalidating offsets the element already held for rows it
never touched, with no check that could catch it.

**And nothing else. There is no way for one element to call another.**

That is the whole of the model rather than a restriction inside it. An element that needs something
done writes data describing it, and whichever element subscribes to that shape does it - which is
what a system and an observer are. Nothing asks; everything subscribes.

So reading and writing components is not on this list because it is not a call, and *calling* is not
on it because there is nothing left for a call to do.

**No `access` on `declare`.** Access mode is how a *system* uses a component, not a property of the
type, so it rides on the query term - `data.md`, *Access mode*. Two elements declaring
`game:Position` must agree on its layout and need not agree on how they touch it.

**No `find`.** `declare` is idempotent - the first caller creates a component and later callers get
the same slot - and both sides already have `sizeof` from the schema header, so a second call has
nothing to do.

**`observe` is the other half of the same thing**: a system subscribes to a shape, an observer
subscribes to a change - `data.md`, *Observers*. `events` is a mask of `ON_ADD`, `ON_SET` and
`ON_REMOVE`, and an observer takes no phase because it has one already: the barrier that applied its
trigger.

**Why it is an entry of its own, and not a system people write.** A system whose query matches an
event component does the same job by polling, and that is the alternative it has to beat: a poll
costs a dispatch and a zero-span walk **every pass, per subscriber, whether or not anything
happened**. Ten subscribers to a rare event cost ten calls a pass forever. An observer costs nothing
when nothing changed - a saving on the hot path proportional to how many elements are listening, and
the hot path is the one budget this design exists to protect.

Either kind **is** a query plus a function, so one call registers the pair. `init`, `shutdown` and
`reload` are reserved names that `system` and `observe` refuse. `terms` is an array of term structs,
flecs' `ecs_term_t` shape rather than a string, so there is no parser in BridgeC and no string
handed over at init. What a query may not ask for is `data.md`, *What a query may not ask for*.

A system declares its phase and **nothing about order**, and never names another element's system.
So no independently shipped element depends on another's internals, and no ordering cycle can span
authors who have never met.

**A system takes no arguments.** Its spans are already in its memory, found through the slot
`system` answered. Frame time is not an argument and not a component BridgeC declares: the system
clock is hardware, so a plugin reads it - usually the one already talking to the OS for input or a
window - and writes the interval into a component of its own, which anything needing it reads under
a grant.

Work with no components to iterate is still a system: one with an empty query, which runs once per
pass of its phase.

**There is no `tick` and no `build`.** Everything per-frame is a system, and a system carries a
phase, so "every frame" and "every frame, alone" are both just phases.

## Whose component it is

A component name is namespaced - `mymod:Explosion`. **An element may always declare a component in
its own namespace, and reaching another namespace's needs a grant from the deployment.** Refused at
init, by name: one check per declaration and nothing per frame.

    [element."mods:bigger-explosions"]
    write = ["game:Transform"]
    read  = ["game:Health"]

This is not a defence against mods. A mod writing `game:PlayerHealth` may be exactly what its author
intends and what the game means to allow. It is the game deciding **what its mod API is**, instead
of that defaulting to every component in the deployment - which is what global identity gives you
without it. A mod's own components need no config at all; touching the game needs one line, at the
point where somebody should be asked.

It composes with `bridge.md`, *Order within a phase is never implicit*: grant two elements write on
`game:Transform` and their systems conflict, so BridgeC refuses to start until the deployment orders
them.

## What no element may do

Four rules, and they hold for both kinds:

    write a component it declared read
    hold a reference into storage across a barrier - a row number, a descriptor
      pointer, whichever word applies
    hold a scratch-arena offset across a phase - including one it logged
    rely on iteration order

**Two of them are about not keeping anything**, and both follow from the barrier - `bridge.md`,
*Phases*. Rows move there, so a reference outlives its meaning the moment one passes. The fourth is
different: iteration order is archetype order, which changes as entities come and go, so there was
never an order to keep.

**None of the four is enforced**, for either kind. Access mode is a scheduling input and nothing
else - `data.md`, *Access mode* - and the other three are about not keeping something, which no
check could see. An element that breaks one gets wrong answers rather than a diagnostic, and what
finds it afterwards is `diagnostics.md`, *What a bad write costs, and what finds it*.

Nothing on this list is a wall. The one refusal that is lives in `module-abi.md`, *What a module
cannot do*, and it is not a shared rule: it is the sandbox guarantee below, stated as a rule. Each
kind adds exactly one refusal of its own, in its own ABI file.

## Three guarantees differ

    sandbox          a module cannot reach memory it was not given. a plugin is native
                     code in BridgeC's process, and nothing contains it
    metering         fuel, timeouts and memory limits apply to a module - `bridge.md`,
                     *What bounds a run*. native code is not meterable
    clean reload     a module's reload is a fresh instance over surviving storage. a
                     plugin can be unloaded and reloaded, but static state and held
                     pointers make it fragile rather than routine

Those are **guarantees BridgeC can make about a module and cannot make about a plugin.** Nothing in
the list is a capability one has and the other lacks.

## So what decides

A plugin exists when something cannot be reached from inside wasm - `plugin-abi.md` says what that
covers. A module exists when the three guarantees above are worth having.

Pure computation can be either. A physics solver as a module pays the wasm execution penalty and
gets bounds checks, fuel, a trap instead of a crash and a clean reload; as a plugin it runs native
and gets none of that. **That is a trade, not a rule**, and the deployment makes it.

## Three mechanics differ

A different three from the guarantees above. A guarantee is something BridgeC can promise about one
kind and not the other; these are two surfaces onto one set of operations. **No capability is in
either list.**

    storage      a module reads spans published into its private memory; a plugin is
                 handed a pointer and a descriptor table
    registering  a module's systems and observers are wasm exports BridgeC resolves by
                 name; a plugin passes a function pointer
    lifecycle    three wasm exports, or three exported symbols

The scheduler cannot tell the two apart once either is registered, which is the property worth
having.
