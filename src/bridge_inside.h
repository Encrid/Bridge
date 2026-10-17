/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_BRIDGE_INSIDE_H
    #define BRIDGE_SRC_BRIDGE_INSIDE_H

    #include "inside.h"

[[nodiscard]] int bridge_open_backends(bridge_t *b, const bridge_config_t *cfg);

[[nodiscard]] int bridge_open_backend(bridge_t *b, const bridge_config_t *cfg,
    uint32_t i);

[[nodiscard]] int bridge_name_phases(bridge_t *b, const bridge_config_t *cfg);

size_t bridge_get_stem_length(const char *identity);

const module_t *bridge_find_pinned(const bridge_t *b);

[[nodiscard]] bool bridge_enter(bridge_t *b, bool *owner);

void bridge_leave(bridge_t *b, bool owner);

[[nodiscard]] int bridge_check_entry(bridge_t *b);

[[nodiscard]] int bridge_check_call(bridge_t *b, const char *repairing);

[[nodiscard]] int bridge_load_now(bridge_t *b, const bridge_artifact_t *a);

[[nodiscard]] int bridge_unload_now(bridge_t *b, const char *identity);

[[nodiscard]] int bridge_reload_now(bridge_t *b, const bridge_artifact_t *a);

[[nodiscard]] int bridge_run_phase_now(bridge_t *b, const char *name);

int bridge_list_modules_now(bridge_t *b, bridge_module_info_t *out, int max);

void bridge_stop(bridge_t *b, const char *identity, const char *what);

void bridge_resume(bridge_t *b, const module_t *m);

void bridge_give_up(bridge_t *b, const module_t *m);

#endif
