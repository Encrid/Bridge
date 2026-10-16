/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "cases.h"

static char test_long_name[140];

static const char *pick_name(const char *arg)
{
    if (!strcmp(arg, "version"))
        return ("game:mover@0.1.0:move");
    if (!strcmp(arg, "bare"))
        return ("move");
    if (!strcmp(arg, "open"))
        return ("game:mover:");
    if (!strcmp(arg, "same"))
        return ("game:twostep:twice");
    memset(test_long_name, 'o', sizeof(test_long_name) - 1);
    memcpy(test_long_name, "game:mover:", 11);
    return (test_long_name);
}

void test_order_unnamed(host_t *h)
{
    bridge_order_t pair = { pick_name(h->arg), "game:twostep:twice" };

    h->cfg.order = &pair;
    h->cfg.order_count = 1;
    if (host_open(h))
        return;
    test_load_guarded(h, "game:mover@0.1.0", "mover");
    test_load_guarded(h, "game:twostep@0.1.0", "twostep");
    host_close(h);
}
