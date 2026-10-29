/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "cases.h"

static int strays;

static void count_sink([[maybe_unused]] void *ud, const bridge_record_t *r)
{
    printf("record[%d] %s/%s: %.*s\n", r->level,
        r->identity ? r->identity : "-", r->system ? r->system : "-",
        (int)r->len, r->text);
    if (r->len >= 7 && !strncmp(r->text, "system ", 7))
        strays++;
}

void test_set_validate(host_t *h)
{
    h->cfg.backend_count = 1;
    h->cfg.validate = BRIDGE_TRUE;
    h->sink = count_sink;
}

void test_scribble(host_t *h)
{
    strays = 0;
    if (!strcmp(h->arg, "off"))
        h->cfg.validate = BRIDGE_FALSE;
    if (host_open(h))
        return;
    test_load_guarded(h, "game:scribble@0.1.0", "scribble");
    host_run_frames(h, 2);
    printf("scribble strays %d\n", strays);
    host_close(h);
}
