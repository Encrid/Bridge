/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/host/plain_run.c -- how this program states a module, and the
** cases in which a plain module runs.
*/

#include "cases.h"

static const char *const TEST_READER_GRANTS[] = {
    "game:Position",
    "game:Marked"
};

/*
** test_state_plain
**
** States a plain module that mints up to 1,024 ids between two barriers.
*/
void test_state_plain(bridge_artifact_t *a, const char *identity,
    const char *path)
{
    host_state_plain(a, identity, path);
    a->cfg.spawns = 1024;
}

/*
** test_state_reader_limited
**
** States the reader of the mover, on the guarded backend, with its grant.
*/
void test_state_reader_limited(bridge_artifact_t *a)
{
    host_state_guarded(a, "view:reader@0.1.0", "reader");
    a->cfg.read = TEST_READER_GRANTS;
    a->cfg.read_count = 2;
}

/*
** test_run_plain
**
** The mover as a shared library, reading and writing through raw pointers.
*/
void test_run_plain(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_plain(&a, "game:mover@0.1.0", TMOVER_PATH);
    host_load(h, &a);
    host_run_frames_unsaid(h->b, 2);
    host_close(h);
}

/*
** test_run_mixed
**
** A guarded system in the phase of a plain one.
*/
void test_run_mixed(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_plain(&a, "game:mover@0.1.0", TMOVER_PATH);
    host_load(h, &a);
    test_state_reader_limited(&a);
    host_load(h, &a);
    host_run_frames_unsaid(h->b, 2);
    host_close(h);
}

/*
** test_refuse_entry
**
** A plain module an entry refuses: the invocation fails when it returns.
*/
void test_refuse_entry(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_plain(&a, "test:flood@0.1.0", TFLOOD_PATH);
    host_load(h, &a);
    (void)bridge_run_phase(h->b, "update");
    host_close(h);
}
