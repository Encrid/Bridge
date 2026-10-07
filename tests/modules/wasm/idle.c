/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/modules/wasm/idle.c -- a shutdown and a reload that do nothing,
** built into every wasm test module that includes wasm_idle.h.
*/

#include "wasm_idle.h"

/*
** shutdown
**
** Does nothing.
*/
BRIDGE_EXPORT(shutdown) int shutdown(void)
{
    return (0);
}

/*
** reload
**
** Does nothing.
*/
BRIDGE_EXPORT(reload) int reload(void)
{
    return (0);
}
