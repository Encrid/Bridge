/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/host/refused_wasm.c -- what is refused for whose it is: an ABI
** that is another's, a backend there is not, a library that cannot be
** opened, no sink.
*/

#include <stdio.h>

#include "cases.h"

/*
** test_create_no_sink
**
** A deck asked for with no sink is refused.
*/
void test_create_no_sink(host_t *h)
{
    bridge_t *b = bridge_create(&h->cfg, nullptr, nullptr);

    printf("%s\n", b ? "created" : "refused: no sink");
    printf("alive\n");
}

/*
** test_state_other_abi
**
** A module built for another ABI is refused.
*/
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

/*
** test_name_no_backend
**
** A module that names a backend the deployment has not is refused.
*/
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

/*
** test_open_no_library
**
** A backend whose library cannot be opened refuses the deck.
*/
void test_open_no_library(host_t *h)
{
    h->backends[0].library = "no-such-backend-library";
    if (host_open(h))
        return;
    test_load_small(h, h->arg);
    host_close(h);
}

/*
** test_state_other_host_abi
**
** A host built against another bridge.h is refused: nothing else of its
** configuration is read.
*/
void test_state_other_host_abi(host_t *h)
{
    h->cfg.abi = BRIDGE_HOST_ABI + 1;
    if (host_open(h))
        return;
    test_load_small(h, h->arg);
    host_close(h);
}
