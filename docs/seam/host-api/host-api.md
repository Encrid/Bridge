# Host API

The surface any host links against. Why there are two targets at all is `README.md`; the reference
host is `ideatek.md`.

The arrangement is idTech3's: the engine is `qcommon/`, and `win32/win_main.c:1212` calls `Com_Init`
then loops on `Com_Frame`.

This is the third seam. `element.md` and the two ABI files are BridgeC to elements; `vm-surface.md`
is BridgeC to a VM backend; this is **host to BridgeC**. Everything in this folder is `bridge.h`,
which is the only header a host includes - one per seam, `README.md`, *Three layers, three seams*.

## BridgeC parses nothing

Which side owns what is `split.md`, *The split*. At this seam it comes to one thing: **BridgeC
parses nothing** - not TOML, not object files, not wasm sections. `bridge_create` takes a struct;
`bridge_load` takes a struct plus bytes and a path, and passes those two straight to the backend
without inspecting either.

So an embedder with its own config system - a database, compiled-in constants, another format -
hands over data rather than being made to write a file for BridgeC to read. And the three platform
parsers that would otherwise sit in the library do not: `bridge_manifest_parse` ships **beside** it
as a helper - `plugin-abi.md`, *Identity*.

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
    int bridge_profile (bridge *b, bridge_profile_row *out, int max);

`bridge_profile` and its row are `profile.md`, *The profile*.

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
        bool                validate, profile;               /* [checks] */
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
        const char *text;                /* THIS CALL ONLY - see `create.md`, *The sink* */
        uint32_t    len;
        uint32_t    when;                /* barrier count, then offset in the phase */
        uint32_t    arg[2];
        int8_t      level;               /* negative is worse - flecs' convention */
    } bridge_record;

    typedef void (*bridge_sink_fn)(void *ud, const bridge_record *r);

    typedef struct {
        const char *identity;
        uint8_t     state;               /* RUNNING | STOPPED */
    } bridge_element_info;

**A phase is a string, not a struct.** It has one property and that is its name - `schedule.md`, *A
phase is a name and nothing else* - which also disposes of a collision: a typedef and a function
share one namespace in C, and `bridge_phase` is the call.

**The host resolves the three layers** - the manifest, the deployment's file, the local one - and
hands one flat struct per element. BridgeC has no notion of where a value came from, which is why
nothing here records a layer.

**Every limit here carries its period, because a number without one means nothing** - `limits.md`,
*What bounds a run*. One is per invocation, three are per call, and one is a ceiling.

**`validate` and `profile` are the two diagnostic switches here**, and both are deployment-wide: one
pass walks every column - `validation.md`, *The checksum pass, when asked* - and the other times
every dispatch, `profile.md`, *The profile*. Nothing else BridgeC does is optional.

**`settings` seeds a component, and nothing more** - `config.md`, *An element's own settings are a
component*, for why that means there is no reconfigure entry here.

### Nothing here is a BridgeC layout

The two structs coming back out carry **resolved values**, never the shapes BridgeC keeps
internally. That is *BridgeC parses nothing* pointing the other way: no formats in, no internal
layouts out, and a host that can be replaced without touching the library.

It is not a preference. The record an element writes into its ring - `log.md`, *The log* -
stores `text` as an offset into a wasm memory a host cannot address, `system` as a slot number, and
no identity at all, since the ring it sits in is what says whose it is. Handed over as-is it would
tell a host nothing it could print. Three of its fields have to be resolved, which is why these are
two structs and not one.

What it costs: two definitions of nearly the same thing, kept in step by hand. What it buys: the
ring's stride is an internal decision - change it and no host recompiles.
