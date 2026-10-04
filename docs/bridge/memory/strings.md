# Variable length

A component cannot hold a `string` or a `list`: a column is an array at a fixed stride. So the bytes
live in an arena and the component holds `(offset, length)`, which is fixed size and therefore an
ordinary field.

**Two arenas, because there are two lifetimes** - and they sit in two different memories, because
the lifetimes come with different readerships: `memory.md`, *Where the seven regions live*. Both are **hunks
rather than a hunk and a zone**, since neither has a free; what idTech3 split between those two, we
split by who can see it.

The **scratch arena** is for anything transient: a message, a formatted number, a name assembled to
be logged. Each module gets one in its own private memory at registration and bumps its own pointer, so
allocating costs no crossing, and Bridge-ASM700 resets them all at each barrier.

It is named for what it is rather than for a frame: its lifetime is a phase.

Which brings a rule, the same shape as the one about rows: **a scratch-arena offset is good for one
phase.** It must never be stored in a component.

The **persistent arena** is for strings a component keeps. Allocating from it is a crossing, and
that is right: storing a string in a component is rare, and making it cheap would invite doing it in
a loop.

## Which of the three a string belongs in

Most strings belong in none of this:

    char name[32] in the component   bounded length. it lives in the column, and
                                     the ECS frees it when the row goes
    the scratch arena                lifetime shorter than a phase. reset at
                                     every barrier, with nothing to ask for
    the persistent arena             unbounded length AND the deployment's
                                     lifetime - a mod name, a path read at init

An **inline array** is in the type set already - `schema.md`, *The type set* - so
`ECS_STRUCT(Named, { char name[32]; })` is an ordinary component with nothing allocated anywhere.
That is the answer to "how do I free it": put it where something already does.

A list of things is not a string in a component. It is an entity each, with a fixed field, destroyed
when it is stale - which the ECS frees.

## Changing one, and the cost of growing

    shorter or the same   write in place. the component holds (offset, length):
                          overwrite the bytes and set the new length. no allocation
    longer                alloc a new range, write, update (offset, length). the
                          old range is dead space for the rest of the run

So **replacing a string is free and growing one is not**, which is the asymmetry to design around.
A module that grows a string once a level will never notice. One that does it per frame exhausts
the arena and fails by name, which under *It is never reclaimed* is fatal rather than wasteful.

**Bridge-ASM700 reuses its own range.** A module's `[.config]` strings are allocated by
Bridge-ASM700 at init, and `init` runs again on every load - so a day of unload-and-load would leak
one copy per cycle. That range has exactly one owner, Bridge-ASM700 itself, which is the property
the next section's refusal turns on. So Bridge-ASM700 keeps each module's config-string range and
writes into it again rather than allocating afresh.

## It is never reclaimed

**Nothing frees persistent-arena space while the process runs.** Not a module, not an unload, not
a reload. One rule with no conditions on it: anything conditional here would make cleanup depend on
something an author was not thinking about.

That is what makes a component's fields behave alike. A shared component survives an unload -
`data.md`, *Structural change* - so its `float x` is still readable afterwards. Under any scheme
that released the writer's arena, its string beside it would not be. One component with two
survival rules by field type is a surprise no diagnostic would explain.

**So there is no `mark` and no `reset`, and the ABI stays at eight.** Every allocator we looked at
has them - `zone.c:444`, `common.c:1576`, flecs' `flecs_stack_restore_cursor`, Source's
`FreeToAllocPoint` - and in all four **the region being reset has a single owner**, whether by
geometry, by one instance per subsystem, or by one author sequencing it. Ours has one writer and
many readers by design, since a string lives in a shared component another module reads under a
grant. A reset would invalidate offsets held by modules that never asked - the reason there are no
immediate forms either: `abi.md`, *The ABI*.

**What it costs, and it is a real cost.** The arena is a **process-lifetime high-water budget**, not
a live-set one: a string overwritten and never read again is dead space, and so is everything an
unloaded module allocated. **An unload-and-load cycle pays it for whatever the module allocates in
`init` itself** - config strings are Bridge-ASM700's and are reused, *Changing one, and the cost of
growing*, but a module that builds its own name table at init builds another one every cycle. A
reload does not pay it, since `init` does not run again.

A deployment that churns modules indefinitely exhausts the arena, and `alloc` then fails fatally
and by name, which is every other budget's discipline. Unbounded churn means restarting - which is
what idTech3 did too, a level restart clearing the hunk.

The bargain is idTech3's, taken one step further: level-lifetime allocation with no per-object
bookkeeping, where our level is the process.
