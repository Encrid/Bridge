/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

static const char *const TEST_READER_GRANTS[] = {
    "game:Position",
    "game:Marked"
};

void test_state_plain(bridge_artifact_t *a, const char *identity,
    const char *path)
{
    host_state_plain(a, identity, path);
    a->cfg.spawns = 1024;
}

void test_state_reader_limited(bridge_artifact_t *a)
{
    host_state_guarded(a, "view:reader@0.1.0", "reader");
    a->cfg.read = TEST_READER_GRANTS;
    a->cfg.read_count = 2;
}

void test_run_plain(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_plain(&a, "game:mover@0.1.0", TMOVER_PATH);
    host_load(h, &a);
    host_run_frames_unsaid(h->w, 2);
    host_close(h);
}

void test_run_mixed(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_plain(&a, "game:mover@0.1.0", TMOVER_PATH);
    host_load(h, &a);
    test_state_reader_limited(&a);
    host_load(h, &a);
    host_run_frames_unsaid(h->w, 2);
    host_close(h);
}

void test_refuse_entry(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_plain(&a, "test:flood@0.1.0", TFLOOD_PATH);
    host_load(h, &a);
    (void)bridge_run_phase(h->w, "update");
    host_close(h);
}
