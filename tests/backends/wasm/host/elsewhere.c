/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

void test_load_game_reader(host_t *h)
{
    bridge_artifact_t a;

    test_state_game_reader(&a);
    host_load(h, &a);
}

static void work_elsewhere(void *arg)
{
    test_elsewhere_t *e = arg;

    if (e->frames < 0)
        e->result = bridge_destroy(e->w);
    if (!e->frames)
        e->result = bridge_run_phase(e->w, "update");
    if (e->frames > 0)
        host_run_frames_unsaid(e->w, e->frames);
}

void test_start_elsewhere(test_elsewhere_t *e, bridge_world_t *w, int frames)
{
    e->w = w;
    e->frames = frames;
    e->thread.run = work_elsewhere;
    e->thread.arg = e;
    host_thread_start(&e->thread);
}

void test_join_elsewhere(test_elsewhere_t *e)
{
    host_thread_join(&e->thread);
}
