/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/support/host_built.c -- a module of the tests' backend, stated as a
** deployment states one.
*/

#include <stdio.h>
#include <string.h>

#include "host.h"

/*
** host_state_built
**
** States a module of the tests' backend, by the path of its library.
*/
void host_state_built(bridge_artifact_t *a, const char *identity,
    const char *path)
{
    host_state_native(a, identity, path);
}

/*
** keep_path
**
** Answers where the library of a module of that name is, kept for as long
** as the program runs: a case states a handful of modules, and the last
** thirty-two are held.
*/
static const char *keep_path(const char *file)
{
    static char paths[32][512];
    static uint32_t next;
    char *path = paths[next++ % 32];

    snprintf(path, sizeof(paths[0]), "%s/%s%s", BUILT_DIR, file, BUILT_SUFFIX);
    return (path);
}

/*
** host_name_module
**
** Names another module of tests/modules for an artifact that was stated:
** by bytes and their length, which the tests' backend takes as the path of
** the module's library. So what a host hands as bytes is shown to arrive.
*/
void host_name_module(bridge_artifact_t *a, const char *file)
{
    a->path = nullptr;
    a->bytes = keep_path(file);
    a->len = strlen(a->bytes);
}

/*
** host_state_wasm
**
** States a module of tests/modules that is written by offsets, with every
** limit a backend may be asked to keep. The backend it names is the one a
** deck of the tests calls "wasm", which is the tests' backend told that it
** keeps them.
*/
void host_state_wasm(bridge_artifact_t *a, const char *identity,
    const char *file)
{
    host_state_native(a, identity, nullptr);
    host_name_module(a, file);
    a->backend = "wasm";
    a->cfg.trusted = BRIDGE_FALSE;
    a->cfg.work_limit = 10'000'000;
    a->cfg.memory_limit = 16 << 20;
    a->cfg.init_timeout = 2000 * HOST_MS;
    a->cfg.reload_timeout = 2000 * HOST_MS;
    a->cfg.shutdown_timeout = 2000 * HOST_MS;
}
