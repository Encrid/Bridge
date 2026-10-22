/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

static const bridge_order_t TEST_FIVE_FIRST = {
    "game:stamp5:stamp",
    "game:stamp9:stamp"
};

static void stamp_twice(host_t *h, bool nine_first)
{
    if (host_open(h))
        return;
    if (nine_first)
        test_load_built(h, "game:stamp9@0.1.0", STAMP9_PATH, 8);
    test_load_built(h, "game:stamp5@0.1.0", STAMP5_PATH, 8);
    if (!nine_first)
        test_load_built(h, "game:stamp9@0.1.0", STAMP9_PATH, 8);
    test_load_watcher(h);
    host_run_frames_unsaid(h->w, 2);
    host_close(h);
}

void test_conflict_by_change(host_t *h)
{
    stamp_twice(h, false);
}

void test_order_changes(host_t *h)
{
    h->cfg.order = &TEST_FIVE_FIRST;
    h->cfg.order_count = 1;
    stamp_twice(h, false);
}

void test_order_changes_loaded_last(host_t *h)
{
    h->cfg.order = &TEST_FIVE_FIRST;
    h->cfg.order_count = 1;
    stamp_twice(h, true);
}

void test_tell_both(host_t *h)
{
    h->cfg.order = &TEST_FIVE_FIRST;
    h->cfg.order_count = 1;
    h->cfg.budget.tables = 1;
    stamp_twice(h, false);
}
