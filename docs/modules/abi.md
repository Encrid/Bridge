# The ABI

What was weighed here and refused is `abi_refused.md`, *What was refused: The ABI*.

At init, which runs each time a module is loaded - `lifecycle.md`, *The runtime lifecycle*. All
three may take names, because registration is not a hot path. **Each takes one struct, by where it
is in the scratch arena**, and `bridge_abi.h` states the three layouts with their sizes and offsets:

    declare(at)        -> slot      name, name_len, kind, size, align, desc, desc_len,
                                    on_unload, storage
    add_system(at)     -> slot      name, name_len, phase, phase_len, terms, term_count,
                                    changes, change_count
    observe(at)        -> slot      name, name_len, events, terms, term_count, changes,
                                    change_count

No entry takes more than four arguments - `style.md`, rule F5 - and a field is named where it is
written. A field left at zero is unset, and `declare` refuses that for `kind`, `on_unload` and
`storage`.

Any time, queued and applied at the next barrier:

    spawn()                        -> id
    destroy(id)
    attach(id, slot, ptr)
    detach(id, slot)
    alloc(len)                     -> offset into the persistent arena

`desc` is the descriptor string `BRIDGE_SCHEMA_STRUCT_DESC` makes of the typedef's own text -
`schema.md`, *There is no schema format*, for what Bridge-ASM700 does with it and what it does not.
`kind` says what it describes: a struct, an enum or a bitmask - `schema.md`, *The type set*. It is
stated, like every other value. A fourth kind is a tag, a component with no bytes, which has no
descriptor: `storage.md`, *A tag*.

`on_unload` is what becomes of the entities carrying the component when this module is unloaded:
`strip`, `sweep` or `delete` - `lifetime.md`, *What unload leaves behind*. It is stated, like every
other value: there is no default.

`storage` is the storage layout the component is kept in: a table, a sparse set, a toggle,
direct-indexed or a singleton - `storage.md`, *Storage layouts*. It is stated, and every module
that declares the component states the same one.

**Every pointer here is an offset into the module's scratch arena**, on every backend: `name`,
`desc`, `phase`, `terms`, `changes`, and `attach`'s `ptr`. A module writes the bytes there first and passes
where they are. An offset means nothing without a memory, so the memory is fixed by the SDK
rather than carried in the value - the rule `log.md`, *The log*, already set for a record's text,
and for the same reason: the scratch arena is the one memory a module can write that Bridge-ASM700
can read and bounds-check. So one signature serves both backends, and every argument is 32 or 64
bits wide - `signatures.md`, *Imports are declared, exports are checked*.

**What an entry was handed is read during the call, and the scratch it was in is then free.**
The scratch arena is reset at a barrier, so a value left there would stay for the rest of the
phase, and a module's `scratch` would have to hold every value it attaches in one, beside its
`changes`, which hold the same bytes: two sizes for one thing. So an SDK's attach copies the value
above what the arena has in use, without counting it as used, and makes the call. A module's
scratch then holds one value at a time. Each backend's SDK does it, in `bridge_lend` and
`bridge_native_lend`, which `BRIDGE_ATTACH` and `BRIDGE_NATIVE_ATTACH` use - and a log line's
text is the one thing put in scratch that stays until the barrier, since it is read there:
`log.md`, *The log*. `tests/host`, case
`bridge_lend-scratch-given-back`, attaches eight kilobytes of values through 512 bytes of scratch.

What the second copy costs was measured on the native backend, where a call pays for nothing
else: an attach with a value of four bytes put through scratch took 19.9 ns against 19.6 to 20.0 without.

**`attach` carries the initial value.** `ptr` says where it is, and it is as long as the type was
declared: Bridge-ASM700 takes a copy of that many bytes at the call, bounds-checking them the way
it checks a log record's text. `BRIDGE_ZEROED` as `ptr` asks for a zeroed value. The copy is what
reaches the row at the barrier, so the scratch arena may be reset in between - which it is, when
the call was an observer's. Since the copy is taken during the call, the SDKs put the value above
what the arena has in use and do not count it as used.

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
`game:Position` must agree on its component layout and need not agree on how they touch it.

**No `find`.** `declare` is idempotent - the first module to declare a component creates it, and a
later one is answered a slot of its own for the same component - and both sides already have
`sizeof` from the schema header, so a second call has nothing to do.

**`observe` is the other half of the same thing**: a system subscribes to a shape, an observer
subscribes to a change - `data.md`, *Observers*. `events` is a mask of `ON_ENTER`, `ON_ATTACH` and
`ON_LEAVE`, and an observer takes no phase because it has one already: the barrier that applied its
trigger.

**An observer reads which entities, not which rows.** Through the slot `observe` answered it finds
a list in its private memory, one record per entity that triggered it and the events that did:

    u64 id
    u32 events    ON_ENTER | ON_ATTACH | ON_LEAVE
    u32 -         padding: zero, and not read

It is called once per barrier, and only when the list is not empty. So an observer has at least
one term: with none there is no change to subscribe to, and `observe` refuses it. It follows an id
the way any reference is followed - `reference.md`, *Reference following* - and an id that answers
nothing was removed or destroyed, which is how `ON_LEAVE` of a destroyed entity is read.

Ids and not a span, because an observer runs after the barrier has restructured and republished -
`schedule.md`, *An observer runs at the barrier, one hop per barrier*. What an observer is owed is
worked out before rows are placed - `storage.md`, *A barrier is worked an entity at a time* - so a
row would be one the entity is no longer in. An id does not move.

**Why it is an entry of its own, and not a system people write.** A system whose query matches a
signal's component does the same job by polling, and that is the alternative it has to beat: a poll
costs a dispatch and a zero-span walk **every pass, per subscriber, whether or not anything
happened**. Ten subscribers to a rare event cost ten calls a pass forever. An observer costs nothing
when nothing changed - a saving on the hot path proportional to how many modules are listening, and
the hot path is the one budget this design exists to protect.

A system or an observer **is** a query plus a function, so one call registers the pair. `init`,
`shutdown` and `reload` are reserved names that `add_system` and `observe` refuse. `terms` is an array
of term structs rather than a string, so there is no parser in Bridge-ASM700 and no string handed
over at init. A term is three things: the
slot `declare` answered, the access, and whether the term must match, must not, or may. What a
query may not ask for is `query.md`, *What a query may not ask for*.

**What a statement must hold, and when it may be made.** `declare`, `add_system` and `observe`
are each refused outside `init`. A system or an observer is refused if its name is one the module
registered already, if a change it states names a slot the module never declared, or if it
states more changes than the module declared types. And an observer whose `events` are none, or
name one there is not, is refused.

**`changes` is what the system attaches or detaches**: the slots of those components, each a
`u32`, and how many. It is a second list and not a kind of term, because a term is one component
a query asks for, and a system attaches a component to entities that do not carry it. A system
that attaches or detaches nothing states that: no changes. An `attach` or a `detach` of what the
running system did not state is refused, and that invocation fails. `destroy` is stated nowhere:
it wins whatever ran before or after it, and what it may be asked of is `grants.md`, *What a
module may destroy*. `init`, `shutdown` and `reload` state nothing either: a
lifecycle call runs alone. What the list is for is `schedule.md`, *Order within a phase is never
implicit*.

A system declares its phase and **nothing about order**, and never names another module's system.
So no independently shipped module depends on another's internals, and no ordering cycle can span
authors who have never met. Among a module's own systems the order it registered them in is the
order: `schedule.md`, *Order within a phase is never implicit*.

**A system takes no arguments.** Its spans are already in its memory, found through the slot
`add_system` answered. Frame time is not an argument and not a component Bridge-ASM700 declares: the system
clock is hardware, so a native module reads it - usually the one already talking to the OS for input or a
window - and writes the interval into a component of its own, which anything needing it reads under
a grant.

Work with no components to iterate is still a system: one with an empty query, which runs once per
pass of its phase.

**There is no `tick` and no `build`.** Everything per-frame is a system, and a system carries a
phase, so the original Bridge's two per-frame entry points are both just systems in a phase.
