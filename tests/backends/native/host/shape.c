/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

void test_load_no_init(host_t *h)
{
    if (host_open(h))
        return;
    test_load(h, "test:noinit@0.1.0", NNOINIT_PATH, BRIDGE_NONE);
    host_close(h);
}
