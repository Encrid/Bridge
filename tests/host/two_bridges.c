/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/host/two_bridges.c -- more than one bridge: two at once in one
** process, and three one after another that mint the same ids.
**
** threads.md, *Threads*
*/

#include <stdio.h>
#include <string.h>

#include "cases.h"

/*
** open_one
**
** Creates one of two bridges, whose records say which it is, and loads the
** mover and the reader into it. nullptr, with a line, if it was refused.
*/
static bridge_t *open_one(host_t *h, test_listener_t *l)
{
    bridge_t *b = bridge_create(&h->cfg, h->sink, l);
    bridge_artifact_t a;

    if (!b) {
        printf("bridge %d refused\n", l->which);
        return (nullptr);
    }
    test_state_wasm(&a, "game:mover@0.1.0", "mover");
    (void)bridge_load(b, &a);
    test_state_game_reader(&a);
    (void)bridge_load(b, &a);
    return (b);
}

/*
** test_run_two
**
** Two bridges in one process, each with its own storage, running at once on
** two threads. The same modules in both, a different number of frames in
** each: the sums are exact, and each is its own.
*/
void test_run_two(host_t *h)
{
    test_counts_t *c = test_get_counts(h);
    test_listener_t which[2] = {{ c, 0 }, { c, 1 }};
    test_elsewhere_t e = {};
    bridge_t *both[2] = {};

    h->cfg.threads = 2;
    both[0] = open_one(h, &which[0]);
    if (both[0])
        both[1] = open_one(h, &which[1]);
    if (!both[1])
        return;
    test_start_elsewhere(&e, both[1], 30);
    host_run_frames_unsaid(both[0], 50);
    test_join_elsewhere(&e);
    printf("two bridges: sums %u and %u\n", c->last_sum[0], c->last_sum[1]);
    (void)bridge_destroy(both[0]);
    (void)bridge_destroy(both[1]);
    printf("alive\n");
}

/*
** mint_run
**
** One run: two spawners in one level, twenty passes, on as many threads as
** the host is configured with. false, with a line, if the bridge was
** refused.
*/
static bool mint_run(host_t *h)
{
    bridge_t *b = bridge_create(&h->cfg, h->sink, h->ud);
    bridge_artifact_t a;
    int f = 0;

    if (!b) {
        printf("bridge_create refused\n");
        return (false);
    }
    test_state_wasm(&a, "test:spawn1@0.1.0", "spawner");
    a.cfg.spawns = 16;
    (void)bridge_load(b, &a);
    a.cfg.identity = "test:spawn2@0.1.0";
    (void)bridge_load(b, &a);
    while (f < 20) {
        (void)bridge_run_phase(b, "update");
        f++;
    }
    (void)bridge_destroy(b);
    return (true);
}

static const char *compare(const test_counts_t *c, int run)
{
    bool same = c->minted_count[0] == c->minted_count[run]
        && !memcmp(c->minted[0], c->minted[run], sizeof(c->minted[0]));

    return (same ? "the same" : "DIFFERENT");
}

/*
** test_mint
**
** Two modules spawn in one level. On one thread, then twice on four: the
** ids each is given are the same all three times, because they were set
** aside for it at the barrier and not taken in whatever order the threads
** arrived.
*/
void test_mint(host_t *h)
{
    constexpr uint32_t threads[3] = { 1, 4, 4 };
    test_counts_t *c = test_get_counts(h);

    c->minting = 0;
    while (c->minting < 3) {
        h->cfg.threads = threads[c->minting];
        if (!mint_run(h))
            return;
        c->minting++;
    }
    printf("minted %u ids a run; on four threads %s, and again %s\n",
        c->minted_count[0], compare(c, 1), compare(c, 2));
    printf("alive\n");
}
