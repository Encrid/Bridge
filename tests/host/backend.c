/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/host/backend.c -- a backend's library: what a destroyed
** deck leaves of it in the process, one built for another backend seam,
** and a setting it does not know.
**
** capabilities.md, *Capabilities, so the swap is honest*
*/

#include <stdio.h>
#include <stdlib.h>

#include "cases.h"

/*
** test_leave_libraries
**
** What a destroyed deck leaves in the process: the library of a backend
** that does not say it can be unloaded, and not the library of one that
** does. A backend that brought in a runtime with threads of its own does
** not say so, since those would not survive it; guarded, the tests'
** backend answers as one. Bridge closes the plain backend's library, and
** every system this is built for then takes it out of the process.
*/
void test_leave_libraries(host_t *h)
{
    bridge_t *b = bridge_create(&h->cfg, h->sink, nullptr);
    const char *guarded = "GONE";
    const char *plain = "gone";

    if (!b || bridge_destroy(b)) {
        printf("FAIL: the deck was not created and destroyed\n");
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

/*
** test_open_other_seam
**
** A backend library that says it is of another backend seam.
*/
void test_open_other_seam(host_t *h)
{
    h->backends[1].library = PLAIN_OTHER_PATH;
    host_open_only(h);
}

/*
** test_state_unknown_setting
**
** A key the backend does not know.
*/
void test_state_unknown_setting(host_t *h)
{
    bridge_setting_t cache = { .key = "cache", .kind = BRIDGE_STRING,
        .v = { .s = "somewhere" } };

    h->backends[1].settings = &cache;
    h->backends[1].setting_count = 1;
    host_open_only(h);
}
