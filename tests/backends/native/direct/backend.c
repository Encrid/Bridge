/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "backend.h"
#include "os.h"

static uint32_t find_entries(void *lib, backend_api_t *api)
{
    void **slots = (void **)api;
    uint32_t i = 0;

    while (i < BACKEND_ENTRY_COUNT) {
        slots[i] = os_lib_get_symbol(lib, BACKEND_ENTRY_NAMES[i]);
        if (!slots[i])
            return (i);
        if (!i && api->backend_get_abi() != BACKEND_ABI)
            return (i);
        i++;
    }
    return (i);
}

static void say_stopped(const char *path, const backend_api_t *api,
    uint32_t at, backend_err_t *err)
{
    const char *entry = BACKEND_ENTRY_NAMES[at];

    if (at || !api->backend_get_abi) {
        snprintf(err->text, sizeof(err->text),
            "backend library '%s' has no entry '%s'", path, entry);
        return;
    }
    snprintf(err->text, sizeof(err->text),
        "backend library '%s' is of backend ABI %u, and this is %u", path,
        api->backend_get_abi(), (unsigned)BACKEND_ABI);
}

void *backend_open(const char *path, backend_api_t *api, backend_err_t *err)
{
    void *lib = os_lib_open(path);
    uint32_t found;

    if (!lib) {
        snprintf(err->text, sizeof(err->text),
            "cannot open backend library '%s'", path);
        return (nullptr);
    }
    found = find_entries(lib, api);
    if (found == BACKEND_ENTRY_COUNT)
        return (lib);
    say_stopped(path, api, found, err);
    os_lib_close(lib);
    return (nullptr);
}
