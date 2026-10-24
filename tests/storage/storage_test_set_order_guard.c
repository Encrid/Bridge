/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>

#include "bridge_inside.h"
#include "storage_test.h"

static char STOPPED[] = "a module stopped it";

enum storage_test_end storage_test_set_order_guard(storage_test_t *t)
{
    bridge_world_t w = { .sink = storage_test_hear, .ud = t };
    const bridge_order_t pair = { "game:m:a", "game:m:b" };
    bool owner = false;

    if (!strcmp(t->what, "setorderstopped"))
        w.stopped_by = STOPPED;
    if (!strcmp(t->what, "setorderbusy"))
        atomic_store(&w.caller, (uint64_t)0xC0FFEE);
    if (!strcmp(t->what, "setorderreentrant"))
        (void)bridge_enter(&w, &owner);
    printf("guard %s: %d\n", t->what, bridge_set_order(&w, &pair, 1));
    if (owner)
        bridge_leave(&w, owner);
    return (STORAGE_TEST_LEFT);
}
