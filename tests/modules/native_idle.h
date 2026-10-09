/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/native_idle.h -- for a native test module whose shutdown
** and reload do nothing. A module that includes this says so, and is built
** with tests/modules/native/nidle.c, which has them: the three lifecycle
** exports are every module's, and none is left out to mean something.
**
** native.md, *Lifecycle*
*/

#ifndef TEST_IDLE_H
    #define TEST_IDLE_H

    #include "bridge_native.h"

/*
** shutdown, reload
**
** Do nothing, and answer 0.
*/
BRIDGE_NATIVE(shutdown);
BRIDGE_NATIVE(reload);

#endif
