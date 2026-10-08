/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/support/posix/host_os.c -- what a host program asks the operating
** system itself, and nothing of ours: on Linux and macOS.
*/

#include <dlfcn.h>
#include <pthread.h>
#include <stdlib.h>

#include "host.h"

/*
** host_is_library_loaded
**
** Answers whether a library is in this process, asked of the operating
** system.
*/
bool host_is_library_loaded(const char *path)
{
    void *lib = dlopen(path, RTLD_NOLOAD | RTLD_NOW);

    if (lib)
        dlclose(lib);
    return (lib != nullptr);
}

/*
** host_get_thread_id
**
** Answers which thread this is, as a native test module says it.
*/
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

/*
** host_thread_start
**
** Starts a thread that calls `t->run` with `t->arg`.
*/
void host_thread_start(host_thread_t *t)
{
    pthread_t *thread = malloc(sizeof(*thread));

    t->handle = thread;
    pthread_create(thread, nullptr, run_thread, t);
}

/*
** host_thread_join
**
** Waits for a thread to end.
*/
void host_thread_join(host_thread_t *t)
{
    pthread_t *thread = t->handle;

    pthread_join(*thread, nullptr);
    free(thread);
}
