/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cases.h"

void test_run_guarded(host_t *h, const char *identity, const char *file,
    int frames)
{
    if (host_open(h))
        return;
    test_load_guarded(h, identity, file);
    host_run_frames_unsaid(h->w, frames);
    host_close(h);
}

void test_order_own(host_t *h)
{
    test_run_guarded(h, "game:twostep@0.1.0", h->arg, 1);
}

static void run_for_frames(host_t *h, const char *identity)
{
    const char *frames = strchr(h->arg, ' ') + 1;
    char file[32];

    snprintf(file, sizeof(file), "%.*s", (int)(frames - 1 - h->arg), h->arg);
    test_run_guarded(h, identity, file, (int)strtol(frames, nullptr, 10));
}

void test_chain(host_t *h)
{
    run_for_frames(h, "game:chain@0.1.0");
}

void test_tell_events(host_t *h)
{
    run_for_frames(h, "game:events@0.1.0");
}

void test_nest(host_t *h)
{
    test_run_guarded(h, "game:nested@0.1.0", h->arg, 1);
}
