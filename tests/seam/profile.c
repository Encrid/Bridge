/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "cases.h"

void test_set_profile(host_t *h)
{
    h->cfg.backend_count = 1;
    h->cfg.profile = BRIDGE_TRUE;
    h->sink = host_print_record;
}

static void say_rows(host_t *h)
{
    bridge_profile_row_t rows[8];
    int n = bridge_profile(h->w, rows, 8);
    int i = 0;

    printf("profile: %d rows\n", n);
    while (i < n && i < 8) {
        printf("row %s calls %llu timed %d\n", rows[i].name,
            (unsigned long long)rows[i].calls, rows[i].nanos ? 1 : 0);
        i++;
    }
}

static void say_max(host_t *h)
{
    bridge_profile_row_t rows[2] = { { "untouched", 0, 0 },
        { "untouched", 0, 0 } };
    int n = bridge_profile(h->w, rows, 1);

    printf("profile max %d, first %s, second %s\n", n, rows[0].name,
        rows[1].name);
}

void test_profile(host_t *h)
{
    if (host_open(h))
        return;
    test_load_guarded(h, "game:events@0.1.0", "events");
    host_run_frames(h, 3);
    if (!strcmp(h->arg, "count"))
        printf("profile count %d\n", bridge_profile(h->w, nullptr, 0));
    else if (!strcmp(h->arg, "max"))
        say_max(h);
    else
        say_rows(h);
    host_close(h);
}
