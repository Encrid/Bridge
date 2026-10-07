/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/stage5/stage5_load.c -- the modules several cases load, and a call
** made from a thread that loaded nothing.
*/

#include "stage5.h"

/*
** stage5_load_reader
**
** Loads the reader, with its grant.
*/
void stage5_load_reader(host_t *h)
{
    bridge_artifact_t a;

    stage5_state_reader(&a);
    host_load(h, &a);
}

/*
** stage5_load_watcher
**
** Loads the watcher, an observer of another namespace, with its grant.
*/
void stage5_load_watcher(host_t *h)
{
    bridge_artifact_t a;

    stage5_state_wasm(&a, "view:watcher@0.1.0", "watcher");
    a.cfg.read = STAGE5_GAME;
    a.cfg.read_count = 2;
    host_load(h, &a);
}

static void work_elsewhere(void *arg)
{
    stage5_elsewhere_t *e = arg;

    if (e->frames < 0)
        e->result = bridge_destroy(e->b);
    if (!e->frames)
        e->result = bridge_run_phase(e->b, "update");
    if (e->frames > 0)
        host_run_frames_unsaid(e->b, e->frames);
}

/*
** stage5_start_elsewhere
**
** Makes a call from another thread.
*/
void stage5_start_elsewhere(stage5_elsewhere_t *e, bridge_t *b, int frames)
{
    e->b = b;
    e->frames = frames;
    e->thread.run = work_elsewhere;
    e->thread.arg = e;
    host_thread_start(&e->thread);
}

/*
** stage5_join_elsewhere
**
** Waits for a call made from another thread.
*/
void stage5_join_elsewhere(stage5_elsewhere_t *e)
{
    host_thread_join(&e->thread);
}
