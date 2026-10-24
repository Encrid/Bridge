/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "cases.h"

void test_set_usage(host_t *h)
{
    h->sink = host_print_record;
}

static void say_world(host_t *h)
{
    bridge_budget_t used = {};
    bridge_module_usage_row_t rows[8] = {};
    int n = bridge_module_usage(h->w, rows, 8);
    uint64_t sys = 0;
    uint64_t persist = 0;
    int i = 0;

    (void)bridge_usage(h->w, &used, nullptr);
    while (i < n && i < 8) {
        sys += rows[i].systems + rows[i].observers;
        persist += rows[i].persistent;
        i++;
    }
    printf("usage mods %llu rows %d systems %s persistent %s\n",
        (unsigned long long)used.modules, n,
        sys == used.systems ? "match" : "differ",
        persist == used.persistent ? "match" : "differ");
}

static void say_most(host_t *h)
{
    bridge_budget_t used = {};
    bridge_budget_t most = {};

    test_load_built(h, "game:stamp5@0.1.0", STAMP5_PATH, 8);
    host_run_frames(h, 1);
    (void)bridge_unload(h->w, "game:stamp5@0.1.0");
    (void)bridge_usage(h->w, &used, nullptr);
    (void)bridge_usage(h->w, nullptr, &most);
    printf("most entities used %llu most %llu kept %s\n",
        (unsigned long long)used.entities, (unsigned long long)most.entities,
        most.entities > used.entities ? "yes" : "no");
}

static void say_max(host_t *h)
{
    bridge_module_usage_row_t rows[2] = { { "untouched", 0, 0, 0, 0, 0 },
        { "untouched", 0, 0, 0, 0, 0 } };
    int n = bridge_module_usage(h->w, rows, 1);

    printf("usage max %d, first %s, second %s\n", n, rows[0].identity,
        rows[1].identity);
}

void test_usage(host_t *h)
{
    if (host_open(h))
        return;
    if (!strcmp(h->arg, "most")) {
        say_most(h);
        host_close(h);
        return;
    }
    test_load_guarded(h, "game:nested@0.1.0", "nested");
    test_load_guarded(h, "game:hearer@0.1.0", "hearer");
    host_run_frames(h, 2);
    if (!strcmp(h->arg, "count"))
        printf("usage count %d\n", bridge_module_usage(h->w, nullptr, 0));
    else if (!strcmp(h->arg, "max"))
        say_max(h);
    else
        say_world(h);
    host_close(h);
}
