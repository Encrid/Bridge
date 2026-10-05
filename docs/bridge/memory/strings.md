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

**Each module allocates from a share of it that is its own**, sized by its `persistent` -
`budgets.md`, *Budgets* - and made of more than one range if a reload enlarged it. Not for
isolation: the bytes are in the shared memory and any module can read them, which is the point.
For order: two modules allocating in one phase would otherwise be handed offsets in whichever order
their threads arrived - `threads.md`, *Threads*.

## Which of the three a string belongs in

Most strings belong in none of this:

    char name[32] in the component   bounded length. it lives in the column, and
                                     the ECS frees it when the row goes
    the scratch arena                lifetime shorter than a phase. reset at
                                     every barrier, with nothing to ask for
    the persistent arena             unbounded length AND the deployment's
                                     lifetime - a mod name, a path read at init

An **inline array** is in the type set already - `schema.md`, *The type set* - so
`BRIDGE_SCHEMA_STRUCT(Named, { char name[32]; })` is an ordinary component with nothing allocated anywhere.
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
the arena and fails by name: nothing gives that space back while the module is loaded.

**Bridge-ASM700 reuses its own range.** A module's `[.config]` strings are allocated by
Bridge-ASM700 at init, from that module's own share, and `init` runs again on every load - so a day
of unload-and-load would leak one copy per cycle wherever the module's share has to stay - *It
is given back when nothing points into it*. So where a share stays, Bridge-ASM700 keeps the
module's config-string range in it, and the next load of the module writes there again when the
text fits, and takes from its new share only when it does not.

## It is given back when nothing points into it

**While a module is loaded, nothing frees what it allocated.** There is no `mark` and no `reset`,
and the ABI stays at eight. Every allocator we looked at has them - `zone.c:444`,
`common.c:1576`, flecs' `flecs_stack_restore_cursor`, Source's `FreeToAllocPoint` - and in all
four **the region being reset has a single owner**, who knows nobody else holds a pointer into
it. A module does not know that: a string lives in a component another module may read under a
grant, and a reset would invalidate offsets held by modules that never asked - the reason there
are no immediate forms either: `abi.md`, *The ABI*.

**At unload its share goes back to the arena, unless a row that is staying holds a string in
it.** Bridge-ASM700 can know what the module could not. A string a component keeps is a
`bridge_text` field - `schema.md`, *The type set* - so every reference there is can be found:
after the module's own components are dropped, every `bridge_text` of every row left is read,
in a field, in an array of them, or in a struct a field holds. If none lies in the leaving
module's share, the share is free for the next load. If one does, the share stays whole, and so
does the string, readable like the float beside it.

So no offset anybody holds in a component is ever made stale, and that is the one rule a reader
has to know: **what a component keeps stays good for as long as the component's row does.** An
offset kept anywhere else - in a module's own memory, or in a field that is two integers and not
a `bridge_text` - is good only for as long as the module that allocated it is loaded.

**What it costs.** An unload reads every text field of every surviving row, once. Never at a
barrier. And a share with one surviving string in it stays dead whole.

**What it buys.** Loading and unloading modules can go on for as long as a deployment runs:
`tests/phase6.c`, case `churn`, does two hundred cycles in an arena with room for one share. It
could not, once: the arena was a high-water mark for the life of the process, and unbounded churn
meant restarting. That was decided before a module's share was its own and before text was a type
the library could see, and it was not read again when both became true.

Refused: never giving anything back, which was the rule. The condition that brings it back is the
reading at unload proving too slow for a deployment that unloads often. Refused: `mark` and
`reset` in the ABI. The condition is a module that must free strings in mid-life and can show
nobody else holds them. Refused: giving back each string a row no longer holds, and not only whole
shares, which is a general allocator inside the arena. The condition is shares kept for one
surviving string being seen to pile up.

The discipline while a module runs is still idTech3's: allocation for a lifetime, with no
per-object bookkeeping. The lifetime is the module's, where theirs was a level's.
