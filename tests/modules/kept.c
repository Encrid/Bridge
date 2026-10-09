/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/kept.c -- declares game:Position as the mover does,
** and states a storage layout for it, KEPT, that is unset, or one that is
** no storage layout, or another than the mover's.
**
** storage.md, *Storage layouts*
*/

#include "test_offsets.h"
#include "game.h"
#include "offsets_idle.h"

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
