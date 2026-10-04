# The ABI

At init, and again whenever an element is loaded while the program runs - `lifecycle.md`, *The
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
is no schema format*, for what Bridge-ASM700 does with it and what it does not.

**`attach` carries the initial value.** `ptr` and `len` point at the element's own memory - an
offset on the wasm backend, a pointer on the native one - and Bridge-ASM700 copies the bytes into
the row when it applies the change, bounds-checking them the way it checks a log record's text. Null
means zeroed.

That is there because **nothing applies immediately.** Every structural change is queued, so a row
exists at the next barrier and not before, and the only thing an element could have wanted sooner
was to fill a row it just made. Carrying the value does that without reaching the row at all.

`spawn` still answers a usable id at once - the id is Bridge-ASM700's to mint - so a parent can be spawned
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

A system or an observer **is** a query plus a function, so one call registers the pair. `init`,
`shutdown` and `reload` are reserved names that `system` and `observe` refuse. `terms` is an array
of term structs, flecs' `ecs_term_t` shape rather than a string, so there is no parser in Bridge-ASM700
and no string handed over at init. What a query may not ask for is `query.md`, *What a query may not
ask for*.

A system declares its phase and **nothing about order**, and never names another element's system.
So no independently shipped element depends on another's internals, and no ordering cycle can span
authors who have never met.

**A system takes no arguments.** Its spans are already in its memory, found through the slot
`system` answered. Frame time is not an argument and not a component Bridge-ASM700 declares: the system
clock is hardware, so a native element reads it - usually the one already talking to the OS for input or a
window - and writes the interval into a component of its own, which anything needing it reads under
a grant.

Work with no components to iterate is still a system: one with an empty query, which runs once per
pass of its phase.

**There is no `tick` and no `build`.** Everything per-frame is a system, and a system carries a
phase, so "every frame" and "every frame, alone" are both just phases.
