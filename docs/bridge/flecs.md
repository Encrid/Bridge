# flecs

Why flecs is embedded rather than swappable, where its memory lives, and the fork that confines
it. The data model built on it is `data.md`.

## Why the VM is swappable and this is not

The VM surface abstracts a **behaviour**: BridgeC hands over bytes, memories and host functions and
asks for a function to be run, never looking inside. That is why a native backend implements the
same fifteen entries and nothing leaks.

An ECS surface would have to abstract a **memory layout**. BridgeC needs four things of flecs: its
column addresses, because a span's `base[]` *are* offsets into its columns; its entity index, to
mirror; a row move it does not publish at all, so the mirror cannot drift; and control of its
allocator, so columns land inside memory 1. None of that is what an ECS does - it is how *this* ECS
stores things, so the surface would be shaped by flecs and leak its only implementation. And
confinement settles it by force - *It can be confined, and the patch is thirteen sites* - because a
library you patch is a fork you ship rather than a backend you swap.

Underneath is the trade the rewrite rests on. **Zero-copy direct access to the ECS's memory is
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

The one place the embedding is not free. A module reads component bytes with plain loads, so those
bytes must sit inside the block backing memory 1 - and flecs, left alone, allocates wherever the
system allocator puts it.

So BridgeC allocates that block, hands it to the VM as memory 1, and gives flecs an allocator that
carves columns out of it. One copy of the data: a plugin reads it through real pointers, a module
reads the same bytes at `pointer - block start`. Neither goes through flecs to do it.

### It can be confined, and the patch is thirteen sites

Left alone, `ecs_os_api` is global: override it and flecs' **whole** heap lands in
module-addressable memory - hash maps, query caches, the block and stack allocators' pages.
Confining it to columns means patching flecs, which a spike measured.

**What a module reads is coverable.** Tag every allocation that touches column data or a table's
entity array. Run a workload with table moves, column resizes, swap-removes and deletes. Then check
that every pointer `ecs_get_id` answers and every `it.entities` an iterator exposes lands in a
tagged allocation:

    4,000 entities      5,417 rows checked, 0 escaped     storage   230 KB
                        4 entity arrays,    0 escaped     internals 1.58 MB
    200,000 entities  270,834 rows checked, 0 escaped     storage  10.9 MB
                        4 entity arrays,    0 escaped     internals 6.83 MB

**And not confining is expensive, which argument could not supply.** Internals are **87% of
module-addressable bytes at 4,000 entities and 38% at 200,000** - a large fixed floor of bootstrap
plus growth that is sublinear against columns. So a stray write is likelier to land in flecs' own
structures than in game state, and that is the failure with no attribution: a crash inside flecs
rather than a wrong number in a component somebody declared.

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

## flecs will not tell you which

The one thing a spike had to settle, because getting it wrong is silent. flecs' events name every
entity whose **components** changed and **no entity whose row merely moved**. Of those 503 changes,
303 had no event naming them, every one a pure row move inside a single table, 0.8 per operation. An
event-driven mirror would point a module at the wrong row that often, and nothing would say so.

So the mirror takes **one more patched site** than the columns did - `flecs_table_delete`, in the
file *It can be confined, and the patch is thirteen sites* already forks, which makes it fourteen.
flecs computes the moved entity and its destination row there and updates its own record with them;
the patch is to say so out loud. That option is cheap only because the columns settled first: a
callback into a library we were not already forking would have been a different question.

**Table ids are BridgeC's own.** `ecs_record_t` is public and carries an `ecs_table_t *`, where a
span needs a dense `u32` to index `column_base`. So BridgeC keeps pointer-to-id itself, off
`EcsOnTableCreate` and `EcsOnTableDelete`, and never reads `ecs_table_t`'s layout.
