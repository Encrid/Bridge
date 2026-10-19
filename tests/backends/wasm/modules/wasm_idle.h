/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_WASM_IDLE_H
    #define TEST_WASM_IDLE_H

    #include "bridge_wasm.h"

BRIDGE_EXPORT(shutdown) int shutdown(void);
BRIDGE_EXPORT(reload) int reload(void);

#endif
