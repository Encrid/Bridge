/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
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
