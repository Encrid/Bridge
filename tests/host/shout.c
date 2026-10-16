/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

void test_log_out_of_range(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    host_state_guarded(&a, "test:s5@0.1.0", "shout");
    host_load(h, &a);
    (void)bridge_run_phase(h->b, "update");
    host_close(h);
}
