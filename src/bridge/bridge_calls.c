/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_inside.h"

int bridge_load(bridge_world_t *w, const bridge_artifact_t *a)
{
    bool owner;
    int result;

    if (!bridge_enter(w, &owner) || !owner)
        return (1);
    result = bridge_load_now(w, a);
    bridge_leave(w, owner);
    return (result);
}

int bridge_unload(bridge_world_t *w, const char *identity)
{
    bool owner;
    int result;

    if (!bridge_enter(w, &owner) || !owner)
        return (1);
    result = bridge_unload_now(w, identity);
    bridge_leave(w, owner);
    return (result);
}

int bridge_reload(bridge_world_t *w, const bridge_artifact_t *artifacts,
    uint32_t count)
{
    bool owner;
    int result;

    if (!bridge_enter(w, &owner) || !owner)
        return (1);
    result = count == 1 ? bridge_reload_now(w, &artifacts[0])
        : bridge_reload_group_now(w, artifacts, count);
    bridge_leave(w, owner);
    return (result);
}

int bridge_run_phase(bridge_world_t *w, const char *name)
{
    bool owner;
    int result;

    if (!bridge_enter(w, &owner) || !owner)
        return (1);
    result = bridge_run_phase_now(w, name);
    bridge_leave(w, owner);
    return (result);
}

int bridge_list_modules(bridge_world_t *w, bridge_module_info_t *out, int max)
{
    bool owner;
    int result;

    if (!bridge_enter(w, &owner))
        return (-1);
    result = bridge_list_modules_now(w, out, max);
    bridge_leave(w, owner);
    return (result);
}
