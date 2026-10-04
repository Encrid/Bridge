# Budgets

Two tiers, because two things are being sized.

**Shared, when BridgeC starts, from `bridge_config`:** the entity ceiling behind the index mirror,
and the persistent arena. One block serves everyone, so no single element could size it. It arrives
as a struct rather than a file, because BridgeC reads none: `host-api.md`, *BridgeC parses nothing*.

**Per element, when that element registers:** its scratch arena and its log ring - which is to say,
this tier sizes that element's memory 2.

Because elements come and go, `max_elements` bounds **how many element records exist at once**, not
how memory 2 is divided. A record is taken at load and freed at unload, and exhaustion refuses the
load, the same discipline as every other budget. The shared tier is untouched - the entity ceiling
and the persistent arena do not care how many elements exist.

**Each element's memory 2 is its own**, sized from that element's own `cfg` when it loads. It is not
a share of anybody's block, and it could not be: the isolation in `module-abi.md`, *Memories*, is
wasm's bounds check, which only separates two elements if each has a memory of its own.

So **a reload that wants more just gets it.** Reload builds a fresh instance over surviving storage
- `element.md`, *Three guarantees differ* - and a fresh instance means a fresh memory 2 at whatever
size the new `cfg` asks for. The block that cannot grow is memory 1, and nothing per-element lives
there.

A ring each rather than one shared matters beyond budgeting: systems run in parallel, so a shared
ring would need atomics on every write, and one per element needs none. BridgeC drains them all at
the barrier.

Every budget is **fixed for as long as the block exists**, and exhaustion is fatal where it happens.
Growing one in place is impossible, since that would relocate a block already handed to the VM. So a
bigger memory 2 arrives with a new instance and a bigger memory 1 does not arrive at all.

## BridgeC defaults none of them

A default is a value nobody in the chain accounted for, which `config.md`, *No unaccountable
values*, refuses: **omitted everywhere is an error**, at create for the shared tier and at load for
an element's. A deployment tired of editing fifty manifests puts an `[element.defaults]` table in
its own config - layer 2 covering what layer 1 left out, and still a value somebody stated.

idTech3 did ship defaults - `DEF_COMHUNKMEGS "64"`, `DEF_COMZONEMEGS "24"` (`common.c:45`) - and had
to, with one hunk for everything and no manifest to read. It also had a **floor**, clamped and
announced rather than refused: `common.c:1517` prints a line and uses `MIN_COMHUNKMEGS` instead. We
refuse there, because clamping is the implicit downgrade `capabilities.md`, *Nothing is downgraded*,
exists to refuse.

**One floor has an argument and the rest do not.** A `log_ring` must hold at least one record: an
element that cannot report anything is the one legal number that produces an incoherent deployment -
`log.md`, *The log*, for why. `fuel` and `memory_limit` have floors too, but they are measurements
rather than arguments: `PLAN.md` phase 2 enforces both and is where a number becomes knowable.
