/* The host seam: the only header a host includes.
 * docs/seam/host-api/host-api.md is the contract. This is as much of it as exists so far:
 * everything but `validate` and the profile, which arrive with the diagnostics they switch on. */
#ifndef BRIDGE_H
#define BRIDGE_H

#include <stddef.h>
#include <stdint.h>

/* To a C++ compiler these are C: the names are not mangled, and a host in C++ links them. */
#ifdef __cplusplus
extern "C" {
#endif

typedef struct bridge bridge;

/* The host seam's number: what this header says. A host states the one it was built or bound
 * against, first in bridge_config, and a library that says another refuses to be created -
 * docs/seam/host-api/host-api.md, *A seam has a number*. */
constexpr uint32_t BRIDGE_HOST_ABI = 1;

/* A host may be in any language that can call C, so every struct here is laid out where it is
 * stated: its size and each member's offset are asserted, and no padding is left to a
 * compiler. Where alignment leaves a gap the gap is a member named `pad`, numbered where a
 * struct has several: whoever writes the struct writes zero there, and nobody reads it. A
 * pointer and a size_t are eight bytes: there is no build of this for any other. */
static_assert(sizeof(void *) == 8 && sizeof(size_t) == 8);

/* A backend the deployment configures: Bridge-ASM700 opens the library, and a module names
 * the backend that runs it. */
enum bridge_setting_kind : uint8_t {
    BRIDGE_BOOL = 1, BRIDGE_INT = 2, BRIDGE_FLOAT = 3, BRIDGE_STRING = 4
};

typedef struct {
    const char *key;
    enum bridge_setting_kind kind;              /* zero is unset */
    uint8_t pad[7];
    union { bool b; int64_t i; double f; const char *s; } v;
} bridge_setting;
static_assert(sizeof(bridge_setting) == 24 && alignof(bridge_setting) == 8);
static_assert(offsetof(bridge_setting, key) == 0 && offsetof(bridge_setting, kind) == 8);
static_assert(offsetof(bridge_setting, pad) == 9 && offsetof(bridge_setting, v) == 16);

typedef struct {
    const char *name;
    const char *library;
    /* the backend's own, passed to it unread: it refuses a key it does not know */
    const bridge_setting *settings; uint32_t setting_count;
    uint32_t pad;
} bridge_backend;
static_assert(sizeof(bridge_backend) == 32 && alignof(bridge_backend) == 8);
static_assert(offsetof(bridge_backend, name) == 0 && offsetof(bridge_backend, library) == 8);
static_assert(offsetof(bridge_backend, settings) == 16 && offsetof(bridge_backend, setting_count) == 24);
static_assert(offsetof(bridge_backend, pad) == 28);

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
    uint32_t pad;
    uint64_t persistent;      /* the persistent arena */
    uint64_t columns;         /* component columns and entity arrays */
} bridge_budget;
static_assert(sizeof(bridge_budget) == 48 && alignof(bridge_budget) == 8);
static_assert(offsetof(bridge_budget, entities) == 0 && offsetof(bridge_budget, tables) == 8);
static_assert(offsetof(bridge_budget, types) == 12 && offsetof(bridge_budget, systems) == 16);
static_assert(offsetof(bridge_budget, terms) == 20 && offsetof(bridge_budget, modules) == 24);
static_assert(offsetof(bridge_budget, pad) == 28 && offsetof(bridge_budget, persistent) == 32);
static_assert(offsetof(bridge_budget, columns) == 40);

/* Two systems that conflict in one phase, in the order they are to run. A system is named by
 * its module's identity without the version, then itself: "game:actors:movement". */
typedef struct { const char *before, *after; } bridge_order;
static_assert(sizeof(bridge_order) == 16 && alignof(bridge_order) == 8);
static_assert(offsetof(bridge_order, before) == 0 && offsetof(bridge_order, after) == 8);

typedef struct {
    uint32_t abi;                   /* BRIDGE_HOST_ABI. first, and always first */
    uint32_t pad2;
    const bridge_backend *backends; uint32_t backend_count; uint32_t pad0;
    const char *const *phases;      uint32_t phase_count;   /* a phase is a name, and no more */
    uint32_t pad1;
    const bridge_order *order;      uint32_t order_count;
    uint32_t threads;               /* how many run a phase, counting the caller's. zero is unset */
    bridge_budget budget;
} bridge_config;
static_assert(sizeof(bridge_config) == 104 && alignof(bridge_config) == 8);
static_assert(offsetof(bridge_config, abi) == 0 && offsetof(bridge_config, pad2) == 4);
static_assert(offsetof(bridge_config, backends) == 8 && offsetof(bridge_config, backend_count) == 16);
static_assert(offsetof(bridge_config, pad0) == 20 && offsetof(bridge_config, phases) == 24);
static_assert(offsetof(bridge_config, phase_count) == 32 && offsetof(bridge_config, pad1) == 36);
static_assert(offsetof(bridge_config, order) == 40 && offsetof(bridge_config, order_count) == 48);
static_assert(offsetof(bridge_config, threads) == 52 && offsetof(bridge_config, budget) == 56);

/* A phase a module names in its code, and the phase of this deployment that it is: a module
 * is written by somebody who never saw the deployment - docs/bridge/run/schedule.md. */
typedef struct { const char *from, *to; } bridge_phase_name;
static_assert(sizeof(bridge_phase_name) == 16 && alignof(bridge_phase_name) == 8);
static_assert(offsetof(bridge_phase_name, from) == 0 && offsetof(bridge_phase_name, to) == 8);

enum bridge_on_error : uint8_t { BRIDGE_FAIL = 1, BRIDGE_SKIP = 2 };    /* zero is unset */
/* A key that is yes or no. It is stated like any other: left at BRIDGE_UNSET it is refused, by
 * its name, and is not taken for false - docs/seam/host-api/config.md. */
enum bridge_flag : uint8_t { BRIDGE_UNSET = 0, BRIDGE_TRUE = 1, BRIDGE_FALSE = 2 };
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

    enum bridge_flag trusted;    /* its code needs no containing */
    enum bridge_flag reloadable; /* its author says it can be reloaded: from the manifest */
    enum bridge_flag hot_reload; /* the deployment allows bridge_reload. it only ever forbids */
    enum bridge_flag pinned;     /* its systems run on the thread that loaded it, and no other */
    enum bridge_on_error on_error;   /* what losing it costs the deployment */
    uint8_t  pad0[7];
    /* each of the next six: zero is unset, BRIDGE_NO_LIMIT is "no limit" */
    int64_t  max_system_failures;    /* each system is counted against it, alone */
    int64_t  work_limit;         /* work one system invocation may do, in the backend's units */
    int64_t  memory_limit;       /* bytes. a ceiling, never re-set */
    int64_t  init_timeout, reload_timeout, shutdown_timeout;    /* nanoseconds, per call */
    uint8_t  log_level;          /* a line above it is not written: enum bridge_level */
    uint8_t  pad1[3];
    uint32_t scratch, log_ring, spans;   /* its private memory, each in bytes */
    /* what is set aside for it alone, so that what it mints does not depend on who ran
     * beside it. each of the three: zero is unset, BRIDGE_NONE is "none" */
    int64_t  spawns;             /* entity ids it may take between two barriers */
    int64_t  persistent;         /* its share of the persistent arena, in bytes, for its life */
    int64_t  changes;            /* bytes of attach, detach and destroy it may queue between
                                  * two barriers: 24 for each, and an attach's value with it */

    const char *const *read;  uint32_t read_count;       /* grants */
    uint32_t pad2;
    const char *const *write; uint32_t write_count;
    uint32_t pad3;

    /* its own settings, and the component of its that they seed - docs/seam/host-api/config.md.
     * `config` is a component's name or BRIDGE_NO_CONFIG; null is unset */
    const char *config;
    const bridge_setting *settings; uint32_t setting_count;
    uint32_t pad4;
    /* the phases it names that this deployment calls otherwise. A name that is not here is
     * looked for among the deployment's own, as it is written */
    const bridge_phase_name *phases; uint32_t phase_count;
    uint32_t pad5;
} bridge_module_config;
#define BRIDGE_AT(member, at) static_assert(offsetof(bridge_module_config, member) == at)
static_assert(sizeof(bridge_module_config) == 184 && alignof(bridge_module_config) == 8);
BRIDGE_AT(identity, 0);    BRIDGE_AT(abi, 8);            BRIDGE_AT(trusted, 12);
BRIDGE_AT(reloadable, 13); BRIDGE_AT(hot_reload, 14);    BRIDGE_AT(pinned, 15);
BRIDGE_AT(on_error, 16);   BRIDGE_AT(pad0, 17);          BRIDGE_AT(max_system_failures, 24);
BRIDGE_AT(work_limit, 32); BRIDGE_AT(memory_limit, 40);  BRIDGE_AT(init_timeout, 48);
BRIDGE_AT(reload_timeout, 56); BRIDGE_AT(shutdown_timeout, 64); BRIDGE_AT(log_level, 72);
BRIDGE_AT(pad1, 73);       BRIDGE_AT(scratch, 76);       BRIDGE_AT(log_ring, 80);
BRIDGE_AT(spans, 84);      BRIDGE_AT(spawns, 88);        BRIDGE_AT(persistent, 96);
BRIDGE_AT(changes, 104);   BRIDGE_AT(read, 112);         BRIDGE_AT(read_count, 120);
BRIDGE_AT(pad2, 124);      BRIDGE_AT(write, 128);        BRIDGE_AT(write_count, 136);
BRIDGE_AT(pad3, 140);      BRIDGE_AT(config, 144);       BRIDGE_AT(settings, 152);
BRIDGE_AT(setting_count, 160); BRIDGE_AT(pad4, 164);     BRIDGE_AT(phases, 168);
BRIDGE_AT(phase_count, 176); BRIDGE_AT(pad5, 180);
#undef BRIDGE_AT

typedef struct {
    bridge_module_config cfg;
    const char *backend;             /* which backend runs it, by name */
    const void *bytes; size_t len;   /* the image, or null */
    const char *path;                /* where it is, or null. both go to the backend */
} bridge_artifact;
static_assert(sizeof(bridge_artifact) == 216 && alignof(bridge_artifact) == 8);
static_assert(offsetof(bridge_artifact, cfg) == 0 && offsetof(bridge_artifact, backend) == 184);
static_assert(offsetof(bridge_artifact, bytes) == 192 && offsetof(bridge_artifact, len) == 200);
static_assert(offsetof(bridge_artifact, path) == 208);

/* what comes back out */
typedef struct {
    const char *identity;            /* null for Bridge-ASM700's own records */
    const char *system;              /* null outside a system */
    const char *text;                /* THIS CALL ONLY */
    uint32_t    len;
    uint32_t    when;                /* barrier count, then offset in the phase */
    uint32_t    arg[2];
    uint8_t     level;               /* lower is worse: enum bridge_level */
    uint8_t     pad[7];
} bridge_record;
static_assert(sizeof(bridge_record) == 48 && alignof(bridge_record) == 8);
static_assert(offsetof(bridge_record, identity) == 0 && offsetof(bridge_record, system) == 8);
static_assert(offsetof(bridge_record, text) == 16 && offsetof(bridge_record, len) == 24);
static_assert(offsetof(bridge_record, when) == 28 && offsetof(bridge_record, arg) == 32);
static_assert(offsetof(bridge_record, level) == 40 && offsetof(bridge_record, pad) == 41);

/* The severities of syslog, RFC 5424, by its numbers: 0 is the worst and 7 the most detail.
 * These four are the ones Bridge-ASM700 and its bindings name; a module may write any of the
 * eight. */
enum bridge_level : uint8_t { BRIDGE_ERR = 3, BRIDGE_WARN = 4, BRIDGE_INFO = 6, BRIDGE_DEBUG = 7 };
constexpr uint8_t BRIDGE_LEVEL_MOST = 7;

typedef void (*bridge_sink_fn)(void *ud, const bridge_record *r);

/* The functions below are the whole of what the library exports: a host links it built, as
 * a shared library or a static one, and reaches nothing else of it - docs/seam/host-api/
 * host-api.md, *A host links a built library*. BRIDGE_BUILDING is the library's own build. */
#if defined(BRIDGE_BUILDING) && defined(_WIN32)
#define BRIDGE_API [[gnu::dllexport]]
#elif defined(BRIDGE_BUILDING)
#define BRIDGE_API [[gnu::visibility("default")]]
#else
#define BRIDGE_API
#endif

/* nullptr if the deployment cannot start; which reason, the sink was told. */
BRIDGE_API [[nodiscard]] bridge *bridge_create(const bridge_config *cfg, bridge_sink_fn sink, void *ud);
/* Every loaded module's shutdown, then everything else. 0 if the bridge is gone. With a pinned
 * module loaded it answers 1 from any thread but the one that loaded it, with a record, and
 * nothing has been torn down: the bridge is still there to be destroyed from the right one. */
BRIDGE_API [[nodiscard]] int bridge_destroy(bridge *b);

/* 0 if the module is loaded, its init answered 0, and the bridge can still run it. Otherwise
 * the load was refused or failed, or stopped the bridge, and a record says why. Between phases
 * only. */
BRIDGE_API [[nodiscard]] int bridge_load(bridge *b, const bridge_artifact *a);

/* Take a module out, by its identity: its shutdown, then the components nobody else declared,
 * each by the policy it was declared with. 0 if it was loaded. Between phases only. */
BRIDGE_API [[nodiscard]] int bridge_unload(bridge *b, const char *identity);

/* Replace a loaded module's instance and keep its storage. 0 if the new instance is running;
 * otherwise a record says why. The old instance still runs where its backend can hold two at
 * once; where it cannot, the module is stopped, with everything it declared, until it is
 * reloaded again or unloaded. Between phases only. */
BRIDGE_API [[nodiscard]] int bridge_reload(bridge *b, const bridge_artifact *a);

/* What is loaded, and whether it still runs: a module at its failure budget is stopped long
 * after any return code. Answers how many there are; fills at most `max`. */
enum bridge_state : uint8_t { BRIDGE_RUNNING = 1, BRIDGE_STOPPED = 2 };
typedef struct {
    const char       *identity;     /* until the next load, unload or reload */
    enum bridge_state state;
    uint8_t           pad[7];
} bridge_module_info;
static_assert(sizeof(bridge_module_info) == 16 && alignof(bridge_module_info) == 8);
static_assert(offsetof(bridge_module_info, identity) == 0 && offsetof(bridge_module_info, state) == 8);
static_assert(offsetof(bridge_module_info, pad) == 9);
BRIDGE_API int bridge_modules(bridge *b, bridge_module_info *out, int max);

/* Run the systems registered to one phase, then its barrier. 0 if the phase ran, whatever
 * its systems did: a system that failed is a record, not a return code. With a pinned module
 * loaded, from the thread that loaded it. */
BRIDGE_API [[nodiscard]] int bridge_phase(bridge *b, const char *name);

#ifdef __cplusplus
}
#endif

#endif
