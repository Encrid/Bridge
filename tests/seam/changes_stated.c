/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

static void load_stamp(host_t *h, int64_t changes)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    host_state_built(&a, "game:stamp5@0.1.0", STAMP5_PATH);
    a.cfg.spawns = 8;
    a.cfg.changes = changes;
    host_load(h, &a);
    host_close(h);
}

void test_queue_too_few(host_t *h)
{
    load_stamp(h, 16);
}

void test_queue_none(host_t *h)
{
    load_stamp(h, BRIDGE_NONE);
}

void test_change_unstated(host_t *h)
{
    if (host_open(h))
        return;
    test_load_built(h, "game:stamp@0.1.0", STAMPUNSTATED_PATH, 8);
    test_load_watcher(h);
    host_run_frames_unsaid(h->b, 2);
    host_close(h);
}

void test_hop_threads(host_t *h)
{
    h->cfg.threads = 4;
    if (host_open(h))
        return;
    test_load_guarded(h, "game:mover@0.1.0", "mover");
    test_load_game_reader(h);
    host_run_frames_unsaid(h->b, 50);
    host_close(h);
}
