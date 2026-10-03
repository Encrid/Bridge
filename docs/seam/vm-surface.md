# VM surface

**Draft.** Open items are tracked in OPEN.md; nothing here is settled until that register is
empty.

The surface a VM backend implements. BridgeC defines it; a shared library provides it. One
implementation wraps wasmtime. Another could load shared libraries and call them directly,
for code that does not need sandboxing.

BridgeC keeps everything that is not the engine's business - `DESIGN.md`, *The split* - so a second
backend does not reimplement a scheduler to swap an interpreter.

What matters at this seam is the far side of that list: config, discovery and the formats are the
host's, so **a backend never sees a path except the one it is handed to `dlopen`**, and never a file
it has to find.

## What a backend is asked for

    vm_create(config)                     -> vm
    vm_destroy(vm)
    vm_capabilities(vm)                   -> flags

    memory_adopt(vm, ptr, len)            -> memory
    module_compile(vm, bytes, len)        -> module
    module_imports(module)                -> names and kinds
    module_destroy(module)

    instance_create(vm, module, memories, count) -> instance
    instance_destroy(instance)
    instance_set_fuel(instance, units)
    instance_set_deadline(instance, nanos)
    instance_set_memory_limit(instance, bytes)

    define_func(instance, module, field, fn, sig)
    instance_func(instance, name, &arity, &results) -> func
    call(func, status)                    -> err

Fifteen entries. The Rust `ModuleRuntime` trait had fourteen, four of which were filesystem and
timer work that has moved out.

`module_imports` is there for one decision: **which memories to hand `instance_create`.** Not every
module imports both - `module-abi.md`, *Memories* - so BridgeC reads the list rather than supplying
memories nobody asked for and letting the link fail.

## Memory is adopted, not created

`memory_adopt` takes pages BridgeC already owns and asks the backend to make them addressable
by a module. It does not ask the backend to allocate. That is the whole of what "BridgeC owns
the memory" means at this seam, and it is why the same region can be handed to a plugin as a
raw pointer and to a module as an import.

There is no read-only variant and no verification pass, because access mode is not enforced -
see `data.md`, *Access mode*.

For a native backend `memory_adopt` is almost nothing - the pointer is already addressable -
which is the shape a good abstraction should have.

## Capabilities, so the swap is honest

`vm_capabilities` is not decoration. A backend says what it can actually do:

    sandbox        the guarantee in element.md, *Three guarantees differ*
    meter          fuel, or an equivalent
    deadline       a running module can be interrupted
    reload         an instance can be replaced while the process runs

`sandbox` is about the **host boundary and nothing else**. It does not mean one module cannot corrupt
another's components: they share a memory by design - see `DESIGN.md`, *What we give up*. idTech3 drew
the same line, `VMI_NATIVE` against `VMI_BYTECODE` per element with a cvar each and shipped set to the
safe one, and got module-against-module isolation only because its three VMs had three blocks and
shared no data.

Each flag is demanded by a **config key**, not by an element: `sandbox` by `trusted = false`, `meter`
by `fuel`, `deadline` by a timeout, `reload` by `hot_reload`. So a mismatch is not an element arguing
with a backend - it is **config contradicting config**, since the deployment chose both. It is
therefore one of BridgeC's own refusals and takes init's path under `on_error`, exactly as a denied
grant or an ambiguous order does: `bridge.md`, *When something fails*.

### Nothing is downgraded

idTech3 does - asked for native with no DLL present, `vm.c:486` prints a line and falls back to
bytecode, and `fs_restrict` forces the safe path regardless of what was asked.
It is sound there because the two paths are two builds of one module with identical semantics. It is
still **implicit**, and a deployment that asked for a guarantee should be told it cannot have one
rather than quietly given something else.

The override is explicit instead: a developer who wants untrusted code on a native backend writes
`trusted = true` - a statement in a file somebody can read, rather than a warning nobody does.

A native backend answers no to `sandbox`, truthfully, and BridgeC then refuses to load anything
marked untrusted onto it. That is what makes a native backend a deliberate
choice for code you wrote rather than a hole you discover.

Without this the surface would let a backend silently stop enforcing the guarantees the
documents above promise, which is the failure worth designing out.

## Imports are declared, exports are checked

The two directions are not symmetric, and the asymmetry is the design rather than an oversight.

    BridgeC -> a module's imports   eight functions, and their signatures VARY
    a module's exports -> BridgeC   every one is () -> i32

So **a signature is declared going in and verified coming out.** `define_func` carries one:

    typedef struct {
        const uint8_t *params;  uint32_t param_count;   /* I32 | I64 */
        const uint8_t *results; uint32_t result_count;
    } vm_sig;

**An arity would not do, and not as a matter of taste.** Five of the eight entries answer a value -
`declare`, `system` and `observe` a slot, `spawn` an id, `alloc` an offset - and a count of
arguments cannot say so. Nor can it say how wide they are: `attach(id, slot, ptr, len)` has four
parameters of which the first is a 64-bit entity id and the rest are 32-bit. And wasmtime's own entry
settles it - `wasmtime_linker_define_func` takes `const wasm_functype_t *ty` (`linker.h:137`), params
and results with their widths, which **cannot be constructed from a number**.

**Two types, not four.** Every entry is `I32` or `I64` - offsets, lengths and slots are 32-bit, an
entity id is 64 - and **no ABI entry takes or returns a float.** `PLAN.md`'s list of four scalars
described a call path that no longer exists.

**Why the type travels as data rather than as the engine's own.** wasmtime wants a `wasm_functype_t`
and a native backend wants nothing at all, so handing over arrays lets each do its own thing, and
passing wasmtime's type would leak the engine into the seam. It also keeps **one copy of the fact**:
a backend told the signature cannot drift from it, where a backend compiling our ABI's signatures
itself would re-derive what BridgeC already holds - the thing `module-abi.md`, *The published
layouts*, refuses for that header.

**What it buys beyond being implementable.** Core wasm checks an import's shape at link time -
`module-abi.md`, *Identity*, for what that catches and what it does not - so a module that declares
`declare` wrongly is **refused at instantiation, by name**. Bind to whatever the module declared
instead and the same module links happily and passes garbage - a load-time refusal traded for
runtime corruption.

`module_imports` is not a second source for this: it answers *which memories to supply*, and nothing
about what shape a function has.

## The backend owns its clock

`instance_set_deadline` takes **nanoseconds, not ticks**, and the backend converts. wasmtime's own
deadline is in epochs - `store.h:224` takes `ticks_beyond_current`, and a tick is one call to
`wasmtime_engine_increment_epoch` (`engine.h:38`) - so **a tick has no duration until somebody
increments on a timer.** Whoever runs that timer owns the clock.

It is the backend rather than the host, even though `DESIGN.md`, *The split*, sends a policy about time
the other way. The host has no handle on an engine internal, a native backend has no epoch at all,
and a host that forgot to tick would silently disable every deadline in the deployment. So the tick
period is a `vm_create` key, and a backend that cannot interrupt says so through `deadline` in
*Capabilities, so the swap is honest* instead of accepting a number it will ignore.

BridgeC asks in real time because that is the only unit both backends can mean the same thing by.
What it is asking for is a **lifecycle call's** timeout; a system is bounded by fuel alone, for the
reason in `bridge.md`, *What bounds a run*. And `instance_set_fuel` is called **before every
dispatch**, not once at load, because fuel only depletes.

## A call takes nothing and answers a status

    err call(func, int32_t *status);

**Every function BridgeC calls has the same signature**, because there are only five kinds of them -
`init`, `shutdown`, `reload`, a system and an observer - and all of them take no arguments and return
an i32. Data
never travels through a call here: a system reads its spans and writes its columns, so there is
nothing to pass. No marshalling at this seam, no arity to arrange, and nothing a backend could get
wrong about either.

`status` is what the function returned; `err` is whether it returned at all. A trap is an ordinary
outcome - a module that reads out of bounds, exhausts a system's fuel, or runs past a lifecycle
call's timeout produces one, and the process keeps running - and `bridge.md`, *When something
fails*, treats the two alike on purpose.

`instance_func` still answers arity and result count beside the handle, and that is a **check rather
than a contract** - the other half of *Imports are declared, exports are checked*: BridgeC refuses an
export whose shape is not `() -> i32` at init instead of discovering it on the first frame. A
backend that cannot tell - a native one, where the symbol came from `dlsym` and carries no type -
answers -1, and BridgeC skips the check rather than refusing to load.

## What the backend does not do

Decide when anything runs. Read the filesystem. Know what a component, an entity or a barrier
is. Own the descriptor table. Choose a storage strategy. Parse a manifest.

**There is no store.** A store is wasmtime's unit of "one thread at a time" - its own header says a
store *"generally cannot be concurrently used"* while being *"cheap to create and cheap to dispose"*,
so one per instance is both idiomatic and what BridgeC wants. The unit at this seam is therefore the
**instance**, and how a backend groups instances internally is its business: wasmtime gives each a
store, a native backend gives each nothing.

Fuel, a lifecycle call's timeout and the memory limit all move with it. They are per element in
config, and a limiter that knows which element is executing can hold co-tenants to their own
ceilings - so scoping them to a VM concept was wrong twice over.

**What this gives up: a backend cannot be told to group instances.** It may group them internally -
wasmtime could put several in one store - but BridgeC has no way to ask and no reason to, since the
grouping policy would be a VM-shaped decision. The condition that would change the answer is a VM
where instances **must** share state to be viable at all - one global heap for all of them, say - at
which point a grouping entry belongs here and the instance is the wrong unit.

The test each entry passes is that it means something to **both** backends. `instance_create` is
instantiate-or-dlopen; `instance_set_fuel` is set-fuel-or-say-you-cannot, which is what `meter` in
*Capabilities, so the swap is honest* exists to say.

So **meaningful-but-unavailable is not meaningless** - that is `instance_set_fuel`, kept. Meaningless
to one backend is a different thing, and that was `store_create`: not "cannot do it" but "the word
names nothing here", which no capability flag could have rescued.

BridgeC hands it bytes, memories and host functions, and asks it to run a function. That is
the entire relationship, and it is why the surface is fifteen entries rather than a second
copy of BridgeC.
