/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/modules/wasm/levels.c -- writes one line at each of the four
** severities the bindings name: which of them a host is handed is its
** config's log_level to say.
*/

#include "bridge_wasm.h"
#include "wasm_idle.h"

/*
** say
**
** Writes an error, a warning, some information and a detail.
*/
BRIDGE_EXPORT(say) int say(void)
{
    BRIDGE_LOG(BRIDGE_LOG_ERR, "an error", 0, 0);
    BRIDGE_LOG(BRIDGE_LOG_WARN, "a warning", 0, 0);
    BRIDGE_LOG(BRIDGE_LOG_INFO, "some information", 0, 0);
    BRIDGE_LOG(BRIDGE_LOG_DEBUG, "a detail", 0, 0);
    return (0);
}

/*
** init
**
** Registers the system.
*/
BRIDGE_EXPORT(init) int init(void)
{
    int none = 0;

    BRIDGE_ADD_SYSTEM("say", "update", &none, 0, nullptr, 0);
    return (0);
}
