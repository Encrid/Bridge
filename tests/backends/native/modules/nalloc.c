/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_native.h"
#include "native_idle.h"

static uint32_t kept;

BRIDGE_NATIVE(say)
{
    bridge_line_t line = { "kept", 4, kept != 0, 0 };

    if (kept)
        line.a1 = api->shared_base[kept];
    bridge_native_log(api, BRIDGE_LOG_INFO, &line);
    return (0);
}

BRIDGE_NATIVE(init)
{
    int none = 0;

    kept = api->alloc(api, 16);
    if (kept)
        api->shared_base[kept] = 42;
    BRIDGE_NATIVE_ADD_SYSTEM(api, "say", "update", &none, 0, nullptr, 0);
    return (0);
}
