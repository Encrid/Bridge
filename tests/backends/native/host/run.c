/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

void test_hear([[maybe_unused]] void *ud, const bridge_record_t *r)
{
    const char *identity = r->identity ? r->identity : "-";
    const char *system = r->system ? r->system : "-";

    printf("record[%d] b%u %s/%s: %.*s %u %u\n", r->level, r->barriers,
        identity, system, (int)r->len, r->text, r->arg[0], r->arg[1]);
}

void test_set(host_t *h)
{
    h->sink = test_hear;
}

void test_load(host_t *h, const char *identity, const char *path,
    int64_t spawns)
{
    bridge_artifact_t a;

    host_state_native(&a, identity, path);
    a.cfg.spawns = spawns;
    host_load(h, &a);
}

void test_run_mover(host_t *h)
{
    if (host_open(h))
        return;
    test_load(h, "game:mover@0.1.0", NMOVER_PATH, 1024);
    host_run_frames_unsaid(h->w, 2);
    host_close(h);
}

void test_run_cover(host_t *h)
{
    if (host_open(h))
        return;
    test_load(h, "test:cover@0.1.0", NCOVER_PATH, 16);
    host_close(h);
}

void test_refuse_entry(host_t *h)
{
    if (host_open(h))
        return;
    test_load(h, "test:flood@0.1.0", NFLOOD_PATH, 1024);
    (void)bridge_run_phase(h->w, "update");
    host_close(h);
}
