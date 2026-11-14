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

void test_state_unknown_setting(host_t *h)
{
    bridge_setting_t setting = { .key = h->arg, .kind = BRIDGE_STRING,
        .v = { .s = "somewhere" } };

    h->backends[1].settings = &setting;
    h->backends[1].setting_count = 1;
    host_open_only(h);
}
