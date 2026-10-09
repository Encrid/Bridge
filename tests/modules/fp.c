/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/fp.c -- declares game:Position with another struct
** body: built against an older header, say.
*/

#include "test_offsets.h"
#include "fp.h"
#include "offsets_idle.h"

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
