/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "cases.h"

static void load_named(host_t *h, size_t length)
{
    char identity[BRIDGE_IDENTITY_MOST + 2];
    size_t name = length - strlen("long:") - strlen("@0.1.0");
    bridge_artifact_t a;

    memcpy(identity, "long:", 5);
    memset(identity + 5, 'a', name);
    memcpy(identity + 5 + name, "@0.1.0", 7);
    printf("an identity of %zu bytes\n", strlen(identity));
    fflush(stdout);
    host_state_guarded(&a, identity, "ok");
    host_load(h, &a);
}

void test_load_long_identity(host_t *h)
{
    if (host_open(h))
        return;
    load_named(h, BRIDGE_IDENTITY_MOST + !strcmp(h->arg, "past"));
    test_say_modules(h);
    host_close(h);
}
