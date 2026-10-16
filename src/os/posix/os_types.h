/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** utils/os/posix/os_types.h -- what a thread, a mutex and an event are where
** there are POSIX threads: Linux and macOS.
*/

#ifndef BRIDGE_OS_TYPES_H
    #define BRIDGE_OS_TYPES_H

    #include <pthread.h>

    #include "os.h"

struct os_thread {
    pthread_t handle;
    os_thread_fn_t fn;
    void *arg;
};

struct os_mutex {
    pthread_mutex_t mutex;
};

struct os_event {
    pthread_mutex_t lock;
    pthread_cond_t cond;
    int set;
};

#endif
