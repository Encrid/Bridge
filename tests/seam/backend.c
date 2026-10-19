/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>

#include "cases.h"

void test_leave_libraries(host_t *h)
{
    bridge_t *b = bridge_create(&h->cfg, h->sink, nullptr);
    const char *guarded = "GONE";
    const char *plain = "gone";

    if (!b || bridge_destroy(b)) {
        printf("FAIL: the world was not created and destroyed\n");
        exit(1);
    }
    if (host_is_library_loaded(TESTS_ALL_PATH))
        guarded = "still loaded";
    if (host_is_library_loaded(TESTS_PATH))
        plain = "STILL LOADED";
    printf("the guarded backend's library is %s, the plain backend's is %s\n",
        guarded, plain);
    printf("alive\n");
}

void test_open_other_seam(host_t *h)
{
    h->backends[1].library = PLAIN_OTHER_PATH;
    host_open_only(h);
}

void test_state_unknown_setting(host_t *h)
{
    bridge_setting_t cache = { .key = "cache", .kind = BRIDGE_STRING,
        .v = { .s = "somewhere" } };

    h->backends[1].settings = &cache;
    h->backends[1].setting_count = 1;
    host_open_only(h);
}
