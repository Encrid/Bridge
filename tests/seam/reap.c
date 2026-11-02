/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <string.h>

#include "cases.h"

static void reap(host_t *h, bool may_write)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_load_built(h, "game:stamp5@0.1.0", STAMP5_PATH, 8);
    host_state_built(&a, "view:reaper@0.1.0", REAPER_PATH);
    a.cfg.read = may_write ? nullptr : TEST_GAME;
    a.cfg.read_count = may_write ? 0 : 1;
    a.cfg.write = may_write ? TEST_GAME : nullptr;
    a.cfg.write_count = may_write ? 1 : 0;
    host_load(h, &a);
    host_run_frames_unsaid(h->w, 2);
    host_close(h);
}

void test_reap(host_t *h)
{
    reap(h, !strcmp(h->arg, "write"));
}

void test_enter_from_foreign_thread(host_t *h)
{
    if (host_open(h))
        return;
    test_load_plain(h, "test:foreign@0.1.0", TFOREIGN_PATH, 8);
    host_run_frames_unsaid(h->w, 1);
    host_close(h);
}

void test_state_no_threads(host_t *h)
{
    h->cfg.threads = (uint32_t)strtol(h->arg, nullptr, 10);
    host_open_only(h);
}
