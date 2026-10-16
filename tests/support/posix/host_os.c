/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <dlfcn.h>
#include <pthread.h>
#include <stdlib.h>

#include "host.h"

bool host_is_library_loaded(const char *path)
{
    void *lib = dlopen(path, RTLD_NOLOAD | RTLD_NOW);

    if (lib)
        dlclose(lib);
    return (lib != nullptr);
}

uint32_t host_get_thread_id(void)
{
    return ((uint32_t)(uintptr_t)pthread_self());
}

static void *run_thread(void *arg)
{
    host_thread_t *t = arg;

    t->run(t->arg);
    return (nullptr);
}

void host_thread_start(host_thread_t *t)
{
    pthread_t *thread = malloc(sizeof(*thread));

    t->handle = thread;
    pthread_create(thread, nullptr, run_thread, t);
}

void host_thread_join(host_thread_t *t)
{
    pthread_t *thread = t->handle;

    pthread_join(*thread, nullptr);
    free(thread);
}
