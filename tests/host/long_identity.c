/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <string.h>

#include "cases.h"

static void load_named(host_t *h, size_t length)
{
    char identity[160];
    size_t name = length - strlen("long:") - strlen("@0.1.0");
    bridge_artifact_t a;

    memcpy(identity, "long:", 5);
    memset(identity + 5, 'a', name);
    memcpy(identity + 5 + name, "@0.1.0", 7);
    host_state_guarded(&a, identity, "ok");
    host_load(h, &a);
}

void test_load_long_identity(host_t *h)
{
    if (host_open(h))
        return;
    load_named(h, (size_t)strtoul(h->arg, nullptr, 10));
    test_say_modules(h);
    host_close(h);
}
