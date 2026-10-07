/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/backend.c -- a backend's library, opened: its entries found by name.
*/

#include <stdio.h>

#include "backend.h"
#include "os.h"

/*
** find_entries
**
** Fills `api` with the library's entries. Every member of backend_api_t is a
** function pointer, so the struct is filled as an array of them, in the order
** BACKEND_ENTRY_NAMES gives. The first entry says which backend seam the rest
** are of: a library of another is not asked for them. Answers how many were
** taken: all of them, or the place of the one it stopped at.
*/
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

/*
** say_stopped
**
** Writes why find_entries stopped at the entry `at`: the library lacks it, or
** it is the first and answered another backend seam's number.
**
** create.md, *Create*
** host_api.md, *A seam has a number*
*/
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

/*
** backend_open
**
** Opens a backend's library and finds its entries. Answers the library, or
** nullptr with `err` saying why, and the library closed.
**
** create.md, *Create*
*/
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
