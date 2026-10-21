/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_BRIDGE_INSIDE_H
    #define BRIDGE_SRC_BRIDGE_INSIDE_H

    #include "inside.h"

[[nodiscard]] int bridge_open_backends(bridge_world_t *w,
    const bridge_config_t *cfg);

[[nodiscard]] int bridge_open_backend(bridge_world_t *w,
    const bridge_config_t *cfg, uint32_t i);

[[nodiscard]] int bridge_name_phases(bridge_world_t *w,
    const bridge_config_t *cfg);

uint32_t bridge_find_phase(char *const *names, uint32_t count,
    const char *name, size_t len);

size_t bridge_get_stem_length(const char *identity);

const module_t *bridge_find_pinned(const bridge_world_t *w);

[[nodiscard]] bool bridge_enter(bridge_world_t *w, bool *owner);

void bridge_leave(bridge_world_t *w, bool owner);

[[nodiscard]] int bridge_check_entry(bridge_world_t *w);

[[nodiscard]] int bridge_check_call(bridge_world_t *w, const char *repairing);

[[nodiscard]] int bridge_load_now(bridge_world_t *w,
    const bridge_artifact_t *a);

[[nodiscard]] int bridge_unload_now(bridge_world_t *w, const char *identity);

[[nodiscard]] int bridge_reload_now(bridge_world_t *w,
    const bridge_artifact_t *a);

[[nodiscard]] int bridge_reload_group_now(bridge_world_t *w,
    const bridge_artifact_t *artifacts, uint32_t count);

[[nodiscard]] int bridge_run_phase_now(bridge_world_t *w, const char *name);

[[nodiscard]] int bridge_set_order_now(bridge_world_t *w,
    const bridge_order_t *pairs, uint32_t count);

int bridge_list_modules_now(bridge_world_t *w, bridge_module_info_t *out,
    int max);

void bridge_stop(bridge_world_t *w, const char *identity, const char *what);

void bridge_resume(bridge_world_t *w, const module_t *m);

void bridge_give_up(bridge_world_t *w, const module_t *m);

#endif
