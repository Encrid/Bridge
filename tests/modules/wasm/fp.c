/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/wasm/fp.c -- declares game:Position with another struct
** body: built against an older header, say.
*/

#include "bridge_wasm.h"
#include "fp.h"
#include "wasm_idle.h"

/*
** init
**
** Declares the position, with the body fp.h gives it.
*/
BRIDGE_EXPORT(init) int init(void)
{
    BRIDGE_COMPONENT("game:Position", position_t, BRIDGE_STRIP, BRIDGE_TABLE);
    return (0);
}
