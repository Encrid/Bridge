/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_wasm.h"
#include "wasm_idle.h"

BRIDGE_EXPORT(say) int say(void)
{
    BRIDGE_LOG(BRIDGE_LOG_ERR, "an error", 0, 0);
    BRIDGE_LOG(BRIDGE_LOG_WARN, "a warning", 0, 0);
    BRIDGE_LOG(BRIDGE_LOG_INFO, "some information", 0, 0);
    BRIDGE_LOG(BRIDGE_LOG_DEBUG, "a detail", 0, 0);
    return (0);
}

BRIDGE_EXPORT(init) int init(void)
{
    int none = 0;

    BRIDGE_ADD_SYSTEM("say", "update", &none, 0, nullptr, 0, nullptr, 0);
    return (0);
}
