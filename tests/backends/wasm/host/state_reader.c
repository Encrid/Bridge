/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

static const char *const TEST_READER_GRANTS[] = {
    "game:Position",
    "game:Marked"
};

void test_state_reader_limited(bridge_artifact_t *a)
{
    host_state_wasm(a, "view:reader@0.1.0", "reader");
    a->cfg.read = TEST_READER_GRANTS;
    a->cfg.read_count = 2;
}
