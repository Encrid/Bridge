/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_INSIDE_H
    #define BRIDGE_SRC_INSIDE_H

    #include "bridge.h"
    #include "bridge_abi.h"
    #include "backend.h"
    #include "os.h"
    #include "pool.h"
    #include "region.h"
    #include "schema.h"
    #include "storage.h"

typedef struct module module_t;
typedef struct declared_type declared_type_t;

typedef struct {
    char *name;
    void *lib;
    backend_api_t api;
    backend_t *be;
    enum backend_capability capabilities;
    backend_memory_t *shared;
    uint64_t reservation;
    uint64_t guard;
    uint64_t granule;
} open_backend_t;

struct declared_type {
    char *name;
    storage_type_t *type;
    schema_t s;
    enum bridge_kind kind;
    enum bridge_storage storage;
    uint32_t size;
    uint32_t align;
    uint64_t fingerprint;
    uint32_t declarers;
    uint32_t users;
    declared_type_t **deps;
    uint32_t dep_count;
    void *zero;
    declared_type_t *replaces;
    bool superseded;
    storage_run_t *runs;
    uint32_t run_count;
};

typedef struct {
    uint64_t id;
    enum bridge_event events;
} pending_t;

typedef struct {
    declared_type_t *touches;
    bool writes;
} system_term_t;

typedef struct {
    module_t *m;
    char *name;
    char *full;
    uint32_t phase;
    storage_terms_t *matched;
    storage_term_t *asked;
    enum bridge_event events;
    uint32_t terms;
    backend_func_t *fn;
    uint32_t level;
    int parked;
    int observer;
    pending_t *pending;
    uint32_t pending_count;
    uint32_t pending_room;
    uint32_t place;
    uint64_t turn;
    int failed;
    char why[256];
    int64_t failures;
    declared_type_t **changes;
    uint32_t change_count;
    system_term_t term[];
} bridge_system_t;

typedef struct {
    uint64_t turn;
    uint32_t at;
} module_mark_t;

struct module {
    bridge_t *b;
    char *identity;
    char *ns;
    char *stem;
    open_backend_t *on;
    backend_module_t *image;
    backend_instance_t *instance;
    backend_memory_t *memory;
    uint8_t *priv;
    size_t priv_len;
    size_t priv_reserve;
    uint32_t scratch;
    uint32_t scratch_len;
    uint32_t ring;
    uint32_t ring_len;
    uint32_t ring_tail;
    uint32_t spans;
    uint32_t spans_len;
    bool published;
    bool had_triggers;
    uint32_t spans_end;
    declared_type_t **declared;
    enum bridge_on_unload *policy;
    bool *may_write;
    uint32_t declared_count;
    bridge_system_t **systems;
    uint32_t system_count;
    bridge_system_t **observers;
    uint32_t observer_count;
    int64_t work_limit;
    uint64_t *ids;
    uint32_t id_count;
    uint32_t id_next;
    uint32_t id_room;
    int ids_short;
    int64_t spawns;
    int64_t persistent;
    uint32_t arena_at;
    uint32_t arena_left;
    uint32_t arena_share;
    range_t *owned;
    uint32_t own_count;
    uint32_t own_next;
    uint8_t *queue;
    uint32_t queue_len;
    uint32_t queue_room;
    char **read;
    char **write;
    uint32_t read_count;
    uint32_t write_count;
    char **phase_from;
    uint32_t *phase_to;
    uint32_t phase_names;
    int pinned;
    uint64_t thread;
    int hot_reload;
    enum bridge_on_error on_error;
    backend_func_t *shutdown;
    int64_t shutdown_timeout;
    int64_t max_failures;
    int64_t failures;
    int changed_now;
    int unplaced_now;
    const void *reached;
    uint32_t left[2];
    uint64_t *writable;
    uint32_t barred;
    declared_type_t *config;
    uint32_t text_at;
    uint32_t text_room;
    int in_init;
    int stopped;
    const void *running;
    uint64_t turn;
    module_mark_t *marks;
    uint32_t mark_count;
    uint32_t mark_room;
    uint32_t mark_at;
    module_t *reloads;
    const char *const *losses;
    uint32_t loss_count;
    uint32_t *loss_rows;
    int refused;
    char why[256];
    module_t *next;
};

enum change_op : uint32_t {
    CHANGE_DESTROY = 1,
    CHANGE_ATTACH = 2,
    CHANGE_DETACH = 3
};

typedef struct {
    uint64_t id;
    declared_type_t *c;
    uint32_t size;
    enum change_op op;
} change_t;

constexpr uint64_t TURN_LIFECYCLE = 1ull << 32;
constexpr uint64_t TURN_SYSTEM = 2ull << 32;

typedef struct {
    char *stem;
    uint32_t at;
    uint32_t room;
} kept_text_t;

struct bridge {
    bridge_sink_fn_t sink;
    void *ud;
    bool in_sink;
    open_backend_t *backends;
    uint32_t backend_count;
    uint64_t reservation;
    uint64_t guard;
    uint64_t granule;
    char *stopped_by;
    char *stopped_stem;
    char stopped_why[256];
    char stopped_name[256];
    char **phases;
    uint32_t phase_count;
    uint32_t barriers;
    uint32_t turns;
    uint32_t parked;
    uint32_t arena_ranges;
    void *run_jobs;
    void **run_handed;
    void **run_lists;
    uint8_t *run_callers;
    void **build_in;
    uint64_t *build_bits;
    char **before;
    char **after;
    uint32_t order_count;
    pool_t *pool;
    uint8_t *shared;
    size_t shared_len;
    size_t shared_reserve;
    bridge_entity_t *index;
    uint32_t entities;
    uint32_t arena;
    uint32_t arena_len;
    uint32_t columns;
    uint32_t columns_len;
    uint32_t sparse_sets;
    uint32_t sparse;
    uint32_t direct_indexed;
    uint32_t direct;
    region_t arena_space;
    region_t column_space;
    int layout_moved;
    uint32_t tables;
    uint32_t max_types;
    uint32_t max_systems;
    uint32_t max_terms;
    storage_t *storage;
    declared_type_t **types;
    uint32_t type_count;
    kept_text_t *kept;
    uint32_t kept_count;
    bridge_system_t **systems;
    uint32_t system_count;
    module_t *modules;
    uint32_t module_count;
    uint32_t max_modules;
};

void module_discard(bridge_t *b, module_t *m);

void module_shut_down(bridge_t *b, module_t *m);

    #include "say.h"
    #include "schedule.h"
    #include "bridge_inside.h"
    #include "block.h"
    #include "abi.h"

#endif
