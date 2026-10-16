/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

void test_load_by_identity(host_t *h)
{
    if (host_open(h))
        return;
    test_load_guarded(h, h->arg, "mover");
    host_run_frames(h, 1);
    host_close(h);
}
