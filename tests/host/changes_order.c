/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** tests/host/changes_order.c -- two systems of one phase, in two
** modules, that attach one component to the same entities. Unordered, the
** second does not load. Ordered, the one stated to run last is the one
** whose value stays, whichever of them was loaded first.
**
** schedule.md, *Order within a phase is never implicit*
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
        test_load_native(h, "game:stamp9@0.1.0", NSTAMP9_PATH, 8);
    test_load_native(h, "game:stamp5@0.1.0", NSTAMP5_PATH, 8);
    if (!nine_first)
        test_load_native(h, "game:stamp9@0.1.0", NSTAMP9_PATH, 8);
    test_load_watcher(h);
    host_run_frames_unsaid(h->b, 2);
    host_close(h);
}

/*
** test_conflict_by_change
**
** Nothing orders the two: the second to load is refused.
*/
void test_conflict_by_change(host_t *h)
{
    stamp_twice(h, false);
}

/*
** test_order_changes
**
** The five is ordered before the nine, and is loaded first.
*/
void test_order_changes(host_t *h)
{
    h->cfg.order = &TEST_FIVE_FIRST;
    h->cfg.order_count = 1;
    stamp_twice(h, false);
}

/*
** test_order_changes_loaded_last
**
** The five is ordered before the nine, and is loaded second.
*/
void test_order_changes_loaded_last(host_t *h)
{
    h->cfg.order = &TEST_FIVE_FIRST;
    h->cfg.order_count = 1;
    stamp_twice(h, true);
}

/*
** test_tell_both
**
** The same, with room for one table, which the entities with a position
** take: one with a mark as well has none.
*/
void test_tell_both(host_t *h)
{
    h->cfg.order = &TEST_FIVE_FIRST;
    h->cfg.order_count = 1;
    h->cfg.budget.tables = 1;
    stamp_twice(h, false);
}
