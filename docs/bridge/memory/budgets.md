# Budgets

What was weighed here and refused is `budgets_refused.md`, *What was refused: Budgets*.

Two tiers, because two things are being sized.

**Shared, when Bridge-ASM700 starts, from `bridge_config_t`:**

    how many there may be at once
    entities      entity indices. Index 0 is no entity
    tables        archetypes: the sets of components entities carry
    types         declared types: components, and the structs, enums and bitmasks their
                  fields name
    systems       systems and observers registered
    terms         terms one of them may have
    modules       modules loaded
    sparse_sets       components kept in a sparse set, or none
    direct_indexed    components kept direct-indexed, or none

    how many bytes
    persistent    the persistent arena
    columns       every table's rows: its entity ids and its columns

One block serves everyone, so no single module could size it. It arrives as a struct rather than a
file, because Bridge-ASM700 reads none: `host_api.md`, *Bridge-ASM700 parses nothing*.

**`sparse_sets` and `direct_indexed` are stated because of what those two storage layouts
keep for every entity there may be**, carried or not: a sparse set's index, and a direct-indexed
component's bits. How many such components there will be is not known when the bridge is created,
and storage allocates nothing after - `storage.md`, *What bounds it*. So the deployment says how
many, and their room is had at create: eight bytes an entity for each sparse set. Either may be
none, which is stated as `BRIDGE_NONE`; zero is unset, and refused. A `declare` past either is
refused, by name. A direct-indexed component's cells are its size for every entity, which is
not known until it is declared: they are taken from `columns` then, and a `declare` they do not
fit is refused - `storage.md`, *Direct-indexed*.

**`tables` is a count of what exists, not of what was ever seen.** A table lives while it has rows
- `storage.md`, *A table* - so the number to state is how many different archetypes
entities carry at one time. It sizes everything storage keeps a table, and one more thing that
is a module's: `column_base` has a cell for each of the module's components in every table there
may be, inside its `spans`. A module whose `spans` cannot hold that is refused at load, by the
arithmetic, and not stopped at whichever barrier made one table too many.

**`types` and `systems` are counts of what is there at once, across every module.** A type two
modules declare is one. **A reload needs no room for a module's systems twice.** The old
instance's systems leave storage before the new instance registers its own, in the places those
had; if the reload fails, the new ones are dropped and the old put back as they were, which
cannot fail, since the places are free again. So `systems` is the number a deployment has, and
not that number and a module more. `tests/stage6.c`, cases `reloadexact` and
`reloadexactfail`, reload a module of one system with `systems` at one, and fail to. The two
keys exist because storage's memory is a product: a table
keeps an offset for every type there may be, and a system keeps a place for every table there may
be. With the two stated, everything storage holds is known and allocated when the bridge is
created, and it allocates nothing after - `storage.md`, *What bounds it*. A `declare` or a
`add_system` past its key is refused there, by name, which fails that module's `init`.

**`terms` is the most one system or observer may ask for**, and it is a key for the same reason:
storage keeps a system's terms in a record made when the bridge is, so the record has a length. It
was a constant, 32, and the number was flecs' default with nothing of ours behind it. No module
compiles against the number, so it is one number and the deployment's.

**Per module, when that module registers:** its scratch arena, its log ring and its spans - which
is to say, this tier sizes that module's private memory. And what is set aside for it in the shared
one, so that what it mints is its own - `threads.md`, *Threads*:

    spawns        how many entity ids it may take between two barriers. They are set aside
                  at a barrier and held: real entities with nothing on them, which no
                  query matches, and which count against the entity ceiling
    persistent    its share of the persistent arena, for as long as it is loaded

`[budget] persistent` is the whole arena, and a module whose share does not fit in one piece of
what is left of it is refused at load. A share is given back when its module is unloaded, unless
a string in it is still held by a row - `strings.md`, *It is given back when nothing points into
it*.

**A reload may state a larger share, and never a smaller one.** A budget is the deployment's to
change by reloading - `config.md`, *A module's own settings are a component* - and this one could
not be, at first: any change was refused, so raising it cost the module's data by an unload. What is
added is carved from what is left of the arena: where what follows the module's share is free it
grows in place, and otherwise it gains a further range, which `alloc` moves to once what is asked no
longer fits in the one before. What was left of that one is dead space, the cost `strings.md`,
*Changing one, and the cost of growing*, already names. Offsets handed out stay good, which is the
rule flecs' own stack allocator keeps when it chains a page. Stating less is refused: what the
module allocated is in its share, and it is still loaded.

A module that spawns nothing, keeps no string, or queues no change, says so: `BRIDGE_NONE`, which
is -2 and not the -1 of a limit stated as none - `config.md`, *No unaccountable values*.

## Nothing is allocated in a phase

**A phase and the barrier after it call the allocator not once.** Everything they use was given
its room when the bridge was created or when a module loaded, from a number somebody stated:

    storage's rows, tables and indexes    at create, from `[budget]`: `storage.md`
    what a dispatch works in              at create, `[budget] systems` long
    a module's block of ids               at load, `spawns` long
    a module's queue of changes           at load, `changes` bytes
    an observer's list of who it heard    at load, as many as the module's `spans` can publish
    the marks that order the queues       at load, one for each of the module's systems
    what a schedule is worked out in      at create: `systems` squared, in bits

`changes` is a key every module states, beside `spawns`: the bytes of `attach`, `detach` and
`destroy` it may queue between two barriers, at 24 a change and an attach's value with it, to a
multiple of eight. A change that does not fit is refused and that invocation fails, as an
allocation past `persistent` does. A module that queues none states `BRIDGE_NONE`.

`tests/storage.c` is built a second time against a library whose every call to the allocator is
counted, and runs the cases `changes`, `tables` and `strip` with a phase failing if the count
moved: cases `alloc-changes`, `alloc-tables`, `alloc-strip`.

**Loading, reloading and unloading a module do allocate**, and creating and destroying a bridge:
a record for the module, for each of its systems and types, and the rooms above. None is the
hot path. What happens when one of those allocations fails is `failure.md`, *Out of memory*.

**One thing among them has no bound.** The place of an unloaded module's settings text is
remembered for its next load, where its share of the arena stayed, and the list of those places
grows by one for every different module so unloaded and is never shortened. A bridge has to
unload a great many different modules, each leaving a string behind, for that to matter.

Because modules come and go, `modules` bounds **how many module records exist at once**, not
how the private memory is divided. A record is taken at load and freed at unload, and exhaustion refuses the
load, the same discipline as every other budget. The shared tier is untouched - the entity ceiling
and the persistent arena do not care how many modules exist.

**Each module's private memory is its own**, sized from that module's own `cfg` when it loads. It is not
a share of anybody's block, and it could not be: the isolation in `wasm.md`, *Memories*, is
wasm's bounds check, which only separates two modules if each has a memory of its own.

So **a reload that wants more just gets it.** Reload builds a fresh instance over surviving storage
- `module.md`, *Three guarantees differ* - and a fresh instance means a fresh private memory at whatever
size the new `cfg` asks for. The block that cannot grow is the shared memory; what a module has in
it - its ids, its share of the arena - is set aside from budgets the whole deployment stated.

A ring each rather than one shared matters beyond budgeting: no write needs an atomic - `log.md`,
*The log*.

Every budget is **fixed for as long as the block exists**, and exhaustion is fatal where it happens.
Growing one in place is impossible, since that would relocate a block already handed to a backend. So a
bigger private memory arrives with a new instance and a bigger shared memory does not arrive at all.

## What running out does

"Fatal where it happens" is fatal **to whoever can be named**, and that differs by where a budget
runs out:

    spawns, persistent,    at an ABI entry      the invocation is abandoned at that call
      entities
    spans                  at a load            the load is refused, if `column_base` alone
                                                does not fit
                           at a barrier         that module is stopped, by name
    tables, columns        at a barrier         the entity is left as it was, and it is a
                                                failure of every module that asked a change of it
    log_ring               at a write           the line is dropped and counted
    modules                at a load            the load is refused
    types, systems,        at a `declare` or    the module's `init` fails
      terms                an `add_system`

**An entry that cannot be honoured abandons the invocation that made it.** `spawn` answers an id and
no status, so a module has nothing to test - `abi.md`, *The ABI* - and the failure cannot be a value.
The call does not return: on the wasm backend it traps, and from there it is an ordinary failed
system, or a failed `init` - `failure.md`, *When something fails*. The same holds for an entry
refused for any other reason: a `declare` without its grant, an `attach` naming a slot that was
never declared. A module that spawns too much costs itself one invocation and nobody else anything.

`spawn` runs out in two ways and says which. A module that used the ids set aside for it is past
its own `spawns`. One whose ids could not all be set aside is past the deployment's `entities`: the
barrier gives it as many as fit, and the ceiling is what its next `spawn` names.

**A module whose spans no longer fit is stopped**, since its systems would otherwise run over
tables they were not told about. Its components stay, as for any stopped module.

**An entity that cannot be placed is left as it was.** A barrier knows what every changed entity
will carry before it moves a row - `storage.md`, *A barrier is worked an entity at a time* - so
it knows there what cannot be had: a table past `tables`, or rows past `columns`. Every change
asked of that entity at that barrier is then dropped, all of them and not some: it keeps the
components and the values it had, and no observer hears of it. Other entities are placed as usual.
**When a table cannot take all the rows arriving, it takes as many as it finds room for, and the
first asked are the ones placed.** It asks the column region for half as many until a number
fits, and then searches back up between that number and the last that did not. Entities are
worked in the order their first change was asked, which is the order the calls ran -
`threads.md`, *Threads* - so which are placed is the same every run. `tests/storage.c`, case
`nocolumns`, has room for four rows of six and places four, where halving alone finds three.

**It is a failure of every module that asked a change of it**, each for the changes that were its
own. A record names the module, the key, and how many of its changes were not applied; and the
barrier counts as one it failed in, toward its `max_system_failures` - `failure.md`, *The budget*.
Two modules that changed one entity are both told, since both lost what they asked. So a module that
outgrows the deployment is stopped as any failing module is, by its own `on_error`, and one that
asked once too often loses one barrier's changes and runs on.

Nothing is taken from outside the block, ever. Until stage 10 a column that did not fit was served
from the operating system and the bridge stopped for good, for two reasons that no longer hold:
nobody could be named, and the storage of the time could not be unwound from the middle of a
change. Here nothing has begun when the shortage is found.

## Bridge-ASM700 defaults none of them

A default is a value nobody in the chain accounted for, which `config.md`, *No unaccountable
values*, refuses: **omitted everywhere is an error**, at create for the shared tier and at load for
a module's. A host is free to let a deployment state one value for
fifty modules; that is still a value somebody stated.

idTech3 did ship defaults - `DEF_COMHUNKMEGS "64"`, `DEF_COMZONEMEGS "24"` (`common.c:45`) - and had
to, with one hunk for everything and no manifest to read. It also had a **floor**, clamped and
announced rather than refused: `common.c:1517` prints a line and uses `MIN_COMHUNKMEGS` instead. We
refuse there, because clamping is the implicit downgrade `capabilities.md`, *Nothing is downgraded*,
exists to refuse.

**One floor has an argument and the rest do not.** A `log_ring` must hold at least one record: a
module that cannot report anything is the one legal number that produces an incoherent deployment -
`log.md`, *The log*, for why. `work_limit` and `memory_limit` have floors too, but they are measurements
rather than arguments: `plan.md` stages 2 and 3 enforce them, and are where a number becomes
knowable.

No budget has a floor that is storage's own: nothing of it is in the block before a module asks.
