/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

void test_tell_places(host_t *h)
{
    test_run_guarded(h, "game:places@0.1.0", "places", 1);
}
