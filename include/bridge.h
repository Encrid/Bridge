/* The host seam: the only header a host includes.
 * docs/seam/host-api/host-api.md is the contract. This is as much of it as exists so far:
 * everything but `validate` and the profile, which arrive with the diagnostics they switch on. */
#ifndef BRIDGE_H
#define BRIDGE_H

#include <stddef.h>
#include <stdint.h>

typedef struct bridge bridge;

/* A backend the deployment configures: Bridge-ASM700 opens the library, and a module names
 * the backend that runs it. */
enum bridge_setting_kind : uint8_t {
    BRIDGE_BOOL = 1, BRIDGE_INT = 2, BRIDGE_FLOAT = 3, BRIDGE_STRING = 4
};

typedef struct {
    const char *key;
    enum bridge_setting_kind kind;              /* zero is unset */
    union { bool b; int64_t i; double f; const char *s; } v;
} bridge_setting;

typedef struct {
    const char *name;
    const char *library;
    /* the backend's own, passed to it unread: it refuses a key it does not know */
    const bridge_setting *settings; uint32_t setting_count;
} bridge_backend;

/* Every one is stated: zero is unset, and refused. docs/bridge/memory/budgets.md. How many
 * there may be at once of each thing, and then how many bytes. */
typedef struct {
    uint64_t entities;        /* entity indices: index 0 is none, so one fewer entities */
    uint32_t tables;          /* archetypes: the sets of components entities carry */
    uint32_t types;           /* declared types: components, and the structs, enums and
                               * bitmasks their fields name */
    uint32_t systems;         /* systems and observers registered */
    uint32_t terms;           /* terms one of them may have */
    uint32_t modules;         /* modules loaded */
    uint64_t persistent;      /* the persistent arena */
    uint64_t columns;         /* component columns and entity arrays */
} bridge_budget;

/* Two systems that conflict in one phase, in the order they are to run. A system is named by
 * its module's identity without the version, then itself: "game:actors:movement". */
typedef struct { const char *before, *after; } bridge_order;

typedef struct {
    const bridge_backend *backends; uint32_t backend_count;
    const char *const *phases;      uint32_t phase_count;   /* a phase is a name, and no more */
    const bridge_order *order;      uint32_t order_count;
    uint32_t threads;               /* how many run a phase, counting the caller's. zero is unset */
    bridge_budget budget;
} bridge_config;

enum bridge_on_error : uint8_t { BRIDGE_FAIL = 1, BRIDGE_SKIP = 2 };    /* zero is unset */
/* Two values and not one, because they say opposite things: a module with no limit may use
 * all there is, and a module with no reservation gets nothing. Each field below takes one of
 * them, and the other, like any other negative number, is refused at load. */
constexpr int64_t BRIDGE_NO_LIMIT = -1;         /* a limit stated as none */
constexpr int64_t BRIDGE_NONE = -2;             /* a reservation stated as none */
/* A module with no settings component says so. No component is named without a colon, so
 * this names none of them; a config left null is unset, and refused. */
#define BRIDGE_NO_CONFIG "none"

typedef struct {
    const char *identity;        /* "tri:triangle@0.1.0" */
    uint32_t    abi;

    bool     trusted;
    bool     reloadable;         /* its author says it can be reloaded: from the manifest */
    bool     hot_reload;         /* the deployment allows bridge_reload. it only ever forbids */
    bool     pinned;             /* its systems run on the thread that loaded it, and no other */
    enum bridge_on_error on_error;   /* what losing it costs the deployment */
    /* each of the next six: zero is unset, BRIDGE_NO_LIMIT is "no limit" */
    int64_t  max_system_failures;    /* raised by a failed barrier, lowered by a clean one */
    int64_t  fuel;               /* units per system invocation */
    int64_t  memory_limit;       /* bytes. a ceiling, never re-set */
    int64_t  init_timeout, reload_timeout, shutdown_timeout;    /* nanoseconds, per call */
    uint8_t  log_level;          /* a line above it is not written: enum bridge_level */
    uint32_t scratch, log_ring, spans;   /* its private memory, each in bytes */
    /* what is set aside for it alone, so that what it mints does not depend on who ran
     * beside it. each: zero is unset, BRIDGE_NONE is "it mints none" */
    int64_t  spawns;             /* entity ids it may take between two barriers */
    int64_t  persistent;         /* its share of the persistent arena, in bytes, for its life */

    const char *const *read;  uint32_t read_count;       /* grants */
    const char *const *write; uint32_t write_count;

    /* its own settings, and the component of its that they seed - docs/seam/host-api/config.md.
     * `config` is a component's name or BRIDGE_NO_CONFIG; null is unset */
    const char *config;
    const bridge_setting *settings; uint32_t setting_count;
} bridge_module_config;

typedef struct {
    bridge_module_config cfg;
    const char *backend;             /* which backend runs it, by name */
    const void *bytes; size_t len;   /* the image, or null */
    const char *path;                /* where it is, or null. both go to the backend */
} bridge_artifact;

/* what comes back out */
typedef struct {
    const char *identity;            /* null for Bridge-ASM700's own records */
    const char *system;              /* null outside a system */
    const char *text;                /* THIS CALL ONLY */
    uint32_t    len;
    uint32_t    when;                /* barrier count, then offset in the phase */
    uint32_t    arg[2];
    uint8_t     level;               /* lower is worse: enum bridge_level */
} bridge_record;

/* The severities of syslog, RFC 5424, by its numbers: 0 is the worst and 7 the most detail.
 * These four are the ones Bridge-ASM700 and its bindings name; a module may write any of the
 * eight. */
enum bridge_level : uint8_t { BRIDGE_ERR = 3, BRIDGE_WARN = 4, BRIDGE_INFO = 6, BRIDGE_DEBUG = 7 };
constexpr uint8_t BRIDGE_LEVEL_MOST = 7;

typedef void (*bridge_sink_fn)(void *ud, const bridge_record *r);

/* nullptr if the deployment cannot start; which reason, the sink was told. */
[[nodiscard]] bridge *bridge_create(const bridge_config *cfg, bridge_sink_fn sink, void *ud);
/* Every loaded module's shutdown, then everything else. 0 if the bridge is gone. With a pinned
 * module loaded it answers 1 from any thread but the one that loaded it, with a record, and
 * nothing has been torn down: the bridge is still there to be destroyed from the right one. */
[[nodiscard]] int bridge_destroy(bridge *b);

/* 0 if the module is loaded, its init answered 0, and the bridge can still run it. Otherwise
 * the load was refused or failed, or stopped the bridge, and a record says why. Between phases
 * only. */
[[nodiscard]] int bridge_load(bridge *b, const bridge_artifact *a);

/* Take a module out, by its identity: its shutdown, then the components nobody else declared,
 * each by the policy it was declared with. 0 if it was loaded. Between phases only. */
[[nodiscard]] int bridge_unload(bridge *b, const char *identity);

/* Replace a loaded module's instance and keep its storage. 0 if the new instance is running;
 * otherwise a record says why. The old instance still runs where its backend can hold two at
 * once; where it cannot, the module is stopped, with everything it declared, until it is
 * reloaded again or unloaded. Between phases only. */
[[nodiscard]] int bridge_reload(bridge *b, const bridge_artifact *a);

/* What is loaded, and whether it still runs: a module at its failure budget is stopped long
 * after any return code. Answers how many there are; fills at most `max`. */
enum bridge_state : uint8_t { BRIDGE_RUNNING = 1, BRIDGE_STOPPED = 2 };
typedef struct {
    const char       *identity;     /* until the next load, unload or reload */
    enum bridge_state state;
} bridge_module_info;
int bridge_modules(bridge *b, bridge_module_info *out, int max);

/* Run the systems registered to one phase, then its barrier. 0 if the phase ran, whatever
 * its systems did: a system that failed is a record, not a return code. With a pinned module
 * loaded, from the thread that loaded it. */
[[nodiscard]] int bridge_phase(bridge *b, const char *name);

#endif
