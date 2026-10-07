/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** tests/modules/wasm/kept.c -- declares game:Position as the mover does,
** and states a storage layout for it, KEPT, that is unset, or one that is
** no storage layout, or another than the mover's.
**
** storage.md, *Storage layouts*
*/

#include "bridge_wasm.h"
#include "game.h"
#include "wasm_idle.h"

/*
** init
**
** Declares the position, with the storage layout the build states.
*/
BRIDGE_EXPORT(init) int init(void)
{
    BRIDGE_COMPONENT("game:Position", position_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    return (0);
}
