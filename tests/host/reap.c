/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** tests/host/reap.c -- a module of another namespace that
** destroys what it finds: with a read grant it has no stake in the entity,
** which stays; with a write grant it has, and the entity goes. And two
** cases of threads: an entry reached from one nobody called on, and a
** deployment that states none.
**
** grants.md, *What a module may destroy*
*/

#include "cases.h"

/*
** reap
**
** Loads a module that stamps and one of another namespace that destroys
** what it finds, with a grant to read or to write.
*/
static void reap(host_t *h, bool may_write)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_load_native(h, "game:stamp5@0.1.0", NSTAMP5_PATH, 8);
    host_state_native(&a, "view:reaper@0.1.0", NREAPER_PATH);
    a.cfg.read = may_write ? nullptr : TEST_GAME;
    a.cfg.read_count = may_write ? 0 : 1;
    a.cfg.write = may_write ? TEST_GAME : nullptr;
    a.cfg.write_count = may_write ? 1 : 0;
    host_load(h, &a);
    host_run_frames_unsaid(h->b, 2);
    host_close(h);
}

/*
** test_reap_with_read
**
** The reaper may read the position: what it destroys stays.
*/
void test_reap_with_read(host_t *h)
{
    reap(h, false);
}

/*
** test_reap_with_write
**
** The reaper may write the position: what it destroys goes.
*/
void test_reap_with_write(host_t *h)
{
    reap(h, true);
}

/*
** test_enter_from_foreign_thread
**
** An entry reached from a thread nobody called on.
*/
void test_enter_from_foreign_thread(host_t *h)
{
    if (host_open(h))
        return;
    test_load_native(h, "test:foreign@0.1.0", NFOREIGN_PATH, 8);
    host_run_frames_unsaid(h->b, 1);
    host_close(h);
}

/*
** test_state_no_threads
**
** A deployment that states no thread.
*/
void test_state_no_threads(host_t *h)
{
    h->cfg.threads = 0;
    host_open_only(h);
}
