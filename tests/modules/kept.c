/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_offsets.h"
#include "game.h"
#include "offsets_idle.h"
#include "on_destroy.h"

BRIDGE_EXPORT(init) int init(void)
{
    BRIDGE_DECLARE_AS(BRIDGE_STRUCT, "game:Position", position_t,
        BRIDGE_DELETE, (enum bridge_storage)KEPT, ON_DESTROY);
    return (0);
}
