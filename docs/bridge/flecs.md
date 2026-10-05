# flecs

Why flecs is embedded rather than swappable, where its memory lives, and the fork that confines
it. The data model built on it is `data.md`.

## Why the backend is swappable and this is not

The backend surface abstracts a **behaviour**: Bridge-ASM700 hands over bytes, memories and host
functions and asks for a function to be run, never looking inside. That is why the native backend
implements the same seventeen entries and nothing leaks.

An ECS surface would have to abstract a **memory layout**. Bridge-ASM700 needs four things of flecs: its
column addresses, because a span's `base[]` *are* offsets into its columns; its entity index, to
mirror; a row move it does not publish at all, so the mirror cannot drift; and control of its
allocator, so columns land inside the shared memory. None of that is what an ECS does - it is how *this* ECS
stores things, so the surface would be shaped by flecs and leak its only implementation. And
confinement settles it by force - *It can be confined* - because a
library you patch is a fork you ship rather than a backend you swap.

Underneath is the trade this design rests on. **Zero-copy direct access to the ECS's memory is
incompatible with treating the ECS as a black box.** A swappable ECS would mean asking it for data,
which is a copy or a call per access - the cost we are here to remove.

## Why flecs rather than our own

The model we reasoned out from scratch turned out to be flecs' model, term for term: components
stored by type, entities as ids carrying a generation, access modes declared for scheduling,
structural change deferred to a merge point, pipeline phases. Writing our own would have produced a
worse flecs.

The cost: 116k lines of C we did not write, in a project whose pitch is minimal and explicit. It is
a deliberate trade. The ECS is where a decade of tuning matters most and where our own attempt would
have been weakest, and it is MIT and already vendored in Bridge-Ironwail.

## Where flecs' memory lives

The one place the embedding is not free. A wasm module reads component bytes with plain loads, so those
bytes must sit inside the block backing the shared memory - and flecs, left alone, allocates wherever the
system allocator puts it.

So Bridge-ASM700 allocates that block, hands it to each backend as the shared memory, and gives
flecs an allocator that carves columns out of it. One copy of the data: a native module reads it
through real pointers, a wasm module reads the same bytes at `pointer - block start`. Neither goes
through flecs to do it.

### It can be confined

Left alone, `ecs_os_api` is global: override it and flecs' **whole** heap lands in
module-addressable memory - hash maps, query caches, the block and stack allocators' pages.
Confining it to columns means patching flecs.

**What a module reads is coverable.** Hand flecs one block for column data and entity arrays. Run a
workload with table moves, column resizes, swap-removes, deletes, a shrink and a component deletion.
Then check that every pointer `ecs_get_id` answers and every `it.entities` an iterator exposes lands
in the block - `tests/confined.c`, on every build:

    4,000 entities      4,431 rows checked, 0 escaped
    200,000 entities  221,539 rows checked, 0 escaped

The same test against a flecs without the patch reports every one of them escaped, which is what
makes the zero mean something.

**And not confining is expensive, which argument could not supply.** The phase 0 spike measured it,
on an upstream snapshot rather than on the release pinned now, so the figures are an order of
magnitude: internals were **87% of module-addressable bytes at 4,000 entities and 38% at 200,000** -
a large fixed floor of bootstrap plus growth that is sublinear against columns. So a stray write is
likelier to land in flecs' own structures than in game state, and that is the failure with no
attribution: a crash inside flecs rather than a wrong number in a component somebody declared.

**The patch is one argument, passed in twenty-three places.** In this flecs a column and a table's
entity array are both an `ecs_vec`, and every call that allocates one passes `NULL` for the
allocator: twenty-three calls, in nine functions of the table storage and the bootstrap. The patch
passes `FLECS_CONFINE` there instead, teaches the four `ecs_vec` functions that touch memory what
that argument means, and replaces the four allocations `flecs_table_shrink` makes by hand.

A `NULL` allocator still means the OS, which the rest of flecs relies on. Refused: making `NULL`
itself mean the block, which would capture every other caller that passes it.

What reaches Bridge-ASM700 is `flecs_confine`: allocate, reallocate and free, each handed its sizes,
set once before `ecs_init`. It is a global, as `ecs_os_api` is and for the same reason: the functions
doing the allocating have no world in scope.

### How it is carried

Bridge-Ironwail vendors the **amalgamation** - `distr/flecs.c` and `distr/flecs.h`, two files rather
than the whole tree - under `third_party/flecs/`, with a README naming the version, the licence and
the date. We take that shape and keep its one rule, *"nothing in these files has been edited"*, by
patching a copy: the build compiles what the script writes, never the vendored files.

**Pristine files plus a patch script**, rather than edited files or a `.patch`. The script is the
record and the applier at once, it is in the language `tools/` already needs, and its failure is the
best of the three: it names the function and the exact source text it could not find, where a reject
file does not.

**It checks the version and a hash of both files before touching anything.** The version alone does
not identify the source: upstream's `master` kept saying 4.1.6 for months after the tag, with
different table storage. The phase 0 spike ran against such a snapshot, and its site list did not
survive the move to the release. Pinned: **the v4.1.6 tag**.

**An upgrade is a new site list, not a version bump.** The storage this patch touches has already
been rewritten upstream - columns are no longer an `ecs_vec` there. The script will fail by name, and
the two tests say when the new list is complete.

**The condition that retires the patch:** flecs taking an allocator for column data upstream. Not
far-fetched - every one of the twenty-three calls exists only because an allocator parameter that is
already there is passed `NULL`.

## flecs will not tell you which

The one thing a spike had to settle, because getting it wrong is silent. flecs' events name every
entity whose **components** changed and **no entity whose row merely moved**. `tests/rowmove.c`
keeps a shadow of where each entity is from the events alone: over 44,845 operations on 20,000
entities it is wrong for 5,374 of the 15,823 that survive, every one moved into a hole by somebody
else's delete. An event-driven mirror would point a module at the wrong row that often, and nothing
would say so.

So the mirror takes **one more edit** than the columns did, in `flecs_table_delete`. flecs computes
the moved entity and its destination row there and updates its own record with them; the patch is to
say so out loud, through `flecs_confine.row_moved`. With it the same shadow is wrong for none. That
option is cheap only because the columns settled first: a callback into a library we were not
already forking would have been a different question.

**One hook, because one site is silent among those Bridge-ASM700 reaches.** A table merge, which a
component deletion causes, moves rows too, and the events do name those. `ecs_table_swap_rows` and
sorting move rows silently and are not hooked: nothing here calls them. The condition that adds a
hook there is Bridge-ASM700 using either.

**Table ids are Bridge-ASM700's own.** `ecs_record_t` is public and carries an `ecs_table_t *`, where a
span needs a dense `u32` to index `column_base`. So Bridge-ASM700 keeps pointer-to-id itself and
never reads `ecs_table_t`'s layout. A table gets its id the first time it is seen holding an entity:
when the mirror places one in it, or when a query matches it. Id 0 is no table, which is what an
entity with nothing on it has. Giving an id back waits for `EcsOnTableDelete`, and for the first
thing that deletes a table: unload.
