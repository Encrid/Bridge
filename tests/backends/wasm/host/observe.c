/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

void test_take_reserved_name(host_t *h)
{
    if (host_open(h))
        return;
    test_load_wasm(h, "game:badobs@0.1.0", "badobs");
    host_close(h);
}
