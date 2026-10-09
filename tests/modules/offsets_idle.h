/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/offsets_idle.h -- for a guarded test module whose shutdown and
** reload do nothing. A module that includes this says so, and is built with
** tests/modules/offsets_idle.c, which has them: the three lifecycle exports
** are every module's, and none is left out to mean something.
**
** lifecycle.md, *The runtime lifecycle*
*/

#ifndef TEST_OFFSETS_IDLE_H
    #define TEST_OFFSETS_IDLE_H

    #include "test_offsets.h"

/*
** shutdown, reload
**
** Do nothing, and answer 0.
*/
BRIDGE_EXPORT(shutdown) int shutdown(void);
BRIDGE_EXPORT(reload) int reload(void);

#endif
