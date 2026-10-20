/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "cases.h"

void test_create_no_sink(host_t *h)
{
    bridge_world_t *w = bridge_create(&h->cfg, nullptr, nullptr);

    printf("%s\n", w ? "created" : "refused: no sink");
    printf("alive\n");
}

void test_state_other_abi(host_t *h)
{
    char identity[64];
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_small(&a, identity, "ok");
    a.cfg.abi = BRIDGE_ABI + 1;
    host_load(h, &a);
    host_close(h);
}

void test_name_no_backend(host_t *h)
{
    char identity[64];
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_small(&a, identity, "ok");
    a.backend = "native";
    host_load(h, &a);
    host_close(h);
}

void test_open_no_library(host_t *h)
{
    h->backends[0].library = "no-such-backend-library";
    if (host_open(h))
        return;
    test_load_small(h, h->arg);
    host_close(h);
}

void test_state_other_host_abi(host_t *h)
{
    h->cfg.abi = BRIDGE_HOST_ABI + 1;
    if (host_open(h))
        return;
    test_load_small(h, h->arg);
    host_close(h);
}
