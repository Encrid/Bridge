# VM surface

The surface a VM backend implements. BridgeC defines it; a shared library provides it. One
implementation wraps wasmtime. Another could load shared libraries and call them directly, for code
that does not need sandboxing.

BridgeC keeps everything that is not the engine's business - `split.md`, *The split* - so a second
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

`memory_adopt` takes pages BridgeC already owns and asks the backend to make them addressable by a
module. It does not ask the backend to allocate. That is the whole of what "BridgeC owns the memory"
means at this seam, and it is why the same region can be handed to a plugin as a raw pointer and to
a module as an import.

There is no read-only variant and no verification pass, because access mode is not enforced - see
`data.md`, *Access mode*.

For a native backend `memory_adopt` is almost nothing - the pointer is already addressable - which
is the shape a good abstraction should have.

## The backend owns its clock

`instance_set_deadline` takes **nanoseconds, not ticks**, and the backend converts. wasmtime's own
deadline is in epochs - `store.h:224` takes `ticks_beyond_current`, and a tick is one call to
`wasmtime_engine_increment_epoch` (`engine.h:38`) - so **a tick has no duration until somebody
increments on a timer.** Whoever runs that timer owns the clock.

It is the backend rather than the host, even though `split.md`, *The split*, sends a policy about
time the other way. The host has no handle on an engine internal, a native backend has no epoch at
all, and a host that forgot to tick would silently disable every deadline in the deployment. So the
tick period is a `vm_create` key, and a backend that cannot interrupt says so through `deadline` in
`capabilities.md`, *Capabilities, so the swap is honest* instead of accepting a number it will ignore.

BridgeC asks in real time because that is the only unit both backends can mean the same thing by.
What it is asking for is a **lifecycle call's** timeout; a system is bounded by fuel alone, for the
reason in `limits.md`, *What bounds a run*. And `instance_set_fuel` is called **before every
dispatch**, not once at load, because fuel only depletes.

## What the backend does not do

Decide when anything runs. Read the filesystem. Know what a component, an entity or a barrier is.
Own the descriptor table. Choose a storage strategy. Parse a manifest.

**There is no store.** A store is wasmtime's unit of "one thread at a time" - its own header says a
store *"generally cannot be concurrently used"* while being *"cheap to create and cheap to
dispose"*, so one per instance is both idiomatic and what BridgeC wants. The unit at this seam is
therefore the **instance**, and how a backend groups instances internally is its business: wasmtime
gives each a store, a native backend gives each nothing.

Fuel, a lifecycle call's timeout and the memory limit all move with it. They are per element in
config, and a limiter that knows which element is executing can hold co-tenants to their own
ceilings - so scoping them to a VM concept would be wrong twice over.

**What this gives up: a backend cannot be told to group instances.** It may group them internally -
wasmtime could put several in one store - but BridgeC has no way to ask and no reason to, since the
grouping policy would be a VM-shaped decision. The condition that would change the answer is a VM
where instances **must** share state to be viable at all - one global heap for all of them, say - at
which point a grouping entry belongs here and the instance is the wrong unit.

The test each entry passes is that it means something to **both** backends. `instance_create` is
instantiate-or-dlopen; `instance_set_fuel` is set-fuel-or-say-you-cannot, which is what `meter` in
`capabilities.md`, *Capabilities, so the swap is honest* exists to say.

So **meaningful-but-unavailable is not meaningless** - that is `instance_set_fuel`, kept.
Meaningless to one backend is a different thing, and that is `store_create`: not "cannot do it" but
"the word names nothing here", which no capability flag could rescue.

BridgeC hands it bytes, memories and host functions, and asks it to run a function. That is the
entire relationship, and it is why the surface is fifteen entries rather than a second copy of
BridgeC.
