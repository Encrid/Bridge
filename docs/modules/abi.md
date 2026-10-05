# The ABI

At init, which runs each time a module is loaded - `lifecycle.md`, *The runtime lifecycle*. All
three may take names, because registration is not a hot path.

    declare(name, name_len, kind, size, align, desc, desc_len, on_unload) -> slot
    system (name, name_len, phase, phase_len, terms, term_count)    -> slot
    observe(name, name_len, events, terms, term_count)              -> slot

Any time, queued and applied at the next barrier:

    spawn()                        -> id
    destroy(id)
    attach(id, slot, ptr, len)
    detach(id, slot)
    alloc(len)                     -> offset into the persistent arena

`desc` is the descriptor string `BRIDGE_SCHEMA_STRUCT` already emits beside the typedef -
`schema.md`, *There is no schema format*, for what Bridge-ASM700 does with it and what it does not. `kind` says what it
describes: a struct, an enum or a bitmask - `schema.md`, *The type set*. It is stated, like every
other value.

`on_unload` is what becomes of the entities carrying the component when this module is unloaded:
`strip`, `sweep` or `delete` - `lifetime.md`, *What unload leaves behind*. It is stated, like every
other value: there is no default.

**Every pointer here is an offset into the module's scratch arena**, on every backend: `name`,
`desc`, `phase`, `terms`, and `attach`'s `ptr`. A module writes the bytes there first and passes
where they are. An offset means nothing without a memory, so the memory is fixed by the binding
rather than carried in the value - the rule `log.md`, *The log*, already set for a record's text,
and for the same reason: the scratch arena is the one memory a module can write that Bridge-ASM700
can read and bounds-check. So one signature serves both backends, and every argument is 32 or 64
bits wide - `signatures.md`, *Imports are declared, exports are checked*.

Refused: a pointer type the backend translates, which would have the backend reading a module's own
memory and pairing each pointer with its length. The condition that brings it back is a value too
large for a scratch arena.

**`attach` carries the initial value.** `ptr` and `len` say where it is, and Bridge-ASM700 takes a
copy of the bytes at the call, bounds-checking them the way it checks a log record's text. Zero
means zeroed. The copy is what reaches the row at the barrier, so the scratch arena may be reset in
between - which it is, when the call was an observer's.

That is there because **nothing applies immediately.** Every structural change is queued, so a row
exists at the next barrier and not before, and the only thing a module could have wanted sooner
was to fill a row it just made. Carrying the value does that without reaching the row at all.

`spawn` still answers a usable id at once - one of those set aside for the module at the last
barrier, `threads.md`, *Threads* - so a parent can be spawned
and referenced by its children in the same system.

**There are no immediate forms.** They could not deliver acting at once: a span is a snapshot from
the last barrier, so a row made immediately would be unreachable anyway. Worse, a row that leaves
is filled by its table's last - `storage.md`, *A table* - so an immediate destroy would
move **another** entity's row, silently invalidating offsets the module already held for rows it
never touched, with no check that could catch it.

**And nothing else. There is no way for one module to call another.**

That is the whole of the model rather than a restriction inside it. A module that needs something
done writes data describing it, and whichever module subscribes to that shape does it - which is
what a system and an observer are. Nothing asks; everything subscribes.

So reading and writing components is not on this list because it is not a call, and *calling* is not
on it because there is nothing left for a call to do.

**No `access` on `declare`.** Access mode is how a *system* uses a component, not a property of the
type, so it rides on the query term - `data.md`, *Access mode*. Two modules declaring
`game:Position` must agree on its layout and need not agree on how they touch it.

**No `find`.** `declare` is idempotent - the first caller creates a component and later callers get
the same slot - and both sides already have `sizeof` from the schema header, so a second call has
nothing to do.

**`observe` is the other half of the same thing**: a system subscribes to a shape, an observer
subscribes to a change - `data.md`, *Observers*. `events` is a mask of `ON_ADD`, `ON_SET` and
`ON_REMOVE`, and an observer takes no phase because it has one already: the barrier that applied its
trigger.

**An observer reads which entities, not which rows.** Through the slot `observe` answered it finds
a list in its private memory, one record per entity that triggered it and the events that did:

    u64 id
    u32 events    ON_ADD | ON_SET | ON_REMOVE

It is called once per barrier, and only when the list is not empty. So an observer has at least
one term: with none there is no change to subscribe to, and `observe` refuses it. It follows an id
the way any reference is followed - `reference.md`, *Reference following* - and an id that answers
nothing was removed or destroyed, which is how `ON_REMOVE` is read.

Ids and not a span, because an observer runs after the barrier has restructured and republished -
`schedule.md`, *An observer runs at the barrier, one hop per barrier*. What an observer is owed is
worked out before rows are placed - `storage.md`, *A barrier is worked an entity at a time* - so a
row would be one the entity is no longer in. An id does not move. Refused: the observer's whole
query as spans, which is what a system gets and cannot say which entity changed - an event would be
found by scanning every event. The condition that brings it back is an observer that wants the shape
and not the change, and such a thing is a system.

**Why it is an entry of its own, and not a system people write.** A system whose query matches an
event component does the same job by polling, and that is the alternative it has to beat: a poll
costs a dispatch and a zero-span walk **every pass, per subscriber, whether or not anything
happened**. Ten subscribers to a rare event cost ten calls a pass forever. An observer costs nothing
when nothing changed - a saving on the hot path proportional to how many modules are listening, and
the hot path is the one budget this design exists to protect.

A system or an observer **is** a query plus a function, so one call registers the pair. `init`,
`shutdown` and `reload` are reserved names that `system` and `observe` refuse. `terms` is an array
of term structs rather than a string, so there is no parser in Bridge-ASM700 and no string handed
over at init. A term is three things: the
slot `declare` answered, the access, and whether the term must match, must not, or may. What a
query may not ask for is `query.md`, *What a query may not ask for*.

A system declares its phase and **nothing about order**, and never names another module's system.
So no independently shipped module depends on another's internals, and no ordering cycle can span
authors who have never met. Among a module's own systems the order it registered them in is the
order: `schedule.md`, *Order within a phase is never implicit*.

**A system takes no arguments.** Its spans are already in its memory, found through the slot
`system` answered. Frame time is not an argument and not a component Bridge-ASM700 declares: the system
clock is hardware, so a native module reads it - usually the one already talking to the OS for input or a
window - and writes the interval into a component of its own, which anything needing it reads under
a grant.

Work with no components to iterate is still a system: one with an empty query, which runs once per
pass of its phase.

**There is no `tick` and no `build`.** Everything per-frame is a system, and a system carries a
phase, so the original Bridge's two per-frame entry points are both just systems in a phase.
