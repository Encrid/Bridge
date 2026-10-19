/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

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
