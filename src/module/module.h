/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_MODULE_H
    #define BRIDGE_SRC_MODULE_H

    #include "config.h"
    #include "inside.h"

constexpr uint32_t MODULE_SCRATCH_AT = 128;

typedef struct {
    bridge_world_t *w;
    const bridge_artifact_t *a;
    const bridge_module_config_t *cfg;
    const char *id;
    open_backend_t *on;
    enum backend_capability demands;
} module_check_t;

typedef struct {
    uint32_t ring;
    uint32_t spans;
    uint32_t end;
} module_private_t;

typedef struct {
    const range_t *owned;
    uint32_t own_count;
    const declared_type_t *c;
    uint32_t rows;
} module_pointing_t;

typedef struct {
    backend_func_t *f;
    const char *name;
    int64_t timeout;
} module_call_t;

typedef struct {
    const bridge_artifact_t *a;
    enum backend_capability demands;
    const char *entry;
} module_start_t;

[[gnu::format(printf, 3, 4)]] int module_refuse(bridge_world_t *w,
    const char *identity, const char *format, ...);

[[gnu::format(printf, 2, 3)]] int module_fail(backend_err_t *err,
    const char *format, ...);

int module_check_set(const module_check_t *k, const char *key, int64_t value);

int module_check_bound(const module_check_t *k, const char *key,
    int64_t value, int64_t none);

int module_check_flag(const module_check_t *k, const char *key,
    enum bridge_flag value);

int module_check_flags(const module_check_t *k);

int module_check_phases(const module_check_t *k);

int module_check_stated(const module_check_t *k);

int module_check_fit(const module_check_t *k);

int module_check_backend(const module_check_t *k);

int module_check_identity(bridge_world_t *w, const char *id);

[[nodiscard]] int module_check(module_check_t *k);

[[nodiscard]] int module_check_room(const module_check_t *k);

module_t *module_find(bridge_world_t *w, const char *identity);

int module_lay_private(const bridge_module_config_t *cfg, uint64_t granule,
    module_private_t *at);

[[nodiscard]] int module_take_private(module_t *m,
    const bridge_module_config_t *cfg, backend_err_t *err);

[[nodiscard]] module_t *module_record(bridge_world_t *w,
    const bridge_artifact_t *a, open_backend_t *o);

void module_drop_systems(bridge_world_t *w, module_t *m);

void module_release_types(bridge_world_t *w, module_t *m, bool by_policy);

uint64_t module_round_share(int64_t persistent);

void module_give_share(bridge_world_t *w, const range_t *owned,
    uint32_t own_count);

void module_settle_share(bridge_world_t *w, module_t *m);

uint32_t module_count_pointing(const bridge_world_t *w, const range_t *owned,
    uint32_t own_count,
    const declared_type_t *c);

[[nodiscard]] int module_call(module_t *m, const module_call_t *call,
    backend_err_t *err);

[[nodiscard]] int module_make_instance(module_t *m, const module_start_t *st,
    backend_err_t *err);

[[nodiscard]] int module_start(module_t *m, const module_start_t *st,
    backend_err_t *err);

void module_shut_down_new(bridge_world_t *w, module_t *m);

#endif
