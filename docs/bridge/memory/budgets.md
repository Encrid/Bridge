# Budgets

Two tiers, because two things are being sized.

**Shared, when Bridge-ASM700 starts, from `bridge_config`:** the entity ceiling behind the index
mirror, the persistent arena, and the columns. One block serves everyone, so no single module could
size it. It
arrives as a struct rather than a file, because Bridge-ASM700 reads none: `host-api.md`,
*Bridge-ASM700 parses nothing*.

**Per module, when that module registers:** its scratch arena, its log ring and its spans - which
is to say, this tier sizes that module's private memory.

Because modules come and go, `max_modules` bounds **how many module records exist at once**, not
how the private memory is divided. A record is taken at load and freed at unload, and exhaustion refuses the
load, the same discipline as every other budget. The shared tier is untouched - the entity ceiling
and the persistent arena do not care how many modules exist.

**Each module's private memory is its own**, sized from that module's own `cfg` when it loads. It is not
a share of anybody's block, and it could not be: the isolation in `wasm.md`, *Memories*, is
wasm's bounds check, which only separates two modules if each has a memory of its own.

So **a reload that wants more just gets it.** Reload builds a fresh instance over surviving storage
- `module.md`, *Three guarantees differ* - and a fresh instance means a fresh private memory at whatever
size the new `cfg` asks for. The block that cannot grow is the shared memory, and nothing per-module lives
there.

A ring each rather than one shared matters beyond budgeting: no write needs an atomic - `log.md`,
*The log*.

Every budget is **fixed for as long as the block exists**, and exhaustion is fatal where it happens.
Growing one in place is impossible, since that would relocate a block already handed to a backend. So a
bigger private memory arrives with a new instance and a bigger shared memory does not arrive at all.

## What running out does

"Fatal where it happens" is fatal **to whoever can be named**, and that differs by where a budget
runs out:

    entities, persistent   at an ABI entry      the invocation is abandoned at that call
    spans                  at a barrier         that module is stopped, by name
    columns                at a barrier         the bridge stops
    log_ring               at a write           the line is dropped and counted
    max_modules            at a load            the load is refused

**An entry that cannot be honoured abandons the invocation that made it.** `spawn` answers an id and
no status, so a module has nothing to test - `abi.md`, *The ABI* - and the failure cannot be a value.
The call does not return: on the wasm backend it traps, and from there it is an ordinary failed
system, or a failed `init` - `failure.md`, *When something fails*. The same holds for an entry
refused for any other reason: a `declare` without its grant, an `attach` naming a slot that was
never declared. A module that spawns too much costs itself one invocation and nobody else anything.

**A module whose spans no longer fit is stopped**, since its systems would otherwise run over
tables they were not told about. Its components stay, as for any stopped module.

**Columns running out stops the bridge, because nobody can be named.** It happens at a barrier,
inside flecs, while the queued changes of several modules are being applied, and what failed is the
deployment's storage. flecs cannot be unwound from the middle of a change, so the allocator serves
what the block could not from the operating system, flecs finishes with its storage whole, and the
bridge then refuses every `bridge_phase` and `bridge_load` until `bridge_destroy`, as after a `fail`
module failed. No module runs again, so nothing ever reads the memory that landed outside the block.

Refused: stopping the bridge for every exhaustion, which is one rule and lets one module's `spawn`
take the deployment down - and the entity ceiling recovers by itself when something is destroyed.
Refused: an entry answering zero for the module to check, which is the error code the ABI chose not
to have.

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
`log.md`, *The log*, for why. `fuel` and `memory_limit` have floors too, but they are measurements
rather than arguments: `PLAN.md` phases 2 and 3 enforce them, and are where a number becomes
knowable.
