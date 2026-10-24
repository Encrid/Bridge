/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

static void load_spot(host_t *h, const char *identity, const char *file)
{
    bridge_artifact_t a;

    host_state_plain(&a, identity, nullptr);
    host_name_module(&a, file);
    host_load(h, &a);
}

void test_declare_other_size(host_t *h)
{
    if (host_open(h))
        return;
    load_spot(h, "five:second@0.1.0", "spot-wide");
    load_spot(h, "five:first@0.1.0", "spot");
    load_spot(h, "five:second@0.1.0", "spot-wide");
    test_say_modules(h);
    host_close(h);
}
