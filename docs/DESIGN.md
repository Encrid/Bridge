# Design

Why BridgeC is shaped the way it is, which side owns what, and what the shape costs. What the
documents are is `README.md`; in what order it gets built is `PLAN.md`.

## Why

Bridge's costs were measured, not guessed. On one machine, release build:

    empty tick                      300 ns
    built-in host call              500 ns
    plugin call                     900 ns
    module to module, same store   2200 ns
    module to module, across stores 6300 ns

A plain C call is 1 to 2 ns. Nothing closes a 200x gap per crossing, and the gap is wasmtime's, not
Bridge's. Removing every heap allocation from the crossing path bought 10%. So the cost of a frame
is the *number* of crossings, and the way to cut it is to stop sending data through calls at all.

One example, our own: the Godot adapter's per-frame work is 18 crossings, 42 us. The same work over
shared storage is one crossing.

## What the design is

**Data is declared once and addressed directly.** At registration an element declares the components
it needs and gets back a slot in a descriptor table, which BridgeC rewrites at every barrier.
Reading and writing is then address arithmetic. No handle, no lookup, no crossing.

**BridgeC owns the memory.** It is the orchestrator, so it owns the region, the schema and the
allocation policy.

**Core wasm, not components, and not WIT.** The component model exists to describe and copy values
across a language boundary. Under declare-at-init nothing is copied, so the canonical ABI has
nothing to do - and it is the one thing that cannot express a memory import or export, which core
modules can. WIT went with it: BridgeC has no type system, only sizes, alignments and fingerprints,
and the schema is a C header both sides compile rather than a format of our own. See `schema.md`.

**No call carries data at all.** Every function BridgeC calls - `init`, `shutdown`, `reload`, a
system, an observer - takes no arguments and returns an i32 status. There is nowhere for data to
travel through a call, which is stronger than restricting what may travel: a design cannot drift
back into passing things. What an element calls *into* still carries scalars, and only two kinds:
`vm-surface.md`, *Imports are declared, exports are checked*.

**No general allocation on the per-frame path.** Anything per-frame comes from a bump region or a
fixed ring, reset or drained at a barrier. Budgets are declared at startup and exhaustion is fatal
where it happens rather than degrading quietly later. That is idTech3's discipline, and it is the
same idea as "data does not cross" applied to memory instead of calls.

**ECS shape, on flecs.** Components are data stored per type, entities are ids, systems are declared
by elements. A plugin exists for what cannot be reached from inside wasm; a module exists for the
three guarantees wasm buys. Otherwise they are the same element - `element.md`.

BridgeC embeds flecs rather than writing an ECS, and no element ever sees it: the ABI is BridgeC's
own. Why that trade, why it is embedded rather than swappable like the VM, and what it costs:
`data.md`, *Why flecs rather than our own*.

**One ABI, eight entries, and both kinds of element get all of it.**

    provides   init, shutdown, reload, and its systems and observers
    gets       declare, system, observe
               spawn, destroy, attach, detach, alloc

**And nothing else - no element can call another.** Nothing asks; everything subscribes, which is
what an ECS already is - `element.md`, *The ABI*. Taking it literally is what removed the call
machinery an earlier draft of this design carried: `expose`, `find_func`, `call`, the service registry,
the signature fingerprint, and affinity propagating along a call graph.

Three things differ mechanically and nothing differs in capability: a plugin passes a function
pointer where a module names an export, it is handed a pointer where a module reads memory 2, and its
lifecycle is three symbols where a module's is three exports. `declare` carries a fingerprint -
`schema.md`, *Fingerprints*, for what that replaces and why a hash is enough.

**A deployment says what a module may touch.** Component names are namespaced, so a module declares
freely in its own namespace and needs a grant to reach another's. That makes a game's mod API
explicit instead of it defaulting to every component present, and it is one check at init.

### No builtins

Bridge shipped `log`, `ctl`, `config` and `profile` as host-implemented interfaces - the measured
"built-in host call, 500 ns". BridgeC has none, and the rule is **no builtin surface**: nothing an
element imports, no component BridgeC declares, and **no name BridgeC invents for a deployment's own
structure**. It still takes config as structs, drains log rings and keeps a clock for its own
records; none of that is reachable from an element.

The last clause is about structure, not about the calling convention. `init`, `shutdown` and `reload`
**are** three names BridgeC chose, and they stay: something has to be called first, and there is no
deployment decision hiding behind that - every loader has an entry name, from `main` to `vmMain`. A
phase name is the other thing entirely, a guess about the shape of a frame that every element would
then have to reference: `bridge.md`, *BridgeC ships no phase names*.

The reason is not this version's tidiness. A builtin is a door that stays open: each one is
defensible on its own and the set only grows, which ends up contradicting the minimality the rest of
this rests on. With none, the ABI is eight entries and has no *reason* to grow - a new capability
is a new plugin, not a new import.

**The test is whether a thing is universal to every deployment.** Config is: BridgeC cannot start
without its phases and its budgets, whatever else a deployment does. Storage and the memories are.
A **clock is not** - a one-run program has no frame, and a component every deployment carried
whether or not it had one would be content baked into the orchestrator. So is an engine, a device, a
filesystem.

The cost is that nothing works out of the box and two authors needing "time" no longer agree for
free. The answer is a default set shipped as **versioned, named elements** - `bridge:clock@1` as
something a deployment depends on - rather than a standard library bolted into the orchestrator.

That is not a hole in the last clause above. A deployment depending on `bridge:clock@1` chose to, can
fork it, and can do without it, where a name baked into the orchestrator would be unavoidable and
unreplaceable.

And it is why there are no phase names either: `bridge.md`, *BridgeC ships no phase names*.

**The VM is a shared library** behind a surface BridgeC defines. One implementation is wasmtime;
another could be native, for code that does not need sandboxing.

**Parallelism is derived; order is declared.** Declared read and write sets say which systems may
run at once - two writers of the same component may not - which makes the hot path lock-free by
construction. They do not say which of two conflicting systems goes *first*, so ordering comes from
phases, and where a phase leaves two conflicting systems unordered BridgeC refuses to start rather
than fall back on load order.

## The split

Every capability belongs to exactly one side. This is the list; the seam documents describe their
own corner of it and point here.

    BridgeC - the library            the host - IdeaTek, or an embedder
    ------------------------------   ----------------------------------------
    storage: flecs, columns, spans   reading files, and every format in them
    the barrier and the republish    TOML, and the three config layers
    the scheduler: phases, the       the manifest, via bridge_manifest_parse
      conflict graph, the refusal      (a helper beside the library)
    the element ABI, all eight       discovery: walking paths, deciding what
    the registry: components,          to load and when
      slots, grants, identities      the loop, and when the program ends
    the memories, arenas, rings      watching for changes, and asking for
    element lifecycle: running         a reload
      init, shutdown, reload         the log sink: what to do with records
    failure policy: on_error,        the compilation cache directory
      the failure budget, the        the journal file, if there is one
      escalation guards              the meaning of its own config keys -
    the VM seam                        [content], [debug], [cache], [pooling]
    the flecs fork: the patch, the
      pin, and carrying both
    what it counts and checks when
      asked, and the two pulls
      that hand the answers back

**The rule behind it**: BridgeC owns what no host could do differently, and the host owns everything
that is a format, a filesystem, or a policy about time. A second host must be able to replace its
whole column without touching the library.

Two of those were unassigned until this list existed. **The compilation cache** is a directory, so
it is the host's - it hands a path to the backend through `vm-surface.md` and BridgeC never opens
it. **The journal** is the host's for the same reason: BridgeC never opens a file, so what is
written and where is not something the library knows. It does not travel the log's path, though -
a log may drop a record and a journal may not: `bridge.md`, *What a pass depends on*.

Three were added once phase 0 had settled them, and each is on its side because nobody else could
hold it. **The flecs fork** is BridgeC's for the reason `memory.md`, *How it is carried*, gives.
**The counters and the checksum pass** are BridgeC's for the reason `config.md`, *One file, two
consumers*, gives, and **the two pulls** that hand their answers back follow the state - only
BridgeC has it, `host-api.md`, *Introspection*. And **the meaning of a host key** is the host's:
`config.md` says which tables are whose, and `ideatek.md`, *Its own keys*, settles what is inside
one.

## What we give up

    wit-bindgen          a header both sides compile, plus a small generator for accessors
    resource types       handles by hand, as godot:host/handles already does
    wac composition      the statically-linked call path
    string, list args    passed as (offset, len) where genuinely needed
    nine config keys     tick_order, autoload, call_timeout, the three restart
                         keys, [store], field_names and the profile's crossing
                         table. See `config.md`, *What the rewrite killed*
    column isolation     a module can write outside the columns it declared
    a synchronous answer no element can ask another a question and wait for the reply

**The last one is what dropping calls costs**, and it is a real loss rather than a tidy one. An
engine callback that consumes a return value inside its own loop cannot be served from storage:
`Array.sort_custom` wants a bool per comparison, now. So `Callable` round trips and `scripts.attach`
behaviour lose their synchronous half, and the two probe checks that assert it - *"a Callable the
adapter owns reaches a module and answers"* and *"a plain Node3D answers get_speed, because a module
is behind it"* - are the two the port does not get back. Everything an engine asks that is **state**
rather than a question is served from storage with no crossing at all: `plugin-abi.md`, *An engine
callback answers from storage*.

Column isolation is the ECS bargain, and it is **partly** recovered. Read and write declarations exist
so the scheduler can keep two writers apart - which is what they are for in any ECS - and they are
not policed. Why no arrangement of memories could express it instead: `module-abi.md`, *Memories*.
An element that writes a component it declared read is a bug in that element, and `diagnostics.md`,
*What a bad write costs, and what finds it*, is how it gets a name.

What is recovered is everything that is not a shared column. A module's spans, its `column_base`,
its scratch arena and its log ring sit in a private memory no other module imports, so the `base`
and `count` its arithmetic reads cannot be corrupted from outside - see `memory.md`, *Where the
seven regions live*. What stays exposed is the columns and the entity index, and only one thing
would change that, which we are declining: reserve address space per component and map only a
module's own columns into its memory, which `wasmtime_config_host_memory_creator_set` makes
possible. It costs a per-component budget, platform-specific mapping code in BridgeC, and - the
actual reason - TLB aliasing, since one physical page mapped for N modules is N entries where one
would do, on the inner loop this rewrite exists to protect.

**The condition that would change the answer: BridgeC being asked to run code the operator did not
choose.** Modding is not that - a player installing a mod chose it - and no engine we looked at
isolates mods from each other. Minecraft has the largest modding ecosystem there is with none at
all. What that ecosystem actually suffers is unattributable crashes and mod conflicts, which is what
attribution and the grants address instead.

Worth stating plainly: this is a regression against Bridge as it stands today, where separate
component memories made it impossible. The ECS is what buys it.

The boundary that matters is untouched either way - `data.md`, *Access mode*.

Kept: sandboxing from the host, traps caught, fuel, lifecycle timeouts, memory limits, hot reload,
the manifest section, threading.

Hot reload improves in one direction and gains a refusal. Today a reloaded module loses its state;
with storage in BridgeC's region **the data survives the swap**, which is what idTech3's `VM_Restart`
was for too - `data.md`, *Lifetime*, for its own comment on the point. A layout change is caught
by the fingerprint and refused by name rather than silently reinterpreted,
and the refusal cannot be waived: starting an element over with its data gone is `unload` then
`load`, exactly as idTech3 does for a native VM it cannot restart in place. See `data.md`,
*Lifetime* and *When the layout changed*.
