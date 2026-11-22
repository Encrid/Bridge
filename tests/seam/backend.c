/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bridge_backend.h"
#include "cases.h"

void test_leave_libraries(host_t *h)
{
    bridge_setting_t told[4];
    bridge_world_t *w = nullptr;
    const char *guarded = "GONE";
    const char *plain = "gone";

    memcpy(told, h->backends[1].settings, sizeof(told));
    if (!strcmp(h->arg, "keep"))
        told[0].v.i &= ~(int64_t)BACKEND_UNLOAD;
    h->backends[1].settings = told;
    w = bridge_create(&h->cfg, h->sink, nullptr);
    if (!w || bridge_destroy(w)) {
        printf("FAIL: the world was not created and destroyed\n");
        exit(1);
    }
    if (host_is_library_loaded(BRIDGE_TESTS_ALL_PATH))
        guarded = "still loaded";
    if (host_is_library_loaded(BRIDGE_TESTS_PATH))
        plain = "STILL LOADED";
    printf("the guarded backend's library is %s, the plain backend's is %s\n",
        guarded, plain);
    printf("alive\n");
}

void test_open_other_seam(host_t *h)
{
    if (!strcmp(h->arg, "other"))
        h->backends[1].library = BRIDGE_PLAIN_OTHER_PATH;
    host_open_only(h);
}

static const char *pick_library(const char *arg)
{
    if (!strcmp(arg, "other"))
        return (BRIDGE_PLAIN_OTHER_PATH);
    if (!strcmp(arg, "hollow"))
        return (BRIDGE_HOLLOW_PATH);
    if (!strcmp(arg, "none"))
        return ("no-such-backend-library");
    return (BRIDGE_TESTS_PATH);
}

void test_open_backend_alone(host_t *h)
{
    backend_api_t api = {};
    backend_err_t err = {};
    void *lib = backend_open(pick_library(h->arg), &api, &err);

    if (!lib) {
        printf("backend_open: refused: %s\n", err.text);
        return;
    }
    printf("backend_open: opened, backend ABI %u, %s\n",
        api.backend_get_abi(), api.call ? "each entry" : "an entry missing");
    backend_close(lib);
}

void test_state_unknown_setting(host_t *h)
{
    bridge_setting_t setting = { .key = h->arg, .kind = BRIDGE_STRING,
        .v = { .s = "somewhere" } };

    h->backends[1].settings = &setting;
    h->backends[1].setting_count = 1;
    host_open_only(h);
}
