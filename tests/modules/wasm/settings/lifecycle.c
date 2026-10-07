/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/modules/wasm/settings/lifecycle.c -- a module whose settings are a
** component: a singleton, given its value by Bridge-ASM700 after init,
** which a system reads back.
**
** KEPT is the storage layout the component is declared with: a singleton,
** as the settings a module names must be, or what a build states in its
** place. DROP has reload register nothing.
**
** config.md, *A module's own settings are a component*
*/

#include "bridge_wasm.h"
#include "settings.h"

uint32_t settings;

/*
** init
**
** Declares the two types the fields are of, then the component and the
** system that reads it.
*/
BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, BRIDGE_OPTIONAL, 0 }};

    BRIDGE_DECLARE(BRIDGE_ENUM, "game:mode_t", mode_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
    BRIDGE_DECLARE(BRIDGE_BITMASK, "game:flags_t", flags_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
    settings = BRIDGE_COMPONENT("game:Settings", settings_t, BRIDGE_DELETE,
        KEPT);
    terms[0].slot = settings;
    BRIDGE_ADD_SYSTEM("show", "update", terms, 1, nullptr, 0);
    return (0);
}

/*
** reload
**
** Registers what init does, over the row that is there. The build with
** DROP registers none of it.
*/
BRIDGE_EXPORT(reload) int reload(void)
{
#ifdef DROP
    return (0);
#else
    return (init());
#endif
}

/*
** shutdown
**
** Does nothing.
*/
BRIDGE_EXPORT(shutdown) int shutdown(void)
{
    return (0);
}
