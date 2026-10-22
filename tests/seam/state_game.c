/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "cases.h"

void test_state_guarded(bridge_artifact_t *a, const char *identity,
    const char *file)
{
    bool few = !strncmp(file, "twostep", 7) || !strncmp(file, "chain", 5)
        || !strcmp(file, "events") || !strcmp(file, "nested")
        || !strcmp(file, "nested2") || !strcmp(file, "scribble");

    host_state_guarded(a, identity, file);
    if (few)
        a->cfg.spawns = 8;
    if (!strcmp(file, "mover"))
        a->cfg.spawns = 1024;
}

void test_load_guarded(host_t *h, const char *identity, const char *file)
{
    bridge_artifact_t a;

    test_state_guarded(&a, identity, file);
    host_load(h, &a);
}

void test_load_plain(host_t *h, const char *identity, const char *path,
    int64_t spawns)
{
    bridge_artifact_t a;

    host_state_plain(&a, identity, path);
    a.cfg.spawns = spawns;
    host_load(h, &a);
}

void test_state_game_reader(bridge_artifact_t *a)
{
    test_state_guarded(a, "view:reader@0.1.0", "reader");
    a->cfg.read = TEST_GAME;
    a->cfg.read_count = 2;
}

uint32_t test_count_threads(const test_counts_t *c)
{
    uint32_t n = 0;
    uint32_t i = 0;
    uint32_t k;

    while (i < c->seen) {
        k = 0;
        while (k < i && c->threads[k] != c->threads[i])
            k++;
        n += k == i;
        i++;
    }
    return (n);
}
