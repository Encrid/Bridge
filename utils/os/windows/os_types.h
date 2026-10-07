/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** utils/os/windows/os_types.h -- what a thread, a mutex and an event are on
** Windows.
*/

#ifndef BRIDGE_OS_TYPES_H
    #define BRIDGE_OS_TYPES_H

    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>

    #include "os.h"

struct os_thread {
    HANDLE handle;
    os_thread_fn_t fn;
    void *arg;
};

struct os_mutex {
    CRITICAL_SECTION section;
};

struct os_event {
    HANDLE handle;
};

#endif
