/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_H
    #define BRIDGE_H

    #include <stddef.h>
    #include <stdint.h>

    #ifdef __cplusplus
extern "C" {
    #endif

    #if defined(BRIDGE_BUILDING) && defined(_WIN32)
        #define BRIDGE_API [[gnu::dllexport]]
    #elif defined(BRIDGE_BUILDING)
        #define BRIDGE_API [[gnu::visibility("default")]]
    #else
        #define BRIDGE_API
    #endif

typedef struct bridge bridge_t;

constexpr uint32_t BRIDGE_HOST_ABI = 1;

static_assert(sizeof(void *) == 8);
static_assert(sizeof(size_t) == 8);

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

typedef struct {
    const char *before;
    const char *after;
} bridge_order_t;
static_assert(sizeof(bridge_order_t) == 16);
static_assert(alignof(bridge_order_t) == 8);
static_assert(offsetof(bridge_order_t, before) == 0);
static_assert(offsetof(bridge_order_t, after) == 8);

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

typedef struct {
    const char *from;
    const char *to;
} bridge_phase_name_t;
static_assert(sizeof(bridge_phase_name_t) == 16);
static_assert(alignof(bridge_phase_name_t) == 8);
static_assert(offsetof(bridge_phase_name_t, from) == 0);
static_assert(offsetof(bridge_phase_name_t, to) == 8);

enum bridge_on_error : uint8_t {
    BRIDGE_FAIL = 1,
    BRIDGE_SKIP = 2
};

enum bridge_flag : uint8_t {
    BRIDGE_UNSET = 0,
    BRIDGE_TRUE = 1,
    BRIDGE_FALSE = 2
};

constexpr int64_t BRIDGE_NO_LIMIT = -1;
constexpr int64_t BRIDGE_NONE = -2;

constexpr char BRIDGE_NO_CONFIG[] = "none";

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

enum bridge_level : uint8_t {
    BRIDGE_ERR = 3,
    BRIDGE_WARN = 4,
    BRIDGE_INFO = 6,
    BRIDGE_DEBUG = 7
};

constexpr uint8_t BRIDGE_LEVEL_MOST = 7;

typedef void (*bridge_sink_fn_t)(void *ud, const bridge_record_t *r);

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

BRIDGE_API [[nodiscard]] bridge_t *bridge_create(const bridge_config_t *cfg,
    bridge_sink_fn_t sink, void *ud);

BRIDGE_API [[nodiscard]] int bridge_destroy(bridge_t *b);

BRIDGE_API [[nodiscard]] int bridge_load(bridge_t *b,
    const bridge_artifact_t *a);

BRIDGE_API [[nodiscard]] int bridge_unload(bridge_t *b, const char *identity);

BRIDGE_API [[nodiscard]] int bridge_reload(bridge_t *b,
    const bridge_artifact_t *a);

BRIDGE_API int bridge_list_modules(bridge_t *b, bridge_module_info_t *out,
    int max);

BRIDGE_API [[nodiscard]] int bridge_run_phase(bridge_t *b, const char *name);

    #ifdef __cplusplus
}
    #endif

#endif
