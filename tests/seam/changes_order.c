/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "cases.h"

static const bridge_order_t TEST_FIVE_FIRST = {
    "game:stamp5:stamp",
    "game:stamp9:stamp"
};

static const bridge_order_t TEST_NINE_FIRST = {
    "game:stamp9:stamp",
    "game:stamp5:stamp"
};

static const bridge_order_t *get_order(const char *word)
{
    if (!strcmp(word, "five-before-nine"))
        return (&TEST_FIVE_FIRST);
    if (!strcmp(word, "nine-before-five"))
        return (&TEST_NINE_FIRST);
    return (nullptr);
}

void test_stamp_twice(host_t *h)
{
    char order[24] = "";
    char first[24] = "";
    uint32_t tables = 0;
    bool nine_first = false;

    if (sscanf(h->arg, "%23s %23s %u", order, first, &tables) == 3)
        h->cfg.budget.tables = tables;
    if (get_order(order)) {
        h->cfg.order = get_order(order);
        h->cfg.order_count = 1;
    }
    nine_first = !strcmp(first, "nine-loaded-first");
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
