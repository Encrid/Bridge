/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_inside.h"

int bridge_load(bridge_t *b, const bridge_artifact_t *a)
{
    bool owner;
    int result;

    if (!bridge_enter(b, &owner) || !owner)
        return (1);
    result = bridge_load_now(b, a);
    bridge_leave(b, owner);
    return (result);
}

int bridge_unload(bridge_t *b, const char *identity)
{
    bool owner;
    int result;

    if (!bridge_enter(b, &owner) || !owner)
        return (1);
    result = bridge_unload_now(b, identity);
    bridge_leave(b, owner);
    return (result);
}

int bridge_reload(bridge_t *b, const bridge_artifact_t *a)
{
    bool owner;
    int result;

    if (!bridge_enter(b, &owner) || !owner)
        return (1);
    result = bridge_reload_now(b, a);
    bridge_leave(b, owner);
    return (result);
}

int bridge_run_phase(bridge_t *b, const char *name)
{
    bool owner;
    int result;

    if (!bridge_enter(b, &owner) || !owner)
        return (1);
    result = bridge_run_phase_now(b, name);
    bridge_leave(b, owner);
    return (result);
}

int bridge_list_modules(bridge_t *b, bridge_module_info_t *out, int max)
{
    bool owner;
    int result;

    if (!bridge_enter(b, &owner))
        return (-1);
    result = bridge_list_modules_now(b, out, max);
    bridge_leave(b, owner);
    return (result);
}
