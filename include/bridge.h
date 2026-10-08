/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** include/bridge.h -- the host seam: the only header a host includes.
**
** What is compiled with this file is a work based on Bridge and is under
** the same licence. Other terms: docs/licence.md.
**
** This is the contract as far as it is built: all of it but the two keys of
** validation.md, *The checksum pass, when asked*, and the pull of
** profile.md, *The profile*.
**
** A host may be in any language that can call C, so every struct here is laid
** out where it is stated: its size and each member's offset are asserted, and
** no padding is left to a compiler. Where alignment leaves a gap the gap is a
** member named `pad`, numbered where a struct has several: whoever writes the
** struct writes zero there, and nobody reads it. A pointer and a size_t are
** eight bytes: there is no build of this for any other.
**
** host_api.md, *A host links a built library*
*/

#ifndef BRIDGE_H
    #define BRIDGE_H

    #include <stddef.h>
    #include <stdint.h>

/*
** To a C++ compiler these are C: the names are not mangled, and a host in C++
** links them.
*/
    #ifdef __cplusplus
extern "C" {
    #endif

/*
** The functions at the end of this file are the whole of what the library
** exports: a host links it built, as a shared library or a static one, and
** reaches nothing else of it. BRIDGE_BUILDING is the library's own build.
*/
    #if defined(BRIDGE_BUILDING) && defined(_WIN32)
        #define BRIDGE_API [[gnu::dllexport]]
    #elif defined(BRIDGE_BUILDING)
        #define BRIDGE_API [[gnu::visibility("default")]]
    #else
        #define BRIDGE_API
    #endif

typedef struct bridge bridge_t;

/*
** The host seam's number: what this header says. A host states the one it was
** built or bound against, first in bridge_config_t, and a library that says
** another refuses to be created.
**
** host_api.md, *A seam has a number*
*/
constexpr uint32_t BRIDGE_HOST_ABI = 1;

static_assert(sizeof(void *) == 8);
static_assert(sizeof(size_t) == 8);

/*
** A setting: a key, and a value of one of four kinds. Zero in `kind` is unset.
**
** host_api.md, *The data*
*/
enum bridge_setting_kind : uint8_t {
    BRIDGE_BOOL = 1,
    BRIDGE_INT = 2,
    BRIDGE_FLOAT = 3,
    BRIDGE_STRING = 4
};

typedef union {
    bool b;
    int64_t i;
    double f;
    const char *s;
} bridge_value_t;

typedef struct {
    const char *key;
    enum bridge_setting_kind kind;
    uint8_t pad[7];
    bridge_value_t v;
} bridge_setting_t;
static_assert(sizeof(bridge_setting_t) == 24);
static_assert(alignof(bridge_setting_t) == 8);
static_assert(offsetof(bridge_setting_t, key) == 0);
static_assert(offsetof(bridge_setting_t, kind) == 8);
static_assert(offsetof(bridge_setting_t, pad) == 9);
static_assert(offsetof(bridge_setting_t, v) == 16);

/*
** A backend the deployment configures: Bridge opens the library, and a
** module names the backend that runs it. `settings` are the backend's own,
** passed to it unread: it refuses a key it does not know.
**
** config.md, *The keys*
*/
typedef struct {
    const char *name;
    const char *library;
    const bridge_setting_t *settings;
    uint32_t setting_count;
    uint32_t pad;
} bridge_backend_t;
static_assert(sizeof(bridge_backend_t) == 32);
static_assert(alignof(bridge_backend_t) == 8);
static_assert(offsetof(bridge_backend_t, name) == 0);
static_assert(offsetof(bridge_backend_t, library) == 8);
static_assert(offsetof(bridge_backend_t, settings) == 16);
static_assert(offsetof(bridge_backend_t, setting_count) == 24);
static_assert(offsetof(bridge_backend_t, pad) == 28);

/*
** How many there may be at once of each thing, and then how many bytes. Every
** one is stated: zero is unset, and refused.
**
**     entities          entity indices: index 0 is none, so one fewer entities
**     tables            archetypes: the sets of components entities carry
**     types             declared types: components, and the structs, enums
**                       and bitmasks their fields name
**     systems           systems and observers registered
**     terms             terms one of them may have
**     modules           modules loaded
**     persistent        the persistent arena
**     columns           component columns and entity arrays
**     sparse_sets, direct_indexed
**                       how many components may be kept in each of the two
**                       storage layouts that hold something for every entity
**                       there may be. Each is a count, or BRIDGE_NONE
**
** budgets.md, *Budgets*
*/
typedef struct {
    uint64_t entities;
    uint32_t tables;
    uint32_t types;
    uint32_t systems;
    uint32_t terms;
    uint32_t modules;
    uint32_t pad;
    uint64_t persistent;
    uint64_t columns;
    int64_t sparse_sets;
    int64_t direct_indexed;
} bridge_budget_t;
static_assert(sizeof(bridge_budget_t) == 64);
static_assert(alignof(bridge_budget_t) == 8);
static_assert(offsetof(bridge_budget_t, entities) == 0);
static_assert(offsetof(bridge_budget_t, tables) == 8);
static_assert(offsetof(bridge_budget_t, types) == 12);
static_assert(offsetof(bridge_budget_t, systems) == 16);
static_assert(offsetof(bridge_budget_t, terms) == 20);
static_assert(offsetof(bridge_budget_t, modules) == 24);
static_assert(offsetof(bridge_budget_t, pad) == 28);
static_assert(offsetof(bridge_budget_t, persistent) == 32);
static_assert(offsetof(bridge_budget_t, columns) == 40);
static_assert(offsetof(bridge_budget_t, sparse_sets) == 48);
static_assert(offsetof(bridge_budget_t, direct_indexed) == 56);

/*
** Two systems that conflict in one phase, in the order they are to run. A
** system is named by its module's identity without the version, then itself:
** "game:actors:movement".
**
** schedule.md, *Order within a phase is never implicit*
*/
typedef struct {
    const char *before;
    const char *after;
} bridge_order_t;
static_assert(sizeof(bridge_order_t) == 16);
static_assert(alignof(bridge_order_t) == 8);
static_assert(offsetof(bridge_order_t, before) == 0);
static_assert(offsetof(bridge_order_t, after) == 8);

/*
** What a deployment is.
**
**     abi        BRIDGE_HOST_ABI. First, and always first
**     backends   one at least: a module is run by a backend
**     phases     a phase is a name, and no more
**     threads    how many run a phase, counting the caller's. Zero is unset
**
** config.md, *The keys*
*/
typedef struct {
    uint32_t abi;
    uint32_t pad2;
    const bridge_backend_t *backends;
    uint32_t backend_count;
    uint32_t pad0;
    const char *const *phases;
    uint32_t phase_count;
    uint32_t pad1;
    const bridge_order_t *order;
    uint32_t order_count;
    uint32_t threads;
    bridge_budget_t budget;
} bridge_config_t;
static_assert(sizeof(bridge_config_t) == 120);
static_assert(alignof(bridge_config_t) == 8);
static_assert(offsetof(bridge_config_t, abi) == 0);
static_assert(offsetof(bridge_config_t, pad2) == 4);
static_assert(offsetof(bridge_config_t, backends) == 8);
static_assert(offsetof(bridge_config_t, backend_count) == 16);
static_assert(offsetof(bridge_config_t, pad0) == 20);
static_assert(offsetof(bridge_config_t, phases) == 24);
static_assert(offsetof(bridge_config_t, phase_count) == 32);
static_assert(offsetof(bridge_config_t, pad1) == 36);
static_assert(offsetof(bridge_config_t, order) == 40);
static_assert(offsetof(bridge_config_t, order_count) == 48);
static_assert(offsetof(bridge_config_t, threads) == 52);
static_assert(offsetof(bridge_config_t, budget) == 56);

/*
** A phase a module names in its code, and the phase of this deployment that
** it is: a module is written by somebody who never saw the deployment.
**
** schedule.md, *A module's phase names are mapped by the deployment*
*/
typedef struct {
    const char *from;
    const char *to;
} bridge_phase_name_t;
static_assert(sizeof(bridge_phase_name_t) == 16);
static_assert(alignof(bridge_phase_name_t) == 8);
static_assert(offsetof(bridge_phase_name_t, from) == 0);
static_assert(offsetof(bridge_phase_name_t, to) == 8);

/*
** What losing a module costs the deployment. Zero is unset.
**
** failure.md, *When something fails*
*/
enum bridge_on_error : uint8_t {
    BRIDGE_FAIL = 1,
    BRIDGE_SKIP = 2
};

/*
** A key that is yes or no. It is stated like any other: left at BRIDGE_UNSET
** it is refused, by its name, and is not taken for false.
**
** config.md, *No unaccountable values*
*/
enum bridge_flag : uint8_t {
    BRIDGE_UNSET = 0,
    BRIDGE_TRUE = 1,
    BRIDGE_FALSE = 2
};

/*
** A limit stated as none, and a reservation stated as none. Two values and
** not one, because they say opposite things: a module with no limit may use
** all there is, and a module with no reservation gets nothing. Each field of
** a module's configuration takes one of them, and the other, like any other
** negative number, is refused at load.
**
** config.md, *No unaccountable values*
*/
constexpr int64_t BRIDGE_NO_LIMIT = -1;
constexpr int64_t BRIDGE_NONE = -2;

/*
** A module with no settings component says so. No component is named without
** a colon, so this names none of them; a config left null is unset, and
** refused.
**
** config.md, *A module's own settings are a component*
*/
constexpr char BRIDGE_NO_CONFIG[] = "none";

/*
** What a module is to this deployment.
**
**     identity      "tri:triangle@0.1.0"
**     trusted       its code needs no containing
**     reloadable    its author says it can be reloaded: from the manifest
**     hot_reload    the deployment allows bridge_reload. It only ever forbids
**     pinned        its systems run on the thread that loaded it, and no
**                   other
**     on_error      what losing it costs the deployment
**
** Each of the next six: zero is unset, BRIDGE_NO_LIMIT is "no limit".
**
**     max_system_failures
**                   each system is counted against it, alone
**     work_limit    work one system invocation may do, in the backend's units
**     memory_limit  bytes. A ceiling, never re-set
**     init_timeout, reload_timeout, shutdown_timeout
**                   nanoseconds, per call
**
**     log_level     a line above it is not written: enum bridge_level, or any
**                   of 1 to 7. Zero is unset
**     scratch, log_ring, spans
**                   its private memory, each in bytes
**
** What is set aside for it alone, so that what it mints does not depend on
** who ran beside it. Each of the three: zero is unset, BRIDGE_NONE is "none".
**
**     spawns        entity ids it may take between two barriers
**     persistent    its share of the persistent arena, in bytes, for its life
**     changes       bytes of attach, detach and destroy it may queue between
**                   two barriers: 24 for each, and an attach's value with it
**
**     read, write   grants
**     config        the component of its own that its settings seed: a
**                   component's name or BRIDGE_NO_CONFIG; null is unset
**     settings      its own settings
**     phases        the phases it names that this deployment calls
**                   otherwise. A name that is not here is looked for among
**                   the deployment's own, as it is written
**     losses        what a reload accepts to lose, each by its name:
**                   'game:Position.z' for a field that goes, 'game:Shield'
**                   for the rows of a toggle that are off. A name that is
**                   here and is not lost is refused, as one that is lost and
**                   is not here
**     loss_count    how many, or BRIDGE_NONE for none: zero is unset
**
** lifetime.md, *When a component layout changed*
*/
typedef struct {
    const char *identity;
    uint32_t abi;
    enum bridge_flag trusted;
    enum bridge_flag reloadable;
    enum bridge_flag hot_reload;
    enum bridge_flag pinned;
    enum bridge_on_error on_error;
    uint8_t pad0[7];
    int64_t max_system_failures;
    int64_t work_limit;
    int64_t memory_limit;
    int64_t init_timeout;
    int64_t reload_timeout;
    int64_t shutdown_timeout;
    uint8_t log_level;
    uint8_t pad1[3];
    uint32_t scratch;
    uint32_t log_ring;
    uint32_t spans;
    int64_t spawns;
    int64_t persistent;
    int64_t changes;
    const char *const *read;
    uint32_t read_count;
    uint32_t pad2;
    const char *const *write;
    uint32_t write_count;
    uint32_t pad3;
    const char *config;
    const bridge_setting_t *settings;
    uint32_t setting_count;
    uint32_t pad4;
    const bridge_phase_name_t *phases;
    uint32_t phase_count;
    uint32_t pad5;
    const char *const *losses;
    int64_t loss_count;
} bridge_module_config_t;
static_assert(sizeof(bridge_module_config_t) == 200);
static_assert(alignof(bridge_module_config_t) == 8);
static_assert(offsetof(bridge_module_config_t, identity) == 0);
static_assert(offsetof(bridge_module_config_t, abi) == 8);
static_assert(offsetof(bridge_module_config_t, trusted) == 12);
static_assert(offsetof(bridge_module_config_t, reloadable) == 13);
static_assert(offsetof(bridge_module_config_t, hot_reload) == 14);
static_assert(offsetof(bridge_module_config_t, pinned) == 15);
static_assert(offsetof(bridge_module_config_t, on_error) == 16);
static_assert(offsetof(bridge_module_config_t, pad0) == 17);
static_assert(offsetof(bridge_module_config_t, max_system_failures) == 24);
static_assert(offsetof(bridge_module_config_t, work_limit) == 32);
static_assert(offsetof(bridge_module_config_t, memory_limit) == 40);
static_assert(offsetof(bridge_module_config_t, init_timeout) == 48);
static_assert(offsetof(bridge_module_config_t, reload_timeout) == 56);
static_assert(offsetof(bridge_module_config_t, shutdown_timeout) == 64);
static_assert(offsetof(bridge_module_config_t, log_level) == 72);
static_assert(offsetof(bridge_module_config_t, pad1) == 73);
static_assert(offsetof(bridge_module_config_t, scratch) == 76);
static_assert(offsetof(bridge_module_config_t, log_ring) == 80);
static_assert(offsetof(bridge_module_config_t, spans) == 84);
static_assert(offsetof(bridge_module_config_t, spawns) == 88);
static_assert(offsetof(bridge_module_config_t, persistent) == 96);
static_assert(offsetof(bridge_module_config_t, changes) == 104);
static_assert(offsetof(bridge_module_config_t, read) == 112);
static_assert(offsetof(bridge_module_config_t, read_count) == 120);
static_assert(offsetof(bridge_module_config_t, pad2) == 124);
static_assert(offsetof(bridge_module_config_t, write) == 128);
static_assert(offsetof(bridge_module_config_t, write_count) == 136);
static_assert(offsetof(bridge_module_config_t, pad3) == 140);
static_assert(offsetof(bridge_module_config_t, config) == 144);
static_assert(offsetof(bridge_module_config_t, settings) == 152);
static_assert(offsetof(bridge_module_config_t, setting_count) == 160);
static_assert(offsetof(bridge_module_config_t, pad4) == 164);
static_assert(offsetof(bridge_module_config_t, phases) == 168);
static_assert(offsetof(bridge_module_config_t, phase_count) == 176);
static_assert(offsetof(bridge_module_config_t, pad5) == 180);
static_assert(offsetof(bridge_module_config_t, losses) == 184);
static_assert(offsetof(bridge_module_config_t, loss_count) == 192);

/*
** A module to load.
**
**     backend       which backend runs it, by name
**     bytes, len    the image, or null
**     path          where it is, or null. Both go to the backend
**
** lifecycle.md, *The runtime lifecycle*
*/
typedef struct {
    bridge_module_config_t cfg;
    const char *backend;
    const void *bytes;
    size_t len;
    const char *path;
} bridge_artifact_t;
static_assert(sizeof(bridge_artifact_t) == 232);
static_assert(alignof(bridge_artifact_t) == 8);
static_assert(offsetof(bridge_artifact_t, cfg) == 0);
static_assert(offsetof(bridge_artifact_t, backend) == 200);
static_assert(offsetof(bridge_artifact_t, bytes) == 208);
static_assert(offsetof(bridge_artifact_t, len) == 216);
static_assert(offsetof(bridge_artifact_t, path) == 224);

/*
** What comes back out.
**
**     identity    null for Bridge's own records
**     system      null outside a system
**     text        this call only
**     when        barrier count, then offset in the phase
**     level       lower is worse: enum bridge_level
**
** create.md, *The sink*
*/
typedef struct {
    const char *identity;
    const char *system;
    const char *text;
    uint32_t len;
    uint32_t when;
    uint32_t arg[2];
    uint8_t level;
    uint8_t pad[7];
} bridge_record_t;
static_assert(sizeof(bridge_record_t) == 48);
static_assert(alignof(bridge_record_t) == 8);
static_assert(offsetof(bridge_record_t, identity) == 0);
static_assert(offsetof(bridge_record_t, system) == 8);
static_assert(offsetof(bridge_record_t, text) == 16);
static_assert(offsetof(bridge_record_t, len) == 24);
static_assert(offsetof(bridge_record_t, when) == 28);
static_assert(offsetof(bridge_record_t, arg) == 32);
static_assert(offsetof(bridge_record_t, level) == 40);
static_assert(offsetof(bridge_record_t, pad) == 41);

/*
** The severities of syslog, RFC 5424, by its numbers: 0 is the worst and 7
** the most detail. These four are the ones Bridge and its bindings
** name; a module may write any of the eight.
**
** log.md, *The log*
*/
enum bridge_level : uint8_t {
    BRIDGE_ERR = 3,
    BRIDGE_WARN = 4,
    BRIDGE_INFO = 6,
    BRIDGE_DEBUG = 7
};

constexpr uint8_t BRIDGE_LEVEL_MOST = 7;

typedef void (*bridge_sink_fn_t)(void *ud, const bridge_record_t *r);

/*
** Whether a loaded module still runs: one at its failure budget is stopped
** long after any return code. `identity` holds until the next load, unload or
** reload.
**
** introspection.md, *Introspection*
*/
enum bridge_state : uint8_t {
    BRIDGE_RUNNING = 1,
    BRIDGE_STOPPED = 2
};

typedef struct {
    const char *identity;
    enum bridge_state state;
    uint8_t pad[7];
} bridge_module_info_t;
static_assert(sizeof(bridge_module_info_t) == 16);
static_assert(alignof(bridge_module_info_t) == 8);
static_assert(offsetof(bridge_module_info_t, identity) == 0);
static_assert(offsetof(bridge_module_info_t, state) == 8);
static_assert(offsetof(bridge_module_info_t, pad) == 9);

/*
** bridge_create
**
** Makes a deck of a deployment. nullptr if the deployment cannot start;
** which reason, the sink was told.
**
** create.md, *Create*
*/
BRIDGE_API [[nodiscard]] bridge_t *bridge_create(const bridge_config_t *cfg,
    bridge_sink_fn_t sink, void *ud);

/*
** bridge_destroy
**
** Every loaded module's shutdown, then everything else. 0 if the deck is
** gone. With a pinned module loaded it answers 1 from any thread but the one
** that loaded it, with a record, and nothing has been torn down: the deck is
** still there to be destroyed from the right one.
**
** create.md, *Destroy*
*/
BRIDGE_API [[nodiscard]] int bridge_destroy(bridge_t *b);

/*
** bridge_load
**
** 0 if the module is loaded, its init answered 0, and the deck can still run
** it. Otherwise the load was refused or failed, or stopped the deck, and a
** record says why. Between phases only.
**
** lifecycle.md, *The runtime lifecycle*
*/
BRIDGE_API [[nodiscard]] int bridge_load(bridge_t *b,
    const bridge_artifact_t *a);

/*
** bridge_unload
**
** Takes a module out, by its identity: its shutdown, then the components
** nobody else declared, each by the policy it was declared with. 0 if it was
** loaded. Between phases only.
**
** lifecycle.md, *The runtime lifecycle*
*/
BRIDGE_API [[nodiscard]] int bridge_unload(bridge_t *b, const char *identity);

/*
** bridge_reload
**
** Replaces a loaded module's instance and keeps its storage. 0 if the new
** instance is running; otherwise a record says why. The old instance still
** runs where its backend can hold two at once; where it cannot, the module is
** stopped, with everything it declared, until it is reloaded again or
** unloaded. Between phases only.
**
** lifecycle.md, *The runtime lifecycle*
*/
BRIDGE_API [[nodiscard]] int bridge_reload(bridge_t *b,
    const bridge_artifact_t *a);

/*
** bridge_list_modules
**
** What is loaded, and whether it still runs. Answers how many there are;
** fills at most `max`.
**
** introspection.md, *Introspection*
*/
BRIDGE_API int bridge_list_modules(bridge_t *b, bridge_module_info_t *out,
    int max);

/*
** bridge_run_phase
**
** Runs the systems registered to one phase, then its barrier. 0 if the phase
** ran, whatever its systems did: a system that failed is a record, not a
** return code. With a pinned module loaded, from the thread that loaded it.
**
** loop.md, *The host owns the loop, and the order within it*
*/
BRIDGE_API [[nodiscard]] int bridge_run_phase(bridge_t *b, const char *name);

    #ifdef __cplusplus
}
    #endif

#endif
