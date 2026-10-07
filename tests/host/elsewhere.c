/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/host/elsewhere.c -- the modules several cases load, and a call
** made from a thread that loaded nothing.
*/

#include "cases.h"

/*
** test_load_game_reader
**
** Loads the reader, with its grant.
*/
void test_load_game_reader(host_t *h)
{
    bridge_artifact_t a;

    test_state_game_reader(&a);
    host_load(h, &a);
}

/*
** test_load_watcher
**
** Loads the watcher, an observer of another namespace, with its grant.
*/
void test_load_watcher(host_t *h)
{
    bridge_artifact_t a;

    test_state_wasm(&a, "view:watcher@0.1.0", "watcher");
    a.cfg.read = TEST_GAME;
    a.cfg.read_count = 2;
    host_load(h, &a);
}

static void work_elsewhere(void *arg)
{
    test_elsewhere_t *e = arg;

    if (e->frames < 0)
        e->result = bridge_destroy(e->b);
    if (!e->frames)
        e->result = bridge_run_phase(e->b, "update");
    if (e->frames > 0)
        host_run_frames_unsaid(e->b, e->frames);
}

/*
** test_start_elsewhere
**
** Makes a call from another thread.
*/
void test_start_elsewhere(test_elsewhere_t *e, bridge_t *b, int frames)
{
    e->b = b;
    e->frames = frames;
    e->thread.run = work_elsewhere;
    e->thread.arg = e;
    host_thread_start(&e->thread);
}

/*
** test_join_elsewhere
**
** Waits for a call made from another thread.
*/
void test_join_elsewhere(test_elsewhere_t *e)
{
    host_thread_join(&e->thread);
}
