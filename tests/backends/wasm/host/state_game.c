/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "cases.h"

void test_state_wasm(bridge_artifact_t *a, const char *identity,
    const char *file)
{
    bool few = !strcmp(file, "twostep") || !strcmp(file, "chain")
        || !strcmp(file, "events") || !strcmp(file, "nested")
        || !strcmp(file, "nested2");

    host_state_wasm(a, identity, file);
    if (few)
        a->cfg.spawns = 8;
    if (!strcmp(file, "mover"))
        a->cfg.spawns = 1024;
}

void test_load_wasm(host_t *h, const char *identity, const char *file)
{
    bridge_artifact_t a;

    test_state_wasm(&a, identity, file);
    host_load(h, &a);
}

void test_state_game_reader(bridge_artifact_t *a)
{
    test_state_wasm(a, "view:reader@0.1.0", "reader");
    a->cfg.read = TEST_GAME;
    a->cfg.read_count = 2;
}
