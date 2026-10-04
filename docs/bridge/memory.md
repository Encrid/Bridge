# Memory

Which block each thing lives in, who may read it, and what it costs when one runs out. What the
data *is* is `data.md`.

## Where the seven regions live

Seven things must be addressable with no call, and they do not have the same readers:

    region              memory   who reads it
    component columns      1     every module that declared the component
    entity index           1     every module that follows a reference
    persistent arena       1     every module holding a string in a component
    query spans            2     only the element they belong to
    column_base            2     only the element they belong to
    scratch arena          2     only the element they belong to
    log ring               2     one per element, and BridgeC draining it

Memory 2 is BridgeC's alone, so it is laid out from offset 0 with nothing to negotiate: a fixed
header says where this element's spans, `column_base`, arena and log ring begin, and the log
level it is filtering at - and where the entity index begins in memory 1, which is the one thing it
reads there that it cannot derive. Every other memory-1 offset is relative to memory 1 itself,
including the `(offset, length)` a component holds into the persistent arena, so nothing else needs
a base.

The span region's size is BridgeC's too, and BridgeC resizes it at a barrier, where nothing holds an
offset into it. That is Quake's progs layout - fixed offsets at the bottom, everything else found
through them - and it is why no element has to declare a span capacity it could not compute, since
span count is archetype count and archetype count is a property of the deployment.

What stays exposed is memory 1: the columns, because shared columns are the design, and the index,
because a module must read it with plain loads and wasm has no read-only import.

## Variable length

A component cannot hold a `string` or a `list`: a column is an array at a fixed stride. So the bytes
live in an arena and the component holds `(offset, length)`, which is fixed size and therefore an
ordinary field.

**Two arenas, because there are two lifetimes** - and they sit in two different memories, because
the lifetimes come with different readerships: *Where the seven regions live*. Both are **hunks
rather than a hunk and a zone**, since neither has a free; what idTech3 split between those two, we
split by who can see it.

The **scratch arena** is for anything transient: a message, a formatted number, a name assembled to
be logged. Each element gets one in its own memory 2 at registration and bumps its own pointer, so
allocating costs no crossing whatsoever, and BridgeC resets them all at each barrier.

Named for what it is rather than for a frame, because it has no relation to one: its lifetime is a
phase.

Which brings a rule, and it is the same shape as the one about rows: **a scratch-arena offset is good
for one phase.** It must never be stored in a component.

The **persistent arena** is for strings a component keeps. Allocating from it is a crossing, and that
is right rather than unfortunate: storing a string in a component is rare, and making it cheap would
invite doing it in a loop.

### Which of the three a string belongs in

Most strings belong in none of this, and that is the first thing to know:

    char name[32] in the component   bounded length. it lives in the column, and
                                     the ECS frees it when the row goes
    the scratch arena                lifetime shorter than a phase. reset at
                                     every barrier, with nothing to ask for
    the persistent arena             unbounded length AND the deployment's
                                     lifetime - a mod name, a path read at init

An **inline array** is in the type set already - `schema.md`, *The type set* - so
`ECS_STRUCT(Named, { char name[32]; })` is an ordinary component with nothing allocated anywhere.
That is the answer to "how do I free it": put it where something already does.

And a list of things is not a string in a component. It is an entity each, with a fixed field,
destroyed when it is stale - which the ECS frees.

### Changing one, and the cost of growing

    shorter or the same   write in place. the component holds (offset, length):
                          overwrite the bytes and set the new length. no allocation
    longer                alloc a new range, write, update (offset, length). the
                          old range is dead space for the rest of the run

So **replacing a string is free and growing one is not**, which is the asymmetry to design around.
An element that grows a string once a level will never notice; one that does it per frame exhausts
the arena and fails by name, which under *It is never reclaimed* is the fatal case rather than a
wasteful one.

**BridgeC reuses its own range.** An element's `[.config]` strings are allocated by BridgeC at init,
and `init` runs again on every load - so a day of unload-and-load would leak one copy per cycle.
That range has exactly one owner, BridgeC itself, which is the property the next section's refusal
turns on, so BridgeC keeps each element's config-string range and writes into it again rather than
allocating afresh.

### It is never reclaimed

**Nothing frees persistent-arena space while the process runs.** Not an element, not an unload, not a
reload. One rule with no conditions on it, which is the point: anything conditional here would make
cleanup depend on something an author was not thinking about.

That is what makes a component's fields behave alike. A shared component survives an unload -
*Structural change* - so its `float x` is still readable afterwards, and under any scheme that
released the writer's arena its string beside it would not be. One component with two survival rules
by field type is the kind of surprise no diagnostic would explain.

**So there is no `mark` and no `reset`, and the ABI stays at eight.** Every allocator we looked at
has them - `zone.c:444`, `common.c:1576`, flecs' `flecs_stack_restore_cursor`, Source's
`FreeToAllocPoint` - and in all four **the region being reset has a single owner**, whether by
geometry, by one instance per subsystem, or by one author sequencing it. Ours has one writer and
many readers by design, since a string lives in a shared component another element reads under a
grant. A reset would invalidate offsets held by elements that never asked, which is what killed the
immediate forms in `element.md`, *The ABI*.

**What it costs, and it is a real cost.** The arena is a **process-lifetime high-water budget**, not
a live-set one: a string overwritten and never read again is dead space, and so is everything an
unloaded element allocated. **An unload-and-load cycle pays it for whatever the element allocates in
`init` itself** - config strings are BridgeC's and are reused, *Changing one, and the cost of
growing*, but an element that builds its own name table at init builds another one every cycle. A
reload does not pay it, since `init` does not run again. The person restarting an element all day is
who meets that, so it is written here rather than discovered.

A deployment that churns elements indefinitely exhausts the arena, and `alloc` then fails fatally
and by name, which is every other budget's discipline rather than an exception to it. Unbounded churn
means restarting - which is what idTech3 did too, a level restart clearing the hunk.

The bargain is idTech3's, taken one step further: level-lifetime allocation with no per-object
bookkeeping, where our level is the process.

## Budgets

Two tiers, because two things are being sized.

**Shared, when BridgeC starts, from `bridge_config`:** the entity ceiling behind the index mirror,
and the persistent arena. One block serves everyone, so no single element could size it - and it
arrives as a struct rather than a file, because BridgeC reads none: `host-api.md`, *BridgeC parses
nothing*.

**Per element, when that element registers:** its scratch arena and its log ring - which is to say,
this tier sizes that element's memory 2.

Because elements come and go, `max_elements` bounds **how many element records exist at once**, not
how memory 2 is divided: a record is taken at load and freed at unload, and exhaustion refuses the
load, which is the same discipline as every other budget rather than an exception to it. The shared
tier is untouched - the entity ceiling and the persistent arena do not care how many elements exist.

**Each element's memory 2 is its own**, sized from that element's own `cfg` when it loads - it is not
a share of anybody's block, and it could not be: the isolation in `module-abi.md`, *Memories*, is
wasm's bounds check, which only separates two elements if each has a memory of its own.

So **a reload that wants more just gets it.** Reload builds a fresh instance over surviving storage -
`element.md`, *Three guarantees differ* - and a fresh instance means a fresh memory 2 at whatever
size the new `cfg` asks for. The block that genuinely cannot grow is memory 1, and nothing
per-element lives there.

A ring each rather than one shared matters beyond budgeting - systems run in parallel, so a shared
ring would need atomics on every write, and one per element needs none. BridgeC drains them all at
the barrier.

Every budget is **fixed for as long as the block exists**, and exhaustion is fatal where it happens.
Growing one in place is what is impossible, since that would relocate a block already handed to the
VM - which is why a bigger memory 2 arrives with a new instance and a bigger memory 1 does not
arrive at all.

### BridgeC defaults none of them

A default is a value nobody in the chain accounted for, which `config.md`, *No unaccountable values*,
refuses: **omitted everywhere is an error**, at create for the shared tier and at load for an
element's. A deployment tired of editing fifty manifests puts an `[element.defaults]` table in its
own config - that is layer 2 covering what layer 1 left out, which is what the layers are for, and it
is still a value somebody stated.

idTech3 did ship defaults - `DEF_COMHUNKMEGS "64"`, `DEF_COMZONEMEGS "24"` (`common.c:45`) - and had
to, with one hunk for everything and no manifest to read. What it also had is a **floor**, clamped
and announced rather than refused: `common.c:1517` prints a line and uses `MIN_COMHUNKMEGS` instead.
We refuse there, because clamping is the implicit downgrade `vm-surface.md`, *Nothing is downgraded*,
exists to refuse.

**One floor has an argument and the rest do not.** A `log_ring` must hold at least one record, since
an element that cannot report anything is the one legal number that produces an incoherent
deployment - *The log*, for why. `fuel` and `memory_limit` have floors too, but they are measurements
rather than arguments: `PLAN.md` phase 1 enforces both and is where a number becomes knowable.

## Threads

**An element is the unit of parallelism.** Its systems are serialised, because they share its
instance and an instance cannot be used concurrently, so what runs at once is *different elements*.
A deployment buys parallelism by splitting into more elements, not by declaring more systems - and
that cannot be worked around, since an element has one heap and two instances would be two elements.

Across elements, two systems whose terms conflict never run at once. BridgeC knows which from the
declared terms, so the hot path is lock-free by construction rather than by care.

Those declarations give **parallelism, not order**. That two systems both write `game:Position` says they
cannot run together; it does not say which should go first, and two systems declaring `inout Position`
have no derivable order at all. Order comes from phases - see `bridge.md`, *Phases* - and where
a phase leaves it ambiguous, BridgeC refuses to start rather than pick.

## Where flecs' memory lives

The one place the embedding is not free. A module reads component bytes with plain loads, so those
bytes must sit inside the block backing memory 1 - and flecs, left alone, allocates wherever the
system allocator puts it.

So BridgeC allocates that block, hands it to the VM as memory 1, and gives flecs an allocator that
carves columns out of it. One copy of the data: a plugin reads it through real pointers, a module
reads the same bytes at `pointer - block start`. Neither goes through flecs to do it.

### It can be confined, and the patch is thirteen sites

Left alone, `ecs_os_api` is global: override it and flecs' **whole** heap lands in
module-addressable memory - hash maps, query caches, the block and stack allocators' pages. Confining
it to columns means patching flecs, which a spike measured rather than guessed.

**What a module reads is coverable.** Tag every allocation that touches column data or a table's
entity array, run a workload with table moves, column resizes, swap-removes and deletes, then check
that every pointer `ecs_get_id` answers and every `it.entities` an iterator exposes lands in a tagged
allocation:

    4,000 entities      5,417 rows checked, 0 escaped     storage   230 KB
                        4 entity arrays,    0 escaped     internals 1.58 MB
    200,000 entities  270,834 rows checked, 0 escaped     storage  10.9 MB
                        4 entity arrays,    0 escaped     internals 6.83 MB

**And not confining is expensive, which is the part argument could not supply.** Internals are
**87% of module-addressable bytes at 4,000 entities and 38% at 200,000** - a large fixed floor of
bootstrap plus growth that is sublinear against columns. So a stray write is likelier to land in
flecs' own structures than in game state, and that is the failure with no attribution: a crash
inside flecs rather than a wrong number in a component somebody declared.

**The patch is thirteen sites in two files** - `src/storage/table.c` for column malloc, realloc and
free across resize, grow, table merge and teardown, and `src/bootstrap.c` for the initial column
allocation and the root table's entity vec, which reading `table.c` alone misses. Three of the
thirteen are not replacements at all: `ecs_vec_init_t`, `ecs_vec_grow_t` and `ecs_vec_append_t`
already take an allocator and those sites pass `NULL`.

### How it is carried

Bridge-Ironwail vendors the **amalgamation** - `distr/flecs.c` and `distr/flecs.h`, two files rather
than the whole tree - under `third_party/flecs/`, with a README naming the version, the licence and
the date. We take that shape and break its one rule, which is *"nothing in these files has been
edited"*: that holds where flecs is used as an ECS and cannot here, since no wrapper outside flecs
can decide where flecs allocates.

**Pristine files plus a patch script**, rather than edited files or a `.patch`. The script is the
record and the applier at once, it is in the language `tools/` already needs, and its failure is the
best of the three: it names the exact source text it could not find, where a reject file does not.
It **checks `FLECS_VERSION` before touching anything**, so an upgrade fails loudly instead of
half-applying. Version pinned at the one the spike ran against, **4.1.6**.

**The condition that retires the patch:** flecs taking an allocator for column data upstream. Not
far-fetched - three of our thirteen sites exist only because an allocator parameter that is already
there is passed `NULL`.
