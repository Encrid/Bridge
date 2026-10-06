# Host API

The surface any host links against. What Bridge-ASM700 requires of the values a host hands over is
`config.md`.

The arrangement is idTech3's: the engine is `qcommon/`, and `win32/win_main.c:1212` calls `Com_Init`
then loops on `Com_Frame`.

This is the third seam. `module.md` and `bridge-abi.h` are Bridge-ASM700 to modules, with each
backend's binding beside that backend - `README.md`, *Three layers, three seams*;
`backend.md` is Bridge-ASM700 to a backend; this is **host to Bridge-ASM700**. Everything in this
folder is `bridge.h`, which is the only header a host includes - one per seam, `README.md`, *Three
layers, three seams*.

## Bridge-ASM700 parses nothing

Which side owns what is `split.md`, *The split*. At this seam it comes to one thing: **Bridge-ASM700
parses nothing** - not TOML, not object files, not wasm sections. `bridge_create` takes a struct;
`bridge_load` takes a struct plus bytes and a path, and passes those two straight to the module's
backend without inspecting either - `lifecycle.md`, *The runtime lifecycle*.

**The one kind of file Bridge-ASM700 opens is a backend library.** Config names each backend and its
library, and `bridge_create` opens them - `create.md`, *Create*. A backend is Bridge-ASM700's own
machinery rather than content, and a deployment that names one it cannot open has not got an
orchestrator to start. Refused: the host opening the library and handing over a table of functions,
which keeps Bridge-ASM700 away from every file and costs every host the same loader. The condition that
brings it back is an embedder that must link a backend statically.

So an embedder with its own config system - a database, compiled-in constants, another format -
hands over data rather than being made to write a file for Bridge-ASM700 to read. And the three platform
parsers that would otherwise sit in the library do not: `bridge_manifest_parse` ships **beside** it
as a helper - `native.md`, *Identity*.

**No discovery either.** Finding modules is filesystem work, so the host finds them and calls
`bridge_load` for each.

## The surface

    /* instance */
    bridge *bridge_create(const bridge_config *cfg, bridge_sink_fn sink, void *ud);
    int     bridge_destroy(bridge *b);

    /* advance */
    int bridge_phase(bridge *b, const char *name);

    /* runtime lifecycle - between phases only */
    int bridge_load  (bridge *b, const bridge_artifact *a);
    int bridge_unload(bridge *b, const char *identity);
    int bridge_reload(bridge *b, const bridge_artifact *a);

    /* introspection */
    int bridge_modules(bridge *b, bridge_module_info *out, int max);
    int bridge_profile (bridge *b, bridge_profile_row *out, int max);

Every answer above the two pulls is `[[nodiscard]]`: a status a host drops is one the compiler
points at, and `bridge_destroy`'s is the one that leaks a bridge when dropped - `create.md`,
*Destroy*.

`bridge_profile` and its row are `profile.md`, *The profile*.

## The data

    typedef struct { const char *before, *after; } bridge_order;

    typedef struct {
        uint64_t entities;        /* how many there may be at once: entity indices */
        uint32_t tables;          /* sets of components */
        uint32_t types;           /* declared types */
        uint32_t systems;         /* systems and observers */
        uint32_t terms;           /* terms one of them may have */
        uint32_t modules;         /* modules loaded */
        uint64_t persistent;      /* how many bytes: the persistent arena */
        uint64_t columns;         /* component columns and entity arrays */
    } bridge_budget;

    typedef struct {
        const char *key;
        enum bridge_setting_kind kind;                       /* bool | i64 | f64 | string */
        union { bool b; int64_t i; double f; const char *s; } v;
    } bridge_setting;

    typedef struct {
        const char *name;            /* what an artifact names: "wasm", "native" */
        const char *library;         /* the shared library Bridge-ASM700 opens */
        const bridge_setting *settings; uint32_t setting_count;  /* for backend_create */
    } bridge_backend;

    typedef struct {
        const bridge_backend *backends; uint32_t backend_count;
        const char *const *phases;   uint32_t phase_count;   /* array order IS the order */
        const bridge_order *order;   uint32_t order_count;
        uint32_t            threads;                         /* counting the caller's */
        bridge_budget       budget;
        bool                validate, profile;               /* [checks] */
    } bridge_config;

    typedef struct {
        const char *identity;        /* "tri:triangle@0.1.0" */
        uint32_t    abi;

        bool     trusted, reloadable, hot_reload, pinned;
        enum bridge_on_error on_error;   /* FAIL | SKIP. zero is unset */
        /* each of the next six: zero is unset, -1 is "no limit" */
        int64_t  max_system_failures;    /* raised by a failed barrier, lowered by a clean one */
        int64_t  fuel;               /* units per system invocation */
        int64_t  memory_limit;       /* bytes. a ceiling, never re-set */
        int64_t  init_timeout, reload_timeout, shutdown_timeout;   /* nanoseconds, per call */
        uint8_t  log_level;
        uint32_t scratch, log_ring, spans;   /* bytes, bytes, bytes */
        /* each of the next two: zero is unset, -1 is "it mints none" */
        int64_t  spawns;             /* entity ids between two barriers */
        int64_t  persistent;         /* its share of the persistent arena, in bytes */

        const char *const *read;  uint32_t read_count;       /* grants */
        const char *const *write; uint32_t write_count;

        const char *config;          /* the component its settings seed, or "none" */
        const bridge_setting *settings; uint32_t setting_count;  /* its own [.config] */
    } bridge_module_config;

    typedef struct {
        bridge_module_config cfg;
        const char *backend;             /* which backend runs it, by name */
        const void *bytes; size_t len;   /* the image, if the host has it */
        const char *path;                /* where it is, if it is anywhere. both go to the
                                            backend, which refuses if the one it needs is null */
    } bridge_artifact;

    /* what comes back out */
    typedef struct {
        const char *identity;            /* null for Bridge-ASM700's own records */
        const char *system;              /* null outside a system */
        const char *text;                /* THIS CALL ONLY - see `create.md`, *The sink* */
        uint32_t    len;
        uint32_t    when;                /* barrier count, then offset in the phase */
        uint32_t    arg[2];
        uint8_t     level;               /* lower is worse */
    } bridge_record;

    typedef void (*bridge_sink_fn)(void *ud, const bridge_record *r);

    typedef struct {
        const char *identity;
        enum bridge_state state;         /* RUNNING | STOPPED */
    } bridge_module_info;

**A phase is a string, not a struct.** It has one property and that is its name - `schedule.md`, *A
phase is a name and nothing else* - which also disposes of a collision: a typedef and a function
share one namespace in C, and `bridge_phase` is the call.

**The host resolves its own sources** - a manifest, a deployment's file, whatever it has - and
hands one flat struct per module. Bridge-ASM700 has no notion of where a value came from, which is why
nothing here records a source.

**Every limit here carries its period, because a number without one means nothing** - `limits.md`,
*What bounds a run*. One is per invocation, three are per call, and one is a ceiling.

**`validate` and `profile` are the two diagnostic switches here**, and both are deployment-wide: one
pass walks every column - `validation.md`, *The checksum pass, when asked* - and the other times
every dispatch, `profile.md`, *The profile*. Nothing else Bridge-ASM700 does is optional.

**A backend refuses a setting it does not know, by name**, and `bridge_create` fails with it: a key
nobody reads is a typo nobody would find, the argument `config.md`, *A module's own settings are a
component*, makes for a module's. Neither backend built so far has a setting, so each refuses any.

**A backend's `settings` are its own** - the cache directory, the pooling limits -
and Bridge-ASM700 passes them to `backend_create` unread: `backend.md`, *The backend owns its clock*.

**A module's `settings` seed a component, and nothing more** - `config.md`, *A module's own
settings are a component*, for why that means there is no reconfigure entry here.

### Nothing here is an ABI layout

The structs coming back out carry **resolved values**, never the shapes Bridge-ASM700 keeps
internally. That is *Bridge-ASM700 parses nothing* pointing the other way: no formats in, no internal
structs out, and a host that can be replaced without touching the library.

It is not a preference. The record a module writes into its ring - `log.md`, *The log* -
stores `text` as an offset into a wasm memory a host cannot address, `system` as a slot number, and
no identity at all, since the ring it sits in is what says whose it is. Handed over as-is it would
tell a host nothing it could print. Three of its fields have to be resolved, which is why these are
two structs and not one.

What it costs: two definitions of nearly the same thing, kept in step by hand. What it buys: the
ring's stride is an internal decision - change it and no host recompiles.
