/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/native/npinned.c -- three systems that each say which
** thread they were on. Loaded pinned, it must be one.
*/

#include "bridge_native.h"
#include "native_idle.h"
#include "nplatform.h"

static int say_thread(const bridge_api_t *api)
{
    bridge_line_t line = { "pin", 3, get_thread_id(), 0 };

    bridge_native_log(api, BRIDGE_LOG_INFO, &line);
    return (0);
}

/*
** one
**
** Says which thread it is on.
*/
BRIDGE_NATIVE(one)
{
    return (say_thread(api));
}

/*
** two
**
** Says which thread it is on.
*/
BRIDGE_NATIVE(two)
{
    return (say_thread(api));
}

/*
** three
**
** Says which thread it is on.
*/
BRIDGE_NATIVE(three)
{
    return (say_thread(api));
}

/*
** init
**
** Declares the three systems: two in one phase, and one in another.
*/
BRIDGE_NATIVE(init)
{
    int none = 0;

    BRIDGE_NATIVE_ADD_SYSTEM(api, "one", "update", &none, 0, nullptr, 0);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "two", "update", &none, 0, nullptr, 0);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "three", "present", &none, 0, nullptr, 0);
    return (0);
}
