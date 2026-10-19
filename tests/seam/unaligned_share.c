/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

static void load_ten(host_t *h, const char *identity, int64_t persistent)
{
    bridge_artifact_t a;

    host_state_plain(&a, identity, nullptr);
    host_name_module(&a, "alloc-twelve");
    a.cfg.persistent = persistent;
    host_load(h, &a);
}

void test_alloc_unaligned_share(host_t *h)
{
    h->cfg.budget.persistent = 100;
    if (host_open(h))
        return;
    load_ten(h, "ten:big@0.1.0", 88);
    load_ten(h, "ten:small@0.1.0", 12);
    test_say_modules(h);
    host_close(h);
}
