/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/wasm_idle.h -- for a wasm test module whose shutdown and
** reload do nothing. A module that includes this says so, and is built with
** tests/modules/wasm/idle.c, which has them: the three lifecycle exports
** are every module's, and none is left out to mean something.
**
** wasm.md, *Exports*
*/

#ifndef TEST_WASM_IDLE_H
    #define TEST_WASM_IDLE_H

    #include "bridge_wasm.h"

/*
** shutdown, reload
**
** Do nothing, and answer 0.
*/
BRIDGE_EXPORT(shutdown) int shutdown(void);
BRIDGE_EXPORT(reload) int reload(void);

#endif
