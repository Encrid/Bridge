# Host API

The surface any host links against. Why there are two targets at all is `PLAN.md`; the reference host
is `ideatek.md`.

The arrangement is idTech3's: the engine is `qcommon/`, and `win32/win_main.c:1212` calls `Com_Init`
then loops on `Com_Frame`.

This is the third seam. `element.md` and the two ABI files are BridgeC to elements; `vm-surface.md`
is BridgeC to a VM backend; this is **host to BridgeC**. Everything in this file is `bridge.h`, which
is the only header a host includes - one per seam, `README.md`, *Three layers, two seams*.

## BridgeC parses nothing

Which side owns what is `DESIGN.md`, *The split*. At this seam it comes to one thing: **BridgeC parses
nothing** - not TOML, not object files, not wasm sections. `bridge_create` takes a struct; `bridge_load`
takes a struct plus bytes and a path, and passes those two straight to the backend without inspecting
either.

So an embedder with its own config system - a database, compiled-in constants, another format - hands
over data rather than being made to write a file for BridgeC to read. And the three platform parsers
that would otherwise sit in the library do not: `bridge_manifest_parse` ships **beside** it as a
helper - `plugin-abi.md`, *Identity*.

**No discovery either.** Walking `[content] paths` is filesystem work, so the host walks and calls
`bridge_load` per element. `autoload` is not a BridgeC key.

## The surface

    /* instance */
    bridge *bridge_create(const bridge_config *cfg, bridge_sink_fn sink, void *ud);
    void    bridge_destroy(bridge *b);

    /* advance */
    int bridge_phase(bridge *b, const char *name);

    /* runtime lifecycle - between phases only */
    int bridge_load  (bridge *b, const bridge_artifact *a);
    int bridge_unload(bridge *b, const char *identity);
    int bridge_reload(bridge *b, const bridge_artifact *a);

    /* introspection */
    int bridge_elements(bridge *b, bridge_element_info *out, int max);

## The data

    typedef struct { const char *before, *after; } bridge_order;

    typedef struct {
        uint64_t entities;        /* the index mirror ceiling */
        uint64_t persistent;      /* the persistent arena */
        uint32_t max_elements;    /* the per-element slot pool */
    } bridge_budget;

    typedef struct {
        const char *const *phases;   uint32_t phase_count;   /* array order IS the order */
        const bridge_order *order;   uint32_t order_count;
        bridge_budget       budget;
        bool                validate;                        /* [checks] - the checksum pass */
    } bridge_config;

    typedef struct {
        const char *key;
        uint8_t     kind;                                    /* bool | i64 | f64 | string */
        union { bool b; int64_t i; double f; const char *s; } v;
    } bridge_setting;

    typedef struct {
        const char *identity;        /* "tri:triangle" */
        uint32_t    abi;

        bool     trusted, hot_reload, pinned;
        uint8_t  on_error;           /* FAIL | SKIP */
        uint32_t max_system_failures;
        int64_t  fuel;               /* units per system invocation */
        int64_t  memory_limit;       /* bytes. a ceiling, never re-set */
        int64_t  init_timeout, reload_timeout, shutdown_timeout;   /* nanoseconds, per call */
        int8_t   log_level;
        uint32_t scratch, log_ring;

        const char *const *read;  uint32_t read_count;       /* grants */
        const char *const *write; uint32_t write_count;

        const bridge_setting *settings; uint32_t setting_count;  /* its own [.config] */
    } bridge_element_config;

    typedef struct {
        bridge_element_config cfg;
        const void *bytes; size_t len;   /* the module image, for the backend */
        const char *path;                /* for dlopen; null for a module */
    } bridge_artifact;

    /* what comes back out */
    typedef struct {
        const char *identity;            /* null for BridgeC's own records */
        const char *system;              /* null outside a system */
        const char *text;                /* THIS CALL ONLY - see *The sink* */
        uint32_t    len;
        uint32_t    when;                /* barrier count, then offset in the phase */
        uint32_t    arg[2];
        uint8_t     kind;                /* LOG | PROFILE */
        int8_t      level;               /* negative is worse - flecs' convention */
    } bridge_record;

    typedef void (*bridge_sink_fn)(void *ud, const bridge_record *r);

    typedef struct {
        const char *identity;
        uint8_t     state;               /* RUNNING | STOPPED */
    } bridge_element_info;

**A phase is a string, not a struct.** It has one property and that is its name - `bridge.md`, *A
phase is a name and nothing else* - which also disposes of a collision: a typedef and a function
share one namespace in C, and `bridge_phase` is the call.

**The host resolves the three layers** - the manifest, the deployment's file, the local one - and
hands one flat struct per element. BridgeC has no notion of where a value came from, which is why
nothing here records a layer.

**Every limit here carries its period, because a number without one means nothing** -
`bridge.md`, *What bounds a run*. One is per invocation, three are per call, and one is a ceiling.

**`validate` is the one diagnostic switch here**, and it is deployment-wide because the pass walks
every column - `diagnostics.md`, *The checksum pass, when asked*. Nothing else BridgeC checks is
optional.

**`settings` seeds a component, and nothing more** - `config.md`, *An element's own settings are a
component*, for why that means there is no reconfigure entry here.

### Nothing here is a BridgeC layout

The two structs coming back out carry **resolved values**, never the shapes BridgeC keeps internally.
That is *BridgeC parses nothing* pointing the other way: no formats in, no internal layouts out, and a
host that can be replaced without touching the library.

It is not a preference. The record an element writes into its ring - `diagnostics.md`, *The log* - stores
`text` as an offset into a wasm memory a host cannot address, `system` as a slot number, and no
identity at all, since the ring it sits in is what says whose it is. Handed over as-is it would tell a
host nothing it could print. Three of its fields have to be resolved, which is why these are two
structs and not one.

What it costs: two definitions of nearly the same thing, kept in step by hand. What it buys is that
the ring's stride is an internal decision - change it and no host recompiles.

## Create

`bridge_create` is `Com_Init` minus the filesystem: take the config, size the shared block, and build
the phase table. It loads nothing - the host does that.

It returns null only when the **deployment** cannot start: **no phases at all**, a phase named
twice, an `[order]` pair naming a system nobody could have, a budget that cannot be satisfied, or a
null `sink`.

**Which of those, the host learns from the sink** - it was handed over as an argument, so BridgeC
emits the record naming the phase or the pair before it returns. That is the case that would have
justified a `bridge_error` and does not. The one failure no record can explain is a null `sink`, and
a host that passed null already knows.

### The sink

`sink` is why an embedded host needs more than idTech3's three functions. BridgeC emits records and
**never opens a file**; the shell writes them, and a host with its own logging takes them as records.

**It is mandatory**, which is the log's own argument - `diagnostics.md`, *The log* - applied one level
out: a library that can fail unreportably is worse than one that refuses to start. Refusing costs
nothing here, since the decision is made before anything has been loaded.

Four properties, all of which fall out of where the drain happens rather than being choices:

    one call per record    the call is native to native, so batching saves no
                           crossing and an array would need a lifetime rule
    on the calling thread  the drain is at a barrier, inside bridge_phase, after
                           parallel systems have joined. a sink needs no lock
    text for this call     the scratch arena resets right after the drain, so a
                           formatted message is gone when the call returns. a host
                           that keeps a line copies it
    identity may be null   BridgeC emits its own records too - a denied grant, a
                           refused load - and those belong to no element

**And there is no `bridge_error`.** A second route to the same answer is a second thing to keep true,
and it would hand back a string where *Introspection* refuses to let a host learn state from
diagnostics. A failed call answers with its return code; what is running answers with
`bridge_elements`; why is a record.

## The host owns the loop, and the order within it

`bridge_phase` runs one named phase and its barrier. **There is no `bridge_step`.** The host calls the
phases it wants, as often as it wants.

**No phase is BridgeC's**, so every name here is one the deployment declared - `bridge.md`,
*BridgeC ships no phase names*.

That is what makes a **fixed timestep** expressible: physics at 60 Hz with a variable framerate means
calling one phase more often than another, which a single advance-everything call cannot say. Same
for a host that owns its render loop and wants `setup` and `update`, then its own work, then
`present`.

It also follows from BridgeC not looping. The host already owns the loop, so it owns what goes in it.

**Two costs, both real.** `[[phase]]` has no `after` relation - nothing needs a dependency graph to
order phases the host is ordering itself. And **the host can forget a phase**: call `update` and never
`present`, and half the deployment quietly does nothing. BridgeC cannot detect that, because it has
no notion of a complete pass.

What survives untouched is the refusal when two systems *within* one phase conflict unordered -
`bridge.md`, *Order within a phase is never implicit*. That was never about phase order.

## BridgeC has no notion of being done

It runs the systems registered to the phase it was asked for, and at the barrier the observers of
whatever changed. If nothing is registered, it runs nothing. There is no idle status, no completion
tracking, and no way for an element to ask the process to stop - an element handles its own
lifecycle, and a system with nothing to do iterates zero spans.

**When the program ends is the host's question.** A shell loops until a signal, or a window closes,
or because it was told to run once. A one-run program is then not a special case: it calls each phase
once and exits.

The one thing BridgeC does track per element is failure - *"past the budget the element is stopped"*,
`bridge.md`, *When something fails*. That is noticing you keep failing, not asking whether you are
finished, and it exists because a system failing every pass forever is what idTech3's escalation
guards against.

## The runtime lifecycle

**Legal only between phases**, never while `bridge_phase` is executing. All three restructure storage
and the schedule, which is what a barrier makes safe. A plugin holding the pointer could break this;
nothing contains a plugin, so it is a contract rather than a guarantee.

**`load`** takes an artifact rather than a path, because BridgeC decides on `cfg` and never opens
anything: check the ABI version, check `trusted` against the backend's `sandbox`, check the grants,
take a per-element budget slot, hand `bytes` or `path` to the backend, call `init` - which calls
`declare`, `system` and `observe` - recompute the conflict graph and **refuse-and-unwind if a new
system or observer conflicts unordered with an existing one**, then publish spans.

**Read, then decide, then load** survives the host owning the filesystem: the identity arrives as data
in `cfg`, so the decision is made with the artifact untouched.

That refusal is the startup rule generalised. At create it refuses to start; here it refuses the load,
and the explicit call is what gives it somebody to tell.

**`unload`** - drop its systems and observers from the schedule; call `shutdown`; drop its exclusive
components, `data.md`, *Structural change*; free its element record; destroy the instance or
`dlclose`.

**Its persistent-arena space is not reclaimed** - `memory.md`, *It is never reclaimed*. That is what
keeps a surviving shared component's string readable, like the float beside it.

**Nothing can be holding anything.** No element ever had a reference to this one - it had a query,
and a query that now matches nothing iterates zero spans. That is what dropping calls bought: unload
has no dangling anything to chase.

**`reload`** takes an artifact too, because a rebuilt element may carry a changed manifest - and that
is also how a setting changes: hand over the new `cfg`. **Storage survives, which is the entire
point, and it is the one outcome nothing else here produces.** The fingerprint decides whether it
can: a changed struct body hashes differently and is refused by name, shared or exclusive alike -
`data.md`, *When the layout changed*.

**There is no `reset` flag.** Dropping this element's data and starting it over is `unload` then
`load`, which is the same sequence under a verb that says so. It also makes `hot_reload` mean
something: it gates reload-in-place, the only thing with no alternative, rather than an outcome a
host could reach anyway.

## Introspection

`bridge_elements` exists for one reason, and it is not a console: **an element that loaded can be
stopped later, and nothing else says so.** Past its failure budget BridgeC stops an element - its
systems and observers stop running and its components stay, `bridge.md`, *When something fails*. That
happens at frame nine thousand, long after any return code the host saw.

**Not for load failures.** The host calls `bridge_load` once per element and gets a code back each
time, so what loaded is something the host already watched happen.

**And it has to be a pull.** A record saying "stopped" would do, except that the log is *"Drop and
count, never block"* - `diagnostics.md`, *The log*. State must never arrive by a route that may drop one,
or a host's picture is permanently wrong with nothing to correct it. Which is also why this call
exists rather than a host reading its own log output back: that is using diagnostics as state.

So two fields, and everything else is deliberately absent. Kind, the ABI version, the grants and the
settings are all things the host handed over. Memory used, systems registered and times failed are
diagnostics, and go where diagnostics go. **An element that is not there does not appear** - the list
is what exists, the host holds what it handed over, and the difference is the answer.
