/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "cases.h"

static const char *const TEST_MOVER_GRANTS[] = {
    "game:Position",
    "game:Marked"
};

void test_state_module(bridge_artifact_t *a, const char *identity,
    const char *file)
{
    host_state_wasm(a, identity, file);
    if (!strcmp(file, "mover"))
        a->cfg.spawns = 1024;
    if (!strcmp(file, "refs"))
        a->cfg.spawns = 128;
    if (!strcmp(file, "namer"))
        a->cfg.spawns = 8;
    if (!strncmp(file, "rows", 4))
        a->cfg.changes = 16 << 20;
}

void test_load_module(host_t *h, const char *identity, const char *file)
{
    bridge_artifact_t a;

    test_state_module(&a, identity, file);
    host_load(h, &a);
}

void test_load_mover(host_t *h)
{
    test_load_module(h, "game:mover@0.1.0", "mover");
}

void test_state_reader(bridge_artifact_t *a)
{
    test_state_module(a, "view:reader@0.1.0", "reader");
    a->cfg.read = TEST_MOVER_GRANTS;
    a->cfg.read_count = 2;
}

void test_load_reader(host_t *h)
{
    bridge_artifact_t a;

    test_state_reader(&a);
    host_load(h, &a);
}
