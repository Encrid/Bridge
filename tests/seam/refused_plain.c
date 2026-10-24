/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

void test_state_limit(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_plain(&a, "game:mover@0.1.0", "no-such-library");
    if (*h->arg == 'w')
        a.cfg.work_limit = 1000;
    if (*h->arg == 'm')
        a.cfg.memory_limit = 1 << 20;
    host_load(h, &a);
    host_close(h);
}

void test_state_no_path(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_plain(&a, "game:mover@0.1.0", nullptr);
    host_load(h, &a);
    host_close(h);
}

void test_state_no_bytes(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_reader_limited(&a);
    a.bytes = nullptr;
    a.len = 0;
    host_load(h, &a);
    host_close(h);
}

void test_name_no_library(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_plain(&a, "game:mover@0.1.0", "no-such-library");
    host_load(h, &a);
    host_close(h);
}
