/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/offsets_idle.c -- a shutdown and a reload that do nothing,
** built into every wasm test module that includes wasm_idle.h.
*/

#include "offsets_idle.h"

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
