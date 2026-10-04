# Plugin ABI

A plugin is an **element** - same capabilities as a module, three guarantees fewer, `element.md`.
What it can *do* is there; this file is the native mechanics only.

It exists for one reason: **something a module cannot reach from inside wasm.** A GPU, an audio
device, input, the system clock, a filesystem, a socket - and an engine that owns those, which is
what Godot is. Hardware, and the native code that is its only door.

So the test is **could this be a module?** A physics library, a codec, a bytecode interpreter -
anything that is pure computation - compiles to wasm, and whether that is worth the three guarantees
is `element.md`, *So what decides*. Bridge-Quake's QuakeC interpreter is a module.

The one place the test bends is a native library that cannot realistically be built for wasm:
threads, intrinsics, hand-written assembly, sheer size. The rule is then "cannot reach **or** cannot
run", which still excludes "I would rather write C".

Read `element.md`, then `data.md` and `memory.md`.

## The invariant

**A plugin's code runs inside a phase** - the scheduler called one of its systems, or one of its
observers at a barrier. Nothing moves inside a phase, and that is what makes reading storage from a
plugin safe with no handshake at all. The rest of this file rests on it.

An engine callback arriving on a foreign thread is the one case that escapes it, and it escapes by
**touching no storage at all**. It parks what it has in the plugin's own memory - a native queue,
the plugin's to size and to lock, the way Godot's `MessageQueue` is Godot's - and one of the
plugin's systems drains it into components in the next phase it runs in. So the phase that drain
sits in is what decides input latency, which is a deployment's choice rather than ours.

## Identity

A named section in the library, read by parsing the file before anything in it runs:

    __attribute__((section("bridge.manifest")))
    const char bridge_manifest[] = "godot:adapter@0.1.0\0abi=1";

The same contents a module's `bridge:manifest` carries, used for the same four things -
`element.md`, *Identity*. What is specific to this kind is **when** it is read.

**A section rather than an exported symbol, because the decision has to come before the code.**
`trusted` and `on_error` are config keyed on identity, and a symbol is only readable after `dlopen`
has run the library's static initialisers - so you would be checking whether a plugin is trusted
after executing it, and a plugin cannot be cleanly unloaded. Read, then decide, then load.

**The section is a convention the host reads, not something BridgeC parses.** BridgeC is handed the
identity as data - `host-api.md`, *The data* - so the section tables are walked by
`bridge_manifest_parse`, a helper beside the library rather than inside it: ELF's `shdr` and
`shstrtab`, PE's section table after the optional header, Mach-O's load commands. One implementation
for every host that wants it, and none for a host whose element metadata comes from elsewhere.

Which also means a sidecar is a **host's choice** rather than wrong. Godot's `example.gdextension`
carries `entry_symbol` and `compatibility_minimum` beside the binary; a host that prefers that shape
reads it and hands over the same struct. It costs a second file that can go missing, and that is now
the host's trade to make.

## Lifecycle

Three exported symbols, the lifecycle every element has:

    int bridge_init(const bridge_api *api);
    int bridge_shutdown(void);
    int bridge_reload(void);          /* optional; absent means cannot be reloaded */

An absent `bridge_reload` is how a plugin says it **cannot** be reloaded, the same convention a
module's missing `reload` export uses - and `hot_reload = false` is a deployment saying it **may
not**, which is a different statement. `module-abi.md`, *Exports*, has why that means the key can
only ever forbid. Each symbol returns a code, which `on_error` governs - `bridge.md`, *When something
fails*.

`bridge_init` runs once at boot, before any element is running and before any module exists.
Everything the plugin sets up happens there: declaring components, registering its systems and its
observers, taking hold of whatever device it exists for.

Having `bridge_shutdown` is what closes the old leak: a plugin holding a device releases it on
teardown rather than when whichever module would have closed it happens to get there - or does not,
having crashed.

## What BridgeC provides

`element.md`'s ABI as a vtable, in C declarations **`bridge-plugin.h`** carries - a plugin compiles
that and `bridge-abi.h`, where a module compiles only the second. Four entries are not simply that
list transliterated:

    slot system (name, name_len, phase, phase_len, terms, term_count, fn);
    slot observe(name, name_len, events, terms, term_count, fn);
    const descriptor *descriptors(void);
    const void       *region(void);

**`system` and `observe` take the function pointer**, where a module's are exports BridgeC resolves
by name. The phase, the events, the terms and the slot are identical, and the scheduler cannot tell
the two kinds apart.

`descriptors()` and `region()` are the plugin-only pair, and they answer for this kind the question
memory 2 answers for a module: where is my data. See *Reaching a component*.

Everything else - `declare`, the structural forms, `alloc` - is `element.md`'s list unchanged.
`attach` takes a real pointer for its initial value where a module passes an offset.

And nothing else. No value marshalling, no call context, no buffer interface, no service registry,
and **no way to call another element** - `element.md`, *The ABI*. The first two have nothing left to
do now that data does not travel through calls at all, and the rest were answering a question
subscription answers.

## Reaching a component

**The same spans a module reads** - `data.md`, *The interface is a query* - rewritten at every
barrier. A query matches many tables, so there is one span per table and a plugin iterates them the
way a module does. What differs is only how it gets there: a module finds the span array through
memory 2's header, and a plugin is handed a pointer.

    const char      *origin = api->region();
    const descriptor *d     = &api->descriptors()[slot];

    for (uint32_t s = 0; s < d->spans; s++) {
        const bridge_span *sp = &d->at[s];
        for (uint32_t r = 0; r < sp->count; r++) {
            thing *t = (thing *)(origin + sp->base[0] + r * sizeof(thing));
        }
    }

`base` is an offset from the start of the shared region - memory 1, identical to what a module sees -
so a plugin adds that region's address, which `region()` answers once since it does not move.

**`sizeof(thing)` comes from the schema header, not from the span.** Stride is never published, for
the reason `data.md` gives: both sides compile the same declaration, so the generator already knows
it.

One span layout for both readers rather than a resolved copy for plugins: two layouts would be two
rewrites at every barrier, and two things to disagree.

A `descriptor` is a plugin's equivalent of a module's query slot - `{ spans, at }` against
`{ spans, at }` - and the only difference is that `at` is a pointer here and an offset there, because
a plugin has the process's address space and a module has memory 2.

The pointer is good for the phase it was read in, for the same reason a module's row number is:
storage may be compacted or migrated at a barrier.

## Threads

A plugin's systems run on whichever thread BridgeC schedules its element on, and different elements
run at once - `plugin-abi.md`, *Threads*. For most plugins that is the end of it.

For the rest it is not, and the case is not theoretical: **Godot must be driven from the thread that
created it.** So a plugin says so, with one key in its manifest:

    pinned = true     every system and observer of this element runs on the thread
                      bridge_init ran on

**It is the author's key, not the deployment's**, which is the *who holds the knowledge* rule
`config.md` is ordered by. A plugin author knows their library must be driven from one thread; a
deployment author has no access to that fact, so declaring it there would be the deployment
guessing, and a wrong guess is a race nobody will find quickly. It reaches BridgeC as a field in
`bridge_element_config`, like every other manifest value.

**The cost is that pinned elements serialise with each other.** They are all on one thread by
construction. That is not a flaw - elements all driving one thing that is not thread-safe **must**
serialise - and it is visible at load rather than as a race.

**Nothing is proxied and nothing is deferred.** Godot's answer to a wrong-thread call is
`call_deferred`, a queue it drains itself; ours is that the wrong thread never happens, because the
element was placed on the right one before it ran. There is no call to proxy in the first place.

A plugin appends to a log ring of its own, exactly as a module does, through a real pointer rather
than a wasm memory - `diagnostics.md`, *The log*. Not a direct call into BridgeC, even though one would
cost two nanoseconds here: a plugin may be running on a thread of its own, and a ring each is what
keeps every writer single and lock-free.

## What a plugin must not do

Not *cannot* - **nothing here is enforced**, and that is the whole difference. The three-memory split
in `module-abi.md` buys a plugin nothing: it holds real pointers, so every region is reachable to it,
including another element's private memory. A module is protected from other modules, never from a
plugin. Component grants are the same story - a plugin is given them to stay honest, not because
anything could stop it reading what it was not granted.

**Put a native handle in a component.** That is this kind's only addition to the list in
`element.md`, and it is the subtle one.

A `VkDevice` or a GL context would fit in a `u64` and the layout would even be sound, but **storage
survives an unload by design**, so the handle outlives the plugin that made it and the next reader
uses it happily. Two plugins that must share a device share it the
way two native libraries in one process always have - by being built to - and that is not BridgeC's
business. BridgeC's storage is for data a component describes, and a pointer is not that.

## An engine callback answers from storage

This is where the design pays. An engine's callbacks mostly ask for **state**, and state is in
storage, so a plugin answers from it with no call and no crossing:

    _process              the module's logic is a system; its output is a component the
                          plugin flushes
    _get, _set            a module's properties are components; the plugin reads them
    _get_property_list    the descriptors are a component array
    _has_point            the shape is a component; the plugin does the test
    a signal firing       an entity carrying an event component; whoever subscribed runs
                          at the barrier - `data.md`, *Observers*

Godot's own documented example needs none of it: a position computed from elapsed time is a system
writing a component. It also fixes a thing that bit us, where a module could not set its own
property because `Object.set` came back as a re-entry.

What is left over is the callback whose return value the engine consumes **inside its own loop**,
with the engine on the stack waiting - `Array.sort_custom` asking a comparator for a bool N log N
times. No component write answers a question that has to be answered now, so **a deployment cannot
express that one**, and `DESIGN.md`, *What we give up*, is where it is written down.
