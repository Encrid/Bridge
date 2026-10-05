/* The host seam: the only header a host includes.
 * docs/seam/host-api/host-api.md is the contract. This is as much of it as exists so far:
 * everything but the profile, which arrives with what it measures. */
#ifndef BRIDGE_H
#define BRIDGE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct bridge bridge;

/* A backend the deployment configures: Bridge-ASM700 opens the library, and a module names
 * the backend that runs it. */
typedef struct {
    const char *key;
    uint8_t     kind;                           /* BRIDGE_BOOL | INT | FLOAT | STRING */
    union { bool b; int64_t i; double f; const char *s; } v;
} bridge_setting;

enum { BRIDGE_BOOL = 1, BRIDGE_INT = 2, BRIDGE_FLOAT = 3, BRIDGE_STRING = 4 };

typedef struct {
    const char *name;
    const char *library;
    /* the backend's own, passed to it unread: it refuses a key it does not know */
    const bridge_setting *settings; uint32_t setting_count;
} bridge_backend;

/* Every one is stated: zero is unset, and refused. docs/bridge/memory/budgets.md. */
typedef struct {
    uint64_t entities;        /* the index mirror ceiling */
    uint64_t persistent;      /* the persistent arena, in bytes */
    uint64_t columns;         /* component columns and entity arrays, in bytes */
    uint32_t max_modules;     /* how many modules are loaded at once */
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

enum { BRIDGE_FAIL = 1, BRIDGE_SKIP = 2 };      /* on_error. zero is unset */
#define BRIDGE_NO_LIMIT (-1)                    /* a limit stated as none */
#define BRIDGE_NONE     (-1)                    /* a reservation stated as none */

typedef struct {
    const char *identity;        /* "tri:triangle@0.1.0" */
    uint32_t    abi;

    bool     trusted;
    bool     reloadable;         /* its author says it can be reloaded: from the manifest */
    bool     hot_reload;         /* the deployment allows bridge_reload. it only ever forbids */
    bool     pinned;             /* its systems run on the thread that loaded it, and no other */
    uint8_t  on_error;           /* BRIDGE_FAIL | BRIDGE_SKIP: what losing it costs the deployment */
    /* each of the next six: zero is unset, BRIDGE_NO_LIMIT is "no limit" */
    int64_t  max_system_failures;    /* raised by a failed barrier, lowered by a clean one */
    int64_t  fuel;               /* units per system invocation */
    int64_t  memory_limit;       /* bytes. a ceiling, never re-set */
    int64_t  init_timeout, reload_timeout, shutdown_timeout;    /* nanoseconds, per call */
    int8_t   log_level;          /* a line above it is not written */
    uint32_t scratch, log_ring, spans;   /* its private memory, each in bytes */
    /* what is set aside for it alone, so that what it mints does not depend on who ran
     * beside it. each: zero is unset, BRIDGE_NONE is "it mints none" */
    int64_t  spawns;             /* entity ids it may take between two barriers */
    int64_t  persistent;         /* its share of the persistent arena, in bytes, for its life */

    const char *const *read;  uint32_t read_count;       /* grants */
    const char *const *write; uint32_t write_count;

    /* its own settings, and the component of its that they seed - docs/seam/host-api/config.md */
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
    int8_t      level;               /* negative is worse - flecs' convention */
} bridge_record;

enum { BRIDGE_WARN = -2, BRIDGE_ERR = -3 };

typedef void (*bridge_sink_fn)(void *ud, const bridge_record *r);

/* NULL if the deployment cannot start; which reason, the sink was told. */
bridge *bridge_create(const bridge_config *cfg, bridge_sink_fn sink, void *ud);
void    bridge_destroy(bridge *b);

/* 0 if the module is loaded, its init answered 0, and the bridge can still run it. Otherwise
 * the load was refused or failed, or stopped the bridge, and a record says why. Between phases
 * only. */
int bridge_load(bridge *b, const bridge_artifact *a);

/* Take a module out, by its identity: its shutdown, then the components nobody else declared,
 * each by the policy it was declared with. 0 if it was loaded. Between phases only. */
int bridge_unload(bridge *b, const char *identity);

/* Replace a loaded module's instance and keep its storage. 0 if the new instance is running;
 * otherwise a record says why, and the old one still is wherever its backend could keep it.
 * Between phases only. */
int bridge_reload(bridge *b, const bridge_artifact *a);

/* What is loaded, and whether it still runs: a module at its failure budget is stopped long
 * after any return code. Answers how many there are; fills at most `max`. */
enum { BRIDGE_RUNNING = 1, BRIDGE_STOPPED = 2 };
typedef struct {
    const char *identity;       /* until the next load, unload or reload */
    uint8_t     state;
} bridge_module_info;
int bridge_modules(bridge *b, bridge_module_info *out, int max);

/* Run the systems registered to one phase, then its barrier. 0 if the phase ran, whatever
 * its systems did: a system that failed is a record, not a return code. With a pinned module
 * loaded, from the thread that loaded it. */
int bridge_phase(bridge *b, const char *name);

#endif
