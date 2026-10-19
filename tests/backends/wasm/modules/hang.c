/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_wasm.h"

static volatile int going = 1;

BRIDGE_EXPORT(init) int init(void)
{
    return (0);
}

BRIDGE_EXPORT(shutdown) int shutdown(void)
{
    while (going)
        continue;
    return (0);
}

BRIDGE_EXPORT(reload) int reload(void)
{
    return (0);
}
