/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_HOST_HELLO_H
    #define BRIDGE_HOST_HELLO_H

    #include "bridge_wasm.h"

    #ifdef REACH
[[clang::import_module("env"), clang::import_name("open")]] int reach(void);
    #endif

#endif
